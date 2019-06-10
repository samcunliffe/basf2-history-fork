#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../SLUntagged.udst.root</input>
    <output>SLUntagged_Validation.root</output>
    <contact>hannah.wakeling@mail.mcgill.ca, philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../SLUntagged.udst.root', path=path)

cutAndCopyLists('B+:all', ['B+:SL0', 'B+:SL1', 'B+:SL2', 'B+:SL3'], '', path=path)

variables.addAlias('decayModeID', 'extraInfo(decayModeID)')
variables.addAlias('d1_p', 'daughter(1,p)')
variables.addAlias('Mmiss2', 'WE_MissM2()')

variablesToHistogram(
    filename='SLUntagged_Validation.root',
    decayString='B+:all',
    variables=[
        ('cosThetaBetweenParticleAndNominalB', 100, -6.0, 4.0),
        ('Mbc', 100, 4.0, 5.3),
        ('d1_p', 100, 0, 5.2)  # Lepton momentum
        ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3)],
    path=path)

process(path)
print(statistics)
