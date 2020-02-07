#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Example steering file - 2016 Belle II Collaboration
#
# Modifications for release-00-08: Torben Ferber (ferber@physics.ubc.ca)
#
########################################################

import os
import sys
import subprocess
import basf2 as b2
from reconstruction import add_ecl_modules

print('\033[1m')  # makes console text bold
print('This steering file is an example of how to use the ECLBackgroundModule.')
print('')
print('There are generally 8 different data sets for each background campaign:')
print('       RBB HER')
print('       RBB LER')
print('       Coulomb HER')
print('       Coulomb LER')
print('       Touschek HER')
print('       Touschek LER')
print('       BHWide HER')
print('       BHWide LER')
print('')
print('Each sample usually consists of 1000 files, each representing 1us of time, for a total sample time of 1000us or 1ms.')
print("In this example, we will use a subset of 100 files (representing 100us) from the 12th Campaign's RBB HER sample.")
print('Samples are generated by Hiro Nakayama-san. Information on them can be found here:')
print('https://confluence.desy.de/display/BI/Background+WebHome')
print('')

# use a subset (100 files) of the 12th Campaign RBB HER sample
inputs = '~nakayama/basf2_opt/release_201506_12th/Work_MCgen/output/output_RBB_HER_study_1??.root'

# Uncomment if you want the whole dataset. RBB and can be replaced with Coulomb and Touschek, and HER with LER
# inputs = "~nakayama/basf2_opt/release_201506_12th/Work_MCgen/output/output_RBB_HER_study_*.root"

# the length of time in us each sample file corrosponds to
timePerFile = 1

# set the sample time based on the number of files that will be opened
sampletime = timePerFile * int(subprocess.check_output('ls ' + inputs + ' | wc -l',
                                                       shell=True))
print('The sampletime is ' + str(sampletime) + 'us')

# You may want to change this to something more descriptive, eg RBB_HER_100us.root
outfile = 'EclBackgroundExample.root'

print('The output will written to ' + outfile)
print('\033[0m')  # turns off bold text

# The background module can produce some ARICH plots for shielding studies. To do this, set this to True
ARICH = False

# Register necessary modules
main = b2.create_path()

# RootInput takes the Monte Carlo events and fills the datastore, so they can be accessed by other modules
simpleinput = b2.register_module('RootInput')
simpleinput.param('inputFileNames', inputs)
main.add_module(simpleinput)

gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# If you want the ARICH plots, you need to load the geometry.
if ARICH:
    geometry = b2.register_module('Geometry')
    geometry.logging.log_level = b2.LogLevel.WARNING
    main.add_module(geometry)

# The ecl background module is a HistoModule. Any histogram registered in it will be written to file by the HistoManager module
histo = b2.register_module('HistoManager')  # Histogram Manager
histo.param('histoFileName', outfile)
main.add_module(histo)

# ecl Background study module
eclBg = b2.register_module('ECLBackground')
eclBg.param('sampleTime', sampletime)
eclBg.param('doARICH', ARICH)
eclBg.param('crystalsOfInterest', [318, 625, 107])  # If you want the dose for specific crystals, put the cell ID here

# Digitization
eclDigi = b2.register_module('ECLDigitizer')
main.add_module(eclDigi)

# ECL reconstruction
add_ecl_modules

# Background
main.add_module(eclBg)

# run it
b2.process(main)

print(b2.statistics)
