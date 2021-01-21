from datetime import datetime

Import("env")

HASP_VER_MAJ=0
HASP_VER_MIN=0
HASP_VER_REV=0

now = datetime.utcnow() # current date and time
BUILD_TIMESTAMP=now.strftime("%Y-%m-%d %H:%M:%S")

# Setting datetime each time triggers a full recompile always
# env.Append(CPPDEFINES=[('BUILD_TIMESTAMP', BUILD_TIMESTAMP)])

# env.Append(CPPDEFINES=[('HASP_VER_MAJ', HASP_VER_MAJ)])
# env.Append(CPPDEFINES=[('HASP_VER_MIN', HASP_VER_MIN)])
# env.Append(CPPDEFINES=[('HASP_VER_REV', HASP_VER_REV)])

buildFlags = env.ParseFlags(env['BUILD_FLAGS'])
# print(buildFlags)

print("*******************************************************")
# Using for loop 
for item in buildFlags.get("CPPDEFINES"): 
    if (type(item) is list):
        if (item[0]=="HASP_VER_MAJ"): HASP_VER_MAJ = item[1]
        if (item[0]=="HASP_VER_MIN"): HASP_VER_MIN = item[1]
        if (item[0]=="HASP_VER_REV"): HASP_VER_REV = item[1]
        print("   * %s = %s" % (item[0],item[1]))
    else:
        print("   * %s" % item)

# access to global build environment
#print(env.Dump())
print("*******************************************************")

env.Replace(PROGNAME="%s_v%s.%s.%s" % (env['PIOENV'],HASP_VER_MAJ,HASP_VER_MIN,HASP_VER_REV))