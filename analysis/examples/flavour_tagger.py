#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck                 *
# * The whole flavor tagging can be done in  *
# * this script.                             *
# ********************************************

from basf2 import *
from modularAnalysis import *
import variables as mc_variables
from ROOT import Belle2
import os

main = create_path()
# ***************************************
roinput = register_module('RootInput')
inputFiles = []
for i in xrange(20):
    inputFiles.append('/remote/pcbelle06/ligioi/dstFiles/B2JpsiKs_mu_e0001r'
                      + '%04d' % (i + 1) + '_s00_BGx0.mdst.root')
roinput.param('inputFileNames', inputFiles)
main.add_module(roinput)
# ***************************************
# main.add_module(register_module('RootInput'))
main.add_module(register_module('Gearbox'))
loadReconstructedParticles(path=main)

# Signal side B_sig
selectParticle('mu+', 'muid >= 0.1', path=main)
selectParticle('pi+', 'piid >= 0.1', path=main)

reconstructDecay('J/psi -> mu+ mu-', '3.0<=M<=3.2', 1, path=main)
reconstructDecay('K_S0 -> pi+ pi-', '0.25<=M<=0.75', 1, path=main)
reconstructDecay('B0 -> K_S0 J/psi', '5.2 <= M <= 5.4', 1, path=main)

# Use only correctly reconstructed B_sig
matchMCTruth('B0', path=main)
applyCuts('B0', 'isSignal > 0.5', path=main)

# Tag side B_tag
buildRestOfEvent('B0', path=main)
buildContinuumSuppression('B0', path=main)
roe_path = create_path()

# Variables for categories on track level - are defined in variables.cc and MetaVariables.cc
variables = dict()
variables['Electron'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(p_missing_CMS)',
    'SemiLeptonicVariables(CosTheta_missing_CMS)',
    'SemiLeptonicVariables(EW90)',
    'eid_ARICH',
    'eid_ECL',
    ]
variables['IntermediateElectron'] = variables['Electron']
variables['Muon'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(p_missing_CMS)',
    'SemiLeptonicVariables(CosTheta_missing_CMS)',
    'SemiLeptonicVariables(EW90)',
    'muid_ARICH',
    ]
variables['IntermediateMuon'] = variables['Muon']
variables['KinLepton'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(p_missing_CMS)',
    'SemiLeptonicVariables(CosTheta_missing_CMS)',
    'SemiLeptonicVariables(EW90)',
    'muid_ARICH',
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
    'eid_ECL',
    ]
variables['Kaon'] = [
    'p_CMS',
    'pt_CMS',
    'cosTheta',
    'pt',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'Kid_ARICH',
    'charge',
    'NumberOfKShortinRemainingROEKaon',
    'ptTracksRoe',
    'distance',
    ]
variables['SlowPion'] = [
    'p_CMS',
    'pt_CMS',
    'cosTheta',
    'p',
    'pt',
    'piid',
    'piid_dEdx',
    'piid_TOP',
    'piid_ARICH',
    'pi_vs_edEdxid',
    'cosTPTO',
    'charge',
    'Kid',
    'eid',
    ]
variables['FastPion'] = variables['SlowPion']
variables['Lambda'] = [
    'lambdaFlavor',
    'NumberOfKShortinRemainingROELambda',
    'M',
    'cosAngleBetweenMomentumAndVertexVector',
    'lambdaZError',
    'MomentumOfSecondDaughter',
    'MomentumOfSecondDaughter_CMS',
    'p_CMS',
    'p',
    'chiProb',
    'distance',
    ]
# distance variable: we assume B0 and daughter of B0 lifetimes (Lambda_c ) are much shorter than Lambda0 lifetime.

# Please choose method:
methods = [('FastBDT', 'Plugin',
           '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
           )]
# methods = [("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10")]
# methods = [("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2")]
# methods = [("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T")]

# Directory to save the weights and the .config files
workingDirectory = os.environ['BELLE2_LOCAL_DIR'] \
    + '/analysis/data/FlavorTagging/TrainedMethods'

print 'FLAVOR TAGGING'
print 'Working directory is: ' + workingDirectory

trackLevelReady = True
trackLevelParticles = [
    ('e+', 'Electron'),
    ('e+', 'IntermediateElectron'),
    ('mu+', 'Muon'),
    ('mu+', 'IntermediateMuon'),
    ('mu+', 'KinLepton'),
    ('K+', 'Kaon'),
    ('pi+', 'SlowPion'),
    ('pi+', 'FastPion'),
    ('Lambda0', 'Lambda'),
    ]
# SignalFraction to calculate probability, -2 if training signal/background ratio should be used
signalFraction = -2

for (symbol, category) in trackLevelParticles:
    particleList = symbol + ':ROE'
    methodPrefix_TL = 'TMVA_' + category + '_TL'
    targetVariable = 'IsFromB(' + category + ')'

    # Select particles in ROE for different categories of flavour tagging.
    if symbol != 'Lambda0':
        selectParticle(particleList,
                       'isInRestOfEvent > 0.5 and chiProb > 0.001',
                       path=roe_path)
    # Check if there is K short in this event
    if symbol == 'K+':
        applyCuts('K+:ROE', '0.1<Kid', path=roe_path)  # Precut done to prevent from overtraining, might be redundant
        selectParticle('pi+:inKaonRoe',
                       'isInRestOfEvent > 0.5 and chiProb > 0.001',
                       path=roe_path)
        reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe',
                         '0.40<=M<=0.60', 1, path=roe_path)
        fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=roe_path)
        # modify confidence level?!

    if symbol == 'Lambda0':
        selectParticle('pi+:inLambdaRoe',
                       'isInRestOfEvent > 0.5 and chiProb > 0.001',
                       path=roe_path)
        selectParticle('p+:inLambdaRoe',
                       'isInRestOfEvent > 0.5 and chiProb > 0.001',
                       path=roe_path)
        reconstructDecay('Lambda0:ROE -> pi-:inLambdaRoe p+:inLambdaRoe',
                         '1.00<=M<=1.23', 1, path=roe_path)
        reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe',
                         '0.40<=M<=0.60', 1, path=roe_path)
        fitVertex('Lambda0:ROE', 0.01, fitter='kfitter', path=roe_path)
        matchMCTruth('Lambda0:ROE', path=roe_path)
        fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=roe_path)
        # printList('Lambda0:ROE', full=True, path=roe_path)

    # How to sextract info about the number of selected Dummy particles, i.e. about the selected Tracks here. (We need this for each category)

    if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_TL):
        print isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_TL)
        print workingDirectory + '/' + methodPrefix_TL
        print 'PROCESSING: trainTMVAMethod on track level'
        trainTMVAMethod(
            particleList,
            variables=variables[category],
            target=targetVariable,
            methods=methods,
            prefix=methodPrefix_TL,
            workingDirectory=workingDirectory,
            path=roe_path,
            )
        trackLevelReady = False
    else:
        print 'PROCESSING: applyTMVAMethod on track level'
        applyTMVAMethod(
            particleList,
            prefix=methodPrefix_TL,
            signalProbabilityName=targetVariable,
            method=methods[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,
            )

eventLevelReady = trackLevelReady

# Eventlevel -> calculation only on targettrack
# eventLevelParticles = [('e+', 'Electron'), ('mu+', 'Muon'), ('K+', 'Kaon'),
                       # ('pi+', 'SlowPion'), ('Lambda0', 'Lambda')]

variables['KaonPion'] = ['HighestProbInCat(K+:ROE, IsFromB(Kaon))',
                         'HighestProbInCat(pi+:ROE, IsFromB(SlowPion))',
                         'cosKaonPion', 'KaonPionHaveOpositeCharges', 'Kid']

eventLevelParticles = [
    ('e+', 'Electron'),
    ('e+', 'IntermediateElectron'),
    ('mu+', 'Muon'),
    ('mu+', 'IntermediateMuon'),
    ('mu+', 'KinLepton'),
    ('K+', 'Kaon'),
    ('pi+', 'SlowPion'),
    ('pi+', 'FastPion'),
    ('K+', 'KaonPion'),
    ('Lambda0', 'Lambda'),
    ]

if eventLevelReady:
    for (symbol, category) in eventLevelParticles:
        particleList = symbol + ':ROE'
        methodPrefix_EL = 'TMVA_' + category + '_EL'
        # Using MetaVariable
        targetVariable = 'IsRightClass(' + category + ')'
        print 'This is the targetVariable: ' + targetVariable

        # if category == 'KinLepton':
      # selectParticle(particleList,
                       # 'isInElectronOrMuonCat < 0.5',
                       # path=roe_path)
        if category == 'KaonPion':
            selectParticle(particleList, 'hasHighestProbInCat(' + particleList
                           + ',' + 'IsFromB(Kaon)) > 0.5', path=roe_path)
        else:
            selectParticle(particleList, 'hasHighestProbInCat(' + particleList
                           + ',' + 'IsFromB(' + category + ')) > 0.5',
                           path=roe_path)

        if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_EL):
            print 'PROCESSING: trainTMVAMethod on event level'
            trainTMVAMethod(
                particleList,
                variables=variables[category],
                target=targetVariable,
                prefix=methodPrefix_EL,
                methods=methods,
                workingDirectory=workingDirectory,
                path=roe_path,
                )
            eventLevelReady = False
        else:
            print 'PROCESSING: applyTMVAMethod on event level'
            applyTMVAMethod(
                particleList,
                prefix=methodPrefix_EL,
                signalProbabilityName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=roe_path,
                )


class RemoveExtraInfoModule(Module):

    """
    Removes the variables added as Extrainfo to the track level particles 
    """

    def event(self):
        for (symbol, _) in trackLevelParticles:
            plist = Belle2.PyStoreObj(symbol + ':ROE')
            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)
                particle.removeExtraInfo()


combinerLevelReady = eventLevelReady

if combinerLevelReady:
    variables = [
        'QrOf(e+:ROE, IsRightClass(Electron), IsFromB(Electron))',
        'QrOf(e+:ROE, IsRightClass(IntermediateElectron), IsFromB(IntermediateElectron))'
            ,
        'QrOf(mu+:ROE, IsRightClass(Muon), IsFromB(Muon))',
        'QrOf(mu+:ROE, IsRightClass(IntermediateMuon), IsFromB(IntermediateMuon))'
            ,
        'QrOf(mu+:ROE, IsRightClass(KinLepton), IsFromB(KinLepton))',
        'QrOf(K+:ROE, IsRightClass(Kaon), IsFromB(Kaon))',
        'QrOf(pi+:ROE, IsRightClass(SlowPion), IsFromB(SlowPion))',
        'QrOf(pi+:ROE, IsRightClass(FastPion), IsFromB(FastPion))',
        'QrOf(K+:ROE, IsRightClass(KaonPion), IsFromB(Kaon))',
        'QrOf(Lambda0:ROE, IsRightClass(Lambda), IsFromB(Lambda))',
        ]

    method_Combiner = [('FastBDT', 'Plugin',
                       '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
                       )]

    if not isTMVAMethodAvailable(workingDirectory + '/' + 'B0Tagger'):
        print 'PROCESSING: trainTMVAMethod on combiner level'
        trainTMVAMethod(
            [],
            variables=variables,
            target='qr_Combined',
            prefix='B0Tagger',
            methods=method_Combiner,
            workingDirectory=workingDirectory,
            path=roe_path,
            )
        combinerLevelReady = False
    else:
        print 'PROCESSING: applyTMVAMethod on combiner level'
        applyTMVAMethod(
            [],
            signalProbabilityName='qr_Combined',
            prefix='B0Tagger',
            signalClass=1,
            method=method_Combiner[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,
            )

if combinerLevelReady:


    class MoveTaggerInformationToBExtraInfoModule(Module):

        """
        Adds the flavor tagging information (q*r) from the MC and from the Combiner as ExtraInfo to the reconstructed B0 particle 
...."""

        def event(self):
            roe = Belle2.PyStoreObj('RestOfEvent')
            info = Belle2.PyStoreObj('EventExtraInfo')
            someParticle = Belle2.Particle(None)
            qr_Combined = 2 * (info.obj().getExtraInfo('qr_Combined') - 0.5)
            qr_MC = 2 * (mc_variables.variables.evaluate('qr_Combined',
                         someParticle) - 0.5)
            ROE_NTracks = roe.obj().getNTracks()
            ROE_NECLClusters = roe.obj().getNECLClusters()
            ROE_NKLMClusters = roe.obj().getNKLMClusters()
            particle = roe.obj().getRelated('Particles')
            particle.addExtraInfo('qr_Combined', qr_Combined)
            particle.addExtraInfo('qr_MC', qr_MC)
            particle.addExtraInfo('ROE_NTracks', ROE_NTracks)
            particle.addExtraInfo('ROE_NECLClusters', ROE_NECLClusters)
            particle.addExtraInfo('ROE_NKLMClusters', ROE_NKLMClusters)
            info.obj().removeExtraInfo()


    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())  # Move and remove extraInfo
    roe_path.add_module(RemoveExtraInfoModule())
else:
    roe_path.add_module(RemoveExtraInfoModule())  # Only remove extra info

main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

if combinerLevelReady:
    variablesToNTuple('B0', ['getExtraInfo(qr_Combined)', 'getExtraInfo(qr_MC)'
                      , 'getExtraInfo(ROE_NTracks)',
                      'getExtraInfo(ROE_NECLClusters)',
                      'getExtraInfo(ROE_NKLMClusters)'], 'TaggingInformation',
                      workingDirectory + '/B0_B0bar_final.root', path=main)

main.add_module(register_module('ProgressBar'))
process(main)
print statistics

B2INFO('')
B2INFO("Training completed. Run 'show_tmva_results TMVA_1_vs_0.root' to view detailed information about the trained methods."
       )
