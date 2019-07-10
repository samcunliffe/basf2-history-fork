#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from optparse import OptionParser
from tracking import add_tracking_reconstruction
from modularAnalysis import inputMdst
from tracking import add_tracking_reconstruction
import os
# --------------------------------------------------------------------
# Performs Geant4 simulation of events generated with evtgen_gen.py
# PXD, SVD, CDC and ARICH detectors are used. Reconstruction of tracks
# is also included and objects needed for ARICH reconstruction are stored
# in output root file.
# --------------------------------------------------------------------

parser = OptionParser()
parser.add_option('-f', '--file', dest='filename',
                  default='ARICHEvents.root')
(options, args) = parser.parse_args()

home = os.environ['BELLE2_LOCAL_DIR']

mypath = create_path()

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# load input ROOT file
inputMdst('None', home + '/B2Kpi_events.root', path=mypath)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
mypath.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'ARICH'])
mypath.add_module(geometry)

# Simulation
simulation = register_module('FullSim')
mypath.add_module(simulation)

# PXD digitization & clustering
pxd_digitizer = register_module('PXDDigitizer')
mypath.add_module(pxd_digitizer)
pxd_clusterizer = register_module('PXDClusterizer')
mypath.add_module(pxd_clusterizer)

# SVD digitization & clustering
svd_digitizer = register_module('SVDDigitizer')
mypath.add_module(svd_digitizer)
svd_clusterizer = register_module('SVDClusterizer')
mypath.add_module(svd_clusterizer)

# CDC digitization
cdcDigitizer = register_module('CDCDigitizer')
mypath.add_module(cdcDigitizer)

# tracking reconstruction
add_tracking_reconstruction(mypath)

# Track extrapolation
ext = register_module('Ext')
mypath.add_module(ext)

# This creates relations between ExtHits (track points on aerogel plane, from
# extrapolated CDC tracks) and ARICHAeroHits (MC hits on aerogel plane).
# It allows to have relevant MC information
# without storing full MCParticles (which are LARGE) into output root file.
arichRELATE = register_module('ARICHRelate')
mypath.add_module(arichRELATE)

# store branches needed for ARICH reconstruction in root file
output = register_module('RootOutput')
output.param('outputFileName', options.filename)
output.param('branchNames', ['ARICHAeroHits', 'ARICHSimHits', 'ExtHits',
                             'ARICHAeroHitsToExtHits', 'Tracks', 'TrackFitResults'])
mypath.add_module(output)

# Show progress of processing
progress = register_module('Progress')
mypath.add_module(progress)

# Process events
process(mypath)

# Print call statistics
print(statistics)
