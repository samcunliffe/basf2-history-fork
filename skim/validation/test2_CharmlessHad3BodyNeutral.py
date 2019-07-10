#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../CharmlessHad3BodyNeutral.udst.root</input>
    <output>CharmlessHad3BodyNeutral_Validation.root</output>
    <contact>khsmith@student.unimelb.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *

charmless3neutralpath = Path()

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
inputMdst('default', '../CharmlessHad3BodyNeutral.udst.root', path=charmless3neutralpath)

from variables import variables
variablesToHistogram(
    filename='CharmlessHad3BodyNeutral_Validation.root',
    decayString='B0:3BodySkim',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 0, 1.5),  # K+ invariant mass
        ('daughter(1, InvM)', 100, 0, 1.5),  # K- invariant mass
        ('daughter(2, InvM)', 100, 0.06, 0.18)],  # pi0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=charmless3neutralpath)

process(charmless3neutralpath)
print(statistics)

import ROOT

# add contact information to histograms
variables_list = [
    'Mbc',
    'deltaE',
    'daughter__bo0__cm__spInvM__bc',
    'daughter__bo1__cm__spInvM__bc',
    'daughter__bo2__cm__spInvM__bc',
    'MbcdeltaE'
]

file = ROOT.TFile("CharmlessHad3BodyNeutral_Validation.root", "UPDATE")
for name in variables_list:
    hist = file.Get(name)
    hist.GetListOfFunctions().Add(ROOT.TNamed("Contact", "khsmith@student.unimelb.edu.au"))
    hist.Write("", ROOT.TObject.kOverwrite)
file.Close()
