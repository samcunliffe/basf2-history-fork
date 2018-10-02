#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
# This tutorial demonstrates how to perform vertexfit with RaveFit
# and four momentum constraint fit with the OrcaKinFit. In this
# example the following decay chain:
#
# Upsilon(4S) -> gamma    A
#                         |
#                         +-> u+ u-
#
# is reconstructed. The  vertexfit is performed on u+ u-, and four
# momentum constraint fit is performed on all final states, and the
# total four momentum is set at that of cms.
#
# Contributors: Yu Hu (July 2018)
# yu.hu@desy.de
#
####################################################################

#
# Import and mdst loading
#
from basf2 import *
from modularAnalysis import add_beamparameters
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import fitKinematic4C
from modularAnalysis import variablesToNtuple
import sys
import pdg
from beamparameters import add_beamparameters
import variableCollections as vc
from stdPhotons import *

# load input ROOT file
inputMdst('default', '/gpfs/group/belle2/tutorial/orcakinfit/mdst_1.root')


# Creates a list of good photon and muons
fillParticleList("gamma:sel", 'E > 0.1 and abs(formula(clusterTiming/clusterErrorTiming)) < 1.0')
fillParticleList("mu-:sel", 'electronID < 0.01 and chiProb > 0.001 and abs(dz) < 3 and dr < 0.1')
pdg.add_particle('A', 9000008, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("A:sel -> mu-:sel mu+:sel", "")
reconstructDecay("A:selvertex -> mu-:sel mu+:sel", "")

# Perform four momentum constraint fit using RaveFit and update the Daughters
vertexRaveDaughtersUpdate("A:selvertex", -1.0, constraint="iptube")

pdg.add_particle('beam', 9000009, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("beam:sel -> A:sel gamma:sel", "")
reconstructDecay("beam:selv -> A:selvertex gamma:sel", "")
reconstructDecay("beam:selv4c -> A:selvertex gamma:sel", "")

# Perform four momentum constraint fit using OrcaKinFit and update the Daughters
fitKinematic4C("beam:selv4c")

# Associates the MC truth
matchMCTruth('beam:sel')
matchMCTruth('beam:selv')
matchMCTruth('beam:selv4c')

# Select variables that we want to store to ntuple
muvars = vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.pid + vc.momentum_uncertainty
gammavars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + vc.mc_hierarchy + vc.momentum_uncertainty
avars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + vc.mc_hierarchy + vc.momentum_uncertainty
uvars = vc.event_meta_data + vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + vc.mc_hierarchy + \
    vc.create_aliases_for_selected(muvars, 'beam -> [A -> ^mu+ ^mu-] gamma') + \
    vc.create_aliases_for_selected(gammavars, 'beam -> A ^gamma') + \
    vc.create_aliases_for_selected(avars, 'beam -> ^A gamma')

uvarsv = uvars + ['chiProb']

uvars4c = uvars + vc.create_aliases(['OrcaKinFitProb',
                                     'OrcaKinFitProb',
                                     'OrcaKinFitChi2',
                                     'OrcaKinFitErrorCode'], 'extraInfo(variable)', "") + \
    vc.create_aliases(['VertexFitChi2',
                       'VertexFitProb'], 'daughter(1,extraInfo(variable))', "A")

# Saving variables to ntuple
output_file = 'B2A424-OrcaKinFit_vc.vertexfit_4Cfit.root'
variablesToNtuple('beam:selv4c', uvars4c,
                  filename=output_file, treename='beamselv4c')
variablesToNtuple('beam:selv', uvarsv,
                  filename=output_file, treename='beamselv')
variablesToNtuple('beam:sel', uvarsv,
                  filename=output_file, treename='beamsel')

#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
