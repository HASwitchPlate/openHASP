"""
    Builder for native platform
"""

from SCons.Script import AlwaysBuild, Default, DefaultEnvironment

env = DefaultEnvironment()

# Preserve C and C++ build flags
backup_cflags = env.get("CFLAGS", [])
backup_cxxflags = env.get("CXXFLAGS", [])

# Scan for GCC compiler
env.Tool("gcc")
env.Tool("g++")

# Restore C/C++ build flags as they were overridden by env.Tool
env.Append(CFLAGS=backup_cflags, CXXFLAGS=backup_cxxflags)

#
# Target: Build executable program
#

target_bin = env.BuildProgram()

#
# Target: Print binary size
#

target_size = env.Alias("size", target_bin, env.VerboseAction(
    "$SIZEPRINTCMD", "Calculating size $SOURCE"))
AlwaysBuild(target_size)

#
# Default targets
#

Default([target_bin])
