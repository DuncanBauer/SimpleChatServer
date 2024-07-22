import os
import platform
import shutil
import sys

root_directory = os.getcwd()
bldcfg = sys.argv[1]

# Copy SDL3 shared library to project build folders
if platform.system() == "Windows":
    src = os.path.join(root_directory, "Client/Vendor/SDL/build/{}/SDL3.dll".format(bldcfg))
    dest = os.path.join(root_directory, "bin/{}/windows/SDL3.dll".format(bldcfg))
    shutil.copyfile(src, dest)
else:
    src = root_directory + "Client/Vendor/SDL/build/{}/SDL3.dll".format(bldcfg)
    dest = os.path.join(root_directory, "bin/{}/linux/SDL3.dll".format(bldcfg))
    shutil.copyfile(src, dest)