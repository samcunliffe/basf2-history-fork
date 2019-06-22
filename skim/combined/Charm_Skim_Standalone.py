#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdMu, stdE, stdMu
from stdV0s import *
from stdPi0s import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'
set_log_level(LogLevel.INFO)

charmpath = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=charmpath)


loadStdSkimPhoton(path=charmpath)
loadStdSkimPi0(path=charmpath)
stdPi('loose', path=charmpath)
stdK('loose', path=charmpath)
stdE('loose', path=charmpath)
stdMu('loose', path=charmpath)
stdPi('all', path=charmpath)
stdK('all', path=charmpath)
stdE('all', path=charmpath)
stdMu('all', path=charmpath)
stdKshorts(path=charmpath)
mergedKshorts(path=charmpath)


from skim.charm import DstToD0PiD0ToHpJmPi0
DstToD0PiD0ToHpJmPi0List = DstToD0PiD0ToHpJmPi0(charmpath)
add_skim("Charm3BodyHadronic", DstToD0PiD0ToHpJmPi0List, path=charmpath)

from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(charmpath)
add_skim("Charm2BodyHadronic", DstToD0PiD0ToHpJmList, path=charmpath)


from skim.charm import DstToD0PiD0ToHpHmPi0
DstToD0PiD0ToHpHmPi0List = DstToD0PiD0ToHpHmPi0(path=charmpath)
add_skim("Charm3BodyHadronic1", DstToD0PiD0ToHpHmPi0List, path=charmpath)


from skim.charm import DstToD0PiD0ToHpHmKs
DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs(charmpath)
add_skim("Charm3BodyHadronic2", DstToD0PiD0ToHpHmKsList, path=charmpath)

from skim.charm import DstToD0PiD0ToHpJmEta
DstToD0PiD0ToHpJmEtaList = DstToD0PiD0ToHpJmEta(charmpath)
add_skim("Charm3BodyHadronic3", DstToD0PiD0ToHpJmEtaList, path=charmpath)

from skim.charm import DstToD0PiD0ToKsOmega
DstList = DstToD0PiD0ToKsOmega(charmpath)
add_skim("Charm2BodyNeutrals2", DstList, path=charmpath)

from skim.charm import DstToD0Neutrals
add_skim("Charm2BodyNeutrals", DstToD0Neutrals(charmpath), path=charmpath)


from skim.charm import CharmRareList
CharmRareList = CharmRareList(charmpath)
add_skim("CharmRare", CharmRareList, path=charmpath)

setSkimLogging(path=charmpath)
process(charmpath)

print(statistics)
