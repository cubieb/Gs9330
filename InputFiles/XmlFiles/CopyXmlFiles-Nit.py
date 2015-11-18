import os
import shutil
import glob

for file in glob.glob("101*nit*.xml"):
	os.remove(file)
	
for file in glob.glob("D:\\Temp\\TestFile\\101*nit*.xml"):
	shutil.copy(file, ".")
   