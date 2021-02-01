#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""""""

import modularAnalysis as ma
from skim.standardlists.dileptons import (loadStdDiLeptons, loadStdJpsiToee,
                                          loadStdJpsiTomumu, loadStdPsi2s2lepton)
from skim.standardlists.lightmesons import (loadStdSkimHighEffTracks,
                                            loadStdSkimHighEffPhi, loadStdSkimHighEffEtaPrime,
                                            loadStdSkimHighEffEta, loadStdSkimHighEffKstar0,
                                            loadStdSkimHighEffRho0, loadStdSkimHighEffOmega,
                                            loadStdSkimHighEffF_0)

from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import loadStdSkimPhoton, stdPhotons
from stdPi0s import loadStdSkimPi0, stdPi0s
from stdV0s import stdKshorts
from variables import variables as vm
from stdKlongs import stdKlongs

__authors__ = [
    "Chiara La Licata <chiara.lalicata@ts.infn.it>",
    "Stefano Lacaprara  <stefano.lacaprara@pd.infn.it>"
]

# __liaison__ = "Chiara La Licata <chiara.lalicata@ts.infn.it>"
__liaison__ = "Yoshiyuki ONUKI <onuki@hep.phys.s.u-tokyo.ac.jp>"


@fancy_skim_header
class TDCPV_qqs(BaseSkim):
    """
    **Physics channels**: bd/u → qqs

    **Decay Channels**:

    * ``B0 -> phi K_S0``
    * ``B0 -> eta K_S0``
    * ``B0 -> eta' K_S0``
    * ``B0 -> eta K*``
    * ``B0 -> eta' K*``
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
    * ``B+ -> eta' K+``

    **Particle lists used**:

    * ``phi:SkimHighEff``
    * ``eta':SkimHighEff``
    * ``eta:SkimHighEff``
    * ``pi0:eff40_Jan2020``
    * ``pi0:skim``
    * ``rho0:SkimHighEff``
    * ``omega:SkimHighEff``
    * ``f_0:SkimHighEff``
    * ``pi+:SkimHighEff``
    * ``K+:SkimHighEff``
    * ``omega:SkimHighEff``
    * ``K*0:SkimHighEff``
    * ``gamma:E15 , cut : 1.4 < E < 4``
    * ``k_S0:merged``
    * ``K+:1%``

    **Cuts used**:

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3``
    * ``nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1``,
    * ``visibleEnergyOfEventCMS>4"``,
    * ``E_ECL_TDCPV<9``
    """

    __authors__ = ["Reem Rasheed", "Chiara La Licata", "Stefano Lacaprara"]
    __description__ = "Skim for time-dependent CP violation analysis b->qqs decays"
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)
        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_Jan2020", path=path)

        loadStdSkimHighEffPhi(path=path)
        loadStdSkimHighEffEta(path=path)
        loadStdSkimHighEffEtaPrime(path=path)
        loadStdSkimHighEffKstar0(path=path)
        loadStdSkimHighEffRho0(path=path)
        loadStdSkimHighEffOmega(path=path)
        loadStdSkimHighEffF_0(path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('gamma:E15', 'gamma:all', '1.4<E<4', path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_qqs_Channels = [
            'phi:SkimHighEff K_S0:merged',
            'eta\':SkimHighEff K_S0:merged',
            'eta:SkimHighEff K_S0:merged',
            'eta\':SkimHighEff K*0:SkimHighEff',
            'eta:SkimHighEff K*0:SkimHighEff',
            'K_S0:merged K_S0:merged K_S0:merged',
            'pi0:skim K_S0:merged',
            'rho0:SkimHighEff K_S0:merged',
            'omega:SkimHighEff K_S0:merged',
            'f_0:SkimHighEff K_S0:merged',
            'pi0:skim pi0:skim K_S0:merged',
            'phi:SkimHighEff K_S0:merged pi0:skim',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged gamma:E15',
            'pi0:skim K_S0:merged gamma:E15',
        ]

        bu_qqs_Channels = [
            'eta\':SkimHighEff K+:SkimHighEff',
        ]

        bd_qqs_List = []
        for chID, channel in enumerate(bd_qqs_Channels):
            ma.reconstructDecay('B0:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TDCPV_qqs' + str(chID), 'nTracks>4', path=path)
            bd_qqs_List.append('B0:TDCPV_qqs' + str(chID))

        bu_qqs_List = []
        for chID, channel in enumerate(bu_qqs_Channels):
            ma.reconstructDecay('B+:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B+:TDCPV_qqs' + str(chID), 'nTracks>4', path=path)
            bu_qqs_List.append('B+:TDCPV_qqs' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'], path=path)

        EventCuts = [
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        tcpvLists = bd_qqs_List + bu_qqs_List

        self.SkimLists = tcpvLists

    def validation_histograms(self, path):
        Kres = 'K_10'
        ma.applyCuts('gamma:loose', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)
        ma.matchMCTruth('B0:signal', path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        ma.variablesToHistogram('B0:signal', variableshisto, filename='TDCPV_Validation.root', path=path)


@fancy_skim_header
class TDCPV_ccs(BaseSkim):
    """
    **Physics channels**:  bd → ccs

    **Decay Channels**:

    * ``B0 -> J/psi (ee/mm) K_S0``
    * ``B0 -> psi(2s) (ee/mm) K_S0``
    * ``B0 -> J/psi (ee/mm) K*``
    * ``B+ -> J/psi (ee/mm) K+``
    * ``B0 -> J/psi (ee/mm) KL``

    **Particle lists used**:

    * ``k_S0:merged``
    * ``pi+:all``
    * ``J/psi:ee``
    * ``J/psi:mumu``
    * ``psi(2S):ee``
    * ``psi(2S):mumu``
    * ``K*0:SkimHighEff``
    * ``K+:SkimHighEff``
    * ``K_L0:all``

    **Cuts used**:

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29 for Ks/K*``
    * ``5.05 < Mbc < 5.29 for KL``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3``
    * ``nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1``,
    * ``visibleEnergyOfEventCMS>4"``,
    * ``E_ECL_TDCPV<9``
    """

    __authors__ = ["Reem Rasheed", "Chiara La Licata", "Stefano Lacaprara"]
    __description__ = "Skim for time-dependent CP violation analysis."
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)

        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_Jan2020", path=path)
        loadStdSkimHighEffKstar0(path=path)

        loadStdJpsiToee(path=path)
        loadStdJpsiTomumu(path=path)
        loadStdPsi2s2lepton(path=path)
        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('K_L0:alleclEcut', 'K_L0:allecl', 'E>0.15', path=path)
        ma.copyLists('K_L0:all', ['K_L0:allklm', 'K_L0:allecl'], writeOut=True, path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'
        btotcpvcuts_KL = '5.05 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_ccs_Channels = ['J/psi:ee K_S0:merged',
                           'J/psi:mumu K_S0:merged',
                           'psi(2S):ll K_S0:merged',
                           'J/psi:ee K*0:SkimHighEff',
                           'J/psi:mumu K*0:SkimHighEff']

        bPlustoJPsiK_Channel = ['J/psi:mumu K+:SkimHighEff',
                                'J/psi:ee K+:SkimHighEff']

        bd_ccs_KL_Channels = ['J/psi:mumu K_L0:all',
                              'J/psi:ee K_L0:all']

        bd_ccs_List = []
        for chID, channel in enumerate(bd_ccs_Channels):
            ma.reconstructDecay('B0:TDCPV_ccs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TDCPV_ccs' + str(chID), 'nTracks>4', path=path)
            bd_ccs_List.append('B0:TDCPV_ccs' + str(chID))

        bPlustoJPsiK_List = []

        for chID, channel in enumerate(bPlustoJPsiK_Channel):
            ma.reconstructDecay('B+:TDCPV_JPsiK' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bPlustoJPsiK_List.append('B+:TDCPV_JPsiK' + str(chID))

        b0toJPsiKL_List = []
        for chID, channel in enumerate(bd_ccs_KL_Channels):
            ma.reconstructDecay('B0:TDCPV_JPsiKL' + str(chID) + ' -> ' + channel, btotcpvcuts_KL, chID, path=path)
            b0toJPsiKL_List.append('B0:TDCPV_JPsiKL' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'], path=path)

        EventCuts = [
            # "foxWolframR2_maskedNaN<0.4 and nTracks>=4",
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        tcpvLists = bd_ccs_List + bPlustoJPsiK_List + b0toJPsiKL_List

        self.SkimLists = tcpvLists

    def validation_histograms(self, path):
        Kres = 'K_10'
        ma.applyCuts('gamma:loose', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)
        ma.matchMCTruth('B0:signal', path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        ma.variablesToHistogram('B0:signal', variableshisto, filename='TDCPV_Validation.root', path=path)


@fancy_skim_header
class TDCPV_ccsKs(BaseSkim):
    """
    **Physics channels**:  bd → ccs with Ks or K* only

    **Decay Channels**:

    * ``B0 -> J/psi (ee/mm) K_S0``
    * ``B0 -> psi(2s) (ee/mm) K_S0``
    * ``B0 -> J/psi (ee/mm) K*``
    * ``B+ -> J/psi (ee/mm) K+``

    **Particle lists used**:

    * ``k_S0:merged``
    * ``pi+:all``
    * ``J/psi:ee``
    * ``J/psi:mumu``
    * ``psi(2S):ee``
    * ``psi(2S):mumu``
    * ``K*0:SkimHighEff``
    * ``K+:SkimHighEff``

    **Cuts used**:

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29 for Ks/K*``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3``
    * ``nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1``,
    * ``visibleEnergyOfEventCMS>4"``,
    * ``E_ECL_TDCPV<9``
    """

    __authors__ = ["Reem Rasheed", "Chiara La Licata", "Stefano Lacaprara"]
    __description__ = "Skim for time-dependent CP violation analysis."
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

        loadStdSkimHighEffTracks('K', path=path)
        loadStdSkimHighEffTracks('pi', path=path)

        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_Jan2020", path=path)
        loadStdSkimHighEffKstar0(path=path)

        loadStdJpsiToee(path=path)
        loadStdJpsiTomumu(path=path)
        loadStdPsi2s2lepton(path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_ccs_Channels = ['J/psi:ee K_S0:merged',
                           'J/psi:mumu K_S0:merged',
                           'psi(2S):ll K_S0:merged',
                           'J/psi:ee K*0:SkimHighEff',
                           'J/psi:mumu K*0:SkimHighEff']

        bPlustoJPsiK_Channel = ['J/psi:mumu K+:SkimHighEff',
                                'J/psi:ee K+:SkimHighEff']

        bd_ccs_List = []
        for chID, channel in enumerate(bd_ccs_Channels):
            ma.reconstructDecay('B0:TDCPV_ccs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TDCPV_ccs' + str(chID), 'nTracks>4', path=path)
            bd_ccs_List.append('B0:TDCPV_ccs' + str(chID))

        bPlustoJPsiK_List = []

        for chID, channel in enumerate(bPlustoJPsiK_Channel):
            ma.reconstructDecay('B+:TDCPV_JPsiK' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bPlustoJPsiK_List.append('B+:TDCPV_JPsiK' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'], path=path)

        EventCuts = [
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        tcpvLists = bd_ccs_List + bPlustoJPsiK_List

        self.SkimLists = tcpvLists

    def validation_histograms(self, path):
        Kres = 'K_10'
        ma.applyCuts('gamma:loose', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)
        ma.matchMCTruth('B0:signal', path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        ma.variablesToHistogram('B0:signal', variableshisto, filename='TDCPV_Validation.root', path=path)


@fancy_skim_header
class TDCPV_ccsKL(BaseSkim):
    """
    **Physics channels**:  bd → ccs with KL

    **Decay Channels**:

    * ``B0 -> J/psi (ee/mm) K_L``

    **Particle lists used**:

    * ``J/psi: ee``
    * ``J/psi: mumu``
    * ``K_0L:all``

    **Cuts used**:

    * ``5.05 < Mbc < 5.29 for KL``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3``
    * ``nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1``,
    * ``visibleEnergyOfEventCMS>4"``,
    * ``E_ECL_TDCPV<9``
    """

    __authors__ = ["Reem Rasheed", "Chiara La Licata", "Stefano Lacaprara"]
    __description__ = "Skim for time-dependent CP violation analysis."
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)

        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_Jan2020", path=path)
        loadStdSkimHighEffKstar0(path=path)

        loadStdJpsiToee(path=path)
        loadStdJpsiTomumu(path=path)
        loadStdPsi2s2lepton(path=path)
        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('K_L0:alleclEcut', 'K_L0:allecl', 'E>0.15', path=path)
        ma.copyLists('K_L0:all', ['K_L0:allklm', 'K_L0:allecl'], writeOut=True, path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts_KL = '5.05 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_ccs_KL_Channels = ['J/psi:mumu K_L0:all',
                              'J/psi:ee K_L0:all']

        b0toJPsiKL_List = []
        for chID, channel in enumerate(bd_ccs_KL_Channels):
            ma.reconstructDecay('B0:TDCPV_JPsiKL' + str(chID) + ' -> ' + channel, btotcpvcuts_KL, chID, path=path)
            b0toJPsiKL_List.append('B0:TDCPV_JPsiKL' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'], path=path)

        EventCuts = [
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        tcpvLists = b0toJPsiKL_List

        self.SkimLists = tcpvLists

    def validation_histograms(self, path):
        Kres = 'K_10'
        ma.applyCuts('gamma:loose', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)
        ma.matchMCTruth('B0:signal', path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        ma.variablesToHistogram('B0:signal', variableshisto, filename='TDCPV_Validation.root', path=path)
