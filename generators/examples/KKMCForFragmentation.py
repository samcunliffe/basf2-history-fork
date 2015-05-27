#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# Run PYTHIA on unfragmented final states
#
# Example steering file
########################################################

from basf2 import *
from ROOT import Belle2

# suppress messages and warnings during processing:
set_log_level(LogLevel.WARNING)

# event info setter
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# use KKMC generated uubar pair as input
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/uubar_nohadronization.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'testoutput.txt')

# get the gearbox
gearbox = register_module('Gearbox')

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

# add root output module
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', './kkmc_uubar.root')

# main
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(kkgeninput)
main.add_module(rootoutput)
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)
process(main)
