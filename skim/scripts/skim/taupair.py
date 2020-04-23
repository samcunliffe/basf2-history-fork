#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for tau-pair analyses """

__authors__ = [
    "Kenji Inami"
]

import basf2 as b2
import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header
from variables import variables as vm


def SetTauGenericSkimVariables(path):
    """
    Set particle lists and variables for TauGeneric skim

    **Input particle lists**: `pi+:all`, `gamma:all`

    **Output particle lists**: ``pi+:tauskim, gamma:tauskim, pi+:S1/S2, gamma:S1/S2``

    **Variables**:

    * ``nGoodTracks``: number of good tracks in an event
    * ``netCharge``: total net charge of good tracks
    * ``nTracksS1/nTracksS2:`` number of good tracks in each hemisphere ``S1/S2`` divided by thrust axis
    * ``invMS1/invMS2``: invariant mass of particles in each hemisphere
    * ``maxPt``: maximum Pt amoung good tracks
    * ``E_ECLtrk``: total ECL energy of good tracks
    * ``maxOp``: maximum opening angle in CM among good tracks

    """
    __author__ = "Kenji Inami"

    # Track and gamma cuts
    trackCuts = '-3.0 < dz < 7.0 and dr < 1.0'
    # trackCuts = 'pt > 0.1 and abs(d0) < 1 and abs(z0) < 5'
    # trackCuts += ' and -0.8660 < cosTheta < 0.9563'
    gammaCuts = 'E > 0.15'
    gammaCuts += ' and -0.8660 < cosTheta < 0.9563'
    ma.cutAndCopyList('pi+:tauskim', 'pi+:all', trackCuts, path=path)
    ma.cutAndCopyList('gamma:tauskim', 'gamma:all', gammaCuts, path=path)

    # Get EventShape variables
    ma.buildEventShape(['pi+:tauskim', 'gamma:tauskim'],
                       allMoments=False, foxWolfram=False, cleoCones=False,
                       sphericity=False, jets=False, path=path)
    ma.buildEventKinematics(['pi+:tauskim', 'gamma:tauskim'], path=path)

    # Split in signal and tag
    ma.cutAndCopyList('pi+:S1', 'pi+:tauskim', 'cosToThrustOfEvent > 0', path=path)
    ma.cutAndCopyList('pi+:S2', 'pi+:tauskim', 'cosToThrustOfEvent < 0', path=path)
    ma.cutAndCopyList('gamma:S1', 'gamma:tauskim', 'cosToThrustOfEvent > 0', path=path)
    ma.cutAndCopyList('gamma:S2', 'gamma:tauskim', 'cosToThrustOfEvent < 0', path=path)

    vm.addAlias('nGoodTracks', 'nParticlesInList(pi+:tauskim)')
    vm.addAlias('netCharge', 'formula(countInList(pi+:tauskim, charge == 1) - countInList(pi+:tauskim, charge == -1))')
    vm.addAlias('nTracksS1', 'nParticlesInList(pi+:S1)')
    vm.addAlias('nTracksS2', 'nParticlesInList(pi+:S2)')
    vm.addAlias('invMS1', 'invMassInLists(pi+:S1, gamma:S1)')
    vm.addAlias('invMS2', 'invMassInLists(pi+:S2, gamma:S2)')
    # vm.addAlias('Evis', 'visibleEnergyOfEventCMS')
    vm.addAlias('maxPt', 'maxPtInList(pi+:tauskim)')
    vm.addAlias('E_ECLtrk', 'formula(totalECLEnergyOfParticlesInList(pi+:tauskim))')
    vm.addAlias('maxOp', 'useCMSFrame(maxOpeningAngleInList(pi+:tauskim))')


def TauList(path):
    """
    Skim for Tau generic decays

    **Skim LFN code**: 18570600

    **Channel**: :math:`e^+ e^- \\to \\tau^+ \\tau^-`

    **Skim Category**: physics, tau

    **Criteria**:

    Ntrk = 2

    1. Number of good tracks = 2, net charge < 2
    2. ``visibleEnergyOfEventCMS < 10 GeV and E_ECLtrk < 6 GeV`` and ``missingMomentumOfEvent_theta < 2.6180``
    3. ``visibleEnergyOfEventCMS > 3 GeV or max P_t > 1 GeV``
    4. max. opening angle < 178 deg.

    Ntrk = 3,4

    1. Number of good tracks = 3 or 4, net charge < 2
    2. ``visibleEnergyOfEventCMS < 10.5 GeV and E_ECLtrk < 6 GeV``
    3. ``visibleEnergyOfEventCMS > 3 GeV or max P_t > 1 GeV``
    4. max. opening angle < 178 deg.
    5. Event divided by thrust axis; No. good tracks in tag side = 1 or 3
    6. ``M_tag < 1.8 GeV`` and ``M_sig < 2.3 GeV``

    Ntrk = 5,6

    1. Number of good tracks = 5 or 6, net charge < 2
    2. Event divided by thrust axis; No. good tracks in tag side = 1 or 3
    3. ``M_tag < 1.8 GeV`` and ``M_sig < 2.3 GeV``

    Returns:
        list name of the TauGeneric skim candidates
    """
    __author__ = "Kenji Inami"

    SetTauGenericSkimVariables(path=path)

    # reconstruct with each Ntrk case
    ma.reconstructDecay('tau+:g2 -> pi+:S1', 'nGoodTracks == 2 and -2 < netCharge <2', path=path)
    ma.reconstructDecay('tau+:g34 -> pi+:S1', '[nGoodTracks == 3 or nGoodTracks == 4] and -2 < netCharge <2', path=path)
    ma.reconstructDecay('tau+:g56 -> pi+:S1', '[nGoodTracks == 5 or nGoodTracks == 6] and -2 < netCharge <2', path=path)

    # Selection criteria
    # Ntrk=2
    ma.applyCuts('tau+:g2', 'visibleEnergyOfEventCMS < 10', path=path)
    ma.applyCuts('tau+:g2', 'E_ECLtrk < 6', path=path)
    ma.applyCuts('tau+:g2', 'missingMomentumOfEvent_theta < 2.6180', path=path)
    ma.applyCuts('tau+:g2', 'visibleEnergyOfEventCMS > 3 or maxPt > 1', path=path)
    ma.applyCuts('tau+:g2', 'maxOp < 3.106686', path=path)
    # Ntrk=3,4
    ma.applyCuts('tau+:g34', 'visibleEnergyOfEventCMS < 10.5', path=path)
    ma.applyCuts('tau+:g34', 'E_ECLtrk < 6', path=path)
    ma.applyCuts('tau+:g34', 'visibleEnergyOfEventCMS > 3 or maxPt > 1', path=path)
    ma.applyCuts('tau+:g34', 'maxOp < 3.106686', path=path)
    ma.applyCuts('tau+:g34',
                 '[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 and invMS2 < 2.3 ] or '
                 '[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 and invMS1 < 2.3 ]', path=path)
    # Ntrk=5,6
    ma.applyCuts('tau+:g56',
                 '[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 and invMS2 < 2.3 ] or '
                 '[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 and invMS1 < 2.3 ]', path=path)

    # For skimming, the important thing is if the final particleList is empty or not.
    eventParticle = ['tau+:g2', 'tau+:g34', 'tau+:g56']
    return eventParticle


def TauLFVList(flag=1, path=None):
    """
    Skim for Tau LFV decays

    **Skim LFN code**: 18360100

    **Channel**: :math:`\\tau \\to l \\gamma, lll, l \\pi^0, l V^0, lhh, llp, phh`

    **Skim category**: physics, tau

    **Input particle lists**: `e+:all`, `mu+:all`, `pi+:all`, `K+:all`, `p+:all`, `gamma:all`, `rho0:all`, `K*0:all`, `phi:all`,
    `f_0:all`

    **Output particle lists**: ``e+:taulfv, mu+:taulfv, pi+:taulfv, K+:taulfv, p+:taulfv``,
    ``gamma:taulfv, pi0:taulfv, K_S0:taulfv, eta:taulfv, eta':taulfv``,
    ``omega:taulfv``

    **Criteria for 1 prong final states **: Number of good tracks < 5, :math:`1.0 < M < 2.0` GeV, :math:`-1.5 < \Delta E < 0.5` GeV

    **Criteria for >1 prong final states **: Number of good tracks < 7, :math:`1.4 < M < 2.0` GeV, :math:`-1.0 < \Delta E < 0.5` GeV

    Returns:
        list name of the TauLFV skim candidates
        * If called as TauLFVList(0), not execute skim (reconstructDecay) and
        only return list of names
        (such as tau+:LFV_lgamma0, tau+:LFV_lgamma1, tau+:LFV_lll0, ...)
    """
    __author__ = "K. Inami"

    # particle selection
    trackCuts = '-3.0 < dz < 7.0 and dr < 1.0'
    ma.cutAndCopyList('e+:taulfv', 'e+:all', trackCuts, path=path)
    ma.cutAndCopyList('mu+:taulfv', 'mu+:all', trackCuts, path=path)
    ma.cutAndCopyList('pi+:taulfv', 'pi+:all', trackCuts, path=path)
    ma.cutAndCopyList('K+:taulfv', 'K+:all', trackCuts, path=path)
    ma.cutAndCopyList('p+:taulfv', 'p+:all', trackCuts, path=path)

    ma.reconstructDecay('K_S0:taulfv -> pi+:all pi-:all', '0.3 < M < 0.7', path=path)

    gammaCuts = 'E > 0.20 and clusterNHits > 1.5 and -0.8660 < cosTheta < 0.9563'
    ma.cutAndCopyList('gamma:taulfv', 'gamma:all', gammaCuts, path=path)

    gammaLooseCuts = 'E > 0.1 and -0.8660 < cosTheta < 0.9563 and clusterNHits > 1.5'
    ma.cutAndCopyLists('gamma:taulfvloose', 'gamma:all', gammaLooseCuts, path=path)
    # pi0
    ma.reconstructDecay('pi0:taulfv -> gamma:taulfvloose gamma:taulfvloose', '0.115 < M < 0.152', path=path)
    # eta
    ma.reconstructDecay('eta:taulfv1 -> gamma:taulfvloose gamma:taulfvloose', '0.4 < M < 0.6', path=path)
    ma.reconstructDecay('eta:taulfv2 -> pi0:taulfv pi-:all pi+:all', '0.4 < M < 0.6', path=path)
    ma.copyLists('eta:taulfv', ['eta:taulfv1', 'eta:taulfv2'], path=path)
    # eta'
    ma.reconstructDecay('eta\':taulfv1 -> pi+:all pi-:all gamma:taulfvloose', '0.8 < M < 1.1', path=path)
    ma.reconstructDecay('eta\':taulfv2 -> pi+:all pi-:all eta:taulfv', '0.8 < M < 1.1', path=path)
    ma.copyLists('eta\':taulfv', ['eta\':taulfv1', 'eta\':taulfv2'], path=path)
    # omega
    ma.reconstructDecay('omega:taulfv -> pi0:taulfv pi-:all pi+:all', '0.73 < M < 0.83', path=path)

    # event selection
    vm.addAlias('netChargeLFV', 'formula(countInList(pi+:taulfv, charge == 1) - countInList(pi+:taulfv, charge == -1))')
    tauLFVCuts1 = 'nParticlesInList(pi+:taulfv) < 5 and 1.0 < M < 2.0 and -1.5 < deltaE < 0.5'
    tauLFVCuts3 = 'nParticlesInList(pi+:taulfv) < 7 and 1.4 < M < 2.0 and -1.0 < deltaE < 0.5'

    tau_lgamma_Channels = ['e+:taulfv gamma:taulfv',
                           'mu+:taulfv gamma:taulfv'
                           ]

    tau_lll_Channels = ['e+:taulfv e+:taulfv e-:taulfv',
                        'mu+:taulfv mu+:taulfv mu-:taulfv',
                        'mu+:taulfv e+:taulfv e-:taulfv',
                        'e+:taulfv mu+:taulfv mu-:taulfv',
                        'e+:taulfv e+:taulfv mu-:taulfv',
                        'mu+:taulfv mu+:taulfv e-:taulfv'
                        ]

    tau_lP01_Channels = ['e+:taulfv pi0:taulfv',
                         'mu+:taulfv pi0:taulfv',
                         'e+:taulfv eta:taulfv1',
                         'mu+:taulfv eta:taulfv1'
                         ]
    tau_lP03_Channels = ['e+:taulfv eta:taulfv2',
                         'mu+:taulfv eta:taulfv2',
                         'e+:taulfv eta\':taulfv',
                         'mu+:taulfv eta\':taulfv',
                         'e+:taulfv K_S0:taulfv',
                         'mu+:taulfv K_S0:taulfv'
                         ]

    tau_lS0_Channels = ['e+:taulfv f_0:all',
                        'mu+:taulfv f_0:all'
                        ]

    tau_lV0_Channels = ['e+:taulfv rho0:all',
                        'mu+:taulfv rho0:all',
                        'e+:taulfv K*0:all',
                        'mu+:taulfv K*0:all',
                        'e+:taulfv anti-K*0:all',
                        'mu+:taulfv anti-K*0:all',
                        'e+:taulfv phi:all',
                        'mu+:taulfv phi:all',
                        'e+:taulfv omega:taulfv',
                        'mu+:taulfv omega:taulfv'
                        ]

    tau_lhh_Channels = ['e+:taulfv pi-:taulfv pi+:taulfv',
                        'mu+:taulfv pi-:taulfv pi+:taulfv',
                        'e-:taulfv pi+:taulfv pi+:taulfv',
                        'mu-:taulfv pi+:taulfv pi+:taulfv',
                        'e+:taulfv K-:taulfv K+:taulfv',
                        'mu+:taulfv K-:taulfv K+:taulfv',
                        'e-:taulfv K+:taulfv K+:taulfv',
                        'mu-:taulfv K+:taulfv K+:taulfv',
                        'e+:taulfv K-:taulfv pi+:taulfv',
                        'mu+:taulfv K-:taulfv pi+:taulfv',
                        'e-:taulfv K+:taulfv pi+:taulfv',
                        'mu-:taulfv K+:taulfv pi+:taulfv',
                        'e+:taulfv K_S0:taulfv K_S0:taulfv',
                        'mu+:taulfv K_S0:taulfv K_S0:taulfv'
                        ]

    tau_bnv_Channels = ['mu+:taulfv mu+:taulfv anti-p-:taulfv',
                        'mu-:taulfv mu+:taulfv p+:taulfv',
                        'anti-p-:taulfv pi+:taulfv pi+:taulfv',
                        'p+:taulfv pi-:taulfv pi+:taulfv',
                        'anti-p-:taulfv pi+:taulfv K+:taulfv',
                        'p+:taulfv pi-:taulfv K+:taulfv'
                        ]

    tau_lgamma_list = []
    for chID, channel in enumerate(tau_lgamma_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_lgamma' + str(chID) + ' -> ' + channel, tauLFVCuts1, chID, path=path)
        tau_lgamma_list.append('tau+:LFV_lgamma' + str(chID))

    tau_lll_list = []
    for chID, channel in enumerate(tau_lll_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_lll' + str(chID) + ' -> ' + channel, tauLFVCuts3, chID, path=path)
        tau_lll_list.append('tau+:LFV_lll' + str(chID))

    tau_lP0_list = []
    for chID, channel in enumerate(tau_lP01_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_lP01' + str(chID) + ' -> ' + channel, tauLFVCuts1, chID, path=path)
        tau_lP0_list.append('tau+:LFV_lP01' + str(chID))
    for chID, channel in enumerate(tau_lP03_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_lP03' + str(chID) + ' -> ' + channel, tauLFVCuts3, chID, path=path)
        tau_lP0_list.append('tau+:LFV_lP03' + str(chID))

    tau_lS0_list = []
    for chID, channel in enumerate(tau_lS0_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_lS0' + str(chID) + ' -> ' + channel, tauLFVCuts3, chID, path=path)
        tau_lS0_list.append('tau+:LFV_lS0' + str(chID))

    tau_lV0_list = []
    for chID, channel in enumerate(tau_lV0_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_lV0' + str(chID) + ' -> ' + channel, tauLFVCuts3, chID, path=path)
        tau_lV0_list.append('tau+:LFV_lV0' + str(chID))

    tau_lhh_list = []
    for chID, channel in enumerate(tau_lhh_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_lhh' + str(chID) + ' -> ' + channel, tauLFVCuts3, chID, path=path)
        tau_lhh_list.append('tau+:LFV_lhh' + str(chID))

    tau_bnv_list = []
    for chID, channel in enumerate(tau_bnv_Channels):
        if(flag):
            ma.reconstructDecay('tau+:LFV_bnv' + str(chID) + ' -> ' + channel, tauLFVCuts3, chID, path=path)
        tau_bnv_list.append('tau+:LFV_bnv' + str(chID))

    tau_lfv_lists = tau_lgamma_list + tau_lll_list + tau_lP0_list + tau_lS0_list + tau_lV0_list + tau_lhh_list + tau_bnv_list
    return tau_lfv_lists


def SetTauThrustSkimVariables(path):
    """
    Set particle lists and variables for TauThrust skim

    **Input particle lists**: `pi+:all`, `gamma:all`

    **Output particle lists**: ``pi+:thrust, gamma:thrust, pi+:thrustS1/thrustS2, pi0:thrust``

    **Variables**:

    * ``nGoodTracksThrust``: number of good tracks in an event
    * ``netChargeThrust``: total net charge of good tracks
    * ``nTracksS1Thrust/nTracksS2Thrust``: number of good tracks in each hemisphere S1/S2 divided by thrust axis

    """
    __author__ = "Ami Rostomyan, Kenji Inami"

    # Track and gamma cuts
    trackCuts = '-3.0 < dz < 7.0 and dr < 1.0'
    ma.cutAndCopyList('pi+:thrust', 'pi+:all', trackCuts, path=path)
    gammaForPi0Cuts = 'E > 0.1 and -0.8660 < cosTheta < 0.9563 and clusterNHits > 1.5'
    ma.cutAndCopyLists('gamma:thrustForPi0', 'gamma:all', gammaForPi0Cuts, path=path)
    ma.reconstructDecay('pi0:thrust -> gamma:thrustForPi0 gamma:thrustForPi0', '0.115 < M < 0.152', path=path)
    gammaCuts = 'E > 0.20 and clusterNHits > 1.5 and -0.8660 < cosTheta < 0.9563'
    gammaCuts += ' and isDaughterOfList(pi0:thrust) == 0'
    ma.cutAndCopyList('gamma:thrust', 'gamma:all', gammaCuts, path=path)

    # Get EventShape variables
    ma.buildEventShape(['pi+:thrust', 'pi0:thrust', 'gamma:thrust'],
                       allMoments=False, foxWolfram=False, cleoCones=False,
                       sphericity=False, jets=False, path=path)
    ma.buildEventKinematics(['pi+:thrust', 'pi0:thrust', 'gamma:thrust'], path=path)

    # Split in signal and tag
    ma.cutAndCopyList('pi+:thrustS1', 'pi+:thrust', 'cosToThrustOfEvent > 0', path=path)
    ma.cutAndCopyList('pi+:thrustS2', 'pi+:thrust', 'cosToThrustOfEvent < 0', path=path)

    vm.addAlias('nGoodTracksThrust', 'nParticlesInList(pi+:thrust)')
    vm.addAlias('netChargeThrust', 'formula(countInList(pi+:thrust, charge == 1) - countInList(pi+:thrust, charge == -1))')
    vm.addAlias('nTracksS1Thrust', 'nParticlesInList(pi+:thrustS1)')
    vm.addAlias('nTracksS2Thrust', 'nParticlesInList(pi+:thrustS2)')


def TauThrustList(path):
    """
    Skim for Tau decays using thrust

    **Skim LFN code**: 18570700

    **Channel**: :math:`e^+ e^- \\to \\tau^+ \\tau^-`

    **Skim category**: physics, tau

    **Criteria**:

    1. ``1 < No. good tracks < 7``
    2. ``net charge == 0``
    3. Event divided by thrust axis; select 1x1, 1x3, 1x5, 3x3 topology
    4. ``0.8 < thrust``
    5. ``visibleEnergyOfEventCMS < 10.4 GeV``
    6. For 1x1 topology, ``thrust < 0.99``

    Returns:
        list name of the TauThrust skim candidates
    """
    __author__ = "Ami Rostomyan, Kenji Inami"

    SetTauThrustSkimVariables(path=path)

    ma.reconstructDecay('tau+:thrust -> pi+:thrustS1', '', path=path)
    eventParticle = ['tau+:thrust']

    # Selection criteria
    ma.applyCuts('tau+:thrust', '1 < nGoodTracksThrust < 7', path=path)  # cut1
    ma.applyCuts('tau+:thrust', 'netChargeThrust == 0', path=path)  # cut2

    topologyCuts = '[nTracksS1Thrust == 1 and nTracksS2Thrust == 1]'  # 1x1
    topologyCuts += ' or [nTracksS1Thrust == 1 and nTracksS2Thrust == 3]'\
        ' or [nTracksS1Thrust == 3 and nTracksS2Thrust == 1]'  # 1x3, 3x1
    topologyCuts += ' or [nTracksS1Thrust == 1 and nTracksS2Thrust == 5]'\
        ' or [nTracksS1Thrust == 5 and nTracksS2Thrust == 1]'  # 1x5, 5x1
    topologyCuts += ' or [nTracksS1Thrust == 3 and nTracksS2Thrust == 3]'  # 3x3

    ma.applyCuts('tau+:thrust', topologyCuts, path=path)  # cut3

    ma.applyCuts('tau+:thrust', '0.8 < thrust', path=path)  # cut4
    ma.applyCuts('tau+:thrust', 'visibleEnergyOfEventCMS < 10.4', path=path)  # cut5

    oneProngPath = b2.create_path()
    ma.applyCuts('tau+:thrust', 'thrust < 0.99', path=oneProngPath)  # cut6 thrust upper cut for 1x1
    sigThrustModule = path.add_module('VariableToReturnValue', variable='nGoodTracksThrust')
    sigThrustModule.if_value('== 2', oneProngPath, b2.AfterConditionPath.CONTINUE)

    # For skimming, the important thing is if the final particleList is empty or not.
    return eventParticle


@fancy_skim_header
class TauLFV(BaseSkim):
    """
    **Channel**: :math:`\\tau \\to l \\gamma, lll, l \\pi^0, l V^0, lhh, llp, phh`

    **Criteria**: :math:`1.58 < M < 1.98` GeV, :math:`|\Delta E|<1` GeV
    """
    __authors__ = ["Phillip Urquijo", "Kenji Inami"]
    __description__ = "Skim for Tau LFV decays."
    __contact__ = ""
    __category__ = "physics, tau"

    RequiredStandardLists = {
        "skim.standardlists.lightmesons": {
            "loadStdLightMesons": [],
        },
        "stdCharged": {
            "stdE": ["loose"],
            "stdK": ["loose"],
            "stdMu": ["loose"],
            "stdPi": ["loose"],
            "stdPr": ["loose"],
        },
        "stdPhotons": {
            "stdPhotons": ["loose"],
        },
        "stdPi0s": {
            "stdPi0s": ["loose"],
            "loadStdSkimPi0": [],
        },
        "stdV0s": {
            "stdKshorts": [],
        },
    }

    def build_lists(self, path):
        tauLFVCuts = "1.58 < M < 1.98 and abs(deltaE) < 1.0"

        tau_lgamma_Channels = ["e+:loose gamma:loose",
                               "mu+:loose gamma:loose"
                               ]

        tau_lll_Channels = ["e-:loose e-:loose e+:loose",
                            "mu-:loose mu-:loose mu+:loose",
                            "mu-:loose e-:loose e+:loose",
                            "e-:loose mu-:loose mu+:loose",
                            "e-:loose e-:loose mu+:loose",
                            "mu-:loose mu-:loose e+:loose"
                            ]

        tau_lP0_Channels = ["e+:loose pi0:skim",
                            "mu+:loose pi0:skim",
                            "e+:loose eta:loose",
                            "mu+:loose eta:loose",
                            "e+:loose eta':loose",
                            "mu+:loose eta':loose",
                            "e+:loose K_S0:merged",
                            "mu+:loose K_S0:merged"
                            ]

        tau_lS0_Channels = ["e+:loose f_0:loose",
                            "mu+:loose f_0:loose"
                            ]

        tau_lV0_Channels = ["e+:loose rho0:loose",
                            "mu+:loose rho0:loose",
                            "e+:loose K*0:loose",
                            "mu+:loose K*0:loose",
                            "e+:loose anti-K*0:loose",
                            "mu+:loose anti-K*0:loose",
                            "e+:loose phi:loose",
                            "mu+:loose phi:loose",
                            "e+:loose omega:loose",
                            "mu+:loose omega:loose"
                            ]

        tau_lhh_Channels = ["e+:loose pi-:loose pi+:loose",
                            "mu+:loose pi-:loose pi+:loose",
                            "e-:loose pi+:loose pi+:loose",
                            "mu-:loose pi+:loose pi+:loose",
                            "e+:loose K-:loose K+:loose",
                            "mu+:loose K-:loose K+:loose",
                            "e-:loose K+:loose K+:loose",
                            "mu-:loose K+:loose K+:loose",
                            "e+:loose K-:loose pi+:loose",
                            "mu+:loose K-:loose pi+:loose",
                            "e-:loose K+:loose pi+:loose",
                            "mu-:loose K+:loose pi+:loose",
                            "e-:loose K_S0:merged K_S0:merged",
                            "mu-:loose K_S0:merged K_S0:merged"
                            ]

        tau_bnv_Channels = ["mu+:loose mu+:loose anti-p-:loose",
                            "mu-:loose mu+:loose p+:loose",
                            "anti-p-:loose pi+:loose pi+:loose",
                            "p+:loose pi-:loose pi+:loose",
                            "anti-p-:loose pi+:loose K+:loose",
                            "p+:loose pi-:loose K+:loose"
                            ]

        tau_lgamma_list = []
        for chID, channel in enumerate(tau_lgamma_Channels):
            ma.reconstructDecay("tau+:LFV_lgamma" + str(chID) + " -> " + channel, tauLFVCuts, chID, path=path)
            tau_lgamma_list.append("tau+:LFV_lgamma" + str(chID))

        tau_lll_list = []
        for chID, channel in enumerate(tau_lll_Channels):
            ma.reconstructDecay("tau+:LFV_lll" + str(chID) + " -> " + channel, tauLFVCuts, chID,
                                path=path, allowChargeViolation=True)
            tau_lll_list.append("tau+:LFV_lll" + str(chID))

        tau_lP0_list = []
        for chID, channel in enumerate(tau_lP0_Channels):
            ma.reconstructDecay("tau+:LFV_lP0" + str(chID) + " -> " + channel, tauLFVCuts, chID, path=path)
            tau_lP0_list.append("tau+:LFV_lP0" + str(chID))

        tau_lS0_list = []
        for chID, channel in enumerate(tau_lS0_Channels):
            ma.reconstructDecay("tau+:LFV_lS0" + str(chID) + " -> " + channel, tauLFVCuts, chID, path=path)
            tau_lS0_list.append("tau+:LFV_lS0" + str(chID))

        tau_lV0_list = []
        for chID, channel in enumerate(tau_lV0_Channels):
            ma.reconstructDecay("tau+:LFV_lV0" + str(chID) + " -> " + channel, tauLFVCuts, chID, path=path)
            tau_lV0_list.append("tau+:LFV_lV0" + str(chID))

        tau_lhh_list = []
        for chID, channel in enumerate(tau_lhh_Channels):
            ma.reconstructDecay("tau+:LFV_lhh" + str(chID) + " -> " + channel, tauLFVCuts, chID,
                                path=path, allowChargeViolation=True)
            tau_lhh_list.append("tau+:LFV_lhh" + str(chID))

        tau_bnv_list = []
        for chID, channel in enumerate(tau_bnv_Channels):
            ma.reconstructDecay("tau+:LFV_bnv" + str(chID) + " -> " + channel, tauLFVCuts, chID, path=path)
            tau_bnv_list.append("tau+:LFV_bnv" + str(chID))

        tau_lfv_lists = tau_lgamma_list + tau_lll_list + tau_lP0_list + tau_lS0_list + tau_lV0_list + tau_lhh_list + tau_bnv_list
        self.SkimLists = tau_lfv_lists

    def validation_histograms(self, path):
        ma.copyLists('tau+:LFV', self.SkimLists, path=path)

        # the variables that are printed out are: M, deltaE
        ma.variablesToHistogram(
            filename='TauLFV_Validation.root',
            decayString='tau+:LFV',
            variables=[('M', 100, 1.50, 2.00), ('deltaE', 120, -1.1, 1.1)],
            variables_2d=[('M', 50, 1.50, 2.00, 'deltaE', 60, -1.1, 1.1)],
            path=path
        )


@fancy_skim_header
class TauGeneric(BaseSkim):
    """
    **Channel**: :math:`e^+ e^- \\to \\tau^+ \\tau^-`

    **Criteria**:

    * ``1 < No. good tracks < 7``
    * ``net charge < 2``
    * Event divided by thrust axis; No. good tracks in tag side = 1 or 3
    * ``visibleEnergyOfEventCMS < 10 GeV and E_ECLtrk < 6 GeV``
    * ``M_tag < 1.8 GeV``
    * ``visibleEnergyOfEventCMS > 3 GeV or max P_t > 1 GeV``
    * ``theta_miss < 150``
    * ``M_sig < 2.3 GeV``
    """
    __authors__ = ["Kenji Inami"]
    __description__ = "Skim for Tau generic decays."
    __contact__ = ""
    __category__ = "physics, tau"

    RequiredStandardLists = {
        "stdCharged": {
            "stdPi": ["all"],
        },
        "stdPhotons": {
            "stdPhotons": ["all"],
        },
    }

    def additional_setup(self, path):
        """
        Set particle lists and variables for TauGeneric skim.

        **Output particle lists**: ``pi+:tauskim, gamma:tauskim, pi+:S1/S2, gamma:S1/S2``

        **Variables**:

        * ``nGoodTracks``: number of good tracks in an event
        * ``netCharge``: total net charge of good tracks
        * ``nTracksS1/nTracksS2:`` number of good tracks in each hemisphere ``S1/S2`` divided by thrust axis
        * ``invMS1/invMS2``: invariant mass of particles in each hemisphere
        * ``maxPt``: maximum Pt amoung good tracks
        * ``E_ECLtrk``: total ECL energy of good tracks
        """

        # Track and gamma cuts
        trackCuts = 'pt > 0.1 and abs(d0) < 1 and abs(z0) < 5'
        # trackCuts += ' and -0.8660 < cosTheta < 0.9563'
        gammaCuts = 'E > 0.15'
        gammaCuts += ' and -0.8660 < cosTheta < 0.9563'
        ma.cutAndCopyList('pi+:tauskim', 'pi+:all', trackCuts, path=path)
        ma.cutAndCopyList('gamma:tauskim', 'gamma:all', gammaCuts, path=path)

        # Get EventShape variables
        ma.buildEventShape(['pi+:tauskim', 'gamma:tauskim'],
                           allMoments=False, foxWolfram=False, cleoCones=False,
                           sphericity=False, jets=False, path=path)
        ma.buildEventKinematics(['pi+:tauskim', 'gamma:tauskim'], path=path)

        # Split in signal and tag
        ma.cutAndCopyList('pi+:S1', 'pi+:tauskim', 'cosToThrustOfEvent > 0', path=path)
        ma.cutAndCopyList('pi+:S2', 'pi+:tauskim', 'cosToThrustOfEvent < 0', path=path)
        ma.cutAndCopyList('gamma:S1', 'gamma:tauskim', 'cosToThrustOfEvent > 0', path=path)
        ma.cutAndCopyList('gamma:S2', 'gamma:tauskim', 'cosToThrustOfEvent < 0', path=path)

        vm.addAlias('nGoodTracks', 'nParticlesInList(pi+:tauskim)')
        vm.addAlias('netCharge', 'formula(countInList(pi+:tauskim, charge == 1) - countInList(pi+:tauskim, charge == -1))')
        vm.addAlias('nTracksS1', 'nParticlesInList(pi+:S1)')
        vm.addAlias('nTracksS2', 'nParticlesInList(pi+:S2)')
        vm.addAlias('invMS1', 'invMassInLists(pi+:S1, gamma:S1)')
        vm.addAlias('invMS2', 'invMassInLists(pi+:S2, gamma:S2)')
        # vm.addAlias('Evis', 'visibleEnergyOfEventCMS')
        vm.addAlias('maxPt', 'maxPtInList(pi+:tauskim)')
        vm.addAlias('E_ECLtrk', 'formula(totalECLEnergyOfParticlesInList(pi+:tauskim))')

    def build_lists(self, path):
        ma.reconstructDecay("tau+:S1 -> pi+:S1", "", path=path)
        eventParticle = ["tau+:S1"]

        # Selection criteria
        ma.applyCuts("tau+:S1", "1 < nGoodTracks < 7", path=path)  # cut1
        ma.applyCuts("tau+:S1", "-2 < netCharge < 2", path=path)  # cut2

        # ma.applyCuts("tau+:S1",
        #          "[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 ] or "
        #          "[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 ]", path=path)  # cut3+cut5
        ma.applyCuts("tau+:S1",
                     "[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 and invMS2 < 2.3 ] or "
                     "[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 and invMS1 < 2.3 ]", path=path)  # cut3+cut5+cut8

        ma.applyCuts("tau+:S1", "visibleEnergyOfEventCMS < 10 and E_ECLtrk < 6", path=path)  # cut4
        ma.applyCuts("tau+:S1", "visibleEnergyOfEventCMS > 3 or maxPt > 1", path=path)  # cut6
        ma.applyCuts("tau+:S1", "missingMomentumOfEvent_theta < 2.6180", path=path)  # cut7

        self.SkimLists = eventParticle

    def validation_histograms(self, path):
        self.additional_setup(path)

        vm.addAlias('Theta_miss', 'formula(missingMomentumOfEvent_theta*180/3.14159)')

        # contact = "kenji@hepl.phys.nagoya-u.ac.jp"

        # TODO: change this to use
        #       validation_tools.metadata.create_validation_histograms, to add contact
        #       info without reopening and closing ROOT file and add 'shifter' flag

        ma.variablesToHistogram(
            filename='TauGeneric_Validation.root',
            decayString='',
            variables=[('nGoodTracks', 7, 1, 8),
                       ('visibleEnergyOfEventCMS', 60, 0, 12),
                       ('E_ECLtrk', 70, 0, 7),
                       ('maxPt', 60, 0, 6),
                       ('invMS1', 60, 0, 3),
                       ('invMS2', 60, 0, 3),
                       ('Theta_miss', 60, 0, 180)],
            variables_2d=[('invMS1', 30, 0, 3, 'invMS2', 30, 0, 3)],
            path=path
        )
        b2.process(path)


@fancy_skim_header
class TauThrust(BaseSkim):
    """
    **Channel**: :math:`e^+ e^- \\to \\tau^+ \\tau^-`

    **Criteria**:

    * ``1 < No. good tracks < 7``
    * ``net charge == 0``
    * Event divided by thrust axis; select 1x1, 1x3, 1x5, 3x3 topology
    * ``0.8 < thrust``
    * ``visibleEnergyOfEventCMS < 10.4 GeV``
    * For 1x1 topology, ``thrust < 0.99``
    """
    __authors__ = ["Ami Rostomyan", "Kenji Inami"]
    __description__ = "Skim for Tau decays using thrust."
    __contact__ = ""
    __category__ = "physics, tau"

    RequiredStandardLists = {
        "stdCharged": {
            "stdPi": ["all"],
        },
        "stdPhotons": {
            "stdPhotons": ["all"],
        },
    }

    def additional_setup(self, path):
        """
        Set particle lists and variables for TauThrust skim.

        **Constructed particle lists**: ``pi+:thrust, gamma:thrust, pi+:thrustS1/thrustS2, pi0:thrust``

        **Variables**:

        * ``nGoodTracksThrust``: number of good tracks in an event
        * ``netChargeThrust``: total net charge of good tracks
        * ``nTracksS1Thrust/nTracksS2Thrust``: number of good tracks in each hemisphere S1/S2 divided by thrust axis
        """
        # Track and gamma cuts
        trackCuts = '-3.0 < dz < 7.0 and dr < 1.0'
        ma.cutAndCopyList('pi+:thrust', 'pi+:all', trackCuts, path=path)
        gammaForPi0Cuts = 'E > 0.1 and -0.8660 < cosTheta < 0.9563 and clusterNHits > 1.5'
        ma.cutAndCopyLists('gamma:thrustForPi0', 'gamma:all', gammaForPi0Cuts, path=path)
        ma.reconstructDecay('pi0:thrust -> gamma:thrustForPi0 gamma:thrustForPi0', '0.115 < M < 0.152', path=path)
        gammaCuts = 'E > 0.20 and clusterNHits > 1.5 and -0.8660 < cosTheta < 0.9563'
        gammaCuts += ' and isDaughterOfList(pi0:thrust) == 0'
        ma.cutAndCopyList('gamma:thrust', 'gamma:all', gammaCuts, path=path)

        # Get EventShape variables
        ma.buildEventShape(['pi+:thrust', 'pi0:thrust', 'gamma:thrust'],
                           allMoments=False, foxWolfram=False, cleoCones=False,
                           sphericity=False, jets=False, path=path)
        ma.buildEventKinematics(['pi+:thrust', 'pi0:thrust', 'gamma:thrust'], path=path)

        # Split in signal and tag
        ma.cutAndCopyList('pi+:thrustS1', 'pi+:thrust', 'cosToThrustOfEvent > 0', path=path)
        ma.cutAndCopyList('pi+:thrustS2', 'pi+:thrust', 'cosToThrustOfEvent < 0', path=path)

        vm.addAlias('nGoodTracksThrust', 'nParticlesInList(pi+:thrust)')
        vm.addAlias('netChargeThrust', 'formula(countInList(pi+:thrust, charge == 1) - countInList(pi+:thrust, charge == -1))')
        vm.addAlias('nTracksS1Thrust', 'nParticlesInList(pi+:thrustS1)')
        vm.addAlias('nTracksS2Thrust', 'nParticlesInList(pi+:thrustS2)')

    def build_lists(self, path):
        SetTauThrustSkimVariables(path=path)
        path = self.skim_event_cuts("nGoodTracksThrust==2", path=path)

        ma.reconstructDecay("tau+:thrust -> pi+:thrustS1", "", path=path)
        eventParticle = ["tau+:thrust"]

        # Selection criteria
        ma.applyCuts("tau+:thrust", "1 < nGoodTracksThrust < 7", path=path)  # cut1
        ma.applyCuts("tau+:thrust", "netChargeThrust == 0", path=path)  # cut2

        topologyCuts = "[nTracksS1Thrust == 1 and nTracksS2Thrust == 1]"  # 1x1
        topologyCuts += " or [nTracksS1Thrust == 1 and nTracksS2Thrust == 3]"\
            " or [nTracksS1Thrust == 3 and nTracksS2Thrust == 1]"  # 1x3, 3x1
        topologyCuts += " or [nTracksS1Thrust == 1 and nTracksS2Thrust == 5]"\
            " or [nTracksS1Thrust == 5 and nTracksS2Thrust == 1]"  # 1x5, 5x1
        topologyCuts += " or [nTracksS1Thrust == 3 and nTracksS2Thrust == 3]"  # 3x3

        ma.applyCuts("tau+:thrust", topologyCuts, path=path)  # cut3
        ma.applyCuts("tau+:thrust", "0.8 < thrust", path=path)  # cut4
        ma.applyCuts("tau+:thrust", "visibleEnergyOfEventCMS < 10.4", path=path)  # cut5
        ma.applyCuts("tau+:thrust", "thrust < 0.99", path=path)  # cut6 thrust upper cut for 1x1

        self.SkimLists = eventParticle

    def validation_histograms(self, path):
        self.additional_setup(path)

        # TODO: change this to use
        #       validation_tools.metadata.create_validation_histograms, to add contact
        #       info without reopening and closing ROOT file and add 'shifter' flag

        ma.variablesToHistogram(
            filename='TauThrust_Validation.root',
            decayString='',
            variables=[('nGoodTracksThrust', 7, 1, 8),
                       ('visibleEnergyOfEventCMS', 60, 0, 12),
                       ('thrust', 50, 0.75, 1)],
            path=path
        )
