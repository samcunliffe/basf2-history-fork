#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Meant to be run after analysis/examples/teacher.py, attaches expert
# output to each Particle (as extra info) and saves the result in a file

import sys
import os
from basf2 import *

if not os.path.isfile('YourMDSTFile.root'):
    B2WARNING('This example requires an MDST file as input. You might need to change the \'inputFileName\' parameter in the steering file, or run basf2 with arguments -i MyFile.root')

# Create main path
main = create_path()

input = register_module('RootInput')
input.param('inputFileName', 'YourMDSTFile.root')
main.add_module(input)

particleloader = register_module('ParticleLoader')
main.add_module(particleloader)

selector = register_module('ParticleSelector')
selector.param('PDG', 11)
selector.param('ListName', 'e-')
main.add_module(selector)

methods = ['Fisher', 'NeuroBayes', 'BDTGradient', 'PDEFoamBoost']

for method in methods:
    expert = register_module('TMVAExpert')
    expert.param('prefix', 'TMVA')
    expert.param('method', method)
    expert.param('listNames', ['e-'])
    expert.param('signalProbabilityName', method + '_Probability')
    main.add_module(expert)

output = register_module('RootOutput')
output.param('outputFileName', 'mdstWithSignalProbabilities.root')
main.add_module(output)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
