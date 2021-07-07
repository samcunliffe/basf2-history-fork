#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This steering file is an example of how to use 'ECLBackground' module.
There are generally 8 different data sets for each background campaign:
    - RBB HER
    - RBB LER
    - Coulomb HER
    - Coulomb LER
    - Touschek HER
    - Touschek LER
    - BHWide HER
    - BHWide LER

Each sample usually consists of 1000 files, each representing 1 micro second
of time, for a total sample time of 1000 micro second (1 ms).

In this example, we will use a subset of 100 files (representing 100 micro second)
from the 12th Campaign's RBB HER sample.

Samples are generated by Hiro Nakayama-san. Information on them can be
found here:
    https://confluence.desy.de/display/BI/Background+WebHome
"""

import subprocess
import basf2 as b2
from reconstruction import add_ecl_modules

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

"""Use a subset (100 files) of the 12th Campaign RBB HER sample

If you want the whole dataset, uncomment the line after.
RBB can be replaced with Coulomb and Touschek, and HER with LER.
"""
inputs = '~nakayama/basf2_opt/release_201506_12th/Work_MCgen/output/output_RBB_HER_study_1??.root'
# inputs = '~nakayama/basf2_opt/release_201506_12th/Work_MCgen/output/output_RBB_HER_study_*.root'

# Length of time in us (micro second) each sample file corrosponds to
timePerFile = 1

# Set the sample time based on the number of files that will be opened
sampletime = timePerFile * int(subprocess.check_output('ls ' + inputs + ' | wc -l',
                                                       shell=True))
print('The sampletime is ' + str(sampletime) + 'micro second')

"""You may want to change this to something more descriptive,
   e.g. 'RBB_HER_100us.root'
"""
outputFile = 'EclBackgroundExample.root'

print('The output will written to ' + outputFile)

"""The background module can produce some ARICH plots for
   shielding studies. To do this, set ARICH to 'True'.
"""
ARICH = False

# Register and add 'RootInput' module
inputFile = b2.register_module('RootInput')
inputFile.param('inputFileNames', inputs)
mainPath.add_module(inputFile)

# Register and add 'Gearbox' module
gearbox = b2.register_module('Gearbox')
mainPath.add_module(gearbox)

# If you want the ARICH plots, you need to load 'Geometry' module.
if ARICH:
    # Register and add 'Geometry' module
    geometry = b2.register_module('Geometry')
    geometry.logging.log_level = b2.LogLevel.WARNING
    mainPath.add_module(geometry)

"""The ECL background module is a HistoModule. Any histogram registered
   in it will be written to file by the HistoManager module.
"""
histoManager = b2.register_module('HistoManager')
histoManager.param('histoFileName', outputFile)
mainPath.add_module(histoManager)

"""Register and add 'ECLBackground' module

ECLBackground module:
    Processes background campaigns and produces histograms.
    This module requires HistoManager module.
"""
eclBackground = b2.register_module('ECLBackground')
eclBackground.param('sampleTime', sampletime)
eclBackground.param('doARICH', ARICH)
# If you want the dose for specific crystals, put the cell ID here.
eclBackground.param('crystalsOfInterest', [318, 625, 107])
mainPath.add_module(eclBackground)

# Register and add 'ECLDigitizer' module
eclDigitizer = b2.register_module('ECLDigitizer')
mainPath.add_module(eclDigitizer)

# Add the ECL reconstruction modules to the path
add_ecl_modules(mainPath)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
