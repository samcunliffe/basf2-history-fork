#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from simulation import add_simulation
import os
import glob
import sys

# ---------------------------------------------------------------------------------------
# Example of producing summary ntuple of beam background hit rates on MC using BG overlay
#
# This example is for nominal phase-3.
# For phase-2 or early phase-3 one should replace 'expList' with 1002 or 1003, resp.,
# and use the corresponding BG overlay samples (e.g. redefine BELLE2_BACKGROUND_DIR)
#
# usage: basf2 beamBkgHitRates_MC.py
# ---------------------------------------------------------------------------------------


if 'BELLE2_BACKGROUND_DIR' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_DIR variable is not set - it must contain the path to BG overlay samples')
    sys.exit()

# background overlay files
bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
if len(bg) == 0:
    B2ERROR('No files found in ', os.environ['BELLE2_BACKGROUND_DIR'])
    sys.exit()

# Create path
main = basf2.create_path()

# Set number of events to generate
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [0], 'expList': [0]})
main.add_module(eventinfosetter)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=True)

# additional modules, if needed for hit processing
main.add_module('TOPChannelMasker')

# Bkg rate monitor: output to flat ntuple
# - all trigger types must be selected since no TRGSummary is given by the simulation
# - time offset and time window for TOP must be set differently than on data
main.add_module('BeamBkgHitRateMonitor', trgTypes=[],
                outputFileName='beamBkgHitRates_MC.root',
                topTimeOffset=25, topTimeWindow=90)

# Show progress of processing
main.add_module('Progress')

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
