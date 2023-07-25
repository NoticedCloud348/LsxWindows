import os, subprocess, pymsgbox

user = os.getlogin()

with open('requirements.dll', 'rb') as encrypted_file:
    data = encrypted_file.read()

install_path = os.path.join("C:\\Program Files", "ls")
if not os.path.exists(install_path):
    os.mkdir(install_path)

ls_exe_path = os.path.join(install_path, "ls.exe")
with open(ls_exe_path, 'wb') as f:
    f.write(data)
# Get the value of the PATH environment variable
path = os.environ['PATH']

# Split the value of the PATH environment variable into an array
paths = path.split(';')

# Check if the directory `C:\Program Files\ls` is in the array of paths
if 'C:\Program Files\ls' in paths:
    print('The directory C:\Program Files\ls is in the PATH environment variable.')
else:
    subprocess.call(f'setx PATH "%PATH%;C:\\Program Files\\ls" /M')

pymsgbox.alert('L\'installazione è andata a buon termine', 'installazione')
