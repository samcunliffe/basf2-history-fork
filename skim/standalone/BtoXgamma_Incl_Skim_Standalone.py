#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP standalone skim steering
#
# B->Xgamma inclusive skim
#
# T.R. Shillington July 2019
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import stdPi
from skimExpertFunctions import setSkimLogging, encodeSkimName, get_test_file
import argparse

# Use argparse to allow the optional --data argument, used only when run on data
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

# basic setup
gb2_setuprel = 'release-03-02-02'
skimCode = encodeSkimName('BtoXgamma')

path = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=path)

# import standard lists
stdPhotons('loose', path=path)
stdPhotons('all', path=path)
stdPi('all', path=path)

# call reconstructed lists from scripts/skim/ewp_incl.py
from skim.ewp import B2XgammaList
XgammaList = B2XgammaList(path=path)
skimOutputUdst(skimCode, XgammaList, path=path)
summaryOfLists(XgammaList, path=path)

# process
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
