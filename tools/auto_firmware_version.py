import gzip
import subprocess
import sys
from importlib import metadata

Import("env")

def install_and_import(package):
    try:
        metadata.version(package)
    except metadata.PackageNotFoundError:
        print(f"Installing {package}...")
        # Gebruik de Python interpreter van de omgeving
        subprocess.check_call([sys.executable, "-m", "pip", "install", package, "--config-settings", "--build-option=--pure", "--use-pep517"])

# Zorg dat dulwich aanwezig is
install_and_import('dulwich')

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

def get_flash_size():
    flash_size = int(env.BoardConfig().get("upload.maximum_size")/1024/1024)
    build_flag = "-D ESP_FLASH_SIZE=" + str(flash_size)
    print ("ESP Flash Size: " + str(flash_size))
    return (build_flag)

env.Append(
    BUILD_FLAGS=[get_firmware_commit_hash(),get_flash_size()]
)

r = Repo('.')
commit_hash = r.head().decode("utf-8")[0:7]
with open("data/edit.htm", "r", encoding="utf-8") as f:
    html=f.read()
html = html.replace("COMMIT_HASH", commit_hash)
with gzip.open('data/static/edit.htm.gz', 'wb') as f:
  f.write(html.encode('utf-8'))

with open("data/main.js", "r", encoding="utf-8") as f:
    html=f.read()
html = html.replace("COMMIT_HASH", commit_hash)
with gzip.open('data/static/main.js.gz', 'wb') as f:
  f.write(html.encode('utf-8'))

with open("data/script.js", "r", encoding="utf-8") as f:
    html=f.read()
html = html.replace("COMMIT_HASH", commit_hash)
with gzip.open('data/static/script.js.gz', 'wb') as f:
  f.write(html.encode('utf-8'))

with open("data/en.json", "r", encoding="utf-8") as f:
    html=f.read()
html = html.replace("COMMIT_HASH", commit_hash)
with gzip.open('data/static/en.json.gz', 'wb') as f:
  f.write(html.encode('utf-8'))

with open("data/style.css", "r", encoding="utf-8") as f:
    html=f.read()
html = html.replace("COMMIT_HASH", commit_hash)
with gzip.open('data/static/style.css.gz', 'wb') as f:
  f.write(html.encode('utf-8'))
