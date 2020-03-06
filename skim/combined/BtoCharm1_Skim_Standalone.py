#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmSkims in one _standalone
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons, loadStdLightMesons, loadStdAllRhoPlus
import skimExpertFunctions as expert


btocharmpath = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=btocharmpath)


stdPi('all', path=btocharmpath)
stdK('all', path=btocharmpath)
loadStdPi0ForBToHadrons(path=btocharmpath)
stdKshorts(path=btocharmpath)
loadStdPi0ForBToHadrons(path=btocharmpath)
loadStdAllRhoPlus(path=btocharmpath)

# B+ to anti-D0(->h+h-)h+ Skim
from skim.btocharm import BsigToD0hTohhList
from skim.standardlists.charm import loadD0_hh_loose
loadD0_hh_loose(path=btocharmpath)
BtoD0h_hh_List = BsigToD0hTohhList(path=btocharmpath)
expert.add_skim('BtoD0h_hh', BtoD0h_hh_List, path=btocharmpath)

# B+ to anti-D0(->Kshh)h+ Skim
from skim.btocharm import BsigToD0hToKshhList
from skim.standardlists.charm import loadD0_Kshh_loose
loadD0_Kshh_loose(path=btocharmpath)
BtoD0h_Kshh_List = BsigToD0hToKshhList(path=btocharmpath)
expert.add_skim('BtoD0h_Kshh', BtoD0h_Kshh_List, path=btocharmpath)

# B+ to anti-D0(->Kspi0)h+ Skim
from skim.btocharm import BsigToD0hToKspi0List
from skim.standardlists.charm import loadD0_Kspi0_loose
loadD0_Kspi0_loose(path=btocharmpath)
BtoD0h_Kspi0_List = BsigToD0hToKspi0List(path=btocharmpath)
expert.add_skim('BtoD0h_Kspi0', BtoD0h_Kspi0_List, path=btocharmpath)
# B+ to anti-D0(->Kspipipi0)h+ Skim
from skim.btocharm import BsigToD0hToKspipipi0List
from skim.standardlists.charm import loadD0_Kspipipi0
loadD0_Kspipipi0(path=btocharmpath)
BtoD0h_Kspipipi0_List = BsigToD0hToKspipipi0List(path=btocharmpath)
expert.add_skim('BtoD0h_Kspipipi0', BtoD0h_Kspipipi0_List, path=btocharmpath)


# B0 -> D- (-> Kspi) pi+
from skim.btocharm import B0toDpi_KspiList
from skim.standardlists.charm import loadStdDplus_Kspi
loadStdDplus_Kspi(path=btocharmpath)
B0toDpi_Kspi_list = B0toDpi_KspiList(path=btocharmpath)
expert.add_skim('B0toDpi_Kspi', B0toDpi_Kspi_list, path=btocharmpath)

# B0 -> D- (-> Kspi) rho+
from skim.btocharm import B0toDrho_KspiList
B0toDrho_Kspi_list = B0toDrho_KspiList(path=btocharmpath)
expert.add_skim('B0toDrho_Kspi', B0toDrho_Kspi_list, path=btocharmpath)


# B0 to D-(K+ pi- pi-)pi+ Skim
from skim.btocharm import loadB0toDpi_Kpipi
from skim.standardlists.charm import loadStdDplus_Kpipi
loadStdDplus_Kpipi(path=btocharmpath)
B0toDpiList = loadB0toDpi_Kpipi(path=btocharmpath)
expert.add_skim('B0toDpi_Kpipi', B0toDpiList, path=btocharmpath)

# B0 -> D- (-> Kpipi) rho+
from skim.btocharm import B0toDrho_KpipiList
B0toDrho_Kpipi_list = B0toDrho_KpipiList(path=btocharmpath)
expert.add_skim('B0toDrho_Kpipi', B0toDrho_Kpipi_list, path=btocharmpath)


expert.setSkimLogging(path=btocharmpath)
b2.process(path=btocharmpath)

# print out the summary
print(b2.statistics)
