import subprocess
import sys

bldcfg = sys.argv[1]

# Build cryptopp static lib
subprocess.run(["MSBuild", "Server/Vendor/cryptopp/cryptlib.vcxproj", "/p:Configuration={}".format(bldcfg)])