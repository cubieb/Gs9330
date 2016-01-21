import os
import shutil
import glob

for file in glob.glob("10*eit*.xml"):
	os.remove(file)
	
for file in glob.glob("D:\\Temp\\TestFile\\10*eit*.xml"):
	shutil.copy(file, ".")
   