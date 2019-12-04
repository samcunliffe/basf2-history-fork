#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
    FEI Hadronic B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180200
    fei training: MC12 based, release-04-00-00 'FEIv4_2019_MC12_release_03_01_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]


from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-04-00-00'

import sys
import os
import glob
skimCode = encodeSkimName('feiHadronicBplus')
fileList = get_test_file("MC12_mixedBGx1")

path = create_path()

inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforBplusHadronic(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)

# Apply final B+ tag cuts
BplusHadronicList = BplusHadronic(path)
skimOutputUdst(skimCode, BplusHadronicList, path=path)
summaryOfLists(BplusHadronicList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
