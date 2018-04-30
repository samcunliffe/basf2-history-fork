#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import *
gb2_setuprel = 'release-01-00-00'
import sys
import os
import glob

fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]

inputMdstList('default', fileList)
stdPhotons('loose')

# Bottomonium Skim
from BottomoniumUpsilon_List import *
YList = UpsilonList()
outputLFN = getOutputLFN('BottomoniumUpsilon')
skimOutputUdst(outputLFN, YList)

summaryOfLists(YList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
