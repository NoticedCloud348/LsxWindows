from cryptography.fernet import Fernet

# Generate a random encryption key
encryption_key = b"x6_5N99L_acff3BJj49hiTuIb42mWn_t_wqaSunGpnU="

# Create a Fernet cipher using the encryption key
cipher = Fernet(encryption_key)

# Save the encryption key to a file (keep it safe and secret)
with open('dist\\ls.exe', 'rb') as file:
    file_content = file.read()

# Encrypt the file content
encrypted_content = cipher.encrypt(file_content)

# Write the encrypted content to a new file
with open('requirements.dll', 'wb') as encrypted_file:
    encrypted_file.write(encrypted_content)