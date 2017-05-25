#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [ccbar Dstar2D0Pip D02KmPip]

"""
<header>
  <output>../2610030000.dst.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

from basf2 import *
from modularAnalysis import generateContinuum
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from HLTTrigger import add_HLT_Y4S
from ROOT import Belle2
import glob

set_random_seed(150922)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create path
main = create_path()

# generate continuum events
decayTable = Belle2.FileSystem.findFile('/decfiles/dec/2610030000.dec')
generateContinuum(noEvents=100, inclusiveP='D0', decayTable=decayTable, path=main)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# HLT L3 simulation
main.add_module('Level3')

# reconstruction
add_reconstruction(main)

# HLT physics trigger
add_HLT_Y4S(main)

# Finally add mdst output
output_filename = "../2610030000.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
