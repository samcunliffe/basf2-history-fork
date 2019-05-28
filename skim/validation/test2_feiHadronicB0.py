#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../feiHadronicBplus.udst.root</input>
    <output>feiHadronicB0_Validation.root</output>
    <contact>sophie.hollitt@adelaide.edu.au, philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../feiHadronicBplus.udst.root', path=path)

variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
variables.addAlias('log10_sigProb', 'log10(sigProb)')
variables.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
variables.addAlias('d0_M', 'daughter(0,M)')
variables.addAlias('decayModeID', 'extraInfo(decayModeID)')

variablesToHistogram(
    filename='feiHadronicB0_Validation.root',
    decayString='B0:generic',
    variables=[
        ('sigProb', 100, 0.0, 1.0),
        ('d0_massDiff', 100, 0.0, 0.5),
        ('d0_M', 100, 0.0, 3.0),
        ('deltaE', 100, -0.2, 0.2),
        ('Mbc', 100, 5.2, 5.3)],
    variables_2d=[('deltaE', 100, -0.2, 0.2, 'Mbc', 100, 5.2, 5.3),
                  ('decayModeID', 25, 0, 25, 'log10_sigProb', 100, -3.0, 0.0)],
    path=path)

process(path)
print(statistics)
