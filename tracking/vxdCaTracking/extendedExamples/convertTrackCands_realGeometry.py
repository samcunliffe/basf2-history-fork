#!/usr/bin/env python
# -*- coding: utf-8 -*-

# example file invoking and testing the ConverterModules that convert genfit::TrackCands to SpacePointTrackCands and vice versa
import os
from basf2 import *
from sys import argv

numEvents = 100

# setting debuglevels for more than one module at once (currently set for the converter modules)
MyLogLevel = LogLevel.INFO
MyDebugLevel = 500

# set for the curling splitter module
MyOtherLogLevel = LogLevel.INFO
MyOtherDebugLevel = 500

initialValue = 0  # 0 for random events

if len(argv) is 1:
    print 'no arguments given, using standard values'
if len(argv) is 2:
    initialValue = int(argv[1])
    print 'using input value ' + str(initialValue) + ' as initialValue'

# module registration
set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRCDC',
               'PXD', 'SVD'])

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # need for MCTrackfinder to work correctly

# digitizers & clusterizers
pxdDigitizer = register_module('PXDDigitizer')

# cant name clusters, because there is no way to pass these names to the TrackFinderMCTruth
pxdClusterizer = register_module('PXDClusterizer')
# pxdClusterizer.param('Clusters','myPXDClusters')

svdDigitizer = register_module('SVDDigitizer')
svdClusterizer = register_module('SVDClusterizer')
# svdClusterizer.param('Clusters','mySVDClusters')

# SpacePoint Creation
# single clusters only
spCreatorSVDsingle = register_module('SpacePointCreatorSVD')
spCreatorSVDsingle.logging.log_level = LogLevel.INFO
spCreatorSVDsingle.param('OnlySingleClusterSpacePoints', True)
spCreatorSVDsingle.param('NameOfInstance', 'singleClusters')
spCreatorSVDsingle.param('SpacePoints', 'singleSP')
# spCreatorSVD.param('SVDClusters', 'singleSP')

# double clusters
spCreatorSVD = register_module('SpacePointCreatorSVD')
spCreatorSVD.logging.log_level = LogLevel.INFO
param_spCreatorSVD = {'OnlySingleClusterSpacePoints': False,
                      'NameOfInstance': 'couplesClusters',
                      'SpacePoints': 'nosingleSP'}
spCreatorSVD.param(param_spCreatorSVD)

spCreatorPXD = register_module('SpacePointCreatorPXD')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
# spCreatorPXD.param('PXDClusters', 'myPXDClusters')
spCreatorPXD.param('SpacePoints', 'pxdOnly')

# MCTrackFinder
mcTrackFinder = register_module('TrackFinderMCTruth')
mcTrackFinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 4,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    'TrueHitMustExist': True,
    }
mcTrackFinder.param(param_mctrackfinder)

# TCConverter, genfit -> SPTC
trackCandConverter = register_module('GFTC2SPTCConverter')
trackCandConverter.logging.log_level = MyLogLevel
trackCandConverter.logging.debug_level = MyDebugLevel
param_trackCandConverter = {  #    'PXDClusters': 'myPXDClusters',
                              #    'SVDClusters': 'mySVDClusters',
                              # use everything there is
                              # does not work at the moment anyway
    'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracks',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'SingleClusterSVDSP': 'singleSP',
    'PXDClusterSP': 'pxdOnly',
    'checkTrueHits': False,
    }
trackCandConverter.param(param_trackCandConverter)

# TCConverter, SPTC -> genfit
btrackCandConverter = register_module('SPTC2GFTCConverter')
btrackCandConverter.logging.log_level = MyLogLevel
btrackCandConverter.logging.debug_level = MyDebugLevel
param_btrackCandConverter = {'SpacePointTCName': 'SPTracks',
                             'genfitTCName': 'myMCTracks'}
btrackCandConverter.param(param_btrackCandConverter)

# TCConverterTest
tcConverterTest = register_module('TCConvertersTest')
tcConverterTest.logging.log_level = MyLogLevel
tcConverterTest.logging.debug_level = MyDebugLevel
param_tcConverterTest = {'SpacePointTCName': 'SPTracks',
                         'genfitTCNames': ['mcTracks', 'myMCTracks'],
                         'SpacePointArrayNames': 'pxdOnly'}
tcConverterTest.param(param_tcConverterTest)

curlingSplitter = register_module('CurlingTrackCandSplitter')
curlingSplitter.logging.log_level = MyOtherLogLevel
curlingSplitter.logging.debug_level = MyOtherDebugLevel
param_curlingSplitter = {  # set to true if you want to analyze the position of SpacePoints and the TrueHits they are related to
    'SpacePointTCName': 'SPTracks',
    'curlingFirstOutName': 'firstOutParts',
    'curlingAllInName': 'allInParts',
    'curlingRestOutName': 'restOutParts',
    'completeCurlerName': 'completeCurler',
    'splitCurlers': True,
    'nTrackStubs': int(0),
    'setOrigin': [0., 0., 0.],
    'positionAnalysis': False,
    'rootFileName': ['SPtoTrueHitAnalysis', 'RECREATE'],
    }
curlingSplitter.param(param_curlingSplitter)

# Path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(svdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdClusterizer)
main.add_module(spCreatorPXD)
main.add_module(spCreatorSVD)
main.add_module(spCreatorSVDsingle)
main.add_module(mcTrackFinder)

main.add_module(trackCandConverter)
main.add_module(btrackCandConverter)
main.add_module(tcConverterTest)

main.add_module(curlingSplitter)

process(main)

print statistics
