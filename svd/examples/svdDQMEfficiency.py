# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
import rawdata as raw
import tracking as tr
from basf2 import conditions as b2conditions
from svd import utils

main = Path()
outputFile = sys.argv[1]

b2conditions.prepend_globaltag('data_reprocessing_prompt')

main.add_module('RootInput', branchNames=['RawSVDs', 'RawPXDs', 'RawCDCs'])
main.add_module('HistoManager', histoFileName=outputFile)

main.add_module('Gearbox')
main.add_module('Geometry')

# raw data unpacking
raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC'])
for module in main.modules():
    if module.name() == 'PXDPostErrorChecker':
        module.param('CriticalErrorMask', 0)

tr.add_tracking_reconstruction(main)

main = remove_module(main, 'FullGridChi2TrackTimeExtractor')


# ROI finder
svdDataRed = register_module('SVDROIFinder')
param_svdDataRed = {
    'recoTrackListName': 'RecoTracks',
    'SVDInterceptListName': 'SVDIntercepts',
}
svdDataRed.param(param_svdDataRed)
# svdDataRed.logging.log_level = LogLevel.INFO
main.add_module(svdDataRed)

dqm = register_module('SVDDQMEfficiency')
dqm.param("svdClustersName", "SVDClusters")
dqm.param("interceptsName", "SVDIntercepts")
dqm.param("histogramDirectoryName", "svdeff")
dqm.param("binsU", 4)
dqm.param("binsV", 6)
dqm.param("minSVDHits", 1)
dqm.param("minCDCHits", 20)
dqm.param("momCut", 0)
dqm.param("ptCut", 1)
dqm.param("maxHalfResidU", 0.05)
dqm.param("maxHalfResidV", 0.05)
# dqm.logging.log_level = LogLevel.ERROR
main.add_module(dqm)

# Process the events
main.add_module('Progress')
print_path(main)
process(main)

# print out the summary
print(statistics)
