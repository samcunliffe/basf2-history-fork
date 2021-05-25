#!/usr/bin/env python3

# This test catches a regression in PyRoot (in root 5.34/19+?, fixed in 5.34/23)
from basf2 import find_file
import ROOT

path = find_file('framework/tests/chaintest_2.root')
f = ROOT.TFile(path)
tree = f.Get('tree')

tree.GetEntry(0)
hits = tree.PXDSimHits
nentries = hits.GetEntriesFast()
print(nentries)
hit = hits[0]
hit.Dump()
