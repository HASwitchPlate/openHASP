Import('env')
import os
import sys
import shutil
import subprocess
import pkg_resources

buildFlags = env.ParseFlags(env['BUILD_FLAGS'])
OUTPUT_DIR = "build_output{}".format(os.path.sep)

platform = env.PioPlatform()
FRAMEWORK_DIR = platform.get_package_dir("framework-arduinoespressif32")
FRAMEWORK_DIR = "{}{}".format(FRAMEWORK_DIR, os.path.sep)

required_pkgs = {'dulwich'}
installed_pkgs = {pkg.key for pkg in pkg_resources.working_set}
missing_pkgs = required_pkgs - installed_pkgs

if missing_pkgs:
    env.Execute('$PYTHONEXE -m pip install dulwich --global-option="--pure" --use-pep517')

from dulwich import porcelain
from dulwich.repo import Repo

def get_firmware_commit_hash():
    r = Repo('.')
    commit_hash = r.head().decode("utf-8")[0:7]
    print ("Commit Hash: " + commit_hash)
    return (commit_hash)

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
    version = 'v' + str(HASP_VER_MAJ) + '.' + str(HASP_VER_MIN) + '.' + str(HASP_VER_REV) + '_' + get_firmware_commit_hash()
    name = str(target[0]).split(os.path.sep)[2]
    name = name.replace('_4MB', '').replace('_8MB', '').replace('_16MB', '').replace('_32MB', '')
    flash_size = env.GetProjectOption("board_upload.flash_size")

    board = env.BoardConfig()
    flash_mode = board.get("build.flash_mode", "dio")
    f_flash = board.get("build.f_flash", "40000000L")
    flash_freq = '40m'
    if (f_flash == '80000000L'):
        flash_freq = '80m'

    mcu = board.get("build.mcu", "esp32")
    bootloader = "{}tools{}sdk{}{}{}bin{}bootloader_{}_{}.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, mcu, os.path.sep, os.path.sep, flash_mode, flash_freq)
    # # if not os.path.isfile(bootloader):
    # #     bootloader = "{}tools{}sdk{}bin{}bootloader_dio_40m.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, os.path.sep, os.path.sep, os.path.sep)
    # if not os.path.isfile(bootloader):
    #     bootloader = "{}tools{}sdk{}{}{}bin{}bootloader_{}_{}.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, mcu, os.path.sep, os.path.sep, flash_mode, flash_freq)
    bootloader = str(target[0]).replace('firmware.bin','bootloader.bin')
    bootloader_location = '0x1000'
    if (mcu == 'esp32s3'):
        bootloader_location = '0x0000'

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
    print(flash_freq)
    print(f_flash)
    print(flash_mode)

    # esptool = 'tools/esptool_with_merge_bin.py'
    esptool = '{}{}esptool.py'.format(platform.get_package_dir("tool-esptoolpy"),os.path.sep)
    print(esptool)
    process = subprocess.Popen(['python', esptool, '--chip', mcu, 'merge_bin', '--output', firmware_dst, '--flash_mode', 'dio', '--flash_size', flash_size, '--flash_freq', flash_freq, bootloader_location, bootloader, '0x8000', partitions, '0xe000', boot_app0, '0x10000', firmware_src],
                        stdout=subprocess.PIPE, 
                        stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    stdout, stderr
    print(stdout.decode("utf-8") )
    print(stderr.decode("utf-8") )

def copy_ota(source, target, env):
    version = 'v' + str(HASP_VER_MAJ) + '.' + str(HASP_VER_MIN) + '.' + str(HASP_VER_REV) + '_' + get_firmware_commit_hash()
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
