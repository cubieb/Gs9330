import os
import shutil
import glob

for file in glob.glob("101*sdt*.xml"):
	os.remove(file)
	
for file in glob.glob("D:\\Temp\\TestFile\\101_006*sdt1*.xml"):
	shutil.copy(file, ".")
   