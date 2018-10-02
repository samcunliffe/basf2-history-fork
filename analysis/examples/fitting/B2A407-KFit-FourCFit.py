#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform four momentum constraint
# fit with the KFit. In this example the following decay chain:
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
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import fourCKFit
from modularAnalysis import matchMCTruth
from modularAnalysis import variablesToNtuple
from stdPhotons import stdPhotons
import variableCollections as vc


# load input ROOT file
filelistSIG = '/group/belle2/tutorial/release_01-00-00/mdst-Y4setaY.root'

inputMdst('default', filelistSIG)

# Creates a list of good pions and kaons with some PID and IP cut
stdPhotons('loose')
fillParticleList('mu+:pid', 'muonID>0.1')


# Reconstructs eta -> gamma gamma
reconstructDecay("eta:gg -> gamma:loose gamma:loose", "")
# Reconstructs Upsilon -> u+ u-
reconstructDecay("Upsilon:uu -> mu+:pid mu-:pid", "M>2.")

# Reconstructs Upsilon(4S) -> Upsilon eta
reconstructDecay("Upsilon(4S) -> eta:gg Upsilon:uu", "")

# Perform four momentum constraint fit using KFit
# keep candidates only passing C.L. value of the fit prob > 0.0 (no cut)
fourCKFit("Upsilon(4S)", 0.0)

# Perform four momentum constraint fit using KFit and update the Daughters
# keep candidates only passing C.L. value of the fit prob > 0.0 (no cut)
# fourCKFitDaughtersUpdate("Upsilon(4S)", 0.0)

# Associates the MC truth to the reconstructed D0
matchMCTruth('Upsilon(4S)')

# Select variables that we want to store to ntuple
muvars = vc.mc_truth + vc.pid + vc.kinematics
gvars = vc.kinematics + vc.mc_truth
etaanduvars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_hierarchy
u4svars = vc.event_meta_data + vc.inv_mass + vc.kinematics + \
    vc.mc_truth + vc.mc_hierarchy + \
    vc.create_aliases(['FourCFitProb', 'FourCFitChi2'], 'extraInfo(variable)', "") + \
    vc.create_aliases_for_selected(etaanduvars, 'Upsilon(4S) -> ^eta ^Upsilon') + \
    vc.create_aliases_for_selected(muvars, 'Upsilon(4S) -> eta [Upsilon -> ^mu+ ^mu-]') + \
    vc.create_aliases_for_selected(gvars, 'Upsilon(4S) -> [eta -> ^gamma ^gamma] Upsilon')

# Saving variables to ntuple
output_file = 'B2A407-KFit-FourCFit.root'
variablesToNtuple('Upsilon(4S)', u4svars,
                  filename=output_file, treename='Upsilon4s')

#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
