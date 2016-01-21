import os
import shutil
import glob
import time

for file in glob.glob("10*.xml"):
	os.remove(file)
	
   