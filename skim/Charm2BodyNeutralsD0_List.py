#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 15/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


def D0ToNeutrals():
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2'
    D0_Channels = ['pi0:skim pi0:skim',
                   'K_S0:all pi0:skim',
                   'K_S0:all K_S0:all',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists
