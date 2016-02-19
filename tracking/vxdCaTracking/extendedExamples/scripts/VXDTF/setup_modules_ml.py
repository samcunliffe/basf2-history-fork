#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


def add_fbdtclassifier_training(path,
                                networkInputName,
                                outputFileName='FBDTClassifier.dat',
                                train=True,
                                storeSamples=True,
                                useSamples=False,
                                samplesFileName='FBDTClassifier_samples.dat',
                                nTrees=100,
                                treeDepth=3,
                                shrinkage=0.15,
                                randRatio=0.5,
                                logLevel=LogLevel.INFO,
                                dbgLvl=1):
    """This function adds the FastBDTClassifierTraining-module to the given path and exposes all its parameters
    @param path the path to which the module should be added
    @param networkInputName parameter passed to the module
    @param outputFileName the filename to which the FBDTClassifier will be stored
    @param train actually do the training
    @param nTrees number of trees in the module
    @param treeDepth the number of layers in the trees
    @param shrinkage the shrinkage parameter
    @param randRatio the ratio of all samples used for training of each tree
    @param logLevel the LogLevel of the module
    @param dbgLvl the debugLevel of the module
    """
    fbdtTrainer = register_module('FastBDTClassifierTraining')
    fbdtTrainer.logging.log_level = logLevel
    fbdtTrainer.logging.debug_level = dbgLvl
    fbdtTrainer.param({'networkInputName': networkInputName,
                       'outputFileName': outputFileName,
                       'train': train,
                       'nTrees': nTrees,
                       'treeDepth': treeDepth,
                       'shrinkage': shrinkage,
                       'randRatio': randRatio,
                       'storeSamples': storeSamples,
                       'useSamples': useSamples,
                       'samplesFileName': samplesFileName
                       })

    path.add_module(fbdtTrainer)


# NOTE: some of these parameters can be disabled by a preprocessor flag in the module code!
def add_ml_threehitfilters(path,
                           networkInputName,
                           fbdtFileName='FBDTClassifier.dat',
                           cutVal=0.5,
                           rec=False,  # this on
                           rFN='MLSegmentProducer_samples.root',  # and this on
                           logLevel=LogLevel.INFO,
                           dbgLvl=1):
    """This function adds the MLSegmentNetworkProducerModule to the given path and exposes its parameters
    @param path the path to which the module should be added
    @param networkInputName parameter passed to the module
    @param fbdtFileName the filename where the FBDT is stored
    """
    ml_segment = register_module('MLSegmentNetworkProducer')
    ml_segment.logging.log_level = logLevel
    ml_segment.logging.debug_level = dbgLvl
    ml_segment.param({'networkInputName': networkInputName,
                      'FBDTFileName': fbdtFileName,
                      'cutValue': cutVal,
                      'record': rec,  # don't forget them here
                      'rootFileName': rFN  # ...
                      })

    path.add_module(ml_segment)


def add_fbdtclassifier_analyzer(path,
                                fbdtFileName,
                                trainSamp,
                                testSamp,
                                outputFN='FBDTAnalyzer_out.root',
                                logLevel=LogLevel.DEBUG,
                                dbgLvl=50):
    """This function analyses all presented training and test samples and stores the outputs into a root file for later analysis
    @param path the path to which the module should be added
    @param fbdtFileName the filename of the FBDTClassifier
    @param trainSamp the file name where the training samples are stored
    @param testSamp the file name where the test smples are stored
    @param outputFN the file name of the root file which is created
    """
    fbdtAnalyzer = register_module('FastBDTClassifierAnalyzer')
    fbdtAnalyzer.logging.log_level = logLevel
    fbdtAnalyzer.logging.debug_level = dbgLvl
    fbdtAnalyzer.param({'fbdtFileName': fbdtFileName,
                        'testSamples': testSamp,
                        'trainSamples': trainSamp,
                        'outputFileName': outputFN
                        })

    path.add_module(fbdtAnalyzer)
