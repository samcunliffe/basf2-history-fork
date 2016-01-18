#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 BBbar events.
#
# Usage: basf2 example.py
#
# Input: None
# Output: output.root
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from HLTTrigger import add_HLT_Y4S

# create path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process 10 events
main.add_module(eventinfosetter)

# generate BBbar events
from beamparameters import add_beamparameters
add_beamparameters(main, "Y4S")
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main)
# or add_simulation(main, components) to simulate a selection of detectors

# HLT L3 simulation
main.add_module('Level3')

# reconstruction
add_reconstruction(main)
# or add_reconstruction(main, components) to run the reconstruction of a selection of detectors

# HLT physics trigger
add_HLT_Y4S(main)

# full output
main.add_module('RootOutput', outputFileName='output.root')

# mdst output
add_mdst_output(main)

# display a progress bar while running
main.add_module('ProgressBar')

# process events and print call statistics
process(main)
print(statistics)
