import os
from colorama import init, Fore
import sys
import platform
import datetime
import argparse

# Initialize colorama for colored formatting in the console
init(autoreset=True)

def get_creation_time(patho):
    if platform.system() == "Windows":
        try:
            ctime = os.path.getctime(patho)
            return datetime.datetime.fromtimestamp(ctime)
        except FileNotFoundError:
            return "Cartella non trovata."
    else:
        return "Getting folder creation time is not supported on this platform."

def mostra_cartelle(patho):
    cartelle = []
    for elemento in os.listdir(patho):
        percorso_elemento = os.path.join(patho, elemento)
        if os.path.isdir(percorso_elemento):
            cartelle.append(elemento)
    return cartelle

def mostra_file(patho):
    file = []
    for elemento in os.listdir(patho):
        percorso_elemento = os.path.join(patho, elemento)
        if not os.path.isdir(percorso_elemento):    
            file.append(elemento)
    return file

# Create the argument parser
parser = argparse.ArgumentParser(description="List folders and files in a directory with various options.")
parser.add_argument("-d", "--directory", metavar="path", help="Choose the directory where to view the files")
parser.add_argument("-s", "--split", action="store_true", help="Separate the folders from the files")
parser.add_argument("-l", "--with_creation_time", action="store_true", help="Show the list with creation time")
parser.add_argument("-sl", "--split_with_creation_time", action="store_true", help="Separate the folders from the files and show the list with creation time")
parser.add_argument("-f", "--find", metavar="filename", help="Find a specific file or folder")
parser.add_argument("-H", "--help_args", action="store_true", help="Show the list of arguments")

# Parse the arguments
args = parser.parse_args()

# Helper function to check if any of the options is provided
def any_option_provided():
    return any([args.directory, args.split, args.with_creation_time, args.split_with_creation_time, args.find])
def only_d():
    return any([args.split, args.with_creation_time, args.split_with_creation_time, args.find])

# Handle the arguments and display the appropriate results
if args.help_args:
    parser.print_help()
else:
    if args.directory:
        path = args.directory
        dirs = True
    else:
        path = os.getcwd()

    if args.find:
        file_found = False
        for root, dirs, files in os.walk(path):
            if args.find in dirs or args.find in files:
                percorso_elemento = os.path.join(path, args.find)
                if os.path.isdir(percorso_elemento):  
                    print(Fore.LIGHTGREEN_EX + args.find)
                else:
                    print(Fore.CYAN + args.find)
                file_found = True
                break
        if not file_found:
            print("File or folder '{}' not found.".format(args.find))
    else:
        if args.split_with_creation_time:
            cartelle_presenti = mostra_cartelle(path)
            if cartelle_presenti:
                print(Fore.GREEN + "Folders in the directory:")
                for cartella in cartelle_presenti:
                    data_creazione = get_creation_time(os.path.join(path, cartella))
                    print(Fore.LIGHTGREEN_EX + "{} {}".format(data_creazione, cartella))
            file_presenti = mostra_file(path)
            print(Fore.BLUE + "Files in the directory:")
            for file in file_presenti:
                data_creazione = get_creation_time(os.path.join(path, file))
                print(Fore.CYAN + "{} {}".format(data_creazione, file))
        else:
            if args.split:
                cartelle_presenti = mostra_cartelle(path)
                if cartelle_presenti:
                    for cartella in cartelle_presenti:
                        print(Fore.LIGHTGREEN_EX + cartella)
                cartelle_presenti = mostra_file(path)
                if cartelle_presenti:
                    for cartella in cartelle_presenti:
                        print(Fore.CYAN + cartella)
            if args.with_creation_time:
                cartelle_presenti = mostra_cartelle(path)
                if cartelle_presenti:
                    for cartella in cartelle_presenti:
                        data_creazione = get_creation_time(os.path.join(path, cartella))
                        print(Fore.LIGHTGREEN_EX + "{} {}".format(data_creazione, cartella))
                file_presenti = mostra_file(path)
                for file in file_presenti:
                    data_creazione = get_creation_time(os.path.join(path, file))
                    print(Fore.CYAN + "{} {}".format(data_creazione, file))
            if not only_d():
                cartelle_presenti = mostra_cartelle(path)
                if cartelle_presenti:
                    for cartella in cartelle_presenti:
                        print(Fore.LIGHTGREEN_EX + cartella)
                file_presenti = mostra_file(path)
                for file in file_presenti:
                    print(Fore.CYAN + file)
