import os
import subprocess
import sys

root_directory = os.getcwd()
bldcfg = sys.argv[1]

# Create executables
subprocess.run(["MSBuild", "ChatServer.sln", "/p:Configuration={}".format(bldcfg)])