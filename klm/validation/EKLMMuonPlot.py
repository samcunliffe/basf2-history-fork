#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>EKLMMuonOutput.root</input>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Creation of EKLM muon validation plots.</description>
</header>
"""

import basf2 as b2
import ROOT
from ROOT import Belle2
from ROOT import TFile, TTree, TH1F, TNamed

ROOT.gROOT.SetBatch(True)
input_file = ROOT.TFile('../EKLMMuonOutput.root')
tree = input_file.Get('tree')
output_file = ROOT.TFile('EKLMMuon.root', 'recreate')
contact = 'Kirill Chilikin (chilikin@lebedev.ru)'

h2dtres = ROOT.TH1F('muon_h2dtres', 'EKLM muon 2d hits time resolution',
                    100, -10, 10)
h2dtres.SetXTitle('ns')
h2dtres.SetYTitle('Events')
tree.Draw('EKLMHit2ds.m_Time-EKLMHit2ds.m_MCTime>>muon_h2dtres')
functions = h2dtres.GetListOfFunctions()
functions.Add(TNamed('Description', 'Time resolution'))
functions.Add(TNamed('Check', 'No bias.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

output_file.cd()
h2dtres.Write()
output_file.Close()
