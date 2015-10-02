#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *

particles = get_default_channels()
feistate = fullEventInterpretation(None, None, particles)

if feistate.is_trained:
    open('FEI_finished_its_training', 'a').close()

# show constructed path
print(feistate.path)

process(feistate.path)

B2WARNING('event() statistics:')
print(statistics)
