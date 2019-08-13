#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../feiSLB0.dst.root</input>
    <output>../feiSLB0.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from skimExpertFunctions import *

path = Path()

fileList = ['../feiSLB0.dst.root']

inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforB0SL(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

# Apply final B0 tag cuts
B0SLList = B0SL(path)
skimOutputUdst('../feiSLB0', B0SLList, path=path)
summaryOfLists(B0SLList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
