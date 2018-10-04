#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Generate 100 generic BBbar events using EvtGen
#
# Contributor(s): Torben Ferber (torben.ferber@desy.de)
#
########################################################
import os

from basf2 import *
from generators import *

# interpret the first input argument as decay file to facilitate testing of dec-files
dec_file = None
final_state = 'mixed'
if len(sys.argv) > 1:
    dec_file = os.path.abspath(sys.argv[1])
    final_state = 'signal'
    print("using following decay file: " + dec_file)


# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# EvtGen
add_evtgen_generator(path=main, finalstate=final_state, signaldecfile=dec_file)

# run
main.add_module("Progress")
main.add_module("RootOutput", outputFileName="evtgen_upsilon4s.root")
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
