#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1110021010.dst.root</input>
  <output>../1110021010.ntup.root</output>
  <contact>Saurabh Sandilya; saurabhsandilya@gmail.com</contact>
</header>
"""

#######################################################
#
# Obtain Delta E and Mbc distribution from the decay:
#
#
#    B0 -> K*0 + gamma
#           |
#           +-> K+ pi-
#
#
# Contributors: Saurabh Sandilya (April 2017)
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *

# load input ROOT file
inputMdst('default', '../1110021010.dst.root')
# default: for analysis of Belle II MC samples produced
# with releases with release-00-08-00 or newer

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

fillParticleList('pi+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4 and piid > 0.1')

fillParticleList('gamma:all', 'E > 0.050 and clusterE9E25 > 0.8')

# reconstruct rho0:pipi
reconstructDecay('rho0:pipi -> pi+:all pi-:all', '0.35 < M < 1.2')
matchMCTruth('rho0:pipi')

# reconstruct B0:sig
reconstructDecay('B0:sig -> rho0:pipi gamma:all', '4.8 < M < 5.8')
matchMCTruth('B0:sig')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
tools = ['EventMetaData', '^B0:sig']
tools += ['RecoStats', '^B0:sig']
tools += ['Kinematics', '^B0:sig -> [^rho0:pipi -> ^pi+:all ^pi-:all] ^gamma:all']
tools += ['InvMass', '^B0:sig -> [^rho0:pipi -> pi+:all pi-:all] gamma:all']
tools += ['DeltaEMbc', '^B0:sig -> [rho0:pipi -> pi+:all pi-:all] gamma:all']
tools += ['MCTruth', '^B0:sig -> [^rho0:pipi -> ^pi+:all ^pi-:all] ^gamma:all']
tools += ['MCHierarchy', 'B0:sig -> [rho0:pipi -> ^pi+:all ^pi-:all] ^gamma:all']
tools += ['PID', 'B0:sig -> [rho0:pipi -> ^pi+:all ^pi-:all] gamma:all']

# write out the flat ntuple
ntupleFile('../1110021010.ntup.root')
ntupleTree('h1', 'B0:sig', tools)


# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
