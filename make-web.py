#!/usr/bin/env python

import os
import stat
from sys import platform
from shutil import rmtree
from subprocess import check_call

def resolve_path(rel_path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), rel_path))

def makedirs_silent(root):
    try:
        os.makedirs(root)
    except:
        pass

if __name__ == "__main__":
    if platform == "linux" or platform == "linux2":
        build_dir = resolve_path("bin/web")
    else:
        build_dir = resolve_path(".\\bin\\web")

    makedirs_silent(build_dir)
    os.chdir(build_dir)

    if platform == "linux" or platform == "linux2":
        os.system("emcmake cmake ../.. -DEMSCRIPTEN=ON -G \"Unix Makefiles\"")
        os.system("make")
    elif platform == "win32":
        os.system("emcmake cmake ../.. -DEMSCRIPTEN=ON -G \"NMake Makefiles\"")
        os.system("nmake")
    else:
        print("Unknown platform")
