#!/usr/bin/env python

import os
import sys
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
    if len(sys.argv) != 2:
        print("Must supply build flags: debug/release")
        exit()

    build_arg = str(sys.argv[1]).lower();

    if build_arg == "debug":
        build_type = "Debug"
    elif build_arg == "release":
        build_type = "Release"
    else:
        print("Must supply valid build flags: debug/release")
        exit()

    if platform == "linux" or platform == "linux2":
        build_dir = resolve_path("bin/web/" + build_type)
    else:
        build_dir = resolve_path(".\\bin\\web\\" + build_type)

    makedirs_silent(build_dir)
    os.chdir(build_dir)

    build_string = "emcmake cmake ../../.. "
    build_string += "-DCMAKE_BUILD_TYPE=" + build_type + " "
    build_string += "-DEMSCRIPTEN=ON "

    if platform == "linux" or platform == "linux2":
        os.system(build_string + " -G \"Unix Makefiles\"")
        os.system("make")
    elif platform == "win32":
        os.system(build_string + " -G \"NMake Makefiles\"")
        os.system("nmake")
    else:
        print("Unknown platform")
