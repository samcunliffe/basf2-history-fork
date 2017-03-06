#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../mdst-xi-lambda.root</input>
  <output>../ana-xi-lambda.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

#######################################################
#
# Obtain p samples for PID performance validation
# from Xi decays to Lambda
#
# ccbar -> Xi- anything
#           |
#           +-> Labmda:ppi pi-
#                |
#                +-> p+ pi-
#
#
# Contributors: Jake Bennett (July 2016)
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *

# load input ROOT file
inputMdst('default', '../mdst-xi-lambda.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

fillParticleList('pi+:xi', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
fillParticleList('pi+:all', 'chiProb > 0.001')
fillParticleList('p+:all', 'chiProb > 0.001')


# reconstruct Lambda:ppi
reconstructDecay('Lambda0:ppi -> p+:all pi-:all', '1.100 < M < 1.130')
massVertexRave('Lambda0:ppi', 0.001)

# reconstruct the Xi+ from the Lambda:ppi and pi+:all
reconstructDecay('Xi-:sig -> Lambda0:ppi pi-:xi', '1.300 < M < 1.340')
vertexRave('Xi-:sig', 0.001)


# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
toolsnu = ['EventMetaData', '^Xi-']
toolsnu += ['Kinematics', '^Xi- -> [^Lambda0 -> ^p+:all ^pi-:all] ^pi-:xi']
toolsnu += ['InvMass[BeforeFit]', '^Xi- -> [^Lambda0 -> p+:all pi-:all] pi-:xi']
toolsnu += ['PID', 'Xi- -> [Lambda0 -> ^p+:all ^pi-:all] ^pi-:xi']
toolsnu += ['Vertex', '^Xi- -> [^Lambda0 -> p+:all pi-:all] pi-:xi']
toolsnu += ['FlightInfo', '^Xi- -> [^Lambda0 -> p+:all pi-:all] pi-:xi']

# write out the flat ntuple
ntupleFile('../ana-xi-lambda.root')
ntupleTree('xitree', 'Xi-:sig', toolsnu)


# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
