#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

main = create_path()

# specify number of events to be generated in job
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process 10 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1
main.add_module(eventinfosetter)

# generate BBbar events
from beamparameters import add_beamparameters
add_beamparameters(main, "Y4S")

main.add_module('EvtGenInput')

# detecor simulation
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'ECL',
]
add_simulation(main, components)
# or add_simulation(main) to simulate all detectors

# reconstruction
add_reconstruction(main, components)
# or add_reconstruction(main) to run the reconstruction of all detectors

# output
output = register_module('RootOutput')
output.param('outputFileName', 'output.root')
main.add_module(output)
main.add_module('ProgressBar')

process(main)

# Print call statistics
print(statistics)
