#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        TC Time offset Calibration module
#
#    usage : %> basf2 TrgEclTimeCalibation.py [1. input root file name] [2. output file name]
#
#    Also you can use wildcard : ex) basf2 TrgEclTimeCalibation.py input\*.root output.root
#
# -----------------------------------------------------------------------------------
from basf2 import *

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 3:
    sys.exit("ReadEclTrgUnpacker.py> # of arg is strange.\n 1.rootname\n 2.output file name\n Exit.")

if argc == 3:
    fname_in = argvs[1]
    fname_out = argvs[2]

set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

string_length = len(fname_in)
string_loc = 0
letter_remove = '\\'
while (string_loc < string_length):
    if fname_in[string_loc] == letter_remove:
        fname_in = fname_in[:string_loc] + fname_in[string_loc + 1::]
        string_length = len(fname_in)
    string_loc += 1
print("input File : ", fname_in)
print("output File : ", fname_out)

# calibration
tcal = register_module('TRGECLTimingCal')
input = register_module('RootInput')
input.param("inputFileName", fname_in)
tcal.param("TRGECLCalSim", 0)              # 0 data, 1 simulation : default 0
tcal.param("TRGECLCalType", 0)              # 0 beam, 1 cosmic : default 0
tcal.param("TRGECLCalTCRef", 184)           # Reference TC : default 184
tcal.param("TRGECLCal3DBhabhaVeto", 1)      # Use 3DBhabhaVeto bit : default 1
tcal.param("TRGECLCalHighEnergyCut", 10)   # TC high energy cut (GeV)  default 9999
tcal.param("TRGECLCalLowEnergyCut", 1)      # TC low energy cut  (GeV) default 0
tcal.param("TRGECLCalofname", fname_out)    # Output root file name

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(tcal)
m_progress = register_module('Progress')
m_progress.param("maxN", 4)
main.add_module(m_progress)


# Process all events
process(main)
print(statistics)
