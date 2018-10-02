#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform mass fit with
# the KFit. In this example the following decay chain:
#
# B0 -> D0 pi0
#       |
#       +-> pi0 pi0
#
# is reconstructed. The mass fits is performed on D0
# candidates (as well as pi0 candidates by default).
#
# Note: This example is build upon
# B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.py
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import massKFit
from modularAnalysis import variablesToNtuple
from stdPi0s import stdPi0s
import variableCollections as vc

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = ['/group/belle2/tutorial/release_01-00-00/mdst-B0D0pi0.root']

inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi0:looseFit" ParticleList
stdPi0s('looseFit')

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
reconstructDecay('D0:pi0pi0 -> pi0:looseFit pi0:looseFit', '1.7 < M < 2.0')

# perform mass fit using KFit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
massKFit('D0:pi0pi0', 0.0)

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
reconstructDecay('B0:all -> D0:pi0pi0 pi0:looseFit', '5.24 < Mbc < 5.29 and abs(deltaE) < 1.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0:all')

# Select variables that we want to store to ntuple
B0_vars = vc.event_meta_data + vc.inv_mass + vc.mc_truth + \
    vc.create_aliases_for_selected(
        vc.inv_mass + vc.mc_truth,
        'B0 -> ^D0 ^pi0') + \
    vc.create_aliases_for_selected(
        vc.cluster, 'B0 -> D0 [pi0 -> ^gamma ^gamma]')

pi0_vars = vc.mc_truth + vc.kinematics + vc.mass_before_fit + vc.event_meta_data + \
    ['extraInfo(BDT)', 'decayAngle(0)'] + \
    vc.create_aliases_for_selected(
        vc.cluster + vc.kinematics, 'pi0 -> ^gamma ^gamma')


# Saving variables to ntuple
output_file = 'B2A401-KFit-MassFit.root'
variablesToNtuple('B0:all', B0_vars,
                  filename=output_file, treename='b0')
variablesToNtuple('pi0:looseFit', pi0_vars,
                  filename=output_file, treename='pi0')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
