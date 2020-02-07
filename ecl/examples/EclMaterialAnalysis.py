#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use EclDataAnalysis
# module to dump ECL related quantities in an ntuple
# starting from dst root file.
#
# The user should provide input and output root filnames
# as first and second argument respectively.
#
# Author: The Belle II Collaboration
# Contributors: Benjamin Oberhof
#
########################################################

import random
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = b2.create_path()

# Event setting and info
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100000],
                       'runList': [1],
                       'expList': [0]})

main.add_module(eventinfosetter)

# random number for generation

# intseed = random.randint(1, 10000000)
b2.set_random_seed(123456)

# Create geometry
# Geometry parameter loader
# gearbox = b2.register_module('Gearbox')

# Geometry builder
# geometry = b2.register_module('Geometry')
# geometry.param('components', ['ECL'])

components = [  # 'MagneticField',
    # 'BeamPipe',
    'ECL',
    'PXD',
    'SVD',
    'CDC',
    # 'EKLM',
    # 'BKLM',
    # 'TOP',
    'ARICH',
]

# Simulation
# g4sim = b2.register_module('FullSim')

# single particle generator settings
pGun = b2.register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.5],
    'thetaGeneration': 'uniform',  # uniformCos,
    'thetaParams': [(12.01), (31.36)],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}

pGun.param(param_pGun)
main.add_module(pGun)

bkgdir = 'bkg/'
# bkg = glob.glob(bkgdir+'*.root')

bkgFiles = [
    bkgdir + 'Coulomb_HER_100us.root',
    bkgdir + 'Coulomb_LER_100us.root',
    bkgdir + 'Coulomb_HER_100usECL.root',
    bkgdir + 'Coulomb_LER_100usECL.root',
    bkgdir + 'RBB_HER_100us.root',
    bkgdir + 'RBB_LER_100us.root',
    bkgdir + 'RBB_HER_100usECL.root',
    bkgdir + 'RBB_LER_100usECL.root',
    bkgdir + 'Touschek_HER_100us.root',
    bkgdir + 'Touschek_LER_100us.root',
    bkgdir + 'Touschek_HER_100usECL.root',
    bkgdir + 'Touschek_LER_100usECL.root',
]

# add_simulation(main)#, bkgfiles=bkgFiles)
# main.add_module(gearbox)
# main.add_module(geometry)
# main.add_module(g4sim)

add_simulation(main)
add_reconstruction(main)

# display = b2.register_module('Display')
# main.add_module(display)

# eclDataAnalysis module
ecldataanalysis = b2.register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', 'EclDataAnalysis_500MeV_100000_Full_FWD.root')
ecldataanalysis.param('doTracking', 0)
main.add_module(ecldataanalysis)

b2.process(main)
print(b2.statistics)
