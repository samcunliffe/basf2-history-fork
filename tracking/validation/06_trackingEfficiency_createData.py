#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 10 charged muon tracks with             #
#    fixed transverse momentum using the ParticleGun            #
#                                                               #
#    written by Michael Ziegler, KIT                            #
#    michael.ziegler2@kit.edu                                   #
#                                                               #
#################################################################

from basf2 import *
from tracking_efficiency_helpers import run_simulation

set_random_seed(123456)

pt_value = 0.25

output_filename = '../trackingEfficiency_pt_%.2fGeV.root' % pt_value

print output_filename

path = create_path()

run_simulation(path, pt_value, output_filename)

process(path)

