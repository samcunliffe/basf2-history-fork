#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Subtract cal pulse time in TOPDigits::m_time
#
# Usage: basf2 subtractCalPulse.py [channel] -i <input_file.root> -o <output_file.root>
#   channel  calibration channel (0-7, default=0)
#   input_file  root file prepared with unpackToTOPDigits.py (calibrationChannel must be set)
#   outpu_file  root output file (default = RootOutput.root)
#
# cal pulse time is subtracted only if exactly two cal pulses are found for given asic
# if this requirement is not satisfied the hits for given asic are marked as c_Junk
# (see python module)
# ---------------------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2
import sys

calChannel = 0
argvs = sys.argv
if len(argvs) > 1:
    calChannel = int(argvs[1])

b2.B2RESULT('using calibration channel ' + str(calChannel))


class SubtractCalSignal(b2.Module):
    ''' subtract time of the first calibration signal '''

    def event(self):
        ''' event function '''

        digits = Belle2.PyStoreArray('TOPDigits')
        t0 = [0 for i in range(64)]
        num = [0 for i in range(64)]
        for digit in digits:
            channel = digit.getChannel()
            i = int(channel / 8)
            ch = int(channel) % 8
            if ch == calChannel and digit.getHitQuality() == 4:
                if num[i] == 0:
                    t0[i] = digit.getTime()
                    num[i] += 1
                else:
                    t0[i] = min(t0[i], digit.getTime())
                    num[i] += 1
        sum = 0
        for digit in digits:
            i = int(digit.getChannel()/8)
            if num[i] == 2:
                digit.subtractT0(t0[i])
                sum += 1
            else:
                digit.setHitQuality(0)
        if sum == 0:
            b2.B2ERROR("No calibration double pulses found in the event")


# Create path
main = b2.create_path()

# input
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# Subtract time of the first calibration signal pulse
main.add_module(SubtractCalSignal())

# output
output = b2.register_module('RootOutput')
main.add_module(output)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
