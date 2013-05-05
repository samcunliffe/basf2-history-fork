#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [1000])
evtmetagen.param('RunList', [1])
evtmetagen.param('ExpList', [1])
main.add_module(evtmetagen)

# generate Bu->D0(K pi) pi events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/generators/evtgen/decayfiles/Bu_D0pi,Kpi.dec')

main.add_module(evtgeninput)

# detector simulation,  digitisers and clusterisers
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ECL',
    'ARICH',
    'BKLM',
    'EKLM',
    'ESTR',
    'Coil',
    'STR',
    ]
add_simulation(main, components)

# reconstruction
add_reconstruction(main, components)

# dst output
output = register_module('RootOutput')
output.param('outputFileName', '../Bu_D0pi,Kpi_GENSIMRECtoDST.dst.root')
main.add_module(output)

# Go!
process(main)

# Print call statistics
print statistics
