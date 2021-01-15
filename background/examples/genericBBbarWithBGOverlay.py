#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import os
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
import glob
import sys

# ----------------------------------------------------------------------------------
# Example of simulation/reconstruction of generic BBbar events with BG overlay.
#
# This example generates BBbar events using EvtGenInput module,
# runs full simulation and digitization,
# then adds measured BG to simulated data using BGOverlayExecutor module,
# runs full reconstruction and finaly writes the results to mdst file.
# ----------------------------------------------------------------------------------

b2.set_log_level(b2.LogLevel.ERROR)

if 'BELLE2_BACKGROUND_DIR' not in os.environ:
    b2.B2ERROR('BELLE2_BACKGROUND_DIR variable is not set - it must contain the path to BG overlay samples')
    sys.exit()

# background overlay files
bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
if len(bg) == 0:
    b2.B2ERROR('No files found in ', os.environ['BELLE2_BACKGROUND_DIR'])
    sys.exit()

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
main.add_module(evtgeninput)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=True)

# set debug level for overlay executor module
for m in main.modules():
    if m.type() == "BGOverlayExecutor":
        m.logging.log_level = b2.LogLevel.DEBUG  # comment or remove to turn off
        m.logging.debug_level = 100
        break

# Reconstruction
add_reconstruction(main)

# Mdst output
add_mdst_output(main)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
