#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file gives an example for the use of the
# HepEvtReader as a master module. The master module sets
# event, run and experiment number for the event in process.
# There can only be one master module. In most cases with
# simulated events this is the EvtMetaGen module. But, if
# you want to use the event numbers given in the HepEvt file
# you should use the HepEvtReader as a master module.
#
# Example steering file - 2011 Belle II Collaboration
########################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(3)

# to run the framework the used modules need to be registered
hepevtreader = register_module('HepevtInput')

# setting the options for the HEPEVT reader:
# (Please note, that not all options are shown in this file.
# More explanations for the options of the HepEvt reader
# can be found in HepEvtReaderFull.py).

# Making the HEPEVT reader the MASTER:
hepevtreader.param('makeMaster', True)

# now the HEPEVT reader needs to be given a run and
# experiment number.
hepevtreader.param('runNum', 3)
hepevtreader.param('expNum', 5)
# The run and experiment number are used to set the correct
# calibration and alignment parameters.
# The default values are runNum = 0 and expNum = 0

# indicate the filename where the hepevtreader
# should read events from
hepevtreader.param('inputFileName', 'BhWide_10events.txt')

# if the events in the HepEvt file are weighted and
# you want to use the event weights use this line
hepevtreader.param('useWeights', True)
# default is useWeight = False, all events have the same weight

# Specific options for the processing of files from the BhWide generator:
hepevtreader.param('nVirtualParticles', 2)
hepevtreader.param('boost2LAB', True)
hepevtreader.param('wrongSignPz', True)

# for a simple simulation job with output to a root file
# these additional modules are needed
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
g4sim = register_module('FullSim')
simpleoutput = register_module('SimpleOutput')

# Setting the option for all non-hepevt reader modules:
paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

simpleoutput.param('outputFileName', 'HepEvtReaderOutput.root')

# creating the path for the processing
main = create_path()

# Add hepevtreader module to path:
main.add_module(hepevtreader)
# and print parameters for hepevtreader
# on startup of process
print_params(hepevtreader)

# Add all other modules for simple processing to path
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)
main.add_module(simpleoutput)

# Process 100 events
process(main, 100)
# if there are less events in the input file
# the processing will be stopped at EOF.
# if all events in the file should be process
# don't put a number here.
