#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck
# * Contributor: Christian Roca
# * This script is needed to train           *
# * and to test the flavor tagger.           *
# ********************************************

from basf2 import *
from modularAnalysis import *
import variables as mc_variables
from ROOT import Belle2
import os


class FlavorTaggerInfoFiller(Module):

    """
    Creates a new FlavorTagInfo DataObject and saves there all the relevant information of the
    FlavorTagging:
    - Track probability of being the right target for every category (right target means
      coming directly from the B)
    - Highest probability track's pointer
    - Event probability of belonging to a given category
    """

    def event(self):
        path = analysis_main
        info = Belle2.PyStoreObj('EventExtraInfo')  # Calls the event extra info were all Flavor Tagging Info is saved
        weightFiles = 'B2JpsiKs_mu'

        roe = Belle2.PyStoreObj('RestOfEvent')
        FlavorTaggerInfo = roe.obj().getRelated('FlavorTagInfos')

        qrCombined = 2 * (info.obj().getExtraInfo('qrCombined') - 0.5)
        B0Probability = info.obj().getExtraInfo('qrCombined')
        B0barProbability = 1 - info.obj().getExtraInfo('qrCombined')

        FlavorTaggerInfo.setUseModeFlavorTagger("Expert")
        FlavorTaggerInfo.setMethod("TMVA")
        FlavorTaggerInfo.setQrCombined(qrCombined)
        FlavorTaggerInfo.setB0Probability(B0Probability)
        FlavorTaggerInfo.setB0barProbability(B0barProbability)

        if not FlavorTaggerInfo:
            B2ERROR('FlavorTaggerInfoFiller: FlavorTag does not exist')
            return

        for (particleList, category) in TrackLevelParticleLists:
            # Load the Particle list in Python after the cuts in Track Level
            plist = Belle2.PyStoreObj(particleList)

            # From the likelihood it is possible to have Kaon category with no actual kaons
            if plist.obj().getListSize() == 0:
                FlavorTaggerInfo.setTargetTrackLevel(category, None)
                FlavorTaggerInfo.setProbTrackLevel(category, 0)

            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)  # Pointer to the particle with highest prob
                if mc_variables.variables.evaluate(
                        'hasHighestProbInCat(' + particleList + ',' + 'isRightTrack(' + category + '))',
                        particle) == 1:
                    # Prob of being the right target track
                    targetProb = particle.getExtraInfo('isRightTrack(' + category + ')')
                    track = particle.getTrack()  # Track of the particle
                    FlavorTaggerInfo.setTargetTrackLevel(category, track)
                    FlavorTaggerInfo.setProbTrackLevel(category, targetProb)
                    break

        for (particleList, category) in EventLevelParticleLists:
            # Load the Particle list in Python after the cuts in Event Level
            plist = Belle2.PyStoreObj(particleList)

            # From the likelihood it is possible to have Kaon category with no actual kaons
            if plist.obj().getListSize() == 0:
                FlavorTaggerInfo.setTargetEventLevel(category, None)
                FlavorTaggerInfo.setProbEventLevel(category, 0)
                FlavorTaggerInfo.setQrCategory(category, 0)

            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)  # Pointer to the particle with highest prob
                if mc_variables.variables.evaluate(
                        'hasHighestProbInCat(' + particleList + ',' + 'isRightCategory(' + category + '))',
                        particle) == 1:
                    # Prob of belonging to a cat
                    categoryProb = particle.getExtraInfo('isRightCategory(' + category + ')')
                    track = particle.getTrack()  # Track of the particle
                    qrCategory = mc_variables.variables.evaluate(AvailableCategories[category][3], particle)

                    # Save information in the FlavorTagInfo DataObject
                    FlavorTaggerInfo.setTargetEventLevel(category, track)
                    FlavorTaggerInfo.setProbEventLevel(category, categoryProb)
                    FlavorTaggerInfo.setQrCategory(category, qrCategory)
                    break


class RemoveEmptyROEModule(Module):

    """
    Detects when a ROE does not contain tracks in order to skip it.
    """

    def event(self):
        self.return_value(0)
        roe = Belle2.PyStoreObj('RestOfEvent')
        B0 = roe.obj().getRelated('Particles')
        if mc_variables.variables.evaluate('isRestOfEventEmpty', B0) == -2:
            B2INFO('FlavorTagger: FOUND NO TRACKS IN ROE! COMBINER OUTPUT IS THE DEFAULT -2.'
                   )
            self.return_value(1)


class RemoveWrongMCMatchedROEs(Module):

    """
    Detects when a ROE corresponds to a wrongly reconstructed event although it is not empty.
    This is done checking the MCerror of Breco.
    """

    def event(self):
        self.return_value(0)
        someParticle = Belle2.Particle(None)
        if mc_variables.variables.evaluate('qrCombined', someParticle) < 0:
            B2INFO('FlavorTagger: FOUND NO B-MESON IN ROE! EVENT WILL BE DISCARDED FOR TRAINING!'
                   )
            self.return_value(1)


class RemoveExtraInfoModule(Module):

    """
    Deletes the Extrainfo saved in the used TrackLevelParticle lists.
    """

    def event(self):
        ModeCode = GetModeCode()
        for particleList in EventLevelParticleLists:
            plist = Belle2.PyStoreObj(particleList[0])
            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)
                particle.removeExtraInfo()
        if ModeCode == 1:
            info = Belle2.PyStoreObj('EventExtraInfo')
            info.obj().removeExtraInfo()


def SetModeCode(mode='Expert'):
    """
    Sets ModeCode= 0 for Teacher or =1 for Expert mode.
    """

    global ModeCode
    if mode == 'Expert':
        ModeCode = 1
    else:
        ModeCode = 0


def GetModeCode():
    """
    Gets the global ModeCode.
    """
    global ModeCode
    if ModeCode == 1:
        return 1
    else:
        return 0

# Methods for Track and Event Levels
methods = [
    ('FastBDT', 'Plugin', 'CreateMVAPdfs:NbinsMVAPdf=100:!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')
]

# Methods for Combiner Level
methodsCombiner = [
    ('FastBDT', 'Plugin', 'CreateMVAPdfs:NbinsMVAPdf=300:!H:!V:NTrees=300:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')
]

# SignalFraction: TMVA feature
# For smooth output set to -1, this will break the calibration.
# For correct calibration set to -2, leads to peaky combiner output.
signalFraction = -2

# Definition of all available categories, 'standard category name':
# ['ParticleList', 'TrackLevel category name', 'EventLevel category name',
# 'CombinerLevel variable name', 'category code']
AvailableCategories = {
    'Electron': [
        'e+:ElectronROE',
        'Electron',
        'Electron',
        'QrOf(e+:ElectronROE, isRightCategory(Electron), isRightTrack(Electron))',
        0],
    'IntermediateElectron': [
        'e+:IntermediateElectronROE',
        'IntermediateElectron',
        'IntermediateElectron',
        'QrOf(e+:IntermediateElectronROE, isRightCategory(IntermediateElectron), isRightTrack(IntermediateElectron))',
        1],
    'Muon': [
        'mu+:MuonROE',
        'Muon',
        'Muon',
        'QrOf(mu+:MuonROE, isRightCategory(Muon), isRightTrack(Muon))',
        2],
    'IntermediateMuon': [
        'mu+:IntermediateMuonROE',
        'IntermediateMuon',
        'IntermediateMuon',
        'QrOf(mu+:IntermediateMuonROE, isRightCategory(IntermediateMuon), isRightTrack(IntermediateMuon))',
        3],
    'KinLepton': [
        'mu+:KinLeptonROE',
        'KinLepton',
        'KinLepton',
        'QrOf(mu+:KinLeptonROE, isRightCategory(KinLepton), isRightTrack(KinLepton))',
        4],
    'IntermediateKinLepton': [
        'mu+:IntermediateKinLeptonROE',
        'IntermediateKinLepton',
        'IntermediateKinLepton',
        'QrOf(mu+:IntermediateKinLeptonROE, isRightCategory(IntermediateKinLepton), isRightTrack(IntermediateKinLepton))',
        5],
    'Kaon': [
        'K+:KaonROE',
        'Kaon',
        'Kaon',
        'weightedQrOf(K+:KaonROE, isRightCategory(Kaon), isRightTrack(Kaon))',
        6],
    'SlowPion': [
        'pi+:SlowPionROE',
        'SlowPion',
        'SlowPion',
        'QrOf(pi+:SlowPionROE, isRightCategory(SlowPion), isRightTrack(SlowPion))',
        7],
    'FastPion': [
        'pi+:FastPionROE',
        'FastPion',
        'FastPion',
        'QrOf(pi+:FastPionROE, isRightCategory(FastPion), isRightTrack(FastPion))',
        8],
    'Lambda': [
        'Lambda0:LambdaROE',
        'Lambda',
        'Lambda',
        'weightedQrOf(Lambda0:LambdaROE, isRightCategory(Lambda), isRightTrack(Lambda))',
        9],
    'FSC': [
        'pi+:SlowPionROE',
        'SlowPion',
        'FSC',
        'QrOf(pi+:SlowPionROE, isRightCategory(FSC), isRightTrack(SlowPion))',
        10],
    'MaximumPstar': [
        'pi+:MaximumPstarROE',
        'MaximumPstar',
        'MaximumPstar',
        'QrOf(pi+:MaximumPstarROE, isRightCategory(MaximumPstar), isRightTrack(MaximumPstar))',
        11],
    'KaonPion': [
        'K+:KaonROE',
        'Kaon',
        'KaonPion',
        'QrOf(K+:KaonROE, isRightCategory(KaonPion), isRightTrack(Kaon))',
        12],
}

# Lists for each Step.
TrackLevelParticleLists = []
EventLevelParticleLists = []
variablesCombinerLevel = []
categoriesCombinationCode = 'CatCode'


def WhichCategories(categories=[
    'Electron',
    'Muon',
    'KinLepton',
    'Kaon',
    'SlowPion',
    'FastPion',
    'Lambda',
    'FSC',
    'MaximumPstar',
    'KaonPion',
]):
    """
    Selection of the Categories that are going to be used.
    """

    if len(categories) > 13 or len(categories) < 2:
        B2FATAL('Flavor Tagger: Invalid amount of categories. At least two are needed. No more than 13 are available'
                )
        B2FATAL(
            'Flavor Tagger: Possible categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", '
            '"KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
        return False
    categoriesCombination = []
    for category in categories:
        if category in AvailableCategories:
            if category != 'MaximumPstar' and (AvailableCategories[category][0],
                                               AvailableCategories[category][1]) \
                    not in TrackLevelParticleLists:
                TrackLevelParticleLists.append((AvailableCategories[category][0],
                                                AvailableCategories[category][1]))
            if (AvailableCategories[category][0],
                    AvailableCategories[category][2]) \
                    not in EventLevelParticleLists:
                EventLevelParticleLists.append((AvailableCategories[category][0],
                                                AvailableCategories[category][2]))
                variablesCombinerLevel.append(AvailableCategories[category][3])
                categoriesCombination.append(AvailableCategories[category][4])
            else:
                B2FATAL('Flavor Tagger: ' + category + ' has been already given')
                return False
        else:
            B2FATAL('Flavor Tagger: ' + category + ' is not a valid category name given')
            B2FATAL('Flavor Tagger: Available categories are  "Electron", "IntermediateElectron", '
                    '"Muon", "IntermediateMuon", "KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastPion", '
                    '"Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
            return False
    global categoriesCombinationCode
    for code in sorted(categoriesCombination):
        categoriesCombinationCode = categoriesCombinationCode + '%02d' % code
    return True


# Variables for categories on track level - are defined in variables.cc and MetaVariables.cc
variables = dict()
variables['Electron'] = [
    'useCMSFrame(p)',
    'useCMSFrame(pt)',
    'p',
    'pt',
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(pMissCMS)',
    'SemiLeptonicVariables(cosThetaMissCMS)',
    'SemiLeptonicVariables(EW90)',
    'eid_ARICH',
    'eid_ECL',
    'chiProb',
]
variables['IntermediateElectron'] = variables['Electron']
variables['Muon'] = [
    'useCMSFrame(p)',
    'useCMSFrame(pt)',
    'p',
    'pt',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(pMissCMS)',
    'SemiLeptonicVariables(cosThetaMissCMS)',
    'SemiLeptonicVariables(EW90)',
    'muid_ARICH',
    'chiProb',
]
variables['IntermediateMuon'] = variables['Muon']
variables['KinLepton'] = [
    'useCMSFrame(p)',
    'useCMSFrame(pt)',
    'p',
    'pt',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(pMissCMS)',
    'SemiLeptonicVariables(cosThetaMissCMS)',
    'SemiLeptonicVariables(EW90)',
    'muid_ARICH',
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
    'eid_ECL',
    'chiProb',
]
variables['IntermediateKinLepton'] = variables['KinLepton']
variables['Kaon'] = [
    'useCMSFrame(p)',
    'useCMSFrame(pt)',
    'cosTheta',
    'pt',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'Kid_ARICH',
    'NumberOfKShortinROEParticleList(K_S0:ROEKaon)',
    'ptTracksRoe',
    'distance',
    'chiProb',
]
variables['SlowPion'] = [
    'useCMSFrame(p)',
    'useCMSFrame(pt)',
    'cosTheta',
    'p',
    'pt',
    'piid',
    'piid_dEdx',
    'piid_TOP',
    'piid_ARICH',
    'pi_vs_edEdxid',
    'cosTPTO',
    'Kid',
    'eid',
    'chiProb',
]
variables['FastPion'] = variables['SlowPion']
variables['Lambda'] = [
    'lambdaFlavor',
    'NumberOfKShortinROEParticleList(K_S0:ROELambda)',
    'M',
    'cosAngleBetweenMomentumAndVertexVector',
    'lambdaZError',
    'daughter(1,p)',
    'daughter(1,useCMSFrame(p))',
    'useCMSFrame(p)',
    'p',
    'chiProb',
    'distance',
]

# Only for Event Level
variables['KaonPion'] = ['HighestProbInCat(K+:KaonROE, isRightTrack(Kaon))',
                         'HighestProbInCat(pi+:SlowPionROE, isRightTrack(SlowPion))',
                         'KaonPionVariables(cosKaonPion)', 'KaonPionVariables(HaveOpositeCharges)', 'Kid']

variables['MaximumPstar'] = [
    'useCMSFrame(p)',
    'useCMSFrame(pt)',
    'p',
    'pt',
    'cosTPTO',
    'ImpactXY',
]

variables['FSC'] = [
    'useCMSFrame(p)',
    'cosTPTO',
    'Kid',
    'FSCVariables(pFastCMS)',
    'FSCVariables(cosSlowFast)',
    'FSCVariables(cosTPTOFast)',
    'FSCVariables(SlowFastHaveOpositeCharges)',
]


def FillParticleLists(mode='Expert', path=analysis_main):
    """
    Fills the particle Lists for each category.
    """

    for (particleList, category) in TrackLevelParticleLists:

        # Select particles in ROE for different categories of flavour tagging.
        if particleList != 'Lambda0:LambdaROE':

            # Filling particle list for actual category
            fillParticleList(particleList, 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)

        # Check if there is K short in this event
        if particleList == 'K+:KaonROE':
            # Precut done to prevent from overtraining, might be redundant
            applyCuts(particleList, '0.1<Kid', path=path)
            fillParticleList('pi+:inKaonRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1',
                             path=path)
            reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe',
                             '0.40<=M<=0.60', True, path=path)
            fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=path)

        if particleList == 'Lambda0:LambdaROE':
            fillParticleList('pi+:inLambdaRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1',
                             path=path)
            fillParticleList('p+:inLambdaRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1',
                             path=path)
            reconstructDecay(particleList + ' -> pi-:inLambdaRoe p+:inLambdaRoe',
                             '1.00<=M<=1.23', True, path=path)
            reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe', '0.40<=M<=0.60', True, path=path)
            fitVertex(particleList, 0.01, fitter='kfitter', path=path)
            if mode != 'Expert':
                matchMCTruth(particleList, path=path)
            fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=path)

    # Filling 'pi+:MaximumPstarROE' particle list
    fillParticleList('pi+:MaximumPstarROE', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)

    return True


def TrackLevel(mode='Expert', weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods', path=analysis_main):
    """
    Trains or tests all categories at track level except KaonPion, MaximumPstar and FSC which are only at the event level.
    """

    B2INFO('TRACK LEVEL')
    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('FlavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    ReadyMethods = 0

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    TrackLevelPathsList = dict()

    for (particleList, category) in TrackLevelParticleLists:

        TrackLevelPath = category + "TrackLevelPath"

        exec('%s = %s' % (TrackLevelPath, 'create_path()'))
        exec('TrackLevelPathsList["' + category + '"]=%s' % TrackLevelPath)

        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.param('particleLists', particleList)
        SkipEmptyParticleList.if_true(TrackLevelPathsList[category], AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        methodPrefixTrackLevel = weightFiles + 'TrackLevel' + category + 'TMVA'
        targetVariable = 'isRightTrack(' + category + ')'

        if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefixTrackLevel):
            if mode == 'Expert':
                B2FATAL(
                    'Flavor Tagger: ' +
                    particleList +
                    ' Tracklevel was not trained. Weight file ' +
                    methodPrefixTrackLevel +
                    '_1.config not found. Stopped')
            else:
                B2INFO('PROCESSING: trainTMVAMethod ' + methodPrefixTrackLevel + ' on track level')
                trainTMVAMethod(
                    particleList,
                    variables=variables[category],
                    target=targetVariable,
                    methods=methods,
                    prefix=methodPrefixTrackLevel,
                    workingDirectory=workingDirectory,
                    path=TrackLevelPathsList[category],
                )
        else:
            B2INFO('PROCESSING: applyTMVAMethod ' + methodPrefixTrackLevel + ' on track level')
            applyTMVAMethod(
                particleList,
                prefix=methodPrefixTrackLevel,
                expertOutputName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=TrackLevelPathsList[category],
            )
            ReadyMethods += 1

    if ReadyMethods != len(TrackLevelParticleLists):
        return False
    else:
        return True


def EventLevel(mode='Expert', weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods', path=analysis_main):
    """
    Trains or tests all categories at event level.
    """

    B2INFO('EVENT LEVEL')

    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('FlavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    ReadyMethods = 0

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    EventLevelPathsList = dict()

    for (particleList, category) in EventLevelParticleLists:

        EventLevelPath = category + "EventLevelPath"
        exec('%s = %s' % (EventLevelPath, 'create_path()'))
        exec('EventLevelPathsList["' + category + '"]=%s' % EventLevelPath)

        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.param('particleLists', particleList)
        SkipEmptyParticleList.if_true(EventLevelPathsList[category], AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        methodPrefixEventLevel = weightFiles + 'EventLevel' + category + 'TMVA'
        targetVariable = 'isRightCategory(' + category + ')'

        if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefixEventLevel):
            if mode == 'Expert':
                B2FATAL(
                    'Flavor Tagger: ' +
                    particleList +
                    ' Eventlevel was not trained. Weight file ' +
                    methodPrefixEventLevel +
                    '_1.config not found. Stopped')
            else:
                B2INFO('PROCESSING: trainTMVAMethod ' + methodPrefixEventLevel + ' on event level')
                trainTMVAMethod(
                    particleList,
                    variables=variables[category],
                    target=targetVariable,
                    prefix=methodPrefixEventLevel,
                    methods=methods,
                    workingDirectory=workingDirectory,
                    path=EventLevelPathsList[category],
                )
        else:
            # if category == 'KinLepton':
                # applyCuts(particleList, 'isInElectronOrMuonCat < 0.5', path=path)
            B2INFO('PROCESSING: applyTMVAMethod ' + methodPrefixEventLevel + ' on event level')
            applyTMVAMethod(
                particleList,
                prefix=methodPrefixEventLevel,
                expertOutputName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=EventLevelPathsList[category],
            )

            ReadyMethods += 1

    if ReadyMethods != len(EventLevelParticleLists):
        return False
    else:
        return True


def CombinerLevel(mode='Expert', weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods',
                  path=analysis_main):
    """
    Trains or tests the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL')

    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('FlavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    methodPrefixCombinerLevel = weightFiles + 'CombinerLevel' \
        + categoriesCombinationCode + 'TMVA'
    if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefixCombinerLevel):
        if mode == 'Expert':
            B2FATAL('FlavorTagger: Combinerlevel was not trained with this combination of categories. Weight file ' +
                    methodPrefixCombinerLevel + '_1.config not found. Stopped')
        else:
            B2INFO('Train TMVAMethod on combiner level')
            trainTMVAMethod(
                [],
                variables=variablesCombinerLevel,
                target='qrCombined',
                prefix=methodPrefixCombinerLevel,
                methods=methodsCombiner,
                workingDirectory=workingDirectory,
                path=path,
            )
    else:
        B2INFO('FlavorTagger: Ready to be used with weightFiles' +
               weightFiles + '. The training process has been finished.')
        if mode == 'Expert':
            B2INFO('Apply TMVAMethod on combiner level')
            applyTMVAMethod(
                [],
                expertOutputName='qrCombined',
                prefix=methodPrefixCombinerLevel,
                signalClass=1,
                method=methodsCombiner[0][0],
                signalFraction=-1,
                transformToProbability=False,
                workingDirectory=workingDirectory,
                path=path,
            )
        else:
            B2FATAL('FlavorTagger: Combinerlevel was already trained with this combination of categories. Weight file ' +
                    methodPrefixCombinerLevel + '_1.config has been found. Please use the "Expert" mode')

        return True


def FlavorTagger(
    mode='Expert',
    weightFiles='B2JpsiKs_mu',
    workingDirectory='.',
    method='TMVA',
    categories=[
        'Electron',
        'IntermediateElectron',
        'Muon',
        'IntermediateMuon',
        'KinLepton',
        'IntermediateKinLepton',
        'Kaon',
        'SlowPion',
        'FastPion',
        'Lambda',
        'FSC',
        'MaximumPstar',
        'KaonPion'],
    path=analysis_main,
):
    """
      Defines the whole flavor tagging process.
      For each Rest of Event built in the steering file.
      The flavor is predicted by Multivariate Methods trained with Variables and MetaVariables which use
      Tracks, ECL- and KLMClusters from the corresponding RestOfEvent dataobject.
      This function can be used to train or to test the FlavorTagger: The available modes are "Teacher" or "Expert".
    """

    if mode != 'Teacher' and mode != 'Expert':
        B2FATAL('FlavorTagger: Wrong mode given: The available modes are "Teacher" or "Expert"')

    # Directory where the weights of the trained Methods are saved
    # workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'

    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('FlavorTagger: THE GIVEN WORKING DIRECTORY "' + workingDirectory + '" DOES NOT EXIST! PLEASE SPECIFY A VALID PATH.')

    if mode == 'Teacher':
        if not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging'):
            os.mkdir(workingDirectory + '/FlavorTagging')
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')
        elif not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging/TrainedMethods'):
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')

    workingDirectory = workingDirectory + '/FlavorTagging/TrainedMethods'

    B2INFO('*** FLAVOR TAGGING ***')
    B2INFO(' ')
    B2INFO('    Working directory is: ' + workingDirectory)
    B2INFO(' ')

    SetModeCode(mode)

    roe_path = create_path()
    emptypath = create_path()

    # If trigger returns 1 jump into empty path skipping further modules in roe_path
    ROEEmptyTrigger = RemoveEmptyROEModule()
    roe_path.add_module(ROEEmptyTrigger)
    ROEEmptyTrigger.if_true(emptypath)

    # Events containing ROE without B-Meson (but not empty) are discarded for training
    if mode == 'Teacher':
        RemoveWrongROEs = RemoveWrongMCMatchedROEs()
        roe_path.add_module(RemoveWrongROEs)
        RemoveWrongROEs.if_true(emptypath)

    # track training or expert
    if WhichCategories(categories):
        if FillParticleLists(mode, roe_path):
            if TrackLevel(mode, weightFiles, workingDirectory, roe_path):
                if EventLevel(mode, weightFiles, workingDirectory, roe_path):
                    CombinerLevel(mode, weightFiles, workingDirectory, roe_path)

    if mode == 'Expert':
            # Initialation of FlavorTagInfo dataObject needs to be done in the main path
        FlavorTagInfoBuilder = register_module('FlavorTagInfoBuilder')
        path.add_module(FlavorTagInfoBuilder)
        roe_path.add_module(FlavorTaggerInfoFiller())  # Add FlavorTag Info filler to roe_path

    # Removes EventExtraInfos and ParticleExtraInfos of the EventParticleLists
    roe_path.add_module(RemoveExtraInfoModule())

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
