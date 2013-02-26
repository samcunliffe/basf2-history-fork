#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

rootFileName = '../MDSTtoUDST'
nOfEvents = -1
logFileName = rootFileName + '.log'
sys.stdout = open(logFileName, 'w')

import os
from basf2 import *

main = create_path()

# --------------------------------------------------------------
# set_log_level(LogLevel.ERROR)
input = register_module('RootInput')
input.param('inputFileName', '../DSTtoMDST.mdst.root')
main.add_module(input)

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')
gearbox = register_module('Gearbox')
main.add_module(progress)
main.add_module(gearbox)

# ----------------------------------------------------------------
# Add the geometry, necessary for Rave
geometry = register_module('Geometry')
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'ECL',
    'BKLM',
    'EKLM',
    'ESTR',
    'Coil',
    'STR',
    ]
geometry.param('Components', components)
main.add_module(geometry)

# ---------------------------------------------------------------
# Analysis module to check basic analysis input containers
analysis = register_module('analysisValidation')
# output root file name (the suffix .root will be added automaticaly)
analysis.param('outputFileName', rootFileName)
# specify the names of Track collections
analysis.param('GFTrackCandidatesColName', 'GFTrackCands')
analysis.param('GFTracksColName', 'GFTracks')
analysis.param('TracksColName', 'Tracks')
analysis.param('TestType', 'dst')
main.add_module(analysis)

# ---------------------------------------------------------------
# Produce a microdst with reconstructed Particles
# the content is not yet defined, but we want to first ensure the Particle class behaves as expected
output = register_module('RootOutput')
output.param('outputFileName', '../MDSTtoUDST.udst.root')
output.param('branchNames', ['MCParticles', 'Particles', 'EventMetaData'])
main.add_module(output)

# Go
process(main)

# Print call statistics
print statistics
