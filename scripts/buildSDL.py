import os
import subprocess
import sys

root_directory = os.getcwd()
bldcfg = sys.argv[1]

# Move to SDL3 build directory
os.chdir("Client/Vendor/SDL")
if not os.path.exists("build"):
    os.mkdir("build")
os.chdir("build")

# Build SDL3 libraries
subprocess.run(["cmake", "-DCMAKE_BUILD_TYPE={}".format(bldcfg), ".."])
subprocess.run(["cmake", "--build", ".", "--config", "{}".format(bldcfg)])

# Return to root directory
os.chdir(root_directory)