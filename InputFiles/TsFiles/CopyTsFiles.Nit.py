import os
import shutil
import glob

for file in glob.glob("D:\\Temp\\TsFiles\\*Nit.ts"):
	os.remove(file)
	
for file in glob.glob("D:\\Temp\\TestFile\\*Nit.ts"):
	shutil.copy(file, ".")
   