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
from stdCharged import stdPi, stdK, stdE, stdMu
from stdV0s import *

from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-03'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('Charm3BodyHadronic2')

c3bh2path = Path()

fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=c3bh2path)


stdKshorts(path=c3bh2path)
mergedKshorts(path=c3bh2path)
stdPi('loose', path=c3bh2path)
stdK('loose', path=c3bh2path)
stdE('loose', path=c3bh2path)
stdMu('loose', path=c3bh2path)
stdPi('all', path=c3bh2path)
stdK('all', path=c3bh2path)
stdE('all', path=c3bh2path)
stdMu('all', path=c3bh2path)

from skim.charm import DstToD0PiD0ToHpHmKs
DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs(c3bh2path)
skimOutputUdst(skimCode, DstToD0PiD0ToHpHmKsList, path=c3bh2path)

summaryOfLists(DstToD0PiD0ToHpHmKsList, path=c3bh2path)


setSkimLogging(path=c3bh2path)
process(c3bh2path)

print(statistics)
