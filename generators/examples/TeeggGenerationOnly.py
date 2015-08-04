#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# 100 radiative Bhabha events are generated using the
# TEEGG generator, configuration: GAMMA-SOFT
#
# Example steering file
########################################################

from basf2 import *
from beamparameters import add_beamparameters

# Set the global log level
set_log_level(LogLevel.INFO)

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)

# Register the BABAYAGA.NLO module
teegg = register_module('TeeggInput')

# WEIGHTED (0) or UNWEIGHTED (1)
teegg.param('UNWEIGHTED', 1)

# CONFIG
teegg.param('CONFIG', 'GAMMA')

# RADCOR and MAXWEIGHTS
teegg.param('RADCOR', 'HARD')
teegg.param('WGHT1M', 1.001)  # SOFT
teegg.param('WGHTMX', 1.150)  # SOFT

# TEVETO
teegg.param('TEVETO', 5.0)
teegg.param('EEVETO', 0.5)

# TGMIN
teegg.param('TGMIN', 12.5)

# TEMIN
# teegg.param('TEMIN', 0.26180* 180.0 / 3.1415)

# CUTOFF
teegg.param('CUTOFF', 0.0070)

# PEGMIN
# teegg.param('PEGMIN', 0.78540* 180.0 / 3.1415)

# EEMIN
# teegg.param('EEMIN', 5.0)

# EGMIN
teegg.param('EGMIN', 0.50)

# UNWGHT
# teegg.param('UNWGHT', 1)

# output
output = register_module('RootOutput')
output.param('outputFileName', './teegg-outfile.root')

# Create the main path and add the modules
main.add_module("Progress")
main.add_module(teegg)
main.add_module(output)
# uncomment the following line if you want event by event info
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print statistics
