#This script is based on the Tasmota rename-firmware.py script. https://github.com/arendst/Tasmota

Import('env')
import os
import shutil

buildFlags = env.ParseFlags(env['BUILD_FLAGS'])
OUTPUT_DIR = "build_output{}".format(os.path.sep)

platform = env.PioPlatform()
FRAMEWORK_DIR = platform.get_package_dir("framework-arduinoespressif32")
FRAMEWORK_DIR = "{}{}".format(FRAMEWORK_DIR, os.path.sep)

def copy_boot_partitions(source, target, env):
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware', 'map']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # create string with location and file names based on variant
    src = str(target[0])
    dst = "{}firmware{}{}".format(OUTPUT_DIR, os.path.sep, "partitions.bin")

    print(src)
    print(dst)

    # check if new target files exist and remove if necessary
    for f in [dst]:
        if os.path.isfile(f):
            os.remove(f)

    # copy firmware.bin to firmware/<variant>.bin
    shutil.copy(src,dst)

    # create string with location and file names based on variant
    src = "{}tools{}partitions{}boot_app0.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, os.path.sep)
    dst = "{}firmware{}{}".format(OUTPUT_DIR, os.path.sep, "boot_app0.bin")

    print(src)
    print(dst)

    # check if new target files exist and remove if necessary
    for f in [dst]:
        if os.path.isfile(f):
            os.remove(f)

    # copy firmware.bin to firmware/<variant>.bin
    shutil.copy(src,dst)

    # create string with location and file names based on variant
    src = "{}tools{}sdk{}bin{}bootloader_dio_40m.bin".format(FRAMEWORK_DIR, os.path.sep, os.path.sep, os.path.sep, os.path.sep)
    dst = "{}firmware{}{}".format(OUTPUT_DIR, os.path.sep, "bootloader_dio_40m.bin")

    print(src)
    print(dst)

    # check if new target files exist and remove if necessary
    for f in [dst]:
        if os.path.isfile(f):
            os.remove(f)

    # copy firmware.bin to firmware/<variant>.bin
    shutil.copy(src,dst)


env.AddPostAction("$BUILD_DIR/partitions.bin", [copy_boot_partitions])
