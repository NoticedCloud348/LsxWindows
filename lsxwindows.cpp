#include <iostream>
#include <windows.h>
#include <AccCtrl.h>
#include <AclAPI.h>
#include <strsafe.h>
#include <sddl.h>
#include <shlwapi.h>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <sstream>
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shlwapi.lib")

void DisplayError(LPCWSTR lpszFunction) {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    wprintf(L"Error %s failed with error %d: %s\n", lpszFunction, dw, (LPCTSTR)lpMsgBuf);

    LocalFree(lpMsgBuf);
}

std::wstring ToWideString(const std::string& str) {
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, wchars_num);
    std::wstring wideStr = wstr;
    delete[] wstr;
    return wideStr;
}

std::string ToNarrowString(const std::wstring& wstr) {
    int chars_num = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    char* str = new char[chars_num];
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, str, chars_num, NULL, NULL);
    std::string narrowStr = str;
    delete[] str;
    return narrowStr;
}

void ShowFolderPermissions(const std::wstring& folderPath) {
    DWORD dwRes, dwDisposition;
    PSID pOwner = NULL, pGroup = NULL;
    PACL pDacl = NULL;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    // Get the security descriptor for the folder
    dwRes = GetNamedSecurityInfo(
        folderPath.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        &pOwner,
        &pGroup,
        &pDacl,
        NULL,
        &pSecurityDescriptor
    );

    if (dwRes != ERROR_SUCCESS) {
        DisplayError(L"GetNamedSecurityInfo");
        return;
    }

    // Get the DACL (Discretionary Access Control List) from the security descriptor
    if (pDacl != NULL) {
        for (DWORD i = 0; i < pDacl->AceCount; i++) {
            ACCESS_ALLOWED_ACE* pAce;
            if (GetAce(pDacl, i, (LPVOID*)&pAce)) {
                PSID pSid = (PSID) & (pAce->SidStart);
                TCHAR szAccountName[MAX_PATH];
                TCHAR szDomainName[MAX_PATH];
                SID_NAME_USE sidUse;
                DWORD dwNameSize = MAX_PATH, dwDomainSize = MAX_PATH;

                // Get the account name associated with the SID
                if (LookupAccountSid(NULL, pSid, szAccountName, &dwNameSize, szDomainName, &dwDomainSize, &sidUse)) {
                    wprintf(L"Trustee: %s\\%s\n", szDomainName, szAccountName);
                    wprintf(L"Access Rights: %s%s%s\n",
                        (pAce->Mask & FILE_GENERIC_READ) ? L"Read " : L"",
                        (pAce->Mask & FILE_GENERIC_WRITE) ? L"Write " : L"",
                        (pAce->Mask & FILE_GENERIC_EXECUTE) ? L"Execute" : L""
                    );
                }
                else {
                    DisplayError(L"LookupAccountSid");
                }
            }
        }
    }
    else {
        wprintf(L"No DACL found for the folder.\n");
    }

    LocalFree(pSecurityDescriptor);
}

std::vector<std::wstring> GetFoldersInDirectory(const std::wstring& path) {
    std::vector<std::wstring> folders;
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((path + L"\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
                folders.push_back(findData.cFileName);
            }
        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }

    return folders;
}

std::vector<std::wstring> GetFilesInDirectory(const std::wstring& path) {
    std::vector<std::wstring> files;
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((path + L"\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(findData.cFileName);
            }
        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }

    return files;
}

std::wstring FormatFileTime(const FILETIME& fileTime) {
    ULARGE_INTEGER largeInt;
    largeInt.LowPart = fileTime.dwLowDateTime;
    largeInt.HighPart = fileTime.dwHighDateTime;
    time_t time = (largeInt.QuadPart - 116444736000000000ULL) / 10000000ULL;

    tm timeInfo;
    localtime_s(&timeInfo, &time);

    int milliseconds = fileTime.dwLowDateTime % 10000000 / 10000;

    std::wstringstream wss;
    wss << std::put_time(&timeInfo, L"%Y-%m-%d %H:%M:%S") << L"." << std::setw(3) << std::setfill(L'0') << milliseconds;

    return wss.str();
}


void PrintFilesAndFolders(const std::wstring& path, bool withCreationTime, bool split) {
    std::vector<std::wstring> folders = GetFoldersInDirectory(path);
    std::vector<std::wstring> files = GetFilesInDirectory(path);

    if (!folders.empty()) {
        if (split == true){
            std::wcout << "\033[1;32m" << L"Folders in the directory:\n";
            std::cout << "\033[0m";
        }
        for (const auto& folder : folders) {
            std::wcout << "\033[1;92m" << folder;
            if (withCreationTime) {
                std::wstring folderPath = path + L"\\" + folder;
                WIN32_FILE_ATTRIBUTE_DATA folderAttrData;
                if (GetFileAttributesEx(folderPath.c_str(), GetFileExInfoStandard, &folderAttrData)) {
                    std::wcout << L"\033[1;93m - Created on \033[1;31m" << FormatFileTime(folderAttrData.ftCreationTime);
                }
            }
            std::cout << "\033[0m";
            std::wcout << L'\n';
        }
    }

    if (!files.empty()) {
        if (split == true) {
            std::wcout << "\033[1;34m" << L"Files in the directory:\n";
        }
        for (const auto& file : files) {
            std::wcout << "\033[1;96m" << file;
            std::cout << "\033[0m";
            if (withCreationTime) {
                std::wstring folderPath = path + L"\\" + file;
                WIN32_FILE_ATTRIBUTE_DATA folderAttrData;
                if (GetFileAttributesEx(folderPath.c_str(), GetFileExInfoStandard, &folderAttrData)) {
                    std::wcout << L"\033[1;33m - Created on \033[1;31m" << FormatFileTime(folderAttrData.ftCreationTime);
                }
            }
            std::cout << "\033[0m";
            std::wcout << L'\n';
        }
    }
}

void PrintFilesAndFoldersD(const std::wstring& path, bool withCreationTime, bool split) {
    std::vector<std::wstring> folders = GetFoldersInDirectory(path);
    std::vector<std::wstring> files = GetFilesInDirectory(path);

    if (!folders.empty()) {
        if (split == true) {
            std::wcout << "\033[1;32m" << L"Folders in the directory:\n";
            std::cout << "\033[0m";
        }
        for (const auto& folder : folders) {
            std::wcout << "\033[1;92m" << folder;
            if (withCreationTime) {
                std::wstring folderPath = path + L"\\" + folder;
                WIN32_FILE_ATTRIBUTE_DATA folderAttrData;
                if (GetFileAttributesEx(folderPath.c_str(), GetFileExInfoStandard, &folderAttrData)) {
                    std::wcout << L"\033[0;93m - Created on \033[0;31m" << FormatFileTime(folderAttrData.ftCreationTime);
                }
            }
            std::cout << "\033[0m";
            std::wcout << L'\n';
        }
    }

    if (!files.empty()) {
        if (split == true) {
            std::wcout << "\033[1;34m" << L"Files in the directory:\n";
        }
        for (const auto& file : files) {
            std::wcout << "\033[1;96m" << file;
            std::cout << "\033[0m";
            if (withCreationTime) {
                std::wstring folderPath = path + L"\\" + file;
                WIN32_FILE_ATTRIBUTE_DATA folderAttrData;
                if (GetFileAttributesEx(folderPath.c_str(), GetFileExInfoStandard, &folderAttrData)) {
                    std::wcout << L"\033[0;33m - Created on \033[0;31m" << FormatFileTime(folderAttrData.ftCreationTime);
                }
            }
            std::cout << "\033[0m";
            std::wcout << L'\n';
        }
    }
}

void SortFilesBySize(std::vector<std::wstring>& files) {
    std::sort(files.begin(), files.end(), [](const std::wstring& a, const std::wstring& b) {
        WIN32_FILE_ATTRIBUTE_DATA fileAttrDataA, fileAttrDataB;
        if (GetFileAttributesEx(a.c_str(), GetFileExInfoStandard, &fileAttrDataA) && GetFileAttributesEx(b.c_str(), GetFileExInfoStandard, &fileAttrDataB)) {
            ULONGLONG sizeA = ((ULONGLONG)fileAttrDataA.nFileSizeHigh << 32) + fileAttrDataA.nFileSizeLow;
            ULONGLONG sizeB = ((ULONGLONG)fileAttrDataB.nFileSizeHigh << 32) + fileAttrDataB.nFileSizeLow;
            return sizeA > sizeB;
        }
        return false;
        });
}

void PrintFilesWithSize(const std::wstring& path) {
    std::vector<std::wstring> files = GetFilesInDirectory(path);
    SortFilesBySize(files);

    if (!files.empty()) {
        std::wcout << L"Files in the directory sorted by size:\n";
        for (const auto& file : files) {
            std::wstring filePath = path + L"\\" + file;
            WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
            if (GetFileAttributesEx(filePath.c_str(), GetFileExInfoStandard, &fileAttrData)) {
                ULONGLONG size = ((ULONGLONG)fileAttrData.nFileSizeHigh << 32) + fileAttrData.nFileSizeLow;
                std::wcout << size << L" bytes - " << file << L'\n';
            }
        }
    }
}

void ShowHelp() {
    std::wcout << L"Usage:";
    std::wcout << L" [-d <directory>] [-s] [-l] [-sl] [-f <filename>] [-H] [-S] [-Sh]\n";
    std::wcout << L"\n";
    std::wcout << L"List folders and files in a directory with various options.";
    std::wcout << L"\n";
    std::wcout << L"Options:\n";
    std::wcout << L"  -d, --directory <path>          Choose the directory where to view the files\n";
    std::wcout << L"  -s, --split                    Separate the folders from the files\n";
    std::wcout << L"  -l, --with_creation_time        Show the list with creation time\n";
    std::wcout << L"  -sl, --split_with_creation_time Separate the folders from the files and show the list with creation time\n";
    std::wcout << L"  -f, --find <filename>           Find a specific file or folder\n";
    std::wcout << L"  -H, --help_args                Show the list of arguments\n";
    std::wcout << L"  -S, --Size                     Sort files by weight\n";
    std::wcout << L"  -Sh, --Size_with_printing       Sort files by weight and print the weight\n";
}

int main(int argc, char* argv[]) {
    std::wstring folderPath;
    folderPath = ToWideString(".");
    if (argc < 2) {
        NULL;
    }
    else {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-d" || arg == "--directory") {
                if (i + 1 < argc) {
                    folderPath = ToWideString(argv[i + 1]);
                    break;
                }
                else {
                    std::wcout << L"Error: Missing directory path.\n";
                    return 1;
                }
            }
        }
    }

    if (folderPath.empty()) {
        std::wcout << L"Error: No directory path specified.\n";
        return 1;
    }

    bool withCreationTime = false;
    bool splitFoldersFiles = false;
    bool sortBySize = false;
    bool sortWithSizePrinting = false;
    int adg;
    adg = 0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-s" || arg == "--split") {
            splitFoldersFiles = true;
        }
        else if (arg == "-l" || arg == "--with_creation_time") {
            withCreationTime = true;
        }
        else if (arg == "-sl" || arg == "--split_with_creation_time") {
            splitFoldersFiles = true;
            withCreationTime = true;
        }
        else if (arg == "-S" || arg == "--Size") {
            sortBySize = true;
        }
        else if (arg == "-Sh" || arg == "--Size_with_printing") {
            sortWithSizePrinting = true;
        }
        else if (arg == "-H" || arg == "--help_args" || arg == "-h") {
            ShowHelp();
            return 0;
        }
        else if (arg == "-d" || arg == "--directory") {
            adg = 1;
        }
        else if (arg == "-f" || arg == "--find" || arg == "-f" || arg == "--find") {
            if (i + 1 < argc) {
                std::wstring fileName = ToWideString(argv[i + 1]);
                std::vector<std::wstring> folders = GetFoldersInDirectory(folderPath);
                std::vector<std::wstring> files = GetFilesInDirectory(folderPath);
                auto itFolder = std::find(folders.begin(), folders.end(), fileName);
                auto itFile = std::find(files.begin(), files.end(), fileName);
                if (itFolder != folders.end()) {
                    std::wcout << L"Found folder: " << *itFolder << L'\n';
                }
                else if (itFile != files.end()) {
                    std::wcout << L"Found file: " << *itFile << L'\n';
                }
                else {
                    std::wcout << L"File or folder '" << fileName << L"' not found.\n";
                }
                return 0;
            }
            else {
                std::wcout << L"Error: Missing filename.\n";
                return 1;
            }
        }
        else {
            if (adg == 0) {
                std::cout << "Error: argument " << arg << " doesnt exist write -h to view valid arguments" << std::endl;
                return 1;
            }
            else {
                adg -= 1;
            }
        }
    }
    if (splitFoldersFiles && withCreationTime) {
        PrintFilesAndFolders(folderPath, true, true);
    }
    else if (splitFoldersFiles) {
        PrintFilesAndFolders(folderPath, false, true);
    }
    else if (sortBySize && sortWithSizePrinting) {
        PrintFilesWithSize(folderPath);
    }
    else if (sortBySize) {
        std::vector<std::wstring> files = GetFilesInDirectory(folderPath);
        SortFilesBySize(files);
        for (const auto& file : files) {
            std::wcout << file << L'\n';
        }
    }
    else if (sortWithSizePrinting) {
        PrintFilesWithSize(folderPath);
    }
    else if (withCreationTime) {
        PrintFilesAndFolders(folderPath, true, false);
    }
    else {
        PrintFilesAndFolders(folderPath, false, false);
    }

    return 0;
}