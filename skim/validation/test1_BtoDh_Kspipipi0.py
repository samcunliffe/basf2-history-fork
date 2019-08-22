#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_Kspipipi0.dst.root</input>
    <output>14120400.udst.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "N. Rout"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi
from stdCharged import stdK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from skimExpertFunctions import encodeSkimName, setSkimLogging


path = Path()

skimCode = encodeSkimName('BtoDh_Kspipipi0')

fileList = ['../BtoDh_Kspipipi0.dst.root']

inputMdstList('default', fileList, path=path)

# Load particle lists
stdPi('all', path=path)
stdK('all', path=path)
loadStdSkimPi0(path=path)
stdKshorts(path=path)


# Kspipipi0 skim
from skim.btocharm import loadDkspipipi0, BsigToDhToKspipipi0List
loadDkspipipi0(path=path)
BtoDhList = BsigToDhToKspipipi0List(path=path)
skimOutputUdst(skimCode, BtoDhList, path=path)
summaryOfLists(BtoDhList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
