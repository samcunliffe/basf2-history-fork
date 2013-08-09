#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

if len(sys.argv) != 3:
    # the program name and the two arguments
    # stop the program and print an error message
    sys.exit('Must provide two input parameters:[output_root_file_name] [#events_to_generate]'
             )

rootFileName = sys.argv[1]
nOfEvents = int(sys.argv[2])
logFileName = rootFileName + '.log'

sys.stdout = open(logFileName, 'w')

import os

from basf2 import *
# set_log_level(LogLevel.ERROR)

# ---------------------------------------------------------------
# EvtGen
# boost all momenta to LAB system
# use specified user decay file
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/analysis/modules/B2Dpi/datafiles/BtoDpi.dec')

# specify number of events to be generated in job
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('evtNumList', [nOfEvents])  # process nOfEvents events
evtmetagen.param('runList', [1])  # from run number 1
evtmetagen.param('expList', [1])  # and experiment number 1

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')

# ---------------------------------------------------------------
# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking+PID detectors
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])

# ---------------------------------------------------------------
# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# ---------------------------------------------------------------
# CDC digitizer
cdcDigitizer = register_module('CDCDigitizer')

# ---------------------------------------------------------------
pxd_digi = register_module('PXDDigitizer')
pxd_cls = register_module('PXDClusterizer')
svd_digi = register_module('SVDDigitizer')
svd_cls = register_module('SVDClusterizer')

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.param('UseCDCHits', True)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UsePXDHits', True)
mctrackfinder.param('GFTrackCandidatesColName', 'GFTrackCands')
mctrackfinder.param('WhichParticles', ['primary'])

# ---------------------------------------------------------------
# match the found track candidates with MCParticles
mcmatching = register_module('CDCMCMatching')

# select the correct collection for the matching
param_mcmatching = {'GFTrackCandidatesColName': 'GFTrackCands'}
mcmatching.param(param_mcmatching)

# ---------------------------------------------------------------
# Track fitting is performed with GenFit
trackfitting = register_module('GenFitter')

# set correct collection name as input and custom collection names as output
# select DAF instead of Kalman as Filter
# set the pdg hypothesis to the simulated one, if you want to fit with
# different pdg hypothesises, set 'allPDG' to true
# param_cdcfitting = {
#     'GFTrackCandidatesColName': 'GFTrackCands',
#     'TracksColName': 'Tracks',
#     'GFTracksColName': 'GFTracks',
#     'GFTracksToMCParticlesColName': 'GFTracksToMCParticles',
#     'StoreFailedTracks': 0,
#     'mcTracks': 1,
#     'pdg': 211,
#     'allPDG': 0,
#     'FilterId': 0,
#     'NIterations': 1,
#     'ProbCut': 0.001,
#     }
#
# cdcfitting.param(param_cdcfitting)
#
# ---------------------------------------------------------------
# dE/dx PID
dedx = register_module('DedxPID')
dedx_params = {'onlyPrimaryParticles': True, 'useSVD': False}
dedx.param(dedx_params)

# ---------------------------------------------------------------
# Track extrapolation
ext = register_module('Ext')
ext.param('GFTracksColName', 'GFTracks')  # input to ext
ext.param('ExtTrackCandsColName', 'ExtTrackCands')  # output from ext
ext.param('ExtRecoHitsColName', 'ExtRecoHits')  # output from ext

# ---------------------------------------------------------------
# TOP - digitization
topdigi = register_module('TOPDigitizer')

# ---------------------------------------------------------------
# TOP reconstruction
topreco = register_module('TOPReconstructor')

# ---------------------------------------------------------------
# ARICH - digitization
arichDigi = register_module('ARICHDigitizer')

# ---------------------------------------------------------------
# ARICH reconstruction
arichRec = register_module('ARICHReconstructor')

# ---------------------------------------------------------------
# Analysis module to study B0 -> Phi Kshort decays
analysis = register_module('B2Dpi')

# output root file name (the suffix .root will be added automaticaly)
analysis.param('outputFileName', rootFileName)

# specify the names of Track collections
analysis.param('GFTrackCandidatesColName', 'GFTrackCands')
analysis.param('GFTracksColName', 'GFTracks')
analysis.param('TracksColName', 'Tracks')

# ---------------------------------------------------------------
# Add all modules to the main path
main = create_path()
main.add_module(evtmetagen)
main.add_module(evtgeninput)

main.add_module(progress)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(g4sim)

main.add_module(cdcDigitizer)

main.add_module(pxd_digi)
main.add_module(pxd_cls)

main.add_module(svd_digi)
main.add_module(svd_cls)

main.add_module(mctrackfinder)

main.add_module(mcmatching)
main.add_module(trackfitting)

# main.add_module(dedx)
# main.add_module(ext)
# main.add_module(topdigi)
# main.add_module(topreco)
# main.add_module(arichDigi)
# main.add_module(arichRec)

# main.add_module(gammaRec)
# main.add_module(pi0Rec)

main.add_module(analysis)

process(main)

# Print call statistics
print statistics
