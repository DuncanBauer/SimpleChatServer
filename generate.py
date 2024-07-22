import os
import subprocess
import sys

root_directory = os.getcwd()
bldcfg = sys.argv[1]

print("Building SDL")
subprocess.call("python3 scripts/buildSDL.py {}".format(bldcfg), shell=True)

# print("Building Cryptopp")
# subprocess.call("python3 scripts/buildCryptopp.py {}".format(bldcfg), shell=True)

# print("Building Spdlog")
# subprocess.call("python3 scripts/buildSpdlog.py {}".format(bldcfg), shell=True)

print("Generating ChatServer Solution")
subprocess.call("python3 scripts/generateSolution.py", shell=True)

print("Building ChatServer Solution")
subprocess.call("python3 scripts/buildSolution.py {}".format(bldcfg), shell=True)