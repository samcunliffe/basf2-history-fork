#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""""""

import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header
from variables import variables as vm


def TCPVList(path):
    """
    Time dependent CP violation skim standalone for the analysis of TDCPV analysis in WG

    **Physics channels**: bd → qqs and bd → ccs

    **Skim code**: 13160100

    **Decay Channels**:

    * ``B0 -> phi K_S0``
    * ``B0 -> eta K_S0``
    * ``B0 -> eta K_S0``
    * ``B0 -> K_S0 K_S0 K_S0``
    * ``B0 -> pi0 K_S0``
    * ``B0 -> rho0 K_S0``
    * ``B0 -> omega  K_S0``
    * ``B0 -> f_0 K_S0``
    * ``B0 -> pi0 pi0 K_S0``
    * ``B0 -> phi K_S0 pi0``
    * ``B0 -> pi+ pi- K_S0``
    * ``B0 -> pi+ pi- K_S0 gamma``
    * ``B0 -> pi0  K_S0 gamma``

    **Particle lists used**:

    * ``phi:loose``

    * ``B0 -> pi0 pi0 K_S0``
    * ``B0 -> phi K_S0 pi0``
    * ``B0 -> pi+ pi- K_S0``
    * ``B0 -> pi+ pi- K_S0 gamma``
    * ``B0 -> pi0  K_S0 gamma``

    **Particle lists used**:

    * ``phi:loose``
    * ``k_S0:all``
    * ``eta:loose``
    * ``pi0:eff40_Jan2020``
    * ``pi0:skim``
    * ``rho0:loose``
    * ``pi+:all``
    * ``gamma:E15 , cut : 1.4 < E < 4``
    * ``omega:loose``
    * ``J/psi: eeLoose``
    * ``J/psi: mumuLoose``
    * ``psi(2S): eeLoose``
    * ``psi(2S): mumuloose``
    * ``K*0:loose``
    * ``phi:loose``


    **Cuts used**:

    * ``5.2 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    """

    __author__ = " Reem Rasheed"

    vm.addAlias('foxWolframR2_maskedNaN', 'ifNANgiveX(foxWolframR2,1)')
    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')

    btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'

    bd_qqs_Channels = [
        'phi:loose K_S0:merged',
        'eta\':loose K_S0:merged',
        'eta:loose K_S0:merged',
        'K_S0:merged K_S0:merged K_S0:merged',
        'pi0:skim K_S0:merged',
        'rho0:loose K_S0:merged',
        'omega:loose K_S0:merged',
        'f_0:loose K_S0:merged',
        'pi0:skim pi0:skim K_S0:merged',
        'phi:loose K_S0:merged pi0:skim',
        'pi+:all pi-:all K_S0:merged',
        'pi+:all pi-:all K_S0:merged gamma:E15',
        'pi0:skim K_S0:merged gamma:E15',
    ]

    bd_ccs_Channels = ['J/psi:eeLoose K_S0:merged',
                       'J/psi:mumuLoose K_S0:merged',
                       'psi(2S):eeLoose K_S0:merged',
                       'psi(2S):mumuLoose K_S0:merged',
                       'J/psi:eeLoose K*0:loose',
                       'J/psi:mumuLoose K*0:loose']

    bPlustoJPsiK_Channel = ['J/psi:mumu K+:1%',
                            'J/psi:ee K+:1%']

    btoD_Channels = ['anti-D0:Kpipipi pi+:all',
                     'anti-D0:Kpi pi+:all',
                     ]

    bd_qqs_List = []
    for chID, channel in enumerate(bd_qqs_Channels):
        ma.reconstructDecay('B0:TCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
        ma.applyCuts('B0:TCPV_qqs' + str(chID), 'nTracks>4', path=path)
        bd_qqs_List.append('B0:TCPV_qqs' + str(chID))

    bd_ccs_List = []
    for chID, channel in enumerate(bd_ccs_Channels):
        ma.reconstructDecay('B0:TCPV_ccs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
        ma.applyCuts('B0:TCPV_ccs' + str(chID), 'nTracks>4', path=path)
        bd_ccs_List.append('B0:TCPV_ccs' + str(chID))

    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut='pt > 0.1 and d0<0.5 and -2<z0<2 and nCDCHits>20', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                       allMoments=False,
                       foxWolfram=True,
                       harmonicMoments=False,
                       cleoCones=False,
                       thrust=False,
                       collisionAxis=False,
                       jets=False,
                       sphericity=False,
                       checkForDuplicates=False,
                       path=path)

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)
    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1', path=path)
    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('E_ECL<9', path=path)

    bPlustoJPsiK_List = []
    bMinustoJPsiK_List = []
    for chID, channel in enumerate(bPlustoJPsiK_Channel):
        ma.reconstructDecay('B+:TCPV_JPsiK' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
        bPlustoJPsiK_List.append('B+:TCPV_JPsiK' + str(chID))
        bMinustoJPsiK_List.append('B-:TCPV_JPsiK' + str(chID))

    bPlustoD_List = []
    bMinustoD_List = []
    for chID, channel in enumerate(btoD_Channels):
        ma.reconstructDecay('B+:TCPV_bToD' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
        bPlustoD_List.append('B+:TCPV_bToD' + str(chID))
        bMinustoD_List.append('B-:TCPV_bToD' + str(chID))

    tcpvLists = bd_qqs_List + bd_ccs_List + bPlustoJPsiK_List + bMinustoJPsiK_List + bMinustoD_List
    return tcpvLists


@fancy_skim_header
class TCPV(BaseSkim):
    """
    **Physics channels**: bd → qqs and bd → ccs

    **Decay Channels**:

    * ``B0 -> phi K_S0``
    * ``B0 -> eta K_S0``
    * ``B0 -> eta K_S0``
    * ``B0 -> K_S0 K_S0 K_S0``
    * ``B0 -> pi0 K_S0``
    * ``B0 -> rho0 K_S0``
    * ``B0 -> omega  K_S0``
    * ``B0 -> f_0 K_S0``
    * ``B0 -> pi0 pi0 K_S0``
    * ``B0 -> phi K_S0 pi0``
    * ``B0 -> pi+ pi- K_S0``
    * ``B0 -> pi+ pi- K_S0 gamma``
    * ``B0 -> pi0  K_S0 gamma``
    * ``B0 -> pi0 pi0 K_S0``
    * ``B0 -> phi K_S0 pi0``
    * ``B0 -> pi+ pi- K_S0``
    * ``B0 -> pi+ pi- K_S0 gamma``
    * ``B0 -> pi0  K_S0 gamma``

    **Particle lists used**:

    * ``phi:loose``
    * ``k_S0:all``
    * ``eta:loose``
    * ``pi0:loose``
    * ``pi0:skim``
    * ``rho0:loose``
    * ``pi+:all``
    * ``gamma:E15 , cut : 1.4 < E < 4``
    * ``omega:loose``
    * ``J/psi: eeLoose``
    * ``J/psi: mumuLoose``
    * ``psi(2S): eeLoose``
    * ``psi(2S): mumuloose``
    * ``K*0:loose``
    * ``phi:loose``

    **Cuts used**:

    * ``5.2 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    """

    __authors__ = ["Reem Rasheed"]
    __description__ = "Skim for time-dependent CP violation analysis."
    __contact__ = ""
    __category__ = "physics, TCPV"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["loose"],
            "stdK": ["all", "loose"],
            "stdMu": ["loose"],
            "stdPi": ["all", "loose"]
        },
        "stdPhotons": {
            "stdPhotons": ["loose"],
            "loadStdSkimPhoton": []
        },
        "stdPi0s": {
            "stdPi0s": ["loose"],
            "loadStdSkimPi0": [],
        },
        "stdV0s": {
            "stdKshorts": [],
        },
        "skim.standardlists.lightmesons": {
            "loadStdPi0ForBToHadrons": [],
            "loadStdLightMesons": [],
        },
        "skim.standardlists.charm": {
            "loadPiForBtoHadrons": [],
            "loadKForBtoHadrons": [],
            "loadStdD0_Kpi": [],
            "loadStdD0_Kpipipi": [],
        },
        "skim.standardlists.dileptons": {
            "loadStdDiLeptons": [],
            "loadStdJpsiToee": [],
            "loadStdJpsiTomumu": [],
        },
    }

    def additional_setup(self, path):
        Kcut = "dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance and kaonID > 0.01"
        ma.fillParticleList('K+:1%', cut=Kcut, path=path)
        ma.cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=path)

    def build_lists(self, path):
        vm.addAlias('foxWolframR2_maskedNaN', 'ifNANgiveX(foxWolframR2,1)')
        vm.addAlias('E_ECL_pi_TCPV', 'totalECLEnergyOfParticlesInList(pi+:TCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TCPV', 'totalECLEnergyOfParticlesInList(gamma:TCPV_eventshape)')
        vm.addAlias('E_ECL_TCPV', 'formula(E_ECL_pi_TCPV+E_ECL_gamma_TCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_qqs_Channels = [
            'phi:loose K_S0:merged',
            'eta\':loose K_S0:merged',
            'eta:loose K_S0:merged',
            'K_S0:merged K_S0:merged K_S0:merged',
            'pi0:skim K_S0:merged',
            'rho0:loose K_S0:merged',
            'omega:loose K_S0:merged',
            'f_0:loose K_S0:merged',
            'pi0:skim pi0:skim K_S0:merged',
            'phi:loose K_S0:merged pi0:skim',
            'pi+:all pi-:all K_S0:merged',
            'pi+:all pi-:all K_S0:merged gamma:E15',
            'pi0:skim K_S0:merged gamma:E15',
        ]

        bd_ccs_Channels = ['J/psi:eeLoose K_S0:merged',
                           'J/psi:mumuLoose K_S0:merged',
                           'psi(2S):eeLoose K_S0:merged',
                           'psi(2S):mumuLoose K_S0:merged',
                           'J/psi:eeLoose K*0:loose',
                           'J/psi:mumuLoose K*0:loose']

        bPlustoJPsiK_Channel = ['J/psi:mumu K+:1%',
                                'J/psi:ee K+:1%']

        btoD_Channels = ['anti-D0:Kpipipi pi+:all',
                         'anti-D0:Kpi pi+:all',
                         ]

        bd_qqs_List = []
        for chID, channel in enumerate(bd_qqs_Channels):
            ma.reconstructDecay('B0:TCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TCPV_qqs' + str(chID), 'nTracks>4', path=path)
            bd_qqs_List.append('B0:TCPV_qqs' + str(chID))

        bd_ccs_List = []
        for chID, channel in enumerate(bd_ccs_Channels):
            ma.reconstructDecay('B0:TCPV_ccs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TCPV_ccs' + str(chID), 'nTracks>4', path=path)
            bd_ccs_List.append('B0:TCPV_ccs' + str(chID))

        ma.fillParticleList(decayString='pi+:TCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TCPV_eventshape', 'gamma:TCPV_eventshape'],
                           allMoments=False,
                           foxWolfram=True,
                           harmonicMoments=False,
                           cleoCones=False,
                           thrust=False,
                           collisionAxis=False,
                           jets=False,
                           sphericity=False,
                           checkForDuplicates=False,
                           path=path)

        ma.buildEventKinematics(inputListNames=['pi+:TCPV_eventshape', 'gamma:TCPV_eventshape'], path=path)

        EventCuts = [
            "foxWolframR2_maskedNaN<0.4 and nTracks>=4",
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        bPlustoJPsiK_List = []
        bMinustoJPsiK_List = []
        for chID, channel in enumerate(bPlustoJPsiK_Channel):
            ma.reconstructDecay('B+:TCPV_JPsiK' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bPlustoJPsiK_List.append('B+:TCPV_JPsiK' + str(chID))
            bMinustoJPsiK_List.append('B-:TCPV_JPsiK' + str(chID))

        bPlustoD_List = []
        bMinustoD_List = []
        for chID, channel in enumerate(btoD_Channels):
            ma.reconstructDecay('B+:TCPV_bToD' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bPlustoD_List.append('B+:TCPV_bToD' + str(chID))
            bMinustoD_List.append('B-:TCPV_bToD' + str(chID))

        tcpvLists = bd_qqs_List + bd_ccs_List + bPlustoJPsiK_List + bMinustoJPsiK_List + bMinustoD_List

        self.SkimLists = tcpvLists

    def validation_histograms(self, path):
        Kres = 'K_10'
        ma.applyCuts('gamma:loose', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)
        ma.matchMCTruth('B0:signal', path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        ma.variablesToHistogram('B0:signal', variableshisto, filename='TCPV_Validation.root', path=path)
