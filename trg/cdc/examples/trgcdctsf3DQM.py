#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger DQM Module
#
#    usage : %> basf2 trgcdctsfDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

from basf2 import *
################
import sys  # get argv
import re
import os.path
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

set_log_level(LogLevel.DEBUG)

main = create_path()

if argc == 2 and argvs[1][-6:0] == ".sroot":
    f_in_root = argvs[1]
    input = register_module('SeqRootInput')
    matchobj = re.search(r"([^\/]+)\.sroot", f_in_root)
    basename = re.sub(r'\.sroot$', '', matchobj.group())
    input.param('inputFileName', f_in_root)
elif argc == 2 and argvs[1][-5:] == ".root":
    f_in_root = argvs[1]
    input = register_module('RootInput')
    matchobj = re.search(r"([^\/]+)\.root", f_in_root)
    basename = re.sub(r'\.root$', '', matchobj.group())
    input.param('inputFileName', f_in_root)
elif argc == 1:
    input = register_module('RootInput')
    input.param('inputFileName', '/home/belle/nkzw/e3.4S/r034*/all/raw/sub00/raw.physics.hlt_hadron.0003.*.root')
    basename = "e3.4S.r034"
else:
    sys.exit("trgcdctsfDQM.py> # of arg is strange. Exit.")

main.add_module(input)
histo = register_module('HistoManager')
histo.param("histoFileName", "dqm.%s.root" % basename)

# Unpacker
trgcdctsfUnpacker = register_module("TRGCDCTSFUnpacker")
trgcdctsfUnpacker.param('TSFMOD', 3)
main.add_module(trgcdctsfUnpacker)
main.add_module(histo)

# DQM
trgcdctsfdqm = register_module('TRGCDCTSFDQM')
trgcdctsfdqm.param('generatePostscript', True)
trgcdctsfdqm.param('TSFMOD', 3)
# postscript file name
psname = "dqm.%s.ps" % basename
trgcdctsfdqm.param('postScriptName', psname)

main.add_module(trgcdctsfdqm)

progress = register_module('Progress')
main.add_module(progress)

process(main)

print(statistics)
