#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to perform detector simulation and      #
# reconstruction (track finding+track fitting+ecl reconstruction+...)    #
# on a previously generated events with beam background mixing.          #
#                                                                        #
#                                                                        #
# The processed events are saved to the output ROOT file that            #
# now contain in addition to the generated particles                     #
# (MCParticle objects stored in the StoreArray<MCParticle>) also         #
# reconstructed MDST objects (Track/ECLCluster/KLMCluster/...).          #
#                                                                        #
# ########################################################################

import basf2 as b2
import modularAnalysis as ma
import simulation as si
import reconstruction as re
import os.path


# check if the required input file exists (from B2A101 example)
if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    b2.B2FATAL(
        'Required input file (B2A101-Y4SEventGeneration-evtgen.root) does not exist. \n'
        'Please run B2A101-Y4SEventGeneration.py tutorial script first.')

# create a path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst('default', 'B2A101-Y4SEventGeneration-evtgen.root', path=my_path)

# simulation
si.add_simulation(path=my_path)

# reconstruction
re.add_reconstruction(path=my_path)

# dump in MDST format
re.add_mdst_output(path=my_path,
                   mc=True,
                   filename='B2A101-Y4SEventGeneration-gsim-BKGx0.root')

# Show progress of processing
my_path.add_module('ProgressBar')

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
