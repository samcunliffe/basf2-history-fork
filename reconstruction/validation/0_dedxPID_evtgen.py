#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from simulation import *
from reconstruction import *

print os.getcwd()

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
main.add_module(eventinfosetter)

pdgs = [11, 13, 211, 321, 2212]
pdgs += [-p for p in pdgs]

generator = register_module('ParticleGun')
generator.param('momentumParams', [0.05, 4.0])
generator.param('nTracks', 8.0)
generator.param('pdgCodes', pdgs)
main.add_module(generator)

#no EKLM
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'ECL',
]
add_simulation(main, components)

add_reconstruction(main, components)

#enable debug output for the module added by add_reconstruction()
for m in main.modules():
    if m.name() == 'DedxPID':
        m.param('enableDebugOutput', True)


output = register_module('RootOutput')
output.param('outputFileName', 'EvtGenSimRec_dedx.root')
# let's keep this small
output.param('branchNames', ['DedxLikelihoods', 'DedxTracks', 'EventMetaData'])
main.add_module(output)

process(main)
print statistics
