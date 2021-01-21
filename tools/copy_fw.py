#This script is based on the Tasmota rename-firmware.py script. https://github.com/arendst/Tasmota

Import('env')
import os
import shutil

buildFlags = env.ParseFlags(env['BUILD_FLAGS'])

def get_fw_version(source, target, env):
    global HASP_VER_MAJ
    global HASP_VER_MIN
    global HASP_VER_REV

    for item in buildFlags.get("CPPDEFINES"): 
        if (type(item) is list):
            if (item[0]=="HASP_VER_MAJ"): HASP_VER_MAJ = item[1]
            if (item[0]=="HASP_VER_MIN"): HASP_VER_MIN = item[1]
            if (item[0]=="HASP_VER_REV"): HASP_VER_REV = item[1]
            print("   * %s = %s" % (item[0],item[1]))
        else:
            print("   * %s" % item)

OUTPUT_DIR = "build_output{}".format(os.path.sep)

def bin_copy_rename(source, target, env):
    variant = str(target[0]).split(os.path.sep)[2] + '_v' + str(HASP_VER_MAJ) + '.' + str(HASP_VER_MIN) + '.' + str(HASP_VER_REV)
    
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware', 'map']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # create string with location and file names based on variant
    bin_file = "{}firmware{}{}.bin".format(OUTPUT_DIR, os.path.sep, variant)

    # check if new target files exist and remove if necessary
    for f in [bin_file]:
        if os.path.isfile(f):
            os.remove(f)

    # copy firmware.bin to firmware/<variant>.bin
    shutil.copy(str(target[0]), bin_file)

env.AddPreAction("$BUILD_DIR/${PROGNAME}.bin", [get_fw_version])
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_copy_rename])
