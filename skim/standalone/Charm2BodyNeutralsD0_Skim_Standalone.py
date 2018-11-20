#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdV0s import *
from stdPi0s import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyNeutralsD0')
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)


loadStdSkimPi0()
loadStdCharged()
stdKshorts()
# stdPi0s()

from skim.charm import D0ToNeutralsList

D0ToNeutralsList = D0ToNeutrals()
skimOutputUdst(skimCode, D0ToNeutralsList)

summaryOfLists(D0ToNeutralsList)


setSkimLogging()
process(analysis_main)

print(statistics)
