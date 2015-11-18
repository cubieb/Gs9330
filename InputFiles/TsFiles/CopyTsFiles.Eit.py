import os
import shutil
import glob

for file in glob.glob("D:\\Temp\\TsFiles\\*Eit*.ts"):
	os.remove(file)
	
for file in glob.glob("D:\\Temp\\TestFile\\*Eit*.ts"):
	shutil.copy(file, ".")
   