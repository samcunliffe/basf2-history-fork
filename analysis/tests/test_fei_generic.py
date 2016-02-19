#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
import shutil
import glob
import sys

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2

from fei.default_channels import get_unittest_channels
from fei.steering import fullEventInterpretation
import fei.provider

fei.provider.MaximumNumberOfMVASamples = int(1e7)
fei.provider.MinimumNumberOfMVASamples = int(10)

inputFile = Belle2.FileSystem.findFile('analysis/tests/mdst5.root')
selection_path = create_path()
selection_path.add_module('RootInput', inputFileName=inputFile, inputFileNames=[])
selection_path.add_module('Gearbox')
selection_path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])

particles = get_unittest_channels()

tempdir = tempfile.mkdtemp()
os.chdir(tempdir)

sys.argv.append('-verbose')

# MCCount Histograms,
feistate = fullEventInterpretation(None, selection_path, particles)
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1

# Train FSP Networks
feistate = fullEventInterpretation(None, selection_path, particles)
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 2
assert len(glob.glob('mu+*')) == 1
assert len(glob.glob('pi+*')) == 1
assert len(glob.glob('K+*')) == 1

# Train pi0 network and independent D networks
feistate = fullEventInterpretation(None, selection_path, particles)
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 5  # V0 Gamma channel is not trained
assert len(glob.glob('mu+*')) == 4
assert len(glob.glob('pi+*')) == 4
assert len(glob.glob('K+*')) == 4
assert len(glob.glob('pi0*')) == 1
assert len(glob.glob('D*')) == 3

# Train D networks
feistate = fullEventInterpretation(None, selection_path, particles)
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('pi0*')) == 4
assert len(glob.glob('D*')) == 14

feistate = fullEventInterpretation(None, selection_path, particles)
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('moduleStatistics_*')) == 1
assert len(glob.glob('D*')) == 20

feistate = fullEventInterpretation(None, selection_path, particles)
assert feistate.is_trained
assert len(glob.glob('Summary*.pickle')) == 1


sys.argv.append('-monitor')

feistate = fullEventInterpretation(None, selection_path, particles)
process(feistate.path)
assert feistate.is_trained
assert len(glob.glob('Monitor_MCCounts.root')) == 1
assert len(glob.glob('Monitor_TagUniqueSignal_*')) == 7
assert len(glob.glob('Monitor_FitVertex_*')) == 12
assert len(glob.glob('Monitor_Final_*')) == 7
assert len(glob.glob('Monitor_CopyParticleList_*')) == 21
assert len(glob.glob('Monitor_MatchParticleList_*')) == 11
assert len(glob.glob('Monitor_SignalProbability_*')) == 10
assert len(glob.glob('Monitor_MakeParticleList_*')) == 22
assert len(glob.glob('Monitor_*')) == 91

feistate.path.modules()[0].param('inputFileName', inputFile)
feistate.path.modules()[0].param('inputFileNames', [])
uncached_path = serialize_path(feistate.path)

shutil.rmtree(tempdir)

tempdir = tempfile.mkdtemp()
os.chdir(tempdir)

sys.argv.append('-cache')
sys.argv.append('cache.pickle')

# MCCount Histograms,
feistate = fullEventInterpretation(None, selection_path, particles)
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1
assert len(glob.glob('cache.pickle')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

# Train FSP Networks
feistate = fullEventInterpretation(None, selection_path, particles)
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 2
assert len(glob.glob('mu+*')) == 1
assert len(glob.glob('pi+*')) == 1
assert len(glob.glob('K+*')) == 1
assert len(glob.glob('cache.pickle.bkp0')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

# Train pi0 network and independent D networks
feistate = fullEventInterpretation(None, selection_path, particles)
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 5  # V0 Gamma channel is not trained
assert len(glob.glob('mu+*')) == 4
assert len(glob.glob('pi+*')) == 4
assert len(glob.glob('K+*')) == 4
assert len(glob.glob('pi0*')) == 1
assert len(glob.glob('D*')) == 3
assert len(glob.glob('cache.pickle.bkp1')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

# Train D networks
feistate = fullEventInterpretation(None, selection_path, particles)
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('pi0*')) == 4
assert len(glob.glob('D*')) == 14
assert len(glob.glob('cache.pickle.bkp2')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

feistate = fullEventInterpretation(None, selection_path, particles)
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('moduleStatistics_*')) == 1
assert len(glob.glob('D*')) == 20
assert len(glob.glob('cache.pickle.bkp3')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

feistate = fullEventInterpretation(None, selection_path, particles)
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
assert len(glob.glob('Summary*.pickle')) == 1
assert len(glob.glob('cache.pickle.bkp4')) == 1
# Check if now all Monitoring histograms are available
assert len(glob.glob('Monitor_MCCounts.root')) == 1
assert len(glob.glob('Monitor_TagUniqueSignal_*')) == 7
assert len(glob.glob('Monitor_FitVertex_*')) == 12
assert len(glob.glob('Monitor_Final_*')) == 7
assert len(glob.glob('Monitor_CopyParticleList_*')) == 21
assert len(glob.glob('Monitor_MatchParticleList_*')) == 11
assert len(glob.glob('Monitor_SignalProbability_*')) == 10
assert len(glob.glob('Monitor_MakeParticleList_*')) == 22
# Additional monitoring stuff during training
assert len(glob.glob('Monitor_GenerateTrainingData_*')) == 11
assert len(glob.glob('Monitor_*')) == 102
assert feistate.is_trained

sys.argv.append('-prune')

feistate = fullEventInterpretation(None, selection_path, particles)
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
feistate.path.modules()[0].param('inputFileNames', [])
feistate.path.add_module('RootOutput')
process(feistate.path)
assert len(glob.glob('cache.pickle.bkp5')) == 1
assert feistate.is_trained
assert os.path.getsize('RootOutput.root') < os.path.getsize('RootInput.root')

sys.argv.pop()
sys.argv.pop()
sys.argv.pop()

sys.argv.append('-dump-path')
feistate = fullEventInterpretation(None, selection_path, particles)
feistate.path.modules()[0].param('inputFileName', inputFile)
feistate.path.modules()[0].param('inputFileNames', [])
cached_path = serialize_path(feistate.path)

print(cached_path)
print(uncached_path)
assert uncached_path == cached_path


shutil.rmtree(tempdir)
