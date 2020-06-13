from datetime import datetime

Import("env")

HASP_VERSION_MAJOR=0
HASP_VERSION_MINOR=0
HASP_VERSION_REVISION=0

now = datetime.utcnow() # current date and time
BUILD_TIMESTAMP=now.strftime("%Y-%m-%d %H:%M:%S")

# Setting datetime each time triggers a full recompile always
# env.Append(CPPDEFINES=[('BUILD_TIMESTAMP', BUILD_TIMESTAMP)])

# env.Append(CPPDEFINES=[('HASP_VERSION_MAJOR', HASP_VERSION_MAJOR)])
# env.Append(CPPDEFINES=[('HASP_VERSION_MINOR', HASP_VERSION_MINOR)])
# env.Append(CPPDEFINES=[('HASP_VERSION_REVISION', HASP_VERSION_REVISION)])

buildFlags = env.ParseFlags(env['BUILD_FLAGS'])
# print(buildFlags)

print("*******************************************************")
# Using for loop 
for item in buildFlags.get("CPPDEFINES"): 
    if (type(item) is list):
        if (item[0]=="HASP_VERSION_MAJOR"): HASP_VERSION_MAJOR = item[1]
        if (item[0]=="HASP_VERSION_MINOR"): HASP_VERSION_MINOR = item[1]
        if (item[0]=="HASP_VERSION_REVISION"): HASP_VERSION_REVISION = item[1]
        print("   * %s = %s" % (item[0],item[1]))
    else:
        print("   * %s" % item)

# access to global build environment
#print(env.Dump())
print("*******************************************************")

env.Replace(PROGNAME="%s_v%s.%s.%s" % (env['PIOENV'],HASP_VERSION_MAJOR,HASP_VERSION_MINOR,HASP_VERSION_REVISION))