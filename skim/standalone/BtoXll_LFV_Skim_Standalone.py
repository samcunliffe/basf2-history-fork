#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP standalone skim steering
#
# B->Xll (LFV modes only) inclusive skim
#
# Trevor Shillington July 2019
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdCharged import stdE, stdK, stdMu, stdPi
import skimExpertFunctions as expert

# basic setup
gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('BtoXll_LFV')

path = b2.Path()
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=path)

# import standard lists
stdE('loose', path=path)
stdMu('loose', path=path)
stdPi('all', path=path)
stdPhotons('all', path=path)

# call reconstructed lists from scripts/skim/ewp.py
from skim.ewp import B2XllListLFV
XllList = B2XllListLFV(path=path)
expert.skimOutputUdst(skimCode, XllList, path=path)
ma.summaryOfLists(XllList, path=path)

# process
expert.setSkimLogging(path=path)
b2.process(path=path)

# print out the summary
print(statistics)
