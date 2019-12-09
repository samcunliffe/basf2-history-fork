#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import modularAnalysis as ma


def loadStdD0(path):
    """
    Create a list of 'D0:all' list, with an invariant mass cut :math:`1.82 < M < 1.92 GeV`, from the following particles lists:

      - 'K-:loose pi+:loose'
      - 'K-:loose pi+:loose pi+:loose pi-:loose'
      - 'K-:loose pi+:loose pi0:loose'
      - 'K_S0:merged pi0:loose'
      - 'K_S0:merged pi+:loose pi-:loose'

    @param path     modules are added to this path
    """
    Dcuts = '1.82 < M < 1.92'
    D_Channels = ['K-:loose pi+:loose',
                  'K-:loose pi+:loose pi+:loose pi-:loose',
                  'K-:loose pi+:loose pi0:loose',
                  'K_S0:merged pi0:loose',
                  'K_S0:merged pi+:loose pi-:loose',
                  ]

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay('D0:std' + str(chID) + ' -> ' + channel, Dcuts, chID, path=path)
        DList.append('D0:std' + str(chID))
    ma.copyLists('D0:all', DList, path=path)
    return DList


def loadStdDplus(path):
    """
    Create a list of 'D+:all' list, with an invariant mass cut :math:`1.82 < M < 1.92 GeV`, from the following particles lists:

      - 'K-:loose pi+:loose pi+:loose'
      - 'K_S0:merged pi+:loose'
      - 'K_S0:merged pi+:loose pi0:loose'
      - 'K_S0:merged pi+:loose pi+:loose pi-:loose'

    @param path     modules are added to this path
    """
    Dcuts = '1.82 < M < 1.92'
    D_Channels = ['K-:loose pi+:loose pi+:loose',
                  'K_S0:merged pi+:loose',
                  'K_S0:merged pi+:loose pi0:loose',
                  'K_S0:merged pi+:loose pi+:loose pi-:loose',
                  ]

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay('D+:std' + str(chID) + ' -> ' + channel, Dcuts, chID, path=path)
        DList.append('D+:std' + str(chID))
    ma.copyLists('D+:all', DList, path=path)
    return DList


def loadStdDstar0(path):
    """
    Create a list of 'D*0:all' list, with an invariant mass cut :math:`1.95 < M < 2.05 GeV`, from the following particles lists:

      - 'D0:all pi0:loose'
      - 'D0:all gamma:loose'

    @param path     modules are added to this path
    """
    Dcuts = '1.95 < M < 2.05'
    D_Channels = ['D0:all pi0:loose',
                  'D0:all gamma:loose']

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay('D*0:std' + str(chID) + ' -> ' + channel, Dcuts, chID, path=path)
        DList.append('D*0:std' + str(chID))
    ma.copyLists('D*0:all', DList, path=path)
    return DList


def loadStdDstarPlus(path):
    """
    Create a list of 'D*++:all' list, with an invariant mass cut :math:`1.95 < M < 2.05 GeV`, from the following particles lists:
      - 'D0:all pi+:all'
      - 'D+:all pi0:loose'

    @param path     modules are added to this path
    """
    Dcuts = '1.95 < M < 2.05'
    D_Channels = ['D0:all pi+:all',
                  'D+:all pi0:loose']

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay('D*+:std' + str(chID) + ' -> ' + channel, Dcuts, chID, path=path)
        DList.append('D*+:std' + str(chID))
    ma.copyLists('D*+:all', DList, path=path)
    return DList
