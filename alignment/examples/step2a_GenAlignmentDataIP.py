#!/usr/bin/env python
# -*- coding: utf-8 -*-

# BELLE II VXD ALIGNMENT
# STEP 2a:
# Fitting of IP data with misaligned geometry and production of data for alignment

import sys
from basf2 import *
from alignment_tools import *

# Set the log level to show only warning, error and, fatal messages
set_log_level(LogLevel.WARNING)
release = str(os.getenv('BELLE2_LOCAL_DIR')) + '/'

# -------------------------------------------------
#          IMPORTANT PROCESSING SETTINGS
# -------------------------------------------------

geometry = 'alignment/misalignedBelle2VXD.xml'  # Gear path

# ------------- INPUT/OUTPUT SETTINGS ------------

# Input root file with merged digits + event data (nothing else)
input_root = 'sampleIP.root'

# Output Mille binary file with data for alignment
alignment_binary = 'ipVXD.mille'

# ------------------------------------------------
# ------------------------------------------------

# Load parameters from xml
gearbox = register_module('Gearbox')
gearbox.param('fileName', geometry)

# Create geometry
geometry = register_module('Geometry')
geometry.param('components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD'])

# input data
input = register_module('RootInput')
input.param('inputFileName', input_root)
input.param('excludeBranchNames', [
    'Tracks',
    'TrackCandsToTrackFitResults',
    'TrackCandsToGF2Tracks',
    'MCParticlesToTracks',
    'TrackFitResults',
    'GF2Tracks',
    'GF2TracksToMCParticles',
    'GF2TracksToTrackFitResults',
])

# GBL fitting -> produces binary data for alignment
gbl = register_module('GBLfit')
gbl.param('milleFileName', alignment_binary)
gbl.param('chi2Cut', 0.0)

gbl.logging.log_level = LogLevel.ERROR

# Display progress of processing
progress = register_module('Progress')

# Event display (comment in path if not needed)
display = register_module('Display')
display.param('fullGeometry', True)
display.param('options', 'DHMPS')

# Add output module
output = register_module('RootOutput')
output.param('outputFileName', 'misalignedIP.root')

# -----------------------------------------------
#               Path construction
# -----------------------------------------------

main = create_path()
main.add_module(input)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(gbl)
# main.add_module(display)
main.add_module(output)
main.add_module(progress)
process(main)

print statistics
