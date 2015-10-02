#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run ARICH part of Belle2 simulation.
# It builds inner detectors geometry (CDC,PXD,SVD,ARICH), uses ParticleGun module
# to shoot particles, performs geant4 simulation, reconstructs tracks and performs
# ARICH reconstruction. Output containing PID likelihood is by default stored in
# 'ARICHEvents.root' file, additionally all objects from the DataStore
# (like detector hits, MCParticles, etc.) can be stored by uncommenting few lines
# near the end of this script
#
# 7/8/2015 Luka Santelj
##############################################################################

from basf2 import *
from optparse import OptionParser
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
import glob

# Options from command line
parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=100,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='ARICHEvents.root')
parser.add_option('-d', '--debug', dest='debugLevel', default=10)
(options, args) = parser.parse_args()
nevents = int(options.nevents)
filename = options.filename
debugLevel = int(options.debugLevel)

# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
set_log_level(LogLevel.INFO)

# Create path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevents], 'runList': [1]})
main.add_module(eventinfosetter)

# Particle gun module
particlegun = register_module('ParticleGun')
# Setting the random seed for particle generation:
set_random_seed(123456)
# Setting the list of particle codes (PDG codes) for the generated particles
# pions and kaons
particlegun.param('pdgCodes', [-211, 211, 321, -321])
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)
# Setting the parameters for the random generation
# of particles momenta:
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.5, 3.5])
# Setting the parameters for the random generation
# of the particle polar angle:
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [17.0, 35.0])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0.0, 0.0])
particlegun.param('yVertexParams', [0.0, 0.0])
particlegun.param('zVertexParams', [0.0, 0.0])
particlegun.param('independentVertices', False)
# Print the parameters of the particle gun
print_params(particlegun)
main.add_module(particlegun)

# Add simulation
components = ['MagneticField', 'CDC', 'PXD', 'SVD', 'ARICH']
# choose second line if you want to add background
add_simulation(main, components)
# add_simulation(main, components, bkgfiles=glob.glob('/sw/belle2/bkg/ARICH*.root'))

# Add reconstruction
components_nr = ['CDC', 'PXD', 'SVD']
add_mc_reconstruction(main, components_nr)

# Add track extrapolation module
ext = register_module('Ext')
main.add_module(ext)

# Add module that creates relation between ExtHits and ARICHAeroHits
arichREL = register_module('ARICHRelate')
main.add_module(arichREL)

# Add ARICH reconstruction module
arichRECO = register_module('ARICHReconstructor')
# track information is obtain from tracking system
# (set to 1 to use MC track information)
arichRECO.param('inputTrackType', 0)
main.add_module(arichRECO)

# Add module fpr ARICH efficiency analysis
arichEfficiency = register_module('ARICHAnalysis')
arichEfficiency.logging.log_level = LogLevel.DEBUG
arichEfficiency.logging.debug_level = debugLevel
arichEfficiency.param('outputFile', filename)
main.add_module(arichEfficiency)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# uncomment for full DataStore outout
# output = register_module('RootOutput')
# output.param('outputFileName', 'FullOutput.root')
# main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
