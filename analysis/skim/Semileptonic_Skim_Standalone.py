#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0 import *
from stdV0s import *
from stdCharm import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'
     ]
inputMdstList(filelist)

loadStdCharged()
loadStdPi0()
loadStdKS()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# SL Skim
from Semileptonic_List import *
SLList = SemileptonicList()
skimOutputUdst('SLUntagged_Standalone', SLList)
summaryOfLists(SLList)

lepList = LeptonicList()
skimOutputUdst('LeptonicUntagged_Standalone', lepList)
summaryOfLists(lepList)

process(analysis_main)

# print out the summary
print(statistics)
