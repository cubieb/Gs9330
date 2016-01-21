import os
import shutil
import glob

for file in glob.glob("D:\\Temp\\TsFiles\\*Bat*.ts"):
	os.remove(file)
	
for file in glob.glob("D:\\Temp\\TestFile\\*Bat*.ts"):
	shutil.copy(file, ".")
   