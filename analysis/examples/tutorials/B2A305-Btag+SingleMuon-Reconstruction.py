#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# B meson decay with missing energy, e.g. :
#
# Y(4S) -> Btag- Bsig+
#                 |
#                 +-> mu+ nu
#
# First the Btag candidate is reconstructed in the
# following decay chain:
#
# Btag- -> D0 pi-
#          |
#          +-> K- pi+
#          +-> K- pi+ pi0
#          +-> K- pi+ pi+ pi-
#          +-> K- K+
#          +-> pi- pi+
#
# The signature of signal B meson decay is single muon,
# therefore by reconstructing the muon one already reconstructs
# the signal B.
#
# Once, Btag and Bsig candidates are reconstructed the
# RestOfEvent is filled for each BtagBsig combination
# with the remaining Tracks, ECLClusters and KLMClusters
# that are not used to reconstruct Btag or Bsig. The
# RestOfEvent object is then used as an input for E_extra,
# MissingMass^2, etc. variables.
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variableCollections as vc
import variableCollectionsTools as vct
from stdPhotons import stdPhotons
import stdCharged as stdc
from stdPi0s import stdPi0s

# check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/B2pi0D_D2hh_D2hhh_B2munu.root'):
    b2.B2FATAL("You need the example data installed. Run `b2install-example-data` in terminal for it.")

# create path
my_path = ma.analysis_main

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/B2pi0D_D2hh_D2hhh_B2munu.root',
             path=my_path)

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdLoosePi(path=my_path)
# creates "K+:loose" ParticleList (and c.c.)
stdc.stdLooseK(path=my_path)
# creates "mu+:loose" ParticleList (and c.c.)
stdc.stdLooseMu(path=my_path)

# creates "pi0:looseFit" ParticleList
stdPi0s(listtype='looseFit',
        path=my_path)

# 1. reconstruct D0 in multiple decay modes
ma.reconstructDecay(decayString='D0:ch1 -> K-:loose pi+:loose',
                    cut='1.8 < M < 1.9',
                    dmID=1,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch2 -> K-:loose pi+:loose pi0:looseFit',
                    cut='1.8 < M < 1.9',
                    dmID=2,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch3 -> K-:loose pi+:loose pi+:loose pi-:loose',
                    cut='1.8 < M < 1.9',
                    dmID=3,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch4 -> K-:loose K+:loose',
                    cut='1.8 < M < 1.9',
                    dmID=4,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch5 -> pi-:loose pi+:loose',
                    cut='1.8 < M < 1.9',
                    dmID=5,
                    path=my_path)

# merge the D0 lists together into one single list
ma.copyLists(outputListName='D0:all',
             inputListNames=['D0:ch1', 'D0:ch2', 'D0:ch3', 'D0:ch4', 'D0:ch5'],
             path=my_path)

# 2. reconstruct Btag+ -> anti-D0 pi+
ma.reconstructDecay(decayString='B+:tag -> anti-D0:all pi+:loose',
                    cut='5.2 < Mbc < 5.29 and abs(deltaE) < 1.0',
                    dmID=1,
                    path=my_path)

ma.matchMCTruth(list_name='B+:tag',
                path=my_path)

# 3. reconstruct Upsilon(4S) -> Btag+ Bsig- -> Btag+ mu-
ma.reconstructDecay(decayString='Upsilon(4S) -> B-:tag mu+:loose',
                    cut="",
                    path=my_path)

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='Upsilon(4S)',
                path=my_path)

# 5. build rest of the event
ma.buildRestOfEvent(list_name='Upsilon(4S)',
                    path=my_path)

# 6. Select variables that we want to store to ntuple
d_vars = vc.mc_truth + vc.kinematics + vc.inv_mass
b_vars = vc.mc_truth + \
    vc.deltae_mbc + \
    vct.convert_to_all_selected_vars(variables_list=d_vars,
                                     decay_string='B- -> ^D0 pi-') + \
    vct.wrap_list(variables_list=['decayModeID'],
                  wrapper='daughter(0,extraInfo(variable))',
                  alias_prefix="D")
mu_vars = vc.mc_truth

u4s_vars = vc.mc_truth + \
    vc.roe_multiplicities + \
    vc.recoil_kinematics + \
    vc.extra_energy + \
    vc.kinematics + \
    vct.convert_to_all_selected_vars(variables_list=b_vars,
                                     decay_string='Upsilon(4S) -> ^B- mu+') + \
    vct.convert_to_all_selected_vars(variables_list=d_vars,
                                     decay_string='Upsilon(4S) -> [B- -> ^D0 pi-] mu+') + \
    vct.convert_to_all_selected_vars(variables_list=mu_vars,
                                     decay_string='Upsilon(4S) -> B- ^mu+')


# 7. Saving variables to ntuple
rootOutputFile = 'B2A305-Btag+SingleMuon-Reconstruction.root'
ma.variablesToNtuple(decayString='B-:tag',
                     variables=b_vars,
                     filename=rootOutputFile,
                     treename='btag',
                     path=my_path)
ma.variablesToNtuple(decayString='Upsilon(4S)',
                     variables=u4s_vars,
                     filename=rootOutputFile,
                     treename='btagbsig',
                     path=my_path)


# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
