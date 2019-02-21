#!/usr/bin/env python

# Wrapper for Arduino core / others that can call esptool.py possibly multiple times

# First patameter is esptool.py path, then a series of command arguments separated with --end
# i.e. upload.py tools/esptool/esptool.py erase_flash --end write_flash file 0x0 --end

import sys
import subprocess

sys.argv.pop(0) # Remove executable name
try:
    esptool = sys.argv.pop(0).replace('\\', '/') # Full path to esptool.py, in UNIX format
except:
    sys.stderr.write("Error in command line, need esptool.py path.\n")
    sys.exit(1)

fakeargs = [];
while len(sys.argv):
    if sys.argv[0] == '--end':
        args = [sys.executable, esptool] + fakeargs
        sys.stderr.write("Running: " + " ".join(args) + "\n")
        if not subprocess.call(args):
            sys.exit(1)
        sys.argv.pop(0) # Remove --end
        fakeargs = []
    else:
        # We silently replace the 921kbaud setting with 460k to enable backward
        # compatibility with the old esptool-ck.exe.  Esptool.py doesn't seem
        # work reliably at 921k, but is still significantly faster at 460kbaud.
        thisarg = sys.argv.pop(0)
        if thisarg == "921600":
            thisarg = "460800"
        fakeargs = fakeargs + [thisarg]
