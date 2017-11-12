#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from basf2 import *
from ROOT import Belle2
from math import pi, tan
import os
from subprocess import call

# set run time library path
rdi_path = '/home/belle2/tasheng/Vivado_2017.2/lib/lnx64.o'
if rdi_path not in os.environ['LD_LIBRARY_PATH']:
    print('please set environment variable first! do either')
    print('export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:' + rdi_path)
    print('or')
    print('setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:' + rdi_path)
    exit(1)

# link to 2D design snapshot
for link in ['xsim.dir', 'innerLRLUT.mif', 'outerLRLUT.mif']:
    if link not in os.listdir(os.getcwd()):
        call(['ln', '-s', '/home/belle2/tasheng/tsim/' + link])

"""
generate tracks with particle gun, simulate CDC and CDC trigger, save the output.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 10000
evtnum = 10
particlegun_params = {
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'inversePt',
    'momentumParams': [2., 10.],
    'thetaGeneration': 'uniform',
    'thetaParams': [35, 145],
    'phiGeneration': 'uniform',
    'phiParams': [46, 135],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0, 0.0],
    'yVertexParams': [0, 0.0],
    'zVertexParams': [-10., 10.]}

# ------------------------- #
# create path up to trigger #
# ------------------------- #

# set random seed
basf2.set_random_seed(seed)
# suppress messages and warnings during processing:
# basf2.set_log_level(basf2.LogLevel.ERROR)

main = basf2.create_path()

empty_path = basf2.create_path()

main.add_module('RootInput', inputFileName='/home/belle2/tasheng/gcr/cdc/cosmic.0001.03898.HLT1.f00007.root')

# main.add_module('EventInfoSetter', evtNumList=evtnum)
main.add_module('Progress')

# main.add_module('Gearbox')
# main.add_module('Geometry', components=['BeamPipe',
#                                         'PXD', 'SVD', 'CDC',
#                                         'MagneticFieldConstant4LimitedRCDC'])
# particlegun = basf2.register_module('ParticleGun')
# particlegun.param(particlegun_params)
# main.add_module(particlegun)

# main.add_module('FullSim')
# main.add_module('CDCDigitizer')

# ---------------------- #
# CDC trigger and output #
# ---------------------- #

firmtsf = register_module('CDCTriggerTSFFirmware')
firmtsf.param('mergerOnly', True)
firmtsf.logging.log_level = basf2.LogLevel.DEBUG
firmtsf.logging.debug_level = 30
firmtsf.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)
main.add_module(firmtsf)

# 2D finder

# firm2d = register_module('CDCTrigger2DFinderFirmware')
# firm2d.logging.log_level = basf2.LogLevel.DEBUG
# firm2d.logging.debug_level = 20
# firm2d.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)
# main.add_module(firm2d)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
