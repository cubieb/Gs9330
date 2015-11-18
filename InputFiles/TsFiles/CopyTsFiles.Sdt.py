import os
import shutil
import glob

for file in glob.glob("D:\\Temp\\TsFiles\\*Sdt*.ts"):
	os.remove(file)
	
for file in glob.glob("D:\\Temp\\TestFile\\*Sdt*.ts"):
	shutil.copy(file, ".")
   