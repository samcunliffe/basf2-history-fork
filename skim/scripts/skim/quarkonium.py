#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for quarkonium analyses: bottomonium, charmonium, resonance """

__authors__ = [
    "Stefano Spataro"
    "Sen Jia"
    "..."
]

import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdMu, stdPi
from stdPhotons import stdPhotons
from stdV0s import stdLambdas
from variables import variables as v

__liaison__ = "Sen Jia <jiasen@buaa.edu.cn>"


@fancy_skim_header
class BottomoniumEtabExclusive(BaseSkim):
    """
    Reconstructed decay modes:

    * ``eta_b -> gamma gamma``

    Selection criteria:

    * ``2 std photon with E > 3.5 GeV``
    * ``7 < M(eta_b) < 10 GeV/c^2``
    * ``foxWolframR2 < 0.995``
    """

    __authors__ = ["Stefano Spataro", "Sen Jia"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    def load_standard_lists(self, path):
        stdPhotons("loose", path=path)

    def build_lists(self, path):
        # create and fill hard photon
        ma.fillParticleList(decayString="pi+:BottomoniumEtab_eventshape", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:BottomoniumEtab_eventshape", cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:BottomoniumEtab_eventshape", "gamma:BottomoniumEtab_eventshape"],
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

        ma.cutAndCopyList("gamma:hard", "gamma:loose", "E>3.5", path=path)
        ma.applyCuts("gamma:hard", "foxWolframR2 < 0.995", path=path)

        # the requirement of 7 < M(eta_b) < 10 GeV/c2
        Etabcuts = "M > 7 and M < 10"

        # eta_b candidates are reconstructed
        Etab_Channels = ["gamma:hard gamma:hard"]

        # define the eta_b decay list
        EtabList = []

        # reconstruct the decay eta_b -> gamma gamma
        for chID, channel in enumerate(Etab_Channels):
            ma.reconstructDecay("eta_b:all" + str(chID) + " -> " + channel, Etabcuts, chID, path=path)
            EtabList.append("eta_b:all" + str(chID))

        self.SkimLists = EtabList


@fancy_skim_header
class BottomoniumUpsilon(BaseSkim):
    """
    Reconstructed decay modes:

    * Y(1S,2S) -> l^+ l^{-} (l = e or mu)

    Selection criteria:

    * 2 tracks with momentum ranging between ``3.5 < p < 15``
    * At least 1 track ``p < 1.5`` or 1 std photon with ``E > 150 MeV``
    * ``M(Y(1S,2S)) > 8 GeV/c^2``
    * ``foxWolframR2 < 0.995``
    """
    __authors__ = ["Stefano Spataro", "Sen Jia"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    def load_standard_lists(self, path):
        stdPhotons("loose", path=path)

    def build_lists(self, path):
        Ycuts = ""
        # create and fill e/mu/pi/photon ParticleLists
        ma.fillParticleList("mu+:BottomoniumUpsilon", "p<15 and p>3.5", path=path)
        ma.fillParticleList("e+:BottomoniumUpsilon", "p<15 and p>3.5", path=path)
        ma.fillParticleList("pi+:BottomoniumUpsilon", "p<1.5 and pt>0.05", path=path)
        ma.cutAndCopyList("gamma:soft", "gamma:loose", "E>0.15", path=path)

        # Y(1S,2S) are reconstructed with e^+ e^- or mu^+ mu^-
        ma.reconstructDecay("Upsilon:ee -> e+:BottomoniumUpsilon e-:BottomoniumUpsilon", "M > 8", path=path)
        ma.reconstructDecay("Upsilon:mumu -> mu+:BottomoniumUpsilon mu-:BottomoniumUpsilon", "M > 8", path=path)
        ma.copyLists("Upsilon:all", ["Upsilon:ee", "Upsilon:mumu"], path=path)

        # require foxWolframR2 < 0.995
        ma.fillParticleList(decayString="pi+:BottomoniumUpsilon_eventshape", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:BottomoniumUpsilon_eventshape", cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:BottomoniumUpsilon_eventshape", "gamma:BottomoniumUpsilon_eventshape"],
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

        ma.applyCuts("Upsilon:all", "foxWolframR2 < 0.995", path=path)

        # Y(1S,2S) with pi+ or photon are reconstructed
        Upsilon_Channels = ["Upsilon:all pi+:BottomoniumUpsilon",
                            "Upsilon:all gamma:soft"]

        # define the Y(1S,2S) decay channel list
        UpsilonList = []

        # reconstruct the decay channel
        for chID, channel in enumerate(Upsilon_Channels):
            ma.reconstructDecay("junction:all" + str(chID) + " -> " + channel, Ycuts, chID, path=path, allowChargeViolation=True)
            UpsilonList.append("junction:all" + str(chID))

        # reture the list
        self.SkimLists = UpsilonList

    #       *two* sets of validation scripts defined.


@fancy_skim_header
class CharmoniumPsi(BaseSkim):
    """
    Reconstructed decay modes:

    * J/psi -> l^+ l^- (l = e or mu)
    * psi(2S) -> l^+ l^- (l = e or mu)

    Selection criteria:

    * 2 tracks with electronID > 0.1 or muonID > 0.1 and 2.7 < M < 4.
      Track-quality requirements are not applied.
    """
    __authors__ = ["Kirill Chilikin"]
    __description__ = "Selection of J/psi and psi(2S) via leptonic decays."
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    def load_standard_lists(self, path):
        stdE("loosepid", path=path)
        stdMu("loosepid", path=path)

    def build_lists(self, path):

        # Reconstruct J/psi or psi(2S).
        ma.reconstructDecay("J/psi:ee -> e+:loosepid e-:loosepid",
                            "2.7 < M < 4", path=path)
        ma.reconstructDecay("J/psi:mumu -> mu+:loosepid mu-:loosepid",
                            "2.7 < M < 4", path=path)

        # Return the lists.
        self.SkimLists = ["J/psi:ee", "J/psi:mumu"]


@fancy_skim_header
class InclusiveLambda(BaseSkim):
    """
    Reconstructed decay
    * :math:`\\Lambda \\to p \\pi^-` (and charge conjugate)

    Selection criteria:
    * proton
    ``nCDCHits > 20``
    ``nSVDHits < 13``
    ``trackFitHypothesisPDG == 2212``
    ``protonID > 0.1``
    * pion:
    ``nCDCHits > 0``
    * Lambda:
    ``cosAngleBetweenMomentumAndVertexVector > 0.99``
    ``flightDistance/flightDistanceErr > 3.``
    * ``0.6 < p,proton/p,Lambda < 1.0 GeV/c``

    f1(p) and f2(p) are analytical functions, p being the Lambda momentum
    (see https://indico.belle2.org/event/2419/contributions/12005/attachments/6228/9670/BottomoniumHyperons_B2GM.pdf
    and https://indico.belle2.org/event/2467/contributions/13962/attachments/7240/11226/Skim_inclusiveLambda.pdf)
    """
    __authors__ = ["Bianca Scavino"]
    __description__ = "Inclusive Lambda skim"
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    def load_standard_lists(self, path):
        stdLambdas(path=path)

    def build_lists(self, path):

        # Add useful alias
        v.addAlias("nCDCHits_proton", "daughter(0, nCDCHits)")
        v.addAlias("nSVDHits_proton", "daughter(0, nSVDHits)")
        v.addAlias("fitHypo_proton", "daughter(0, trackFitHypothesisPDG)")
        v.addAlias("protonID_proton", "daughter(0, protonID)")
        v.addAlias("nCDCHits_pion", "daughter(1, nCDCHits)")
        v.addAlias("momRatio_protonLambda", "formula(daughter(0, p)/p)")
        v.addAlias('flightSignificance', 'formula(flightDistance/flightDistanceErr)')

        # Apply selection to Lambdas
        ma.applyCuts("Lambda0:merged", "nCDCHits_proton > 20 and nSVDHits_proton < 13 and fitHypo_proton == 2212", path=path)
        ma.applyCuts("Lambda0:merged", "nCDCHits_pion > 0", path=path)
        ma.applyCuts("Lambda0:merged", "cosAngleBetweenMomentumAndVertexVector > 0.99", path=path)
        ma.applyCuts("Lambda0:merged", "0.6 < momRatio_protonLambda < 1.", path=path)
        ma.applyCuts("Lambda0:merged", "flightSignificance > 3.", path=path)
        ma.applyCuts("Lambda0:merged", "protonID_proton > 0.1", path=path)

        # Return the lists.
        self.SkimLists = ["Lambda0:merged"]
