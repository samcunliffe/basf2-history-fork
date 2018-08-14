#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform Vertex fits
# using Rave and how to use TagV and save Delta t.
# The following  decay chain:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> K- pi+
#
# is reconstructed B0 vertex is fitted using the two m
# the side vertex is fittes and Delta t (in ps.) is
# calculated
#
# Note: This example uses the signal MC sample created in
# MC campaign 5, therefore it can be ran only on KEKCC computers.
#
# Contributors: L. Li Gioi (October 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from vertex import vertexRave
from modularAnalysis import buildRestOfEvent
from modularAnalysis import fillParticleList
from vertex import TagV
from stdCharged import *

# Add signal MC files for release 9.
# 150000 events!
# Consider using -n flag in command line to limit number of events, e.g.:
# > basf2 B2A410-TagVertex.py -n 1000
filelistSIG = \
    ['/group/belle2/tutorial/release_01-00-00/mdst_000001_prod00002439_task00000001.root'
     ]

inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "mu+:loose" ParticleList (and c.c.)
stdLooseMu()

# create Ks -> pi+ pi- list from V0
# keep only candidates with 0.4 < M(pipi) < 0.6 GeV
fillParticleList('K_S0:pipi', '0.4 < M < 0.6')

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with 3.0 < M(mumu) < 3.2 GeV
reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', '3.0 < M < 3.2')

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with 5.2 < M(J/PsiKs) < 5.4 GeV
reconstructDecay('B0:jspiks -> J/psi:mumu K_S0:pipi', '5.2 < M < 5.4')

# perform B0 kinematic vertex fit using only the mu+ mu-
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('B0:jspiks', 0.0, 'B0 -> [J/psi -> ^mu+ ^mu-] K_S0')

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jspiks')

# perform MC matching (MC truth asociation). Always before TagV
matchMCTruth('B0:jspiks')

# calculate the Tag Vertex and Delta t (in ps)
# breco: type of MC association.
TagV('B0:jspiks', 'breco')

# Select variables that we want to store to ntuple
import variableCollections as vc

fshars = vc.pid + vc.track + vc.mc_truth
jpsiandk0svars = vc.inv_mass + vc.vertex + mc_vc.vertex + vc.mc_truth
bvars = vc.event_meta_data + vc.inv_mass + deltae_mbs + vc.ckm_kinematics + \
    vc.vertex + mc_vc.vertex + vc.mc_truth + vc.tag_vertex + mc_vc.tag_vertex + \
    delta_t + mc_delta_t + \
    vc.convert_to_all_selected_vars(fshars, 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
    vc.convert_to_all_selected_vars(jpsiandk0svars, 'B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]')


# Saving variables to ntuple
from modularAnalysis import variablesToNtuple
output_file = 'B2A410-TagVertex.root'
variablesToNtuple('B0:jspiks', bvars,
                  filename=output_file, treename='B0tree')


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
