#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform detector simulation
# and reconstruction (track finding+track fitting+ecl reconstruction+...)
# on a previously generated events.
#
# See simulation examples for more details
#
# The processed events are saved to the output ROOT file that
# now contain in addition to the generated particles
# (MCParticle objects stored in the StoreArray<MCParticle>) also
# reconstructed MDST objects (Track/ECLCluster/KLMCluster/...).
# Contributors: A. Zupanc (June 2014) I. Komarov (2018)
#
######################################################
import basf2 as b2
import mdst as mdst
import simulation as si
import reconstruction as re
import modularAnalysis as ma

# We previously generated events but we want to simulate them with the
# currently best values, not whatever whas valid back then. So we have to
# disable reuse of the same globaltag from when the events were generated
b2.conditions.disable_globaltag_replay()

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu_evtgen.root', 'examples', False),
             path=my_path)

# simulation
si.add_simulation(path=my_path)

# reconstruction
re.add_reconstruction(path=my_path)

# dump in MDST format
mdst.add_mdst_output(path=my_path,
                     mc=True,
                     filename='B2pi0D_D2hh_D2hhh_B2munu.root')

# Show progress of processing
progress = b2.register_module('ProgressBar')
my_path.add_module(progress)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
