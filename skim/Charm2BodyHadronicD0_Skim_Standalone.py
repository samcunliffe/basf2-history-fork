
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
from stdCharged import *


set_log_level(LogLevel.INFO)

import sys
from skimExpertFunctions import *
import os
import glob
gb2_setuprel = 'release-01-00-00'

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_00051*_prod00000198_task0000051*.root']


inputMdstList('default', fileList)


loadStdCharged()

from Charm2BodyHadronicD0_List import *

D0ToHpJmList = D0ToHpJm()
skimOutputUdst('Charm2BodyHadronicD0', D0ToHpJmList)

summaryOfLists(D0ToHpJmList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

print(statistics)
