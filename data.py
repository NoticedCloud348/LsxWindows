with open('dist\\ls.exe', 'rb') as file:
    file_content = file.read()

with open('requirements.dll', 'wb') as encrypted_file:
    encrypted_file.write(file_content)
