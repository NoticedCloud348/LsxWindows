from cryptography.fernet import Fernet
import os, subprocess, pymsgbox

# Ottieni il nome utente dell'utente corrente
user = os.getlogin()

# Chiave di crittografia (sostituisci con la tua chiave)
encryption_key = "encryption key"
cipher = Fernet(encryption_key)

# Leggi i dati criptati dal file "requirements.dll"
with open('requirements.dll', 'rb') as encrypted_file:
    encrypted_data = encrypted_file.read()

# Decrittografa i dati
decrypted_data = cipher.decrypt(encrypted_data)

# Verifica se la cartella "C:\Program Files\ls" esiste, altrimenti creala
install_path = os.path.join("C:\\Program Files", "ls")
if not os.path.exists(install_path):
    os.mkdir(install_path)

# Salva i dati decrittati nel file "ls.exe"
ls_exe_path = os.path.join(install_path, "ls.exe")
with open(ls_exe_path, 'wb') as f:
    f.write(decrypted_data)

# Aggiungi il percorso all'eseguibile "ls.exe" alla variabile di ambiente PATH
subprocess.call(f'setx PATH "%PATH%;C:\\Program Files\\ls"')

# Mostra un messaggio di conferma dopo l'installazione
pymsgbox.alert('L\'installazione è andata a buon termine', 'installazione')
