#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skims for LFV Z'
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

skimCode = encodeSkimName('LFVZpInvisible')
import sys
import os
import glob

lfvzppath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('MC9', fileList, path=lfvzppath)
stdPi('loose', path=lfvzppath)
stdK('loose', path=lfvzppath)
stdE('loose', path=lfvzppath)
stdMu('loose', path=lfvzppath)
stdPi('all', path=lfvzppath)
stdK('all', path=lfvzppath)
stdE('all', path=lfvzppath)
stdMu('all', path=lfvzppath)

from skim.dark import LFVZpInvisibleList
SysList = LFVZpInvisibleList(path=lfvzppath)
skimOutputUdst(skimCode, SysList, path=lfvzppath)
summaryOfLists(SysList, path=lfvzppath)

setSkimLogging(path=lfvzppath)
process(lfvzppath)

print(statistics)
