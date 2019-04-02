#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skim using thrust
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

skimCode = encodeSkimName('TauThrust')

tauthrustskim = Path()

fileList = get_test_file("mixedBGx1", "MC11")

inputMdstList('default', fileList, path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# Tau Skim
from skim.taupair import *
tauList = TauThrustList(path=tauthrustskim)

skimOutputUdst(skimCode, tauList, path=tauthrustskim)
summaryOfLists(tauList, path=tauthrustskim)

setSkimLogging(path=tauthrustskim)
process(tauthrustskim)

# print out the summary
print(statistics)
