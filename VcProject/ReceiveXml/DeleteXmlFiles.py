import os
import shutil
import glob
import time

regex="1_[0-9][0-9][0-9]_??t*.xml"

os.renames("ok", "notok")
for file in glob.glob(regex):
	os.remove(file)
os.renames("notok", "ok")	
   