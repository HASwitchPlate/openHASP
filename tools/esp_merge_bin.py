Import('env')
import os
import sys
import shutil
import subprocess

buildFlags = env.ParseFlags(env['BUILD_FLAGS'])
OUTPUT_DIR = "build_output{}".format(os.path.sep)

platform = env.PioPlatform()
FRAMEWORK_DIR = platform.get_package_dir("framework-arduinoespressif32")
FRAMEWORK_DIR = "{}{}".format(FRAMEWORK_DIR, os.path.sep)


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


def copy_merge_bins(source, target, env):
    version = 'v' + str(HASP_VER_MAJ) + '.' + str(HASP_VER_MIN) + '.' + str(HASP_VER_REV)
    name = str(target[0]).split(os.path.sep)[2]
    name = name.replace('_4MB', '').replace('_8MB', '').replace('_16MB', '').replace('_32MB', '')
    flash_size = env.GetProjectOption("board_upload.flash_size")

    bootloader = "{}tools{}sdk{}esp32{}bin{}bootloader_dio_40m.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, os.path.sep, os.path.sep, os.path.sep)
    if not os.path.isfile(bootloader):
        bootloader = "{}tools{}sdk{}bin{}bootloader_dio_40m.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, os.path.sep, os.path.sep, os.path.sep)
    partitions = "{}{}partitions.bin".format(env.subst("$BUILD_DIR"), os.path.sep)
    boot_app0 = "{}tools{}partitions{}boot_app0.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, os.path.sep)
    firmware_dst ="{}firmware{}{}_full_{}_{}.bin".format(OUTPUT_DIR, os.path.sep, name, flash_size, version)
    firmware_src = str(target[0])

    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware', 'map']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # check if new target files exist and remove if necessary
    for f in [firmware_dst]:
        if os.path.isfile(f):
            os.remove(f)

    print(bootloader)
    print(partitions)
    print(boot_app0)
    print(firmware_src)
    print(firmware_dst)
    print(flash_size)

    process = subprocess.Popen(['python', 'tools/esptool_with_merge_bin.py', '--chip', 'esp32', 'merge_bin', '--output', firmware_dst, '--flash_mode', 'dio', '--flash_size', flash_size, '0x1000', bootloader, '0x8000', partitions, '0xe000', boot_app0, '0x10000', firmware_src],
                        stdout=subprocess.PIPE, 
                        stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    stdout, stderr
    print(stdout.decode("utf-8") )
    print(stderr.decode("utf-8") )

def copy_ota(source, target, env):
    version = 'v' + str(HASP_VER_MAJ) + '.' + str(HASP_VER_MIN) + '.' + str(HASP_VER_REV)
    name =str(target[0]).split(os.path.sep)[2]
    name = name.replace('_4MB', '').replace('_8MB', '').replace('_16MB', '').replace('_32MB', '')

    firmware_src = str(target[0])
    firmware_dst ="{}firmware{}{}_ota_{}.bin".format(OUTPUT_DIR, os.path.sep, name, version)

    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware', 'map']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # check if new target files exist and remove if necessary
    for f in [firmware_dst]:
        if os.path.isfile(f):
            os.remove(f)

    print(firmware_src)
    print(firmware_dst)

    # copy firmware.bin to firmware/<variant>.bin
    shutil.copy(firmware_src, firmware_dst)

env.AddPreAction("$BUILD_DIR/${PROGNAME}.bin", [get_fw_version])
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [copy_merge_bins])
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [copy_ota])
