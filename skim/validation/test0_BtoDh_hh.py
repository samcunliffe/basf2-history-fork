#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [B+ -> K+ (anti-D0 -> K+ pi-)]cc

"""
<header>
  <output>BtoDh_hh.dst.root</output>
  <contact>niharikarout@physics.iitm.ac.in</contact>
</header>
"""

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2

b2.set_random_seed(12345)


# create path
main = b2.create_path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1213030001.dec'))
main.add_module(evtgeninput)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)


# Finally add mdst output
output_filename = "../BtoDh_hh.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
