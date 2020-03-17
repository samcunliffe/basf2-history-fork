#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# Test script to produce SVD DQM plots
#
# Author:  Giulia Casarosa
#
# Usage: basf2 runEventT0DQM.py -i "<path/to/file.root>"
#
# 2020 Belle II Collaboration
#############################################################

import sys
import os
from basf2 import *
import svd.overlay_utils as svdou
import simulation as simu
import glob

tag = "unused"
if len(sys.argv) == 2:
    tag = sys.argv[1]

# PREPARE YOUR INPUT FILES - not working properly at the moment:
# function provides output rootfile with SVDShaperDigits only
'''
main = create_path()

svdou.prepare_svd_overlay(main,\
 ["/gpfs/fs02/belle2/group/detector/SVD/overlayFiles/cosmics/reco_firstCollisions_exp0010_run00311_120.root"]
)

process(main)
'''

# EXAMPLE OF OVERLAY
main = create_path()

set_random_seed(1)

# set the exp/run event informations
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [10])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')

# register HistoModules for DQM plots
main.add_module("HistoManager", histoFileName="SVDDQMOutput_"+str(tag)+".root")

# generate signal
main.add_module('EvtGenInput')

# add default simulation
bkgDir = '/group/belle2/BGFile/OfficialBKG/early_phase3/prerelease-04-00-00a/overlay/phase31/BGx1/set0/*.root'
bg = glob.glob(bkgDir)
if len(bg) == 0:
    B2ERROR('No files found in ', bkgDir)
    sys.exit()
simu.add_simulation(main, bkgfiles=bg, usePXDDataReduction=False, forceSetPXDDataReduction=True)

if str(tag) == "xTalk" or str(tag) == "cosmics":
    svdou.overlay_svd_data(main, str(tag))


# add offline ZS for DQM
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)
main.add_module('SVDDQMExpressReco', offlineZSShaperDigits='SVDShaperDigitsZS5')

main.add_module('RootOutput', outputFileName='RootOutput_100evt_'+str(tag)+'.root')
main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
