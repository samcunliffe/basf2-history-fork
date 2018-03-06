#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# *****************************************************************************

# title           : 2A_CalculateAlignmentCosmics.py
# description     : Calculate alignment using cosmic data (Phase2)
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 8. 2. 2018

# *****************************************************************************

from basf2 import *
import os
import sys
import ROOT

from ROOT import Belle2
from caf import backends
from caf.framework import Calibration, CAF
from alignment import MillepedeCalibration
import tracking
from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction

import reconstruction as reco
import modularAnalysis as ana

inputFiles = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]
if not len(inputFiles):
    print(' Please specify input files for calibration via the -i option of basf2')
    sys.exit(1)

# Pre-collector full standard reconstruction path
main = create_path()
main.add_module("RootInput")
main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'EKLM',
    'BKLM',
    'ECL']

# reconstruction of cosmic track
add_svd_reconstruction(main)
add_pxd_reconstruction(main)
tracking.add_geometry_modules(main, components=components)
tracking.add_cr_tracking_reconstruction(
    main,
    components=components,
    prune_tracks=False,
    skip_geometry_adding=True,
    event_time_extraction=True,
    merge_tracks=False,
    data_taking_period='phase2',
    top_in_counter=False,
    use_second_cdc_hits=False)

main.add_module('Ext')

reconstruction.add_ecl_modules(main, components)
main.add_module('ECLTrackShowerMatch')
main.add_module('ECLElectronId')

main.add_module('EKLMReconstructor')
main.add_module('BKLMReconstructor')
main.add_module('KLMK0LReconstructor')
main.add_module('Muid')
main.add_module('KLMExpert')
main.add_module('ClusterMatcher')

# if magnetic field is in components:
main.add_module(
    "MergerCosmicTracks",
    recoTracksStoreArrayName="RecoTracks",
    MergedRecoTracksStoreArrayName="CosmicRecoTracks",
    usingMagneticField=True)

# if magnetic field is not in components:
"""
main.add_module(
    "MergerCosmicTracks",
    recoTracksStoreArrayName="RecoTracks",
    MergedRecoTracksStoreArrayName="CosmicRecoTracks",
    usingMagneticField=False)
"""

main.add_module(
    "TrackCreator",
    recoTrackColName="CosmicRecoTracks",
    trackColName="CosmicTracks",
    trackFitResultColName="CosmicTrackFitResult",
    useClosestHitToIP=True)

# Now use analysis to select alignment tracks/decays
# Select single muons for aligment...
# ana.fillParticleList('mu+:bbmu', 'muonID > 0.1 and useLabFrame(p) > 0.5', True, path)
# Pre-fit with beam+vertex constraint decays for muon pairs
# ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 2.', writeOut=True, path=path)
# ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=path)
# ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=path, constraint='ipprofile')

millepede = MillepedeCalibration(['VXDAlignment'],
                                 tracks=['CosmicRecoTracks'],
                                 particles=[],
                                 vertices=[],
                                 primary_vertices=[],
                                 path=main)
# tracks=['CosmicRecoTracks'],

# For simulated data:
# millepede.algo.invertSign()

millepede.algo.steering().command('Fortranfiles')
millepede.algo.steering().command('constraints.txt')

# millepede.fixPXDYing()
# millepede.fixPXDYang()
# millepede.fixSVDPat()
# millepede.fixSVDMat()

# Fix all ladders (only ladder=1 in Beast II)
# ladder = 1
# for layer in range(1, 7):
#    millepede.fixVXDid(layer, ladder, 0)

beast2_sensors = [
    (1, 1, 1), (1, 1, 2),
    (2, 1, 1), (2, 1, 2),
    (3, 1, 1), (3, 1, 2),
    (4, 1, 1), (4, 1, 2), (4, 1, 3),
    (5, 1, 1), (5, 1, 2), (5, 1, 3), (5, 1, 4),
    (6, 1, 1), (6, 1, 2), (6, 1, 3), (6, 1, 4), (6, 1, 5)
]

# for sensor_id in beast2_sensors:
#    layer, ladder, sensor = sensor_id
#    millepede.fixVXDid(layer, ladder, sensor, parameters=[1, 2, 3, 4, 5, 6])

# Note that here we replace the helper class by real Calibration class of CAF
millepede = millepede.create('beast2_alignment', inputFiles)

caf = CAF()

# Uncomment following line to use different global tag for payloads (not found in local DB below)
# Default can be loaded from basf2.get_default_global_tags()
#
# millepede.use_central_database(global_tag=basf2.get_default_global_tags())


# For testing misalignment, set it up in a local DB and uncomment following (with path to your local DB)
#
# millepede.use_local_database(os.path.abspath('localdb/database.txt'), directory="")


# Uncomment following to run on batch system (KEKCC)
#
# millepede.max_files_per_collector_job = 1
# millepede.backend_args = {"queue": "s"}
# caf.backend = backends.LSF()


# Or to run with local backend with e.g. 10 processes, do:
# millepede.max_files_per_collector_job = 1
# caf.backend = backends.Local(6)

caf.add_calibration(millepede)

caf.output_dir = 'caf_phase2'
caf.run()
