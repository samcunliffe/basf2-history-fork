#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This steering file demonstrates the use of the new SVD reconstruction path.
# Currently it ends by creating SVDRecoDigits, clustering is not yet in place.
#
##############################################################################

from basf2 import *
from dump_digits import dump_digits
from dump_clusters import dump_clusters

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Register modules

# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# Add the PXD digitizer, too, to avoid problems with empty events.
PXDDIGI = register_module('PXDDigitizer')
# SVD digitization module
SVDDIGI = register_module('SVDDigitizer')
SVDDIGI.param('StartSampling', -31.44)
SVDDIGI.param('GenerateShaperDigits', True)
# SVDDIGI.param('signalsList', 'SVDSignalsList.csv')
SVDDIGI.param('RandomizeEventTimes', True)
SVDDIGI.param('TimeFrameLow', -180)
SVDDIGI.param('TimeFrameHigh', 150)
# SVD signal reconstructor
SVDSIGR = register_module('SVDNNShapeReconstructor')
# Write RecoDigits so that we can check them
SVDSIGR.param('WriteRecoDigits', True)
# Direct clusterizer
SVDCLUST1 = register_module('SVDClusterizerDirect')
SVDCLUST1.param('Clusters', 'SVDClustersDirect')
# RecoDigit clusterizer
SVDCLUST2 = register_module('SVDNNClusterizer')
# Save output of simulation
output = register_module('RootOutput')

# ============================================================================
# Set a fixed random seed for particle generation:
set_random_seed(1028307)

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)

# Set the number of events to be processed (100 events)
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})

# Set output filename
output.param('outputFileName', 'SVDTestOutput.root')

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDIGI)
main.add_module(SVDDIGI)
main.add_module(SVDSIGR)
main.add_module(SVDCLUST1)
main.add_module(dump_clusters('direct_clusters_dump.txt', 'SVDClustersDirect'))
main.add_module(SVDCLUST2)
main.add_module(dump_clusters('clusters_dump.txt', 'SVDClusters'))
main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
