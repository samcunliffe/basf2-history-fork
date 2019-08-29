#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run all dark skims at once
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu, stdK, stdE
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *

from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file

gb2_setuprel = 'release-03-02-00'

fileList = get_test_file("mixedBGx1", "MC12")

darkskimpath = Path()
inputMdstList('default', fileList, path=darkskimpath)

stdMu('all', path=darkskimpath)
stdMu('loose', path=darkskimpath)
stdE('all', path=darkskimpath)
stdE('loose', path=darkskimpath)
stdPi('all', path=darkskimpath)
stdPi('loose', path=darkskimpath)
stdK('all', path=darkskimpath)
stdK('loose', path=darkskimpath)
stdPi0s('loose', path=darkskimpath)
stdPhotons('loose', path=darkskimpath)
stdKshorts(path=darkskimpath)
loadStdSkimPi0(path=darkskimpath)
loadStdSkimPhoton(path=darkskimpath)

cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=darkskimpath)

from skim import dark
add_skim('ALP3Gamma', dark.ALP3GammaList(path=darkskimpath), path=darkskimpath)
add_skim('LFVZpVisible', dark.LFVZpVisibleList(path=darkskimpath), path=darkskimpath)
add_skim('SinglePhotonDark', dark.SinglePhotonDarkList(path=darkskimpath), path=darkskimpath)
add_skim('DimuonPlusMissingEnergy', dark.DimuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
add_skim('ElectronMuonPlusMissingEnergy', dark.ElectronMuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
add_skim('DielectronPlusMissingEnergy', dark.DielectronPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)

setSkimLogging(path=darkskimpath)
process(darkskimpath)

# print out the summary
print(statistics)
