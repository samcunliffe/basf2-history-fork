#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *

from reconstruction import add_reconstruction
from modularAnalysis import *

# Create main path
main = create_path()

input = register_module('RootInput')
input.param('inputFileName', '../GenericB_GENSIMRECtoDST.dst.root')
main.add_module(input)

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')
gearbox = register_module('Gearbox')
main.add_module(progress)
main.add_module(gearbox)

# ----------------------------------------------------------------
loadReconstructedParticles(main)

selectParticle(main, 'K-', -321, [''])
selectParticle(main, 'pi+', 211, [''])
selectParticle(main, 'pi-', -211, [''])
selectParticle(main, 'pi0', 111, [''])
selectParticle(main, 'gamma', 22, [''])
selectParticle(main, 'e+', 11, [''])
selectParticle(main, 'e-', -11, [''])
selectParticle(main, 'mu+', 13, [''])
selectParticle(main, 'mu-', -13, [''])

makeParticle(
    main,
    'KS0',
    310,
    ['pi-', 'pi+'],
    0.4,
    0.6,
    )

# ----> NtupleMaker module
ntuple1 = register_module('NtupleMaker')
# output root file name (the suffix .root will be added automaticaly)
ntuple1.param('strFileName', '../GenericB.ntup.root')

# check PID efficiency
ntuple1.param('strTreeName', 'pituple')
ntuple1.param('strListName', 'pi+')
ntuple1.param('strTools', [
    'EventMetaData',
    '^pi+',
    'Kinematics',
    '^pi+',
    'MCTruth',
    '^pi+',
    'MCKinematics',
    '^pi+',
    'PID',
    '^pi+',
    ])
main.add_module(ntuple1)

# check pi0 resolution
ntuple2 = register_module('NtupleMaker')
ntuple2.param('strTreeName', 'pi0tuple')
ntuple2.param('strListName', 'pi0')
ntuple2.param('strTools', [
    'EventMetaData',
    'pi0',
    'MCTruth',
    'pi0 -> ^gamma ^gamma',
    'Kinematics',
    '^pi0 -> ^gamma ^gamma',
    ])
main.add_module(ntuple2)

# quick search for KS0 candidates
ntuple3 = register_module('NtupleMaker')
ntuple3.param('strTreeName', 'kstuple')
ntuple3.param('strListName', 'KS0')
ntuple3.param('strTools', [
    'EventMetaData',
    'KS0',
    'MCTruth',
    '^KS0 -> ^pi+ ^pi-',
    'Kinematics',
    '^KS0 -> ^pi+ ^pi-',
    ])
main.add_module(ntuple3)

# dump all event summary information
ntuple4 = register_module('NtupleMaker')
ntuple4.param('strTreeName', 'eventTuple')
ntuple4.param('strListName', '')
ntuple4.param('strTools', [
    'EventMetaData',
    'B-',
    'RecoStats',
    'B-',
    'DetectorStatsRec',
    'B-',
    'DetectorStatsSim',
    'B-',
    ])
main.add_module(ntuple4)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
