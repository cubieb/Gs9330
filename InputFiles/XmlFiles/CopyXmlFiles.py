import os
import shutil
import glob
import time

regex="1_[0-9][0-9][0-9]_??t*.xml"
for file in glob.glob(regex):
	os.remove(file)
	
for file in glob.glob(os.path.join("../TestFile/RightXml", regex)):
	shutil.copy(file, ".")

   