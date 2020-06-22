#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    Script to simulate 2000 charged-muon single-track events   #
#    using the ParticleGun for BKLM and EKLM validation.        #
#                                                               #
#    written by Leo Piilonen, VT                                #
#    piilonen@vt.edu                                            #
#                                                               #
#################################################################

"""
<header>
    <output>muon-KLMValidation.root</output>
    <contact>martina.laurenza@roma3.infn.it</contact>
    <description>Create events with 1 muon track for BKLM and EKLM validation.</description>
</header>
"""

import glob
import os

import basf2 as b2
import simulation as sim
import reconstruction as rec

b2.set_random_seed(981543)
b2.set_log_level(b2.LogLevel.ERROR)

output_filename = '../muon-KLMValidation.root'
b2.B2INFO('The output file name is ' + output_filename)

main_path = b2.create_path()

main_path.add_module('EventInfoSetter',
                     evtNumList=2000)

main_path.add_module('Progress')
main_path.add_module('ProgressBar')

main_path.add_module('ParticleGun',
                     pdgCodes=[-13, 13],
                     nTracks=1,
                     varyNTracks=0,
                     momentumGeneration='uniform',
                     momentumParams=[0.5, 5.0],
                     thetaGeneration='uniformCos',
                     thetaParams=[18., 155.],
                     phiGeneration='uniform',
                     phiParams=[0., 360.],
                     vertexGeneration='fixed',
                     xVertexParams=[0.0],
                     yVertexParams=[0.0],
                     zVertexParams=[0.0])


sim.add_simulation(path=main_path)
rec.add_reconstruction(path=main_path)

main_path.add_module('RootOutput',
                     outputFileName=output_filename,
                     branchNames=['MCParticles', 'KLMMuidLikelihoods', 'KLMDigits', 'BKLMHit2ds', 'EKLMHit2ds'])

b2.process(main_path)
print(b2.statistics)
