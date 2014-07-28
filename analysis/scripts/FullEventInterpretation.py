#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#
# This FullEventInterpretation algorithm uses a functional approach (see actorFramework.py).
# All tasks are implemented in Functions called actors.
# Each Actor has requirements and provides return values.
# E.g. SignalProbability requires the path, a method, variables and a ParticleList.
#                        provides SignalProbability for the ParticleList.
#      CreatePreCutDistribution require among others the SignalProbability for the ParticleLists
#                        provides PreCutDistribution
#      PreCutDetermination requires among others a PreCutDistribution
#                         provides PreCuts
#      ParticleListFromChannel requires among others PreCuts
# ... and so on ...
#
# The actors are added to the Sequence of actors in the FullEventInterpretation function at the end of this file.
# Afterwards the dependencies between the Actors are solved, and the Actors are called in the correct order, with the required parameters.
# If no further Actors can be called without satisfying all their requirements, the FullReoncstruction function returns.
# Therefore the end user has to run the FullEventInterpretation several times, until all Distributions, Classifiers, ... are created.
#

from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True

import pdg

import actorFramework
from actorFunctions import *

import collections
import argparse
import copy


class Particle(object):
    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FullReconstruction algorithm,
    and provides a method (see TMVA Interface), variables (see VariableManager) and the decay channels of the particle.
    In total this class contains: name, method, variables, efficiency (of preCuts) and all decay channels of a particle.
    """

    ## Create new class called MVAConfiguration via namedtuple. namedtuples are like a struct in C
    MVAConfiguration = collections.namedtuple('MVAConfiguration', 'name, type, config, variables, target, targetCluster')
    ## Create new class called PreCutConfiguration via namedtuple. namedtuples are like a struct in C
    PreCutConfiguration = collections.namedtuple('PreCutConfiguration', 'variable, method, efficiency, purity')
    ## Create new class called PostCutConfiguration via namedtuple. namedtuples are like a struct in C
    PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value')
    ## Create new class called DecayChannel via namedtuple. namedtuples are like a struct in C
    DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters, mvaConfig, preCutConfig, postCutConfig')

    def __init__(self, name, mvaConfig, preCutConfig=PreCutConfiguration(variable='Mass', method='S/B', efficiency=0.70, purity=0.001),
                 postCutConfig=PostCutConfiguration(value=0.0001), label='generic'):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param name is the correct pdg name as a string of the particle.
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig intermediate cut configuration
            @param postCutConfig post cut configuration
            @param label additional label for this particle
        """
        ## The name of the particle as correct pdg name e.g. K+, pi-, D*0.
        self.name = name
        ## Additional label like hasMissing or has2Daughters
        self.label = label
        ## multivariate analysis configuration
        self.mvaConfig = mvaConfig
        ## Decay channels, added by addChannel method.
        self.channels = []
        ## intermediate cut configuration
        self.preCutConfig = preCutConfig
        ## post cut configuration
        self.postCutConfig = postCutConfig

    @property
    def daughters(self):
        """ Property which returns all daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    @property
    def isFSP(self):
        """ Returns true if the particle is a final state particle """
        return self.channels == []

    def addChannel(self, daughters, mvaConfig=None, preCutConfig=None, postCutConfig=None, addExtraVars=True):
        """
        Appends a new decay channel to the Particle object.
        @param daughters is a list of pdg particle names e.g. ['pi+','K-']
        @param mvaConfig multivariate analysis configuration
        @param preCutConfig pre cut configuration
        @param postCutConfig pre cut configuration
        @param group groups channels into groups with similar kinematic e.g. missing particles, two tracks, three tracks, ...
        """
        preCutConfig = copy.deepcopy(self.preCutConfig if preCutConfig is None else preCutConfig)
        postCutConfig = copy.deepcopy(self.postCutConfig if postCutConfig is None else postCutConfig)
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        if addExtraVars:
            mvaConfig.variables.extend(['daughter{i}(getExtraInfo(SignalProbability))'.format(i=i) for i in range(0, len(daughters))])
        self.channels.append(Particle.DecayChannel(name=self.name + '_' + self.label + ' ==> ' + ' '.join(daughters),
                                                   daughters=daughters,
                                                   mvaConfig=mvaConfig,
                                                   preCutConfig=preCutConfig,
                                                   postCutConfig=postCutConfig))
        return self

    def __str__(self):
        """ Convert particle object in a readable string which contains all configuration informations """
        output = '{name} -- {label}\n'.format(name=self.name, label=self.label)

        def compareMVAConfig(x, y):
            return x.name == y.name and x.type == y.type and x.config == y.config and x.target == y.target and x.targetCluster == y.targetCluster

        def compareCutConfig(x, y):
            return x == y

        if self.isFSP:
            output += '    PreCutConfiguration: variable={p.variable}, method={p.method}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=self.preCutConfig)
            output += '    PostCutConfiguration: value={p.value}\n'.format(p=self.postCutConfig)
            output += '    MVAConfiguration: name={m.name}, type={m.type}, config={m.config}, target={m.target}, targetCluster={m.targetCluster}\n'.format(m=self.mvaConfig)
            output += '    Variables: ' + ', '.join(self.mvaConfig.variables) + '\n'
        else:
            samePreCutConfig = all(compareCutConfig(channel.preCutConfig, self.preCutConfig) for channel in self.channels)
            samePostCutConfig = all(compareCutConfig(channel.postCutConfig, self.postCutConfig) for channel in self.channels)
            sameMVAConfig = all(compareMVAConfig(channel.mvaConfig, self.mvaConfig) for channel in self.channels)
            commonVariables = reduce(lambda x, y: set(x).intersection(y), [channel.mvaConfig.variables for channel in self.channels])
            if sameMVAConfig:
                output += '    All channels use the same MVA configuration\n'
                output += '    MVAConfiguration: name={m.name}, type={m.type}, target={m.target}, targetCluster={m.targetCluster}, config={m.config}\n'.format(m=self.mvaConfig)
            output += '    Shared Variables: ' + ', '.join(commonVariables) + '\n'

            if samePreCutConfig:
                output += '    All channels use the same PreCut configuration\n'
                output += '    PreCutConfiguration: variable={p.variable}, method={p.method}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=self.preCutConfig)

            if samePostCutConfig:
                output += '    All channels use the same PostCut configuration\n'
                output += '    PostCutConfiguration: value={p.value}\n'.format(p=self.postCutConfig)

            for channel in self.channels:
                output += '    {name}\n'.format(name=channel.name)
                if not samePreCutConfig:
                    output += '    PreCutConfiguration: variable={p.variable}, method={p.method}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=channel.preCutConfig)
                if not samePostCutConfig:
                    output += '    PostCutConfiguration: value={p.value}\n'.format(p=channel.postCutConfig)
                if not sameMVAConfig:
                    output += '    MVAConfiguration: name={m.name}, type={m.type}, config={m.config}, target={m.target}, targetCluster={m.targetCluster}\n'.format(m=channel.mvaConfig)
                output += '        Individual Variables: ' + ', '.join(set(channel.mvaConfig.variables).difference(commonVariables)) + '\n'
        return output


def FullEventInterpretation(path, particles):
    """
    The Full Event Interpretation algorithm.
    Alle the Actors defined above are added to the sequence and are executed in an order which fulfills all requirements.
    This function returns if no more Actors can be called without violate some requirements.
        @param path the basf2 module path
        @param particles sequence of particle objects which shall be reconstructed by this algorithm
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('-verbose', '--verbose', dest='verbose', action='store_true', help='Output additional information')
    parser.add_argument('-nosignal', '--no-signal-classifiers', dest='nosig', action='store_true', help='Do not train classifiers')
    args = parser.parse_args()

    particles = addIncompleteChannels(particles, args.verbose)

    # Add the basf2 module path
    seq = actorFramework.Sequence()
    seq.addFunction(CountMCParticles, path='Path')

    # Now loop over all given particles, foreach particle we add some Resources and Actors.
    for particle in particles:
        ########## RESOURCES #############
        seq.addResource('Name_{p}_{l}'.format(p=particle.name, l=particle.label), particle.name)
        seq.addResource('Label_{p}_{l}'.format(p=particle.name, l=particle.label), particle.label)
        seq.addResource('Object_{p}_{l}'.format(p=particle.name, l=particle.label), particle)

        if particle.channels == []:
            seq.addResource('MVAConfig_{p}_{l}'.format(p=particle.name, l=particle.label), particle.mvaConfig)
            seq.addResource('PostCutConfig_{p}_{l}'.format(p=particle.name, l=particle.label), particle.postCutConfig)
        for channel in particle.channels:
            seq.addResource('Name_{c}'.format(c=channel.name), channel.name)
            seq.addResource('MVAConfig_{c}'.format(c=channel.name), channel.mvaConfig)
            seq.addResource('PreCutConfig_{c}'.format(c=channel.name), channel.preCutConfig)
            seq.addResource('PostCutConfig_{c}'.format(c=channel.name), channel.postCutConfig)

        ########### RECONSTRUCTION ACTORS ##########
        if particle.isFSP:
            seq.addFunction(SelectParticleList,
                            path='Path',
                            particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                            particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label))
            seq.addFunction(CopyParticleLists,
                            path='Path',
                            particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                            particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                            inputLists=['RawParticleList_{p}_{l}'.format(p=particle.name, l=particle.label)],
                            postCuts=['PostCut_{p}_{l}'.format(p=particle.name, l=particle.label)])
        else:
            for channel in particle.channels:
                seq.addFunction(MakeAndMatchParticleList,
                                path='Path',
                                particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                                particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                                channelName='Name_{c}'.format(c=channel.name),
                                inputLists=['ParticleList_{d}'.format(d=daughter) for daughter in channel.daughters],
                                preCut='PreCut_{c}'.format(c=channel.name))
                seq.addFunction(FitVertex,
                                path='Path',
                                channelName='Name_{c}'.format(c=channel.name),
                                particleList='RawParticleList_{c}'.format(c=channel.name))

            seq.addFunction(CopyParticleLists,
                            path='Path',
                            particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                            particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                            inputLists=['RawParticleList_{c}'.format(c=channel.name) for channel in channels],
                            postCuts=['PostCut_{c}'.format(c=channel.name) for channel in channels])

        ############# PRECUT DETERMINATION ############
        if not particle.channels.isFSP:
            seq.addFunction(PreCutDetermination,
                            channelNames=['Name_{c}'.format(c=channel.name) for channel in particle.channels],
                            preCutConfig=['PreCutConfig_{c}'.format(c=channel.name) for channel in particle.channels],
                            preCutHistograms=['PreCutHistogram_{c}'.format(c=channel.name) for channel in particle.channels])

            for channel in particle.channels:
                additionalDependencies = []
                if channel.preCutConfig.variable == 'daughterProductOfSignalProbability':
                    additionalDependencies = ['SignalProbability_{d}'.format(d=daughter) for daughter in channel.daughters]

                seq.addFunction(CreatePreCutHistogram,
                                path='Path',
                                particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                                particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                                channelName='Name_{c}'.format(c=channel.name),
                                preCutConfig='PreCutConfig_{c}'.format(c=channel.name),
                                daughterLists=['ParticleList_{d}'.format(d=daughter) for daughter in channel.daughters],
                                additionalDependencies=additionalDependencies)

        ############## POSTCUT DETERMINATION #############
        if particle.isFSP:
            seq.addFunction(PostCutDeterminationFSP,
                            particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                            particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                            postCutConfig='PostCutConfig_{p}_{l}'.format(p=particle.name, l=particle.label),
                            signalProbability='SignalProbability_{p}_{l}'.format(p=particle.name, l=particle.label))
        else:
            seq.addFunction(PostCutDeterminationNonFSP,
                            channelNames=['Name_{c}'.format(c=channel.name) for channel in particle.channels],
                            postCutConfigs=['PostCutConfig_{c}'.format(c=channel.name) for channel in particle.channels],
                            signalProbabilities=['SignalProbability_{c}'.format(c=channel.name) for channel in particle.channels])

        ########### SIGNAL PROBABILITY ACTORS #######
        # The classifier part of the FullReconstruction.
        if not args.nosig:
            if particle.isFSP:
                seq.addFunction(SignalProbability,
                                path='Path',
                                particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                                particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                                channelName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                                mvaConfig='MVAConfig_{p}_{l}'.format(p=particle.name, l=particle.label),
                                particleList='RawParticleList_{p}_{l}'.format(p=particle.name, l=particle.label))
            else:
                for channel in particle.channels:

                    additionalDependencies = []
                    if any('SignalProbability' in variable for variable in channel.mvaConfig.variables):
                        additionalDependencies += ['SignalProbability_{d}'.format(d=daughter) for daughter in channel.daughters]
                    if any(variable in ['dx', 'dy', 'dz', 'dr'] for variable in channel.mvaConfig.variables):
                        additionalDependencies += ['VertexFit_{c}'.format(c=channel.name)]

                    seq.addFunction(SignalProbability,
                                    path='Path',
                                    particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                                    particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                                    channelName='Name_{c}'.format(c=channel.name),
                                    mvaConfig='MVAConfig_{c}'.format(c=channel.name),
                                    particleList='RawParticleList_{c}'.format(c=channel.name),
                                    additionalDependencies=additionalDependencies)

                seq.addResource('SignalProbability_{p}_{l}'.format(p=particle.name, l=particle.label), 'Dummy',
                                requires=['SignalProbability_{c}'.format(c=channel.name) for channel in particle.channels], strict=False)
                seq.addResource('SignalProbability_{p}_{l}'.format(p=pdg.conjugate(particle.name), l=particle.label), 'Dummy',
                                requires=['SignalProbability_{p}_{l}'.format(p=particle.name, l=particle.label)])

        ################ Information ACTORS #################
        for channel in particle.channels:

            seq.addFunction(WriteAnalysisFileForChannel,
                            particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                            particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                            channelName='Name_{c}'.format(c=channel.name),
                            preCutConfig='PreCutConfig_{c}'.format(c=channel.name),
                            preCut='PreCut_{c}'.format(c=channel.name),
                            preCutHistogram='PreCutHistogram_{c}'.format(c=channel.name),
                            mvaConfig='MVAConfig_{c}'.format(c=channel.name),
                            signalProbability='SignalProbability_{c}'.format(c=channel.name),
                            postCutConfig='PostCutConfig_{c}'.format(c=channel.name),
                            postCut='PostCut_{c}'.format(c=channel.name))

        if particle.isFSP:
            seq.addFunction(WriteAnalysisFileForFSParticle,
                            particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                            particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                            mvaConfig='MVAConfig_{p}_{l}'.format(p=particle.name, l=particle.label),
                            signalProbability='SignalProbability_{p}_{l}'.format(p=particle.name, l=particle.label),
                            postCutConfig='PostCutConfig_{p}_{l}'.format(p=particle.name, l=particle.label),
                            postCut='PostCut_{p}_{l}'.format(p=particle.name, l=particle.label),
                            mcCounts='MCParticleCounts')
        else:
            seq.addFunction(WriteAnalysisFileForCombinedParticle,
                            particleName='Name_{p}_{l}'.format(p=particle.name, l=particle.label),
                            particleLabel='Label_{p}_{l}'.format(p=particle.name, l=particle.label),
                            channelPlaceholders=['Placeholders_' + channel.name for channel in particle.channels],
                            mcCounts='MCParticleCounts')

    #TODO: don't hardcode B0/B+ here
    seq.addFunction(VariablesToNTuple,
                    path='Path',
                    particleName='Name_B0_generic',
                    particleLabel='Label_B0_generic',
                    particleList='ParticleList_B0_generic',
                    signalProbability='SignalProbability_B0_generic')

    seq.addFunction(VariablesToNTuple,
                    path='Path',
                    particleName='Name_B+_generic',
                    particleLabel='Label_B+_generic',
                    particleList='ParticleList_B+_generic',
                    signalProbability='SignalProbability_B+_generic')

    seq.addFunction(WriteAnalysisFileSummary,
                    finalStateParticlePlaceholders=['Placeholders_{p}_{l}'.format(p=particle.name, l=particle.label) for particle in particles if particle.isFSP],
                    combinedParticlePlaceholders=['Placeholders_{p}_{l}'.format(p=particle.name, l=particle.label) for particle in particles if not particle.isFSP],
                    ntuples=['VariablesToNTuple_B0_generic'],  # , 'VariablesToNTuple_B+'])
                    mcCounts='MCParticleCounts',
                    particles=['Object_{p}_{l}'.format(p=particle.name, l=particle.label)for particle in particles])
    seq.addNeeded('SignalProbability_B+_generic')
    seq.addNeeded('SignalProbability_B0_generic')
    seq.addNeeded('ParticleList_B0_generic')
    seq.addNeeded('ParticleList_B+_generic')
    seq.addNeeded('FEIsummary.pdf')

    seq.run(path, args.verbose)
