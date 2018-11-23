#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__author__ = "K. Smith"


import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from stdLightMesons import *
from stdCharged import stdLoosePi
from stdCharged import stdLooseK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
from skimExpertFunctions import *


fileList = ['../CharmlessHad3BodyNeutral.dst.root']

inputMdstList('MC9', fileList)

# Load particle lists
stdPhotons('loose')
stdLooseK()
stdKshorts()
stdLoosePi()
stdPi0s('loose')
stdPi0s('all')
loadStdSkimPi0()
loadStdLightMesons()

# Hadronic B0 skim
from skim.btocharmless import *
Had3BodyList = CharmlessHad3BodyB0List() + CharmlessHad3BodyBmList()
skimOutputUdst('CharmlessHad3BodyNeutral.udst.root', Had3BodyList)
summaryOfLists(Had3BodyList)

# Suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
