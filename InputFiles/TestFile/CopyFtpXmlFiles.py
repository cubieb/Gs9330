import os
import fnmatch
import shutil
import glob
import time
from ftplib import FTP

host = "192.168.3.251"
path = "101"

for file in glob.glob("10*.xml"):
    os.remove(file)

ftp = FTP()
ftp.connect(host)  # connect to host, default port
ftp.login()        # user anonymous, passwd anonymous@
ftp.cwd(path)      # change into "ftpPath" directory

files = ftp.nlst()
for file in fnmatch.filter(files, "10*.xml"):
    print(file)
    fout = open(file, 'wb')
    ftp.retrbinary('RETR %s' % file, fout.write) 
    fout.close()

ftp.quit()
   