#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file generates MCParticles via the parti-
# cle gun, performs a full Geant4 simulation and saves
# the result in a root file.
# In total 100 events are processed.
#
# The following parameters are used for the particle gun:
#  Tracks per event:      10
#  Particles:             electrons / pions / protons (and anti particles)
#  Theta [default]:       17 to 150 degree
#  Phi [default]:         0 to 360 degree
#  Momentum:              200 MeV to 1 GeV
#
# For more details on the options for the particle gun
# look at the example steering file in the generators
# package.
# For more details on the options for the simulation look
# at the example steering file in the simulation package.
#
# Example steering file - 2011 Belle II Collaboration
########################################################

from basf2 import *

set_log_level(3)

# Register modules that will be called
evtmetagen = register_module('EvtMetaGen')
hepevtreader = register_module('HepevtInput')
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
g4sim = register_module('FullSim')
simpleoutput = register_module('SimpleOutput')

# setting the options for the HEPEVT reader:
# this is the path to an example steering file in the
# generators package. It contains 10 QED background events
hepevtreader.param('inputFileList',
                   ['../../generators/examples/BhWide_10events.txt'])
# You can find and download more Hepevt files from the Twiki page.

# Setting the option for the EvtMetaGenModule reader modules:
evtmetagen.param('EvtNumList', [100, 200])  # we want to process 100 events un the first run, 200 in the second
evtmetagen.param('RunList', [11, 2])  # run number of the first run is 11, for the second run it is 2
evtmetagen.param('ExpList', [1, 1])  # and for both runs the experiment number is 1

# Set parameters for paramloader:
paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

# setting verbosity for simulation (5 very high, 0 low)
# this sets how verbose the Geant4 simulation is
g4sim.param('TrackingVerbosity', 0)

# Set parameters for simpleoutput:
# simpleoutput creates a simple root files where a tree structure gives
# access to all containers in the data store at the moment
# the module is called
simpleoutput.param('outputFileName', 'simpleHepEvtReaderOutput.root')

# the path sets the order of the module execution
# Create paths
main = create_path()
main.add_module(evtmetagen)  # the event meta module
main.add_module(hepevtreader)  # the hepevt reader module
main.add_module(paramloader)  # get the geometry parameters
main.add_module(geobuilder)  # build the geometry
main.add_module(g4sim)  # run the simulation
main.add_module(simpleoutput)  # put the information from datastore to root file

# start processing of 10 events
process(main, 10)

