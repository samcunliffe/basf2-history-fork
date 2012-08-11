#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file is based on
# simulation/examples/FullGeant4SimulationParticleGun.py
#
# Muons (and ani-muons) are simulated for EKLM testing
#
# 100 events for experiment and run number 1 are created.
#
# The following parameters are used:
#  Number of events:      100
#  Tracks per event:      2
#  Particles:             mu+ / mu -
#  Theta [default]:       17 to 150 degree
#  Phi [default]:         0 to 360 degree
#  Momentum:              50 MeV to 3 GeV
#
# Example steering file - 2011 Belle II Collaboration
########################################################

import os
import random
from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# EvtMetaGen - generate event meta data
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [20])
evtmetagen.param('RunList', [1])

# Particle gun
particlegun = register_module('PGunInput')
particlegun.param('nTracks', 1)
particlegun.param('PIDcodes', [13, -13])
particlegun.param('pPar1', 20)
particlegun.param('pPar2', 20.01)

particlegun.param('thetaPar1', 25)
particlegun.param('thetaPar2', 25.01)

particlegun.param('phiPar1', 20)
particlegun.param('phiPar2', 20.01)

# Geometry parameter loader
paramloader = register_module('Gearbox')

# Geometry builder
geobuilder = register_module('Geometry')
geobuilder.log_level = LogLevel.INFO

geobuilder.param('Components', ['EKLM'])

# Full Geant4 simulation
g4sim = register_module('FullSim')

# Root file output
simpleoutput = register_module('SimpleOutput')
simpleoutput.param('outputFileName', 'muForEKLM.root')

# MC printuots
mcprint = register_module('PrintMCParticles')

# ------------  EKLM-related modules

# EKLM digi
eklmdigi = register_module('EKLMDigitizer')
eklmdigi.log_level = LogLevel.INFO
# EKLM reco
eklmreco = register_module('EKLMReconstructor')
eklmreco.log_level = LogLevel.INFO

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmetagen)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)

main.add_module(eklmdigi)
main.add_module(eklmreco)

main.add_module(simpleoutput)

# Process 100 events
process(main)

