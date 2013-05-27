#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################## This steering file
# shows all options for the generation of four fermion final state events.
#
# 100 four fermion final state QED events are generated using the KoralW
# Fortran generator and some plots are shown at the end.
#
# Example steering file - 2011 Belle II Collaboration
########################################################

import sys
import math
from basf2 import *

## Set the global log level
logging.log_level = LogLevel.WARNING

## Load the required libraries
import ROOT
ROOT.gSystem.Load('libframework')
ROOT.gSystem.Load('libgenerators_koralw')
ROOT.gSystem.Load('libgenerators')
from ROOT import Belle2

## Create some histograms to be filled
h_nTracks = ROOT.TH1D('nTracks', 'Number of Tracks per Event;#', 20, 0, 20)
h_pdg = ROOT.TH1D('pid', 'Particle code of particles', 100, -50, 50)
h_momentum = ROOT.TH1D('momentum', 'Momentum of particles', 200, 0, 8)
h_pt = ROOT.TH1D('pt', 'Transverse Momentum of particles', 200, 0, 2)
h_phi = ROOT.TH1D('phi', 'Azimuth angle of particles', 200, -180, 180)
h_theta = ROOT.TH1D('theta', 'Polar angle of particles', 200, 0, 180)
h_costheta = ROOT.TH1D('costheta', 'Cosinus of the polar angle of particles',
                       200, -1, 1)
h_vertex = ROOT.TH2D(
    'xyvertex',
    'XY Vertex of particles',
    200,
    -10,
    10,
    200,
    -10,
    10,
    )


class ShowMCParticles(Module):

    """Simple module to collect some information about MCParticles"""

    def __init__(self):
        """Initialize the module"""

        super(ShowMCParticles, self).__init__()

    def event(self):
        """Fill the histograms with the values of the MCParticle collection"""

        mcParticles = Belle2.PyStoreArray('MCParticles')
        nTracks = mcParticles.getEntries()
        h_nTracks.Fill(nTracks)
        for i in range(nTracks):
            mc = mcParticles[i]
            if mc.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                p = mc.getMomentum()
                h_momentum.Fill(p.Mag())
                h_pt.Fill(p.Perp())
                h_phi.Fill(p.Phi() / math.pi * 180)
                h_theta.Fill(p.Theta() / math.pi * 180)
                h_costheta.Fill(math.cos(p.Theta()))
                h_pdg.Fill(mc.getPDG())
                h_vertex.Fill(mc.getProductionVertex().X(),
                              mc.getProductionVertex().Y())


evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'evtNumList': [100], 'runList': [1]})

## Register the BHWideInput module
koralw = register_module('KoralWInput')

# Set the mode for the boost of the generated particles 0 = no boost 1 = BELLE
# II 2 = BELLE
koralw.param('BoostMode', 1)

# Set a random seed for the event generation
koralw.param('RandomSeed', 2710)

# Set the logging level for the KoralW module to INFO in order to see the total
# cross section
koralw.set_log_level(LogLevel.INFO)

## Register the Progress module and the Python histogram module
progress = register_module('Progress')
gearbox = register_module('Gearbox')
showMCPart = ShowMCParticles()

## Create the main path and add the modules
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(koralw)
main.add_module(showMCPart)

# generate events
process(main)

# show call statistics
print statistics

# Create a Canvas to show histograms
c = ROOT.TCanvas('Canvas', 'Canvas', 1536, 768)
c.Divide(4, 2, 1e-5, 1e-5)

# Draw all histograms
histograms = [
    h_nTracks,
    h_pdg,
    h_momentum,
    h_pt,
    h_theta,
    h_costheta,
    h_phi,
    ]
for (i, h) in enumerate(histograms):
    c.cd(i + 1)
    h.SetMinimum(0)
    h.Draw()
c.cd(i + 2)
h_vertex.Draw('colz')
c.Update()

# Wait for enter to be pressed
sys.stdin.readline()
