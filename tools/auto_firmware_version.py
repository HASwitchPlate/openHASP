import pkg_resources

Import("env")

required_pkgs = {'dulwich'}
installed_pkgs = {pkg.key for pkg in pkg_resources.working_set}
missing_pkgs = required_pkgs - installed_pkgs

if missing_pkgs:
    env.Execute('$PYTHONEXE -m pip install dulwich --global-option="--pure"')

from dulwich import porcelain
from dulwich.repo import Repo

def get_firmware_specifier_build_flag():
    build_version = porcelain.describe('.')  # '.' refers to the repository root dir
    build_flag = "-D AUTO_VERSION=\\\"" + build_version + "\\\""
    print ("Firmware Revision: " + build_version)
    return (build_flag)

def get_firmware_commit_hash():
    r = Repo('.')
    commit_hash = r.head().decode("utf-8")[0:7]
    build_flag = "-D COMMIT_HASH=\\\"" + commit_hash + "\\\""
    print ("Commit Hash: " + commit_hash)
    return (build_flag)

env.Append(
    BUILD_FLAGS=[get_firmware_commit_hash()]
)