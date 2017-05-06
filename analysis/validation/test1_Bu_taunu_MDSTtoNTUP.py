#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Validation script for B --> tau nu
# FEI B-tag reconstruction, with hadronic and semileptonic tag
#
# Contributors: M. Merola (April 2017)
#
######################################################
import sys
import glob
import os.path

from basf2 import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from reconstruction import add_mdst_output
from modularAnalysis import *
from stdPhotons import *
from stdPi0s import *


def remove_module(path, name):
    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path


from ROOT import Belle2

# To access the FEI training weight files (FEI trained on MC7, also available
# at KEKCC /home/belle2/tkeck/fei/Belle2_Generic_2017_Track14_1/ )
use_central_database('test_merola', LogLevel.WARNING, 'fei_database')

gb2_setuprel = "release-00-08-00"


from variables import variables

variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
variables.addAlias('daugSigProbProduct', 'daughterProductOf(extraInfo(SignalProbability))')
variables.addAlias('rank', 'extraInfo(sigProb_rank)')
variables.addAlias('rhodM', 'daughter(0,abs(formula(M-0.775)))')
variables.addAlias('dmID', 'extraInfo(decayModeID)')
variables.addAlias('uniqueSignal', 'extraInfo(uniqueSignal)')
variables.addAlias('d0_dmID', 'daughter(0,extraInfo(decayModeID))')
variables.addAlias('d1_dmID', 'daughter(1,extraInfo(decayModeID))')
variables.addAlias('d0_d0_dmID', 'daughter(0,daughter(0,extraInfo(decayModeID)))')
variables.addAlias('d0_d1_dmID', 'daughter(0,daughter(1,extraInfo(decayModeID)))')
variables.addAlias('d1_d0_dmID', 'daughter(1,daughter(0,extraInfo(decayModeID)))')
variables.addAlias('d1d2_M', 'daughterInvariantMass(1,2)')
variables.addAlias('d1d3_M', 'daughterInvariantMass(1,3)')
variables.addAlias('d1d4_M', 'daughterInvariantMass(1,4)')
variables.addAlias('d2d3_M', 'daughterInvariantMass(2,3)')
variables.addAlias('d2d4_M', 'daughterInvariantMass(2,4)')
variables.addAlias('d3d4_M', 'daughterInvariantMass(3,4)')
variables.addAlias('d1d2d3_M', 'daughterInvariantMass(1,2,3)')
variables.addAlias('d2d3d4_M', 'daughterInvariantMass(2,3,4)')
variables.addAlias('d1d2d3d4_M', 'daughterInvariantMass(1,2,3,4)')

# Momenta
variables.addAlias('d0_p', 'daughter(0,p)')
variables.addAlias('d0_pCMS', 'daughter(0,useCMSFrame(p))')
variables.addAlias('d1_p', 'daughter(1,p)')
variables.addAlias('d2_p', 'daughter(2,p)')
variables.addAlias('d3_p', 'daughter(3,p)')
variables.addAlias('d0_d0_p', 'daughter(0,daughter(0,p))')
variables.addAlias('d0_d0_pCMS', 'daughter(0,daughter(0,useCMSFrame(p)))')
variables.addAlias('d0_d1_pCMS', 'daughter(0,daughter(1,useCMSFrame(p)))')
variables.addAlias('d0_d1_p', 'daughter(0,daughter(1,p))')
variables.addAlias('d0_d2_p', 'daughter(0,daughter(2,p))')
variables.addAlias('d0_d3_p', 'daughter(0,daughter(3,p))')
variables.addAlias('d0_d1_d0_p', 'daughter(0,daughter(1,daughter(0,p)))')
variables.addAlias('d0_d1_d1_p', 'daughter(0,daughter(1,daughter(1,p)))')
variables.addAlias('d0_d1_d0_pCMS', 'daughter(0,daughter(1,daughter(0,useCMSFrame(p))))')
variables.addAlias('d0_d1_d1_pCMS', 'daughter(0,daughter(1,daughter(1,useCMSFrame(p))))')
variables.addAlias('d0_d0_d2_p', 'daughter(0,daughter(0,daughter(2,p)))')
variables.addAlias('d0_d0_d3_p', 'daughter(0,daughter(0,daughter(3,p)))')

variables.addAlias('d0_d0_M', 'daughter(0,daughter(0,InvM))')
variables.addAlias('d0_d1_M', 'daughter(0,daughter(1,InvM))')
variables.addAlias('d0_d0_d1_M', 'daughter(0,daughter(0,daughter(1,InvM)))')
variables.addAlias('d0_M', 'daughter(0,InvM)')
variables.addAlias('d1_M', 'daughter(1,InvM)')

variables.addAlias('d0_d0_costheta', 'daughter(0,daughter(0,cosTheta))')
variables.addAlias('d0_costheta', 'daughter(0,cosTheta)')
variables.addAlias('d0_costhetaCMS', 'daughter(0,useCMSFrame(cosTheta))')
variables.addAlias('d0_d0_costhetaCMS', 'daughter(0,daughter(0,useCMSFrame(cosTheta)))')
variables.addAlias('d0_d1_costhetaCMS', 'daughter(0,daughter(1,useCMSFrame(cosTheta)))')
variables.addAlias('d0_d1_d0_costhetaCMS', 'daughter(0,daughter(1,daughter(0,useCMSFrame(cosTheta))))')
variables.addAlias('d0_d1_d1_costhetaCMS', 'daughter(0,daughter(1,daughter(1,useCMSFrame(cosTheta))))')
variables.addAlias('d1_costheta', 'daughter(1,cosTheta)')

variables.addAlias('d0_d0_E', 'daughter(0,daughter(0,E))')
variables.addAlias('d0_d1_E', 'daughter(0,daughter(1,E))')
variables.addAlias('d0_E', 'daughter(0,E)')
variables.addAlias('d1_E', 'daughter(1,E)')

variables.addAlias('d0_deltaE', 'daughter(0,deltaE)')
variables.addAlias('d0_Mbc', 'daughter(0,Mbc)')
variables.addAlias('d1_deltaE', 'daughter(1,deltaE)')
variables.addAlias('d1_Mbc', 'daughter(1,Mbc)')


# PDG ID
variables.addAlias('d0_PDG', 'daughter(0,PDG)')
variables.addAlias('d1_PDG', 'daughter(1,PDG)')
variables.addAlias('d2_PDG', 'daughter(2,PDG)')
variables.addAlias('d3_PDG', 'daughter(3,PDG)')
variables.addAlias('d0_d0_PDG', 'daughter(0,daughter(0,PDG))')
variables.addAlias('d0_d1_PDG', 'daughter(0,daughter(1,PDG))')
variables.addAlias('d0_d2_PDG', 'daughter(0,daughter(2,PDG))')
variables.addAlias('d0_d3_PDG', 'daughter(0,daughter(3,PDG))')
variables.addAlias('d0_d1_d0_PDG', 'daughter(0,daughter(1,daughter(0,PDG)))')
variables.addAlias('d0_d1_d1_PDG', 'daughter(0,daughter(1,daughter(1,PDG)))')
variables.addAlias('d0_d0_d2_PDG', 'daughter(0,daughter(0,daughter(2,PDG)))')
variables.addAlias('d0_d0_d3_PDG', 'daughter(0,daughter(0,daughter(3,PDG)))')


variables.addAlias('d0_mcPDG', 'daughter(0,mcPDG)')
variables.addAlias('d1_mcPDG', 'daughter(1,mcPDG)')
variables.addAlias('d1_d0_mcPDG', 'daughter(1,daughter(0,mcPDG))')
variables.addAlias('d1_d0_d0_mcPDG', 'daughter(1,daughter(0,daughter(0,mcPDG)))')
variables.addAlias('d1_d0_d1_mcPDG', 'daughter(1,daughter(0,daughter(1,mcPDG)))')

variables.addAlias('tau_pi0_gamma1_clusterTiming', 'daughter(0, daughter(1,daughter(0,clusterTiming)))')
variables.addAlias('tau_pi0_gamma2_clusterTiming', 'daughter(0, daughter(1,daughter(1,clusterTiming)))')
variables.addAlias('tau_pi0_gamma1_clusterTheta', 'daughter(0, daughter(1,daughter(0,clusterTheta)))')
variables.addAlias('tau_pi0_gamma2_clusterTheta', 'daughter(0, daughter(1,daughter(1,clusterTheta)))')
variables.addAlias('tau_pi0_gamma1_E', 'daughter(0, daughter(1,daughter(0,E)))')
variables.addAlias('tau_pi0_gamma2_E', 'daughter(0, daughter(1,daughter(1,E)))')


# PID
variables.addAlias('tau_DLLKaon', 'daughter(0, daughter(0,DLLKaon))')
variables.addAlias('tau_DLLMuon', 'daughter(0, daughter(0,DLLMuon))')
variables.addAlias('tau_DLLElec', 'daughter(0, daughter(0,DLLElec))')
variables.addAlias('tau_DLLPion', 'daughter(0, daughter(0,DLLPion))')
variables.addAlias('tau_muid', 'daughter(0, daughter(0,muid))')
variables.addAlias('tau_eid', 'daughter(0, daughter(0,eid))')
variables.addAlias('tau_piid', 'daughter(0, daughter(0,piid))')
variables.addAlias('tau_Kid', 'daughter(0, daughter(0,Kid))')


outputRootFile = "taunu_validation.root"

path = create_path()

# load the FEI reconstruction path
# this has to be included in the input sandbox
# it is located at /home/belle2/tkeck/fei/Belle2_Generic_2017_Track14_1/paths/
fei_pickle = 'basf2_final_path_without_selection.pickle'
if not os.path.isfile(fei_pickle):
    sys.exit(
        'basf2_path.pickle not found at: ' +
        fei_pickle +
        '\n'
        'Please provide the fei_path.pickle file from an existing FEI training by setting the fei_pickle parameter in this script.')

fei_path = get_path_from_file(fei_pickle)
path.add_path(fei_path)

filelist = [""]

inputMdstList('default', filelist)

# the skim condition is TRUE for all events
skimALL = register_module('VariableToReturnValue')
skimALL.param("variable", 'True')
skimALL.if_value('==1', path, AfterConditionPath.CONTINUE)
analysis_main.add_module(skimALL)


# apply some very loose cuts to reduce the number
# of Btag candidates
applyCuts('B+:generic', 'Mbc>5.22 and abs(deltaE)<0.200 and sigProb>0.001')
applyCuts('B+:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.001')

# rank Btag canidates according to their SignalProbability
# 'sigProb' is alias for 'extraInfo(SignalProbability)'
rankByHighest('B+:generic', 'sigProb')
rankByHighest('B+:semileptonic', 'sigProb')


cutAndCopyList('B+:genericRank', 'B+:generic', 'rank==1')
cutAndCopyList('B+:semileptonicRank', 'B+:semileptonic', 'rank==1')


# MC7 && 8 - 95% eff working points
electrons = ('e-:loose', 'p>0.01 and eid>0.510')
pions = ('pi-:loose', 'p>0.01 and piid > 0.437')
muons = ('mu-:loose', 'p>0.01 and muid > 0.01')  # WP not available
kaons = ('K-:loose', 'p>0.01 and Kid > 0.293')


stdPhotons('loose')

photoncuts1 = '[E > 0.058 and abs(clusterTiming)<18 and clusterReg==1] or [E > 0.062 and abs(clusterTiming)<21 '
photoncuts2 = 'and clusterReg==2] or [E > 0.040 and abs(clusterTiming)<38 and clusterReg==3]'
photoncuts = photoncuts1 + photoncuts2

cutAndCopyList('gamma:mysel', 'gamma:loose', photoncuts)


fillParticleLists([muons, kaons, pions, electrons])

reconstructDecay('pi0 -> gamma:mysel gamma:mysel', '0.11 < M < 0.16', 1, True)

reconstructDecay('tau-:ch1 -> mu-:loose', '', 3, True)
reconstructDecay('tau-:ch2 -> e-:loose', '', 4, True)
reconstructDecay('tau-:ch3 -> pi-:loose', '', 5, True)
reconstructDecay('tau-:ch4 -> pi-:loose pi0', '0.625 < M < 0.925', 6, True)

copyLists('tau-', ['tau-:ch1', 'tau-:ch2', 'tau-:ch3', 'tau-:ch4'])

reconstructDecay('B-:taunu -> tau- ', '0.000 < M < 6.000', 12, True)

reconstructDecay('Upsilon(4S):BhadBtaunu -> B+:genericRank B-:taunu', '')
reconstructDecay('Upsilon(4S):BslBtaunu -> B+:semileptonicRank B-:taunu', '')

rankByLowest('Upsilon(4S):BhadBtaunu', 'formula(daughter(1,rhodM))', 0, 'rankY4StaunuHad')
rankByLowest('Upsilon(4S):BslBtaunu', 'formula(daughter(1,rhodM))', 0, 'rankY4SlnuSl')

buildRestOfEvent('Upsilon(4S):BhadBtaunu')
buildRestOfEvent('Upsilon(4S):BslBtaunu')


buildRestOfEvent('B+:genericRank')
buildRestOfEvent('B+:semileptonicRank')


ROETracks = ('ROETracks', '', '')
ROEClustersCuts1 = '[clusterE1E9 > 0.4 or E > 0.075] and [[E > 0.062 and abs(clusterTiming)<18 and clusterReg==1] or '
ROEClustersCuts2 = '[E > 0.060 and abs(clusterTiming)<20 and clusterReg==2] or '
ROEClustersCuts3 = '[E > 0.056 and abs(clusterTiming)<44 and clusterReg==3]]'

ROEclusters = ('ROEclusters', '', ROEClustersCuts1 + ROEClustersCuts2 + ROEClustersCuts3)

appendROEMasks('Upsilon(4S):BhadBtaunu', [ROEclusters, ROETracks])
appendROEMasks('Upsilon(4S):BslBtaunu', [ROEclusters, ROETracks])

variables.addAlias('ROE_eextraSel', 'ROE_eextra(ROEclusters)')
variables.addAlias('ROE_neextraSel', 'ROE_neextra(ROEclusters)')
variables.addAlias('ROE_mcMissFlagsSel', 'ROE_mcMissFlags(ROEclusters)')
variables.addAlias('ROE_chargeSel', 'ROE_charge(ROEclusters)')
variables.addAlias('ROE_ESel', 'ROE_E(ROEclusters)')
variables.addAlias('ROE_PSel', 'ROE_P(ROEclusters)')
variables.addAlias('nAllROETracks', 'nROETracks(ROETracks)')
variables.addAlias('nROEECLClustersSel', 'nROEECLClusters(ROEclusters)')
variables.addAlias('nROENeutralECLClustersSel', 'nROENeutralECLClusters(ROEclusters)')


applyCuts('Upsilon(4S):BhadBtaunu', 'nAllROETracks==0')
applyCuts('Upsilon(4S):BslBtaunu', 'nAllROETracks==0')


buildContinuumSuppression('B+:genericRank')
buildContinuumSuppression('B+:semileptonicRank')


# perform MC matching
matchMCTruth('gamma:mysel')
matchMCTruth('K-:loose')
matchMCTruth('pi-:loose')
matchMCTruth('mu-:loose')
matchMCTruth('e-:loose')
matchMCTruth('tau-')
matchMCTruth('pi0')
matchMCTruth('Upsilon(4S):BhadBtaunu')
matchMCTruth('Upsilon(4S):BslBtaunu')
matchMCTruth('B+:genericRank')
matchMCTruth('B+:semileptonicRank')
matchMCTruth('B-:taunu')


tools4StauHad = ['EventMetaData', '^Upsilon(4S)']
tools4StauHad += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S)']
tools4StauHad += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S)']
tools4StauHad += ['CustomFloats[missPx(ROEclusters,0):missPy(ROEclusters,0)]', '^Upsilon(4S)']
tools4StauHad += ['CustomFloats[missPz(ROEclusters,0):missP(ROEclusters,0)]', '^Upsilon(4S)']
tools4StauHad += ['CustomFloats[missPTheta(ROEclusters,0):missE(ROEclusters,0):missingMass:missM2(ROEclusters,0)]', '^Upsilon(4S)']
tools4StauHad += ['CustomFloats[R2:cosTBTO:cosTBz]', 'Upsilon(4S) -> ^B+:genericRank B-:taunu']
tools4StauHad += ['CustomFloats[sigProb:rank]', 'Upsilon(4S) -> ^B+:genericRank B-:taunu']
tools4StauHad += ['CustomFloats[extraInfo(rankY4SlnuSl):extraInfo(rankY4SlnuHad):daughter(1,rhodM)]', '^Upsilon(4S)']
tools4StauHad += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS]',
                  'Upsilon(4S) ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[d0_d1_costhetaCMS:d0_d1_d0_costhetaCMS:d0_d1_d1_costhetaCMS]',
                  'Upsilon(4S) ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[d0_d0_E:d0_d1_E]', 'Upsilon(4S) ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S) ->  ^B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[d0_p:d0_pCMS:d0_d0_p:d0_d0_pCMS]', 'Upsilon(4S) ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[d0_d1_pCMS:d0_d1_d0_pCMS:d0_d1_d1_pCMS]', 'Upsilon(4S) ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[d0_mcPDG:d1_mcPDG:d1_d0_mcPDG:d1_d0_d0_mcPDG:d1_d0_d1_mcPDG]',
                  '^Upsilon(4S) ->  B+:genericRank B-:taunu']
tools4StauHad += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID:d0_d1_dmID:d1_d0_dmID]',
                  'Upsilon(4S) ->  ^B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                  'Upsilon(4S) ->  ^B+:genericRank ^B-:taunu']
tools4StauHad += ['MCTruth', 'Upsilon(4S) ->  ^B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[tau_DLLKaon:tau_Kid]', 'Upsilon(4S)  ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[tau_DLLMuon:tau_muid]', 'Upsilon(4S)  ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[tau_DLLElec:tau_eid]', 'Upsilon(4S)  ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[tau_DLLPion:tau_piid]', 'Upsilon(4S)  ->  B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[tau_pi0_gamma1_clusterTiming:tau_pi0_gamma2_clusterTiming]',
                  'Upsilon(4S) -> B+:genericRank ^B-:taunu']
tools4StauHad += ['CustomFloats[tau_pi0_gamma1_clusterTheta:tau_pi0_gamma2_clusterTheta]',
                  'Upsilon(4S) -> B+:genericRank ^B-:taunu']


tools4StauSL = ['EventMetaData', '^Upsilon(4S)']
tools4StauSL += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S)']
tools4StauSL += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S)']
tools4StauSL += ['CustomFloats[missPx(ROEclusters,0):missPy(ROEclusters,0)]', '^Upsilon(4S)']
tools4StauSL += ['CustomFloats[missPz(ROEclusters,0):missP(ROEclusters,0)]', '^Upsilon(4S)']
tools4StauSL += ['CustomFloats[missPTheta(ROEclusters,0):missE(ROEclusters,0):missingMass:missM2(ROEclusters,0)]', '^Upsilon(4S)']
tools4StauSL += ['CustomFloats[R2:cosTBTO:cosTBz]', 'Upsilon(4S) -> ^B+:semileptonicRank B-:taunu']
tools4StauSL += ['CustomFloats[sigProb:rank]', 'Upsilon(4S) -> ^B+:semileptonicRank B-:taunu']
tools4StauSL += ['CustomFloats[extraInfo(rankY4SlnuSl):extraInfo(rankY4SlnuSL):daughter(1,rhodM)]', '^Upsilon(4S)']
tools4StauSL += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS]',
                 'Upsilon(4S) -> B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[d0_d1_costhetaCMS:d0_d1_d0_costhetaCMS:d0_d1_d1_costhetaCMS]',
                 'Upsilon(4S) -> B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[d0_d0_E:d0_d1_E]', 'Upsilon(4S) ->  B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[d0_p:d0_pCMS:d0_d0_p:d0_d0_pCMS]', 'Upsilon(4S) ->  B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[d0_d1_pCMS:d0_d1_d0_pCMS:d0_d1_d1_pCMS]', 'Upsilon(4S) ->  B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[d0_mcPDG:d1_mcPDG:d1_d0_mcPDG:d1_d0_d0_mcPDG:d1_d0_d1_mcPDG]',
                 '^Upsilon(4S) ->  B+:semileptonicRank B-:taunu']
tools4StauSL += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID:d0_d1_dmID:d1_d0_dmID]',
                 'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                 'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['MCTruth', 'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[tau_DLLKaon:tau_Kid]', 'Upsilon(4S)  ->  B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[tau_DLLMuon:tau_muid]', 'Upsilon(4S)  ->  B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[tau_DLLElec:tau_eid]', 'Upsilon(4S)  ->  B+:semileptonicRank ^B-:taunu']
tools4StauSL += ['CustomFloats[tau_DLLPion:tau_piid]', 'Upsilon(4S)  ->  B+:semileptonicRank ^B-:taunu']


# create and fill flat Ntuple
toolsBP = ['EventMetaData', '^B+']
toolsBP += ['DeltaEMbc', '^B+']
toolsBP += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+']
toolsBP += ['CustomFloats[d0_dmID:d1_dmID:d0_d0_dmID:d1_d0_dmID:d0_d1_dmID]', '^B+']
toolsBP += ['CustomFloats[d1d2_M:d1d3_M:d1d4_M:d2d3_M:d2d4_M:d3d4_M]', '^B+']
toolsBP += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', '^B+']
toolsBP += ['CustomFloats[d1d2d3_M:d2d3d4_M:d1d2d3d4_M]', '^B+']
toolsBP += ['CustomFloats[isSignal:isExtendedSignal]', '^B+']
toolsBP += ['MCTruth', '^B+']

toolsBPSL = ['EventMetaData', '^B+']
toolsBPSL += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B+']
toolsBPSL += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+']
toolsBPSL += ['CustomFloats[d0_dmID:d1_dmID:d0_d0_dmID:d1_d0_dmID:d0_d1_dmID]', '^B+']
toolsBPSL += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', '^B+']
toolsBPSL += ['CustomFloats[isSignalAcceptMissingNeutrino]', '^B+']
toolsBPSL += ['MCTruth', '^B+']


# write out the flat ntuple
ntupleFile(outputRootFile)
ntupleTree('BPHadTag', 'B+:genericRank', toolsBP)
ntupleTree('BPSLTag', 'B+:semileptonicRank', toolsBPSL)
ntupleTree('BHadBtaunu', 'Upsilon(4S):BhadBtaunu', tools4StauHad)
ntupleTree('BSLBtaunu', 'Upsilon(4S):BslBtaunu', tools4StauSL)


process(analysis_main)

print(statistics)
