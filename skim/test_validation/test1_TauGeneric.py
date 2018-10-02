#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__author__ = "Kenji Inami"

import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from skimExpertFunctions import *

fileList = ['../TauGeneric.dst.root']

inputMdstList('MC9', fileList)

# Hadronic B0 skim
from skim.taupair import *
tauList = TauList()
skimOutputUdst('TauGeneric.udst.root', tauList)
summaryOfLists(tauList)

# Suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
