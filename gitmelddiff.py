#!/usr/bin/python

import sys
import os

sys.stdout.write("meld '%s' '%s'\n" % (sys.argv[2], sys.argv[5]))
os.system("meld '%s' '%s'" % (sys.argv[2], sys.argv[5]))
