#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
# This tutorial demonstrates how to perform four momentum constraint
# fit with the OrcaKinFit. In this example the following decay chain:
#
# Upsilon(4S) -> eta               Upsilon
#                 |                  |
#                 +->gamma gamma     +-> u+ u-
#
# is reconstructed. The four momentum constraint fit is performed on
# all final states, and the total four momentum is set at that of cms
#
# Contributors: Yu Hu (March 2017)
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
from beamparameters import add_beamparameters
import variableCollections as vc
from stdPhotons import *

beamparameters = add_beamparameters(analysis_main, "Y4S")

# load input ROOT file
inputMdst('MC9', '/gpfs/group/belle2/tutorial/orcakinfit/Y4SEventGeneration-gsim-BKGx0_eta_100.root')

# Creates a list of good photon and mu
stdPhotons('loose')
fillParticleList('mu+:pid', 'chiProb > 0.001 and p > 1.0')

# Reconstructs eta -> gamma gamma
reconstructDecay("eta:gg -> gamma:loose gamma:loose", "")
# Reconstructs Upsilon -> u+ u-
reconstructDecay("Upsilon:uu -> mu+:pid mu-:pid", "M>2.")

# Reconstructs Upsilon(4S) -> Upsilon eta
reconstructDecay("Upsilon(4S) -> eta:gg Upsilon:uu", "")
reconstructDecay("Upsilon(4S):4c -> eta:gg Upsilon:uu", "")


# Perform four momentum constraint fit using OrcaKinFit
fitKinematic4C("Upsilon(4S):4c")

# Associates the MC truth to the reconstructed Upsilon(4S)
matchMCTruth('Upsilon(4S)')
matchMCTruth('Upsilon(4S):4c')


# Select variables that we want to store to ntuple
muvars = vc.mc_truth + vc.pid + vc.kinematics
gvars = vc.kinematics + vc.mc_truth + vc.inv_mass
etaanduvars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_hierarchy
u4svars = vc.event_meta_data + vc.inv_mass + vc.kinematics + \
    vc.mc_truth + vc.mc_hierarchy + \
    vc.create_aliases(['FourCFitProb', 'FourCFitChi2'], 'extraInfo(variable)', "") + \
    vc.create_aliases_for_selected(etaanduvars, 'Upsilon(4S) -> ^eta ^Upsilon') + \
    vc.create_aliases_for_selected(muvars, 'Upsilon(4S) -> eta [Upsilon -> ^mu+ ^mu-]') + \
    vc.create_aliases_for_selected(gvars, 'Upsilon(4S) -> [eta -> ^gamma ^gamma] Upsilon')

u4svars_4c = u4svars + vc.create_aliases(['OrcaKinFitProb',
                                          'OrcaKinFitChi2',
                                          'OrcaKinFitErrorCode'], 'extraInfo(variable)', "")

u4svars_def = u4svars + vc.create_aliases(['chiProb'], 'extraInfo(variable)', "")


# Saving variables to ntuple
output_file = 'B2A423-Orcakinfit_4CFit.root'
variablesToNtuple('Upsilon(4S)', u4svars_def,
                  filename=output_file, treename='Upsilon4s')
variablesToNtuple('Upsilon(4S):4c', u4svars_4c,
                  filename=output_file, treename='Upsilon4s_4c')

#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
