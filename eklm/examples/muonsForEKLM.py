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
evtmetagen.param('EvtNumList', [10])

# Particle gun
particlegun = register_module('PGunInput')
particlegun.param('nTracks', 2)
particlegun.param('PIDcodes', [13, -13])
particlegun.param('pPar1', 0.05)
particlegun.param('pPar2', 3)

# Geometry parameter loader
paramloader = register_module('ParamLoaderXML')
paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

# Geometry builder
geobuilder = register_module('GeoBuilder')

# Full Geant4 simulation
g4sim = register_module('FullSim')

# Root file output
simpleoutput = register_module('SimpleOutput')
simpleoutput.param('outputFileName', 'muForEKLM.root')

# ------------  EKLM-related modules

# EKLM digi
eklmdigi = register_module('EKLMDigitization')
# EKLM reco
eklmreco = register_module('EKLMReconstruction')

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
