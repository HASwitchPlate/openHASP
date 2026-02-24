Import("env")
import subprocess

# Use system clang with full SDK paths
env.Replace(CC="clang", CXX="clang++")

# Get SDK path from xcrun
try:
    sdk_path = subprocess.check_output(["xcrun", "--show-sdk-path"]).decode().strip()
    env.Append(CPPPATH=[f"{sdk_path}/usr/include/c++/v1"])
    env.Append(CXXFLAGS=["-stdlib=libc++"])
    env.Append(LINKFLAGS=["-stdlib=libc++"])
except:
    pass

env.Replace(BUILD_SCRIPT="tools/osx_build_script.py")

#print('=====================================')
#print(env.Dump())
