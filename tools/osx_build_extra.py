Import("env")

env.Append(
    BUILD_FLAGS=[
        "-I/usr/local/include",
        "-L/usr/local/lib",
        "-DTARGET_OS_MAC=1"],
)

env.Replace(CC="gcc-10", CXX="g++-10")

env.Replace(BUILD_SCRIPT="tools/osx_build_script.py")

#print('=====================================')
print(env.Dump())
