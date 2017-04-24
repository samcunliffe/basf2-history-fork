#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#####################################################################
# VXDTF2 Example Scripts - Step 0 - Simulation
#
# This script can be used to produce MC data from particle guns or
# Y(4S) events for the training and validation of the VXDTF 1 and 2.
#
# The number of events which will be simulated can be set via the
# basf2 commandline option -n.
# The name of the root output file can be defined with the option -o.
#
# E.g.: 'basf2 eventSimulation.py -n 1000 -o trainingSample.root'
#
# The settings for the particle gun(s) and EvtGen simulation can be
# adapted in this script. Some convenience functions are outsourced
# to setup_modules.py.
#
#
# Contributors: Jonas Wagner, Felix Metzner
#####################################################################


from basf2 import *
from beamparameters import add_beamparameters
from simulation import add_simulation

# If later the use of bg is wanted, you can as well import setup_bg
from setup_modules import setup_sim

# ---------------------------------------------------------------------------------------

# Set Random Seed for reproducable simulation. 0 means really random.
set_random_seed(12345)

# Set log level. Can be overridden with the "-l LEVEL" flag for basf2.
set_log_level(LogLevel.ERROR)

# ---------------------------------------------------------------------------------------
main = create_path()

eventinfosetter = register_module('EventInfoSetter')
main.add_module(eventinfosetter)

eventinfoprinter = register_module('EventInfoPrinter')
main.add_module(eventinfoprinter)

progress = register_module('Progress')
main.add_module(progress)

# ---------------------------------------------------------------------------------------
# Simulation Settings:

# Particle Gun:
# One can add more particle gun modules if wanted.
particlegun = register_module('ParticleGun')
particlegun.logging.log_level = LogLevel.WARNING
param_pGun = {
    'pdgCodes': [13, -13],   # 13 = muon --> negatively charged!
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.1, 4]
}
"""
              'momentumGeneration': 'fixed',
              'momentumParams': [2, 2],           # 2 values: [min, max] in GeV
              'thetaGeneration': 'fixed',
              'thetaParams': [90., 90.],               # 2 values: [min, max] in degree
              'phiGeneration': 'uniform',
              'phiParams': [0., 90.],                  # [min, max] in degree
              'vertexGeneration': 'uniform',
              'xVertexParams': [-0.1, 0.1],            # in cm...
              'yVertexParams': [-0.1, 0.1],
              'zVertexParams': [-0.5, 0.5],
             }
"""

particlegun.param(param_pGun)
main.add_module(particlegun)

# EvtGen Simulation:
# TODO: There are newer convenience functions for this task -> Include them!
# Beam parameters
beamparameters = add_beamparameters(main, "Y4S")
evtgenInput = register_module('EvtGenInput')
evtgenInput.logging.log_level = LogLevel.WARNING
main.add_module(evtgenInput)


# ---------------------------------------------------------------------------------------


# setup the geometry (the Geometry and the Gearbox will be ignore in add_simulation if they are already in the path)
setup_Geometry(path)

# Detector Simulation:
add_simulation(path=main,
               components=['BeamPipe',
                           'MagneticFieldConstant4LimitedRSVD',
                           'PXD',
                           'SVD',
                           'CDC'])


# ---------------------------------------------------------------------------------------
# Setting up the MC based track finder.
mctrackfinder = register_module('TrackFinderMCTruthRecoTracks')
mctrackfinder.param('UseCDCHits', False)
mctrackfinder.param('UseSVDHits', True)
# Always use PXD hits! For SVD only, these will be filtered later when converting to SPTrackCand
mctrackfinder.param('UsePXDHits', True)
mctrackfinder.param('Smearing', False)
mctrackfinder.param('MinimalNDF', 6)
mctrackfinder.param('WhichParticles', ['primary'])
mctrackfinder.param('RecoTracksStoreArrayName', 'MCRecoTracks')
main.add_module(mctrackfinder)


# Root output. Default filename can be overriden with '-o' basf2 option.
rootOutput = register_module('RootOutput')
rootOutput.param('outputFileName', "MyRootFile.root")
main.add_module(rootOutput)

log_to_file('createSim.log', append=False)

process(main)
print(statistics)
