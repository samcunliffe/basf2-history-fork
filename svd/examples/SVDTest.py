#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run SVD part of Belle2 simulation.
# It uses ParicleGun module to generate tracks,
# (see "generators/example/ParticleGunFull.py" for detailed usage)
# builds PXD and SVD geometry, performs geant4 and SVD simulation,
# and stores output in a root file.
#
##############################################################################

from basf2 import *

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Register modules

# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# SVD digitization module
SVDDIGI = register_module('SVDDigitizer')
# SVD clusterizer
SVDCLUST = register_module('SVDClustering')
# Save output of simulation
output = register_module('SimpleOutput')

# ============================================================================
# Set a fixed random seed for particle generation:
set_random_seed(1028307)

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)

# ============================================================================
# Print the parameters of the particle gun
print_params(particlegun)

# Set the number of events to be processed (100 events)
evtmetagen.param({'EvtNumList': [100], 'RunList': [1]})

# Set output filename
output.param('outputFileName', 'SVDTestOutput.root')

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

# SVDDIGI.param('statisticsFilename', 'SVDDiags.root')
SVDDIGI.param('PoissonSmearing', True)
SVDDIGI.param('ElectronicEffects', True)
SVDDIGI.param('statisticsFilename', 'SVDTestDiags.root')
SVDDIGI.param('storeWaveforms', True)

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(SVDDIGI)
main.add_module(SVDCLUST)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
#
