#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> D0 pi0
#       |
#       +-> pi0 pi0
#
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (Demeber 2017)
#
######################################################

import basf2 as b2
import modularAnalysis as ma
import variableCollections as vc
import variableCollectionsTools as vct
from stdPi0s import stdPi0s


# check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/B02D0pi_D02pi0pi0.root'):
    b2.B2FATAL("You need the example data installed. Run `b2install-example-data` in terminal for it.")

# create path
my_path = ma.analysis_main

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/B02D0pi_D02pi0pi0.root',
             path=my_path)


# use standard final state particle lists
#
# creates "pi0:looseFit" ParticleList
# https://confluence.desy.de/display/BI/Physics+StandardParticles
stdPi0s(listtype='looseFit', path=my_path)

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
ma.reconstructDecay(decayString='D0:pi0pi0 -> pi0:looseFit pi0:looseFit',
                    cut='1.7 < M < 2.0',
                    path=my_path)

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
ma.reconstructDecay(decayString='B0:all -> D0:pi0pi0 pi0:looseFit',
                    cut='5.24 < Mbc < 5.29 and abs(deltaE) < 1.0',
                    path=my_path)

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='B0:all',
                path=my_path)


# Select variables that we want to store to ntuple
B0_vars = vc.event_meta_data + \
    vc.inv_mass + \
    vc.mc_truth + \
    vct.convert_to_all_selected_vars(
        variable_list=vc.inv_mass + vc.mc_truth,
        decay_string='B0 -> ^D0 ^pi0') + \
    vct.convert_to_all_selected_vars(
        variable_list=vc.cluster,
        decay_string='B0 -> D0 [pi0 -> ^gamma ^gamma]')

pi0_vars = vc.mc_truth + \
    vc.kinematics + \
    vc.mass_before_fit + \
    vc.event_meta_data + \
    ['extraInfo(BDT)', 'decayAngle(0)'] + \
    vct.convert_to_all_selected_vars(
        variable_list=vc.cluster + vc.kinematics,
        decay_string='pi0 -> ^gamma ^gamma')


# Saving variables to ntuple
output_file = 'B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.root'
ma.variablesToNtuple('B0:all', B0_vars,
                     filename=output_file,
                     treename='b0',
                     path=my_path)
ma.variablesToNtuple('pi0:looseFit', pi0_vars,
                     filename=output_file,
                     treename='pi0',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
