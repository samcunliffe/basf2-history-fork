#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim list building functions for charm analyses.

.. Note::
    The Hp, Hm and Jm in the function name represent arbitrary charged particles with
    positive or negative charge. The Hp and Jm in `XToD0_D0ToHpJm`, mean a positive particle
    and a negative particle that could be either conjugate or non-conjugate. The Hp and Hm
    in `DstToD0Pi_D0ToHpHmPi0`, mean a positive particle and a negative particle that
    must be conjugate.
"""

from functools import lru_cache

import modularAnalysis as ma
import vertex
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import loadStdSkimPhoton, stdPhotons
from stdPi0s import loadStdSkimPi0
from stdV0s import stdKshorts
from variables import variables as vm

__liaison__ = "Guanda Gong <gonggd@mail.ustc.edu.cn>"


@fancy_skim_header
class XToD0_D0ToHpJm(BaseSkim):
    """
    Skims :math:`D^0`'s reconstructed by `XToD0_D0ToHpJm.D0ToHpJm`.
    """

    __authors__ = ["Giulia Casarosa"]
    __description__ = "Skim list for D0 to two charged FSPs."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    # Cached static method, so that its contents are only executed once for a single path.
    # Factored out into a separate function here, so it is available to other skims.

    @staticmethod
    @lru_cache()
    def D0ToHpJm(path):
        """
        **Decay Modes**:

        * :math:`D^{0}\\to \\pi^+ K^-`,
        * :math:`D^{0}\\to \\pi^+ \\pi^-`,
        * :math:`D^{0}\\to K^+ \\pi^-`,

        **Additional Cuts**:

        * ``Tracks: abs(d0) < 1, abs(z0) < 3, 0.296706 < theta < 2.61799``
        * ``1.80 < M(D0) < 1.93``
        * ``pcms(D0) > 2.2``

        Parameters:
            path (basf2.Path): Skim path to be processed.

        Returns:
            List of D0 particle list names.
        """
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:mygood", mySel, path=path)
        ma.fillParticleList("K+:mygood", mySel, path=path)

        charmcuts = "1.80 < M < 1.93 and useCMSFrame(p)>2.2"
        D0_Channels = [
            "pi+:mygood K-:mygood",
            "pi+:mygood pi-:mygood",
            "K+:mygood K-:mygood",
        ]

        D0List = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpJm" + str(chID) + " -> " + channel, charmcuts, chID, path=path)
            D0List.append("D0:HpJm" + str(chID))

        return D0List

    def build_lists(self, path):
        """Builds :math:`D^0` skim lists defined in `XToD0_D0ToHpJm.D0ToHpJm`."""
        self.SkimLists = self.D0ToHpJm(path)


@fancy_skim_header
class XToD0_D0ToNeutrals(BaseSkim):
    """
    Skims :math:`D^0`'s reconstructed by `XToD0_D0ToNeutrals.D0ToNeutrals`.
    """

    __authors__ = ["Giulia Casarosa"]
    __description__ = "Skim list for D0 to neutral FSPs."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    # Cached static method, so that its contents are only executed once for a single path
    # Factored out into a separate function here, so it is available to other skims.

    @staticmethod
    @lru_cache()
    def D0ToNeutrals(path):
        """
        **Decay Modes**:

        * :math:`D^{0}\\to \\pi^{0} \\pi^{0}`
        * :math:`D^{0}\\to K_{S} \\pi^{0}`
        * :math:`D^{0}\\to K_{S} K_{S}`

        **Additional Cuts**:

        * ``1.78 < M(D0) < 1.94, pcms(D0) > 2.2``

        Parameters:
            path (basf2.Path): Skim path to be processed.

        Returns:
            List of D0 particle list names.
        """
        charmcuts = "1.78 < M < 1.94 and useCMSFrame(p)>2.2"
        D0_Channels = ["pi0:skim pi0:skim",
                       "K_S0:merged pi0:skim",
                       "K_S0:merged K_S0:merged",
                       ]

        D0List = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:2Nbdy" + str(chID) + " -> " + channel, charmcuts, chID, path=path)
            D0List.append("D0:2Nbdy" + str(chID))

        return D0List

    def build_lists(self, path):
        """Builds :math:`D^0` skim lists defined in `XToD0_D0ToNeutrals.D0ToNeutrals`."""
        self.SkimLists = self.D0ToNeutrals(path)


@fancy_skim_header
class DstToD0Pi_D0ToRare(BaseSkim):
    """
    **Decay Modes**:

    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to ee`,
    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\mu \\mu`,
    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to e^+ \\mu^-`,
    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to e^- \\mu^+`,
    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ \\pi^-`,

    **Additional Cuts**:

    * ``1.78 < M(D0) < 1.94``
    * ``0 < Q < 0.02``
    * ``pcms(D*) > 2.2``
    """

    __authors__ = ["Doris Yangsoo Kim", "Jaeyeon Kim"]
    __description__ = "Skim list for D*+ to pi+ D0, D0 to rare decay."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdE("loose", path=path)
        stdMu("loose", path=path)
        stdPi("loose", path=path)
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        charmcuts = "1.78 < M < 1.94"
        Dstcuts = "0 < Q < 0.02 and 2.2 < useCMSFrame(p)"

        D0_Channels = ["gamma:skim gamma:skim",
                       "e+:loose e-:loose",
                       "e+:loose mu-:loose",
                       "e-:loose mu+:loose",
                       "mu+:loose mu-:loose",
                       "pi+:loose pi-:loose"]
        DstList = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:Rare" + str(chID) + " -> " + channel, charmcuts, chID, path=path)
            ma.reconstructDecay("D*+:" + str(chID) + " -> pi+:loose D0:Rare" + str(chID),
                                Dstcuts, chID, path=path)
            DstList.append("D*+:" + str(chID))

        self.SkimLists = DstList


@fancy_skim_header
class XToDp_DpToKsHp(BaseSkim):
    """
    **Decay Modes**:

    * :math:`D^+_{(S)} \\to K_{S} \\pi^+`,
    * :math:`D^+_{(S)} \\to K_{S} K^+`,

    **Additional Cuts**:

    * ``Tracks not from Ks: same as D0ToHpJm()``
    * ``Ks: directly taken from stdKshort()``
    * ``1.72 < M(D+) < 1.98, pcms(D+) > 2``
    """

    __authors__ = ["Guanda Gong"]
    __description__ = "Skim list for D_(s)+ to Ks h+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:kshp", mySel, path=path)
        ma.fillParticleList("K+:kshp", mySel, path=path)
        ma.cutAndCopyList('K_S0:kshp', 'K_S0:merged', 'formula(flightDistance/flightDistanceErr) > 2', path=path)

        Dpcuts = "1.72 < M < 2.2 and useCMSFrame(p)>2"
        Dp_Channels = ["K_S0:kshp pi+:kshp",
                       "K_S0:kshp K+:kshp",
                       ]

        DpList = []
        for chID, channel in enumerate(Dp_Channels):
            ma.reconstructDecay("D+:KsHp" + str(chID) + " -> " + channel, Dpcuts, chID, path=path)
            DpList.append("D+:KsHp" + str(chID))

        self.SkimLists = DpList


@fancy_skim_header
class XToDp_DpToHpHmJp(BaseSkim):
    """
    **Decay Modes**:

    * :math:`D^+_{(S)} \\to K^+ K^- \\pi^+`,
    * :math:`D^+_{(S)} \\to \\pi^+ \\pi^- K^+`,
    * For :math:`D^+_{(S)} \\to K_{S} h^+`, which includes a :math:`K_{S}` vertex, please check `XToDp_DpToKsHp`

    **Additional Cuts**:

    * ``Tracks: from the standard loose list``
    * ``1.7 < M(D+) < 2.2, pcms(D+) > 2``
    """

    __authors__ = ["Aman Sangal"]
    __description__ = "Skim list for D_(s)+ to h+ h- j+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)

    def build_lists(self, path):
        Dpcuts = "1.7 < M < 2.2 and useCMSFrame(p)>2"

        Dp_Channels = ["pi+:loose pi-:loose K+:loose",
                       "K+:loose K-:loose pi+:loose",
                       ]

        DpList = []
        for chID, channel in enumerate(Dp_Channels):
            ma.reconstructDecay("D+:HpHmJp" + str(chID) + " -> " + channel, Dpcuts, chID, path=path)
            DpList.append("D+:HpHmJp" + str(chID))

        self.SkimLists = DpList


@fancy_skim_header
class LambdacTopHpJm(BaseSkim):
    """
    **Decay Modes**:

    * :math:`\\Lambda_c^+ \\to p K^- \\pi^+`

    **Additional Cuts**:

    * ``Tracks: from the standard loose list``
    * ``2.2 < M(Lambda_c) < 2.4, pcms(Lambda_c) > 2.1``
    """

    __authors__ = ["Justin Guilliams"]
    __description__ = "Skim list for Lambda_c+ -> p K- pi+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)
        stdPr("loose", path=path)

    def build_lists(self, path):

        LambdacCuts = "2.2 < M < 2.4 and useCMSFrame(p) > 2.1"
        LambdacChannels = ["p+:loose K-:loose pi+:loose",
                           ]

        LambdacList = []
        for chID, channel in enumerate(LambdacChannels):
            ma.reconstructDecay("Lambda_c+:pHpJm" + str(chID) + " -> " + channel, LambdacCuts, chID, path=path)
            LambdacList.append("Lambda_c+:pHpJm" + str(chID))

        self.SkimLists = LambdacList


@fancy_skim_header
class DstToDpPi0_DpToHpPi0(BaseSkim):
    """
    **Decay Modes**:
    * :math:`D^{*+}\\to \\pi^{0} D^{+}, D^+ \\to \\pi^+ \\pi^0`

    **Additional Cuts**:

    * ``Tracks: same as D0ToHpJm()``
    * ``pi0: directly taken from pi0:skim standard list``
    * ``1.72 < M(D+) < 1.98, pcms(D+) > 2``
    * ``0 < Q < 0.018``
    """

    __authors__ = ["Johannes Brand"]
    __description__ = "Skim list for D*+ to D+ pi0, D+ to h+ pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:hppi0", mySel, path=path)

        Dpcuts = "1.72 < M < 1.98 and useCMSFrame(p)>2"
        Dp_Channels = ["pi+:hppi0 pi0:skim",
                       ]

        DstList = []
        for chID, channel in enumerate(Dp_Channels):
            ma.reconstructDecay("D+:HpPi0" + str(chID) + " -> " + channel, Dpcuts, chID, path=path)
            ma.reconstructDecay("D*+:HpPi0" + str(chID) + " -> D+:HpPi0" + str(chID) + " pi0:skim", "0 < Q < 0.018", path=path)
            DstList.append("D*+:HpPi0" + str(chID))

        self.SkimLists = DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpJm(XToD0_D0ToHpJm):
    """
    **Decay Modes**:

    1. :math:`D^{*+}\\to D^{0} \\pi^+`, where the D^{0} is reconstructed by `XToD0_D0ToHpJm.D0ToHpJm`

    **Additional Cuts**:

    1. ``loose Cuts in D0ToHpJm()``

    2. ``0 < Q < 0.018``
    """

    __authors__ = "Giulia Casarosa"
    __description__ = "Same as `XToD0_D0ToHpJm`, but requiring the D0 is from D*+ -> D0 pi+ process."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)

    def build_lists(self, path):
        D0List = self.D0ToHpJm(path)

        Dstcuts = '0 < Q < 0.018'

        DstList = []
        for chID, channel in enumerate(D0List):
            ma.reconstructDecay('D*+:HpJm' + str(chID) + ' -> D0:HpJm' + str(chID) + ' pi+:mygood', Dstcuts, chID, path=path)
            DstList.append('D*+:HpJm' + str(chID))

        self.SkimLists = DstList

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.reconstructDecay('D0:HpJm0_test -> pi+:loose K-:loose', '1.80 < M < 1.93 and useCMSFrame(p)>2.2', path=path)
        ma.reconstructDecay('D*+:HpJm0_test -> D0:HpJm0_test pi+:loose', '0 < Q < 0.018', path=path)

        vm.addAlias('M_D0', 'daughter(0,InvM)')
        vm.addAlias('Pcms_D0', 'daughter(0,useCMSFrame(p))')
        vm.addAlias('d0_spi', 'daughter(1,d0)')
        vm.addAlias('z0_spi', 'daughter(1,z0)')
        vm.addAlias('dr_spi', 'daughter(1,dr)')
        vm.addAlias('dz_spi', 'daughter(1,dz)')
        vm.addAlias('Pcms_spi', 'daughter(1,useCMSFrame(p))')
        vm.addAlias('Pcms_Dst', 'useCMSFrame(p)')

        histogramFilename = 'DstToD0Pi_D0ToHpJm_Validation.root'
        myEmail = 'Guanda Gong <gonggd@mail.ustc.edu.cn>'

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='D*+:HpJm0_test',
            variables_1d=[
                ('M_D0', 100, 1.80, 1., 'Mass distribution of $D^{0}$', myEmail,
                 'mass of D0 (mean=1.86483)', 'Please check agreement of: mean, sigma and ratio of signal and background',
                 'M(D^{0}) [GeV/c^{2}]', 'shifter'),
                ('Pcms_D0', 100, 2, 6, 'momentum of $D_{0}$ in CMS Frame', myEmail,
                 'CMS momentum of D0', 'Please check agreement of lineshape',
                 '$P_{cms}(D^{0}) [GeV/c^{2}]', 'shifter'),
                ('d0_spi', 100, -1.2, 1.2, 'd0 of slow pi', myEmail,
                 'd0 of slow pion', 'provided for the SVD and PXD group',
                 'd0_spi [cm]', 'shifter'),
                ('z0_spi', 100, -3.3, 3.3, 'z0 of slow pi', myEmail,
                 'z0 of slow pion', 'provided for the SVD and PXD group',
                 'z0_spi [cm]', 'shifter'),
                ('dr_spi', 100, -1.2, 1.2, 'dr of slow pi', myEmail,
                 'dr of slow pion', 'provided for the SVD and PXD group',
                 'dr_spi [cm]', 'shifter'),
                ('dz_spi', 100, -3.3, 3.3, 'dz of slow pi', myEmail,
                 'dz of slow pion', 'provided for the SVD and PXD group',
                 'dz_spi [cm]', 'shifter'),
                ('Pcms_spi', 100, 0, 0.8, 'momentum of slow pi in CMS Frame', myEmail,
                 'CMS momentum of slow pion', 'Please check agreement of lineshape',
                 'P_{cms}(#pi_{s}) [GeV/c]', 'shifter'),
                ('Pcms_Dst', 100, 2, 6, 'momentum of $D_{*}$ in CMS Frame', myEmail,
                 'CMS momentum of slow pion', 'Please check agreement of lineshape',
                 'P_{cms}(D*) {GeV/c}', 'shifter'),
                ('Q', 100, 0, 0.018, 'Released energy in $D^{*}$ decay', myEmail,
                 'Q = M(D0 pi) - M(D0) - M(pi), and it peaks around 0.006 GeV',
                 'Please check agreement of: mean, sigma and ratio of signal and background',
                 'Q [GeV]', 'shifter'),
            ],
            path=path)


@fancy_skim_header
class DstToD0Pi_D0ToHpJmPi0(BaseSkim):
    """
    **Decay Modes**:

    * :math:`RS: D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K^- \\pi^+ \\pi^{0}`
    * :math:`WS: D^{*-}\\to \\pi^- D^{0}, D^{0}\\to K^- \\pi^+ \\pi^{0}`


    **Additional Cuts**:

    * ``1.70 < M(D0) < 2.10``
    * ``M(D*)-M(D0) < 0.16``
    * ``pcms(D*) > 2.0``
    """

    __authors__ = ["Emma Oxford"]
    __description__ = (
        "Skim list for D*+ to pi+ D0, D0 to pi0 and two charged FSPs, where the kinds "
        "of two charged FSPs are different. The wrong sign(WS) mode, D*- to pi- D0, is "
        "also included."
    )
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"
        charmcuts = "1.70 < M < 2.10"
        ma.cutAndCopyList("pi0:myskim", "pi0:skim", "", path=path)  # additional cuts removed 27 Jun 2019 by Emma Oxford

        DstList = []
        # NOTE: renamed to avoid particle list name clashes
        ma.reconstructDecay("D0:HpJmPi0_withPID -> K-:loose pi+:loose pi0:myskim", charmcuts, path=path)
        ma.reconstructDecay("D*+:HpJmPi0RS_withPID -> D0:HpJmPi0_withPID pi+:loose", Dstcuts, path=path)
        ma.reconstructDecay("D*-:HpJmPi0WS_withPID -> D0:HpJmPi0_withPID pi-:loose", Dstcuts, path=path)
        ma.copyLists("D*+:HpJmPi0_withPID", ["D*+:HpJmPi0RS_withPID", "D*+:HpJmPi0WS_withPID"], path=path)
        DstList.append("D*+:HpJmPi0_withPID")

        self.SkimLists = DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpHmPi0(BaseSkim):
    """
    **Decay Modes**:

    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ \\pi^- \\pi^{0}`
    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K^+ K^- \\pi^{0}`

    **Additional Cuts**:

    * ``1.70 < M(D0) < 2.10``
    * ``M(D*)-M(D0) < 0.16``
    * ``pcms(D*) > 2.0``
    """

    __authors__ = ["Emma Oxford"]
    __description__ = "Skim list for D*+ to pi+ D0, D0 to pi0 and two conjugate charged FSPs."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"
        charmcuts = "1.70 < M < 2.10"
        ma.cutAndCopyList("pi0:myskim", "pi0:skim", "", path=path)  # additional cuts removed 27 Jun 2019 by Emma Oxford
        D0_Channels = [
            "pi+:loose pi-:loose pi0:myskim",
            "K+:loose K-:loose pi0:myskim",
        ]

        DstList = []

        for chID, channel in enumerate(D0_Channels):
            # NOTE: renamed to avoid particle list name clashes
            ma.reconstructDecay("D0:HpHmPi0" + str(chID) + "_withPID" + " -> " + channel, charmcuts, chID, path=path)
            ma.reconstructDecay(
                "D*+:HpHmPi0" + str(chID) + "_withPID" + " -> D0:HpHmPi0" + str(chID) + "_withPID" + " pi+:loose",
                Dstcuts, chID, path=path)
            DstList.append("D*+:HpHmPi0" + str(chID) + "_withPID")

        self.SkimLists = DstList


@fancy_skim_header
class DstToD0Pi_D0ToKsOmega(BaseSkim):
    """
    **Decay Modes**:

    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} \\pi^+ \\pi^- \\pi^{0}`


    **Additional Cuts**:

    * ``0.11 < M(pi0) < 0.15, p(pi0) > 25``
    * ``1.7< M(D0) < 2.0, pcms(D0) > 2.4``
    * ``Q < 0.018``
    """

    __authors__ = []
    __description__ = "Skim list for D*+ to pi+ D0, D0 to Ks pi+ pi- pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:ksomega", mySel, path=path)

        ma.cutAndCopyList("pi0:mypi0", "pi0:skim", "0.11 < M < 0.15 and p > 0.25 ", path=path)
        ma.reconstructDecay("omega:3pi -> pi+:ksomega pi-:ksomega pi0:mypi0", "", path=path)

        charmcuts = "1.7 < M < 2 and useCMSFrame(p)>2.4"
        ma.reconstructDecay("D0:KsOmega -> K_S0:merged omega:3pi", charmcuts, path=path)

        DstList = []
        ma.reconstructDecay("D*+:KsOmega -> D0:KsOmega pi+:ksomega", "0 < Q < 0.018", path=path)
        DstList.append("D*+:KsOmega")

        self.SkimLists = DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpHmHpJm(BaseSkim):
    """
    **Decay Modes**:

    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K^- \\pi^+ \\pi^- \\pi^+`
    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ \\pi^- \\pi^- \\pi^+`


    **Additional Cuts**:

    * ``Tracks: same as D0ToHpJm()``
    * ``1.7 < M(D0) < 2.0, pcms(D0) > 2.3``
    * ``Q < 0.022``
    """

    __authors__ = ["Kavita Lalwani, Chanchal Sharma"]
    __description__ = "Skim list for D*+ to K- pi+ pi- pi+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdK("all", path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:hphmhpjm", mySel, path=path)
        ma.fillParticleList("K+:hphmhpjm", mySel, path=path)

        D0_Channels = [
            "K-:hphmhpjm pi+:hphmhpjm pi-:hphmhpjm pi+:hphmhpjm",
            "pi+:hphmhpjm pi-:hphmhpjm pi+:hphmhpjm pi-:hphmhpjm",
        ]

        D0cuts = "1.75 < M < 1.95"
        Dstcuts = "0 < Q < 0.022 and useCMSFrame(p)>2.3"

        DstList = []
        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpHmHpJm" + str(chID) + " -> " + channel, D0cuts, chID, path=path)

            ma.reconstructDecay("D*+:HpHmHpJm" + str(chID) + " -> pi+:hphmhpjm D0:HpHmHpJm" + str(chID), Dstcuts, chID, path=path)
            DstList.append("D*+:HpHmHpJm" + str(chID))

        self.SkimLists = DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpJmEta(BaseSkim):
    """
    **Decay Modes**:

    * :math:`RS: D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K^- \\pi^+ \\eta, eta\\to \\gamma \\gamma`
    * :math:`WS: D^{*-}\\to \\pi^- D^{0}, D^{0}\\to K^- \\pi^+ \\eta, eta\\to \\gamma \\gamma`


    **Additional Cuts**:

    * ``0.49 < M(eta) < 0.55, p(eta) > 0.28``
    * ``1.78 < M(D0) < 1.93, pcms(D0) > 2.2``
    * ``Q < 0.018``
    """

    __authors__ = []
    __description__ = (
        "Skim list for D*+ to pi+ D0, D0 to eta and two charged FSPs, where the kinds "
        "of two charged FSPs are different. The wrong sign(WS) mode, D*- to pi- D0, is "
        "also included."
    )
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        ma.reconstructDecay("eta:myskim -> gamma:loose gamma:loose", "0.49 < M < 0.55 and p > 0.28", path=path)
        Dstcuts = "0 < Q < 0.018"
        charmcuts = "1.78 < M < 1.93 and useCMSFrame(p) > 2.2"

        DstList = []
        ma.reconstructDecay("D0:HpJmEta -> K-:loose pi+:loose eta:myskim", charmcuts, path=path)
        vertex.treeFit("D0:HpJmEta", 0.001, path=path)
        ma.reconstructDecay("D*+:HpJmEtaRS_eta -> D0:HpJmEta pi+:loose", Dstcuts, path=path)
        ma.reconstructDecay("D*-:HpJmEtaWS_eta -> D0:HpJmEta pi-:loose", Dstcuts, path=path)
        vertex.kFit("D*+:HpJmEtaRS_eta", conf_level=0.001, path=path)
        vertex.kFit("D*+:HpJmEtaWS_eta", conf_level=0.001, path=path)
        DstList.append("D*+:HpJmEtaRS_eta")
        DstList.append("D*+:HpJmEtaWS_eta")

        self.SkimLists = DstList


@fancy_skim_header
class DstToD0Pi_D0ToNeutrals(XToD0_D0ToNeutrals):
    """
    **Decay Modes**:

    * :math:`D^{*+}\\to \\pi^+ D^{0}`, where the D^{0} is reconstructed by `XToD0_D0ToNeutrals.D0ToNeutrals`.

    **Additional Cuts**:

    * ``1.70 < M(D0) < 2.10``
    * ``M(D*)-M(D0) < 0.16``
    * ``pcms(D*) > 2.0``
    """

    __authors__ = ["Giulia Casarosa", "Emma Oxford"]
    __description__ = "Same as `XToD0_D0ToNeutrals`, but requiring that the D0 is from D* decay."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:2Nbdy", mySel, path=path)

        D0List = self.D0ToNeutrals(path)

        Dstcuts = "0 < Q < 0.02"

        DstList = []
        for chID, channel in enumerate(D0List):
            ma.reconstructDecay("D*+:2Nbdy" + str(chID) + " -> pi+:2Nbdy " + channel, Dstcuts, chID, path=path)
            DstList.append("D*+:2Nbdy" + str(chID))

        self.SkimLists = DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpHmKs(BaseSkim):
    """
    **Decay Modes**:

    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} \\pi^+ \\pi^-`
    * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} K^+ K^-`

    **Additional Cuts**:

    * ``Tracks: same as D0ToHpJm()``
    * ``Ks: directly taken from stdKshort()``
    * ``1.7 < M(D0) < 2.0``
    * ``Q < 0.022``
    * ``pcms(D*) > 2.3``
    """

    __authors__ = ["Yeqi Chen"]
    __description__ = "Skim list for D*+ to pi+ D0, D0 to Ks and two conjugate charged FSPs."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        stdKshorts(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:hphmks", mySel, path=path)
        ma.fillParticleList("K+:hphmks", mySel, path=path)

        D0cuts = "1.7 < M < 2.0"
        Dstcuts = "0 < Q < 0.022 and useCMSFrame(p)>2.3"

        D0_Channels = ["pi-:hphmks pi+:hphmks K_S0:merged",
                       "K-:hphmks K+:hphmks K_S0:merged"
                       ]
        DstList = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpHmKs" + str(chID) + " -> " + channel, D0cuts, chID, path=path)

            ma.reconstructDecay("D*+:HpHmKs" + str(chID) + " -> pi+:hphmks D0:HpHmKs" + str(chID), Dstcuts, chID, path=path)
            DstList.append("D*+:HpHmKs" + str(chID))

        self.SkimLists = DstList


@fancy_skim_header
class EarlyData_DstToD0Pi_D0ToHpJmPi0(BaseSkim):
    """
    Cut criteria are not finally decided, and could be changed. Please check the
    code in the master branch to get up-to-date information.
    """

    __authors__ = []
    __description__ = "An special version of `DstToD0Pi_D0ToHpJmPi0` to deal with Early Data."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 0.5 and abs(z0) < 1.0"  # IP cut, tighter than previous skims
        mySel += " and 0.296706 < theta < 2.61799"  # CDC acceptance cut
        ma.fillParticleList("pi+:myhjp0", mySel, path=path)
        ma.fillParticleList("K+:myhjp0", mySel, path=path)

        ma.cutAndCopyList("pi0:myhjp0", "pi0:skim", "", path=path)  # see analysis/scripts/stdPi0s.py for cuts

        D0cuts = "1.70 < M < 2.10"
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"

        eventcuts = "nCleanedTracks(abs(d0) < 0.5 and abs(z0) < 1.0) >= 3"
        path = self.skim_event_cuts(eventcuts, path=path)

        DstList = []
        ma.reconstructDecay("D0:HpJmPi0 -> K-:myhjp0 pi+:myhjp0 pi0:myhjp0", D0cuts, path=path)
        ma.reconstructDecay("D*+:HpJmPi0RS -> D0:HpJmPi0 pi+:myhjp0", Dstcuts, path=path)
        ma.reconstructDecay("D*-:HpJmPi0WS -> D0:HpJmPi0 pi-:myhjp0", Dstcuts, path=path)
        ma.copyLists("D*+:HpJmPi0", ["D*+:HpJmPi0RS", "D*+:HpJmPi0WS"], path=path)
        DstList.append("D*+:HpJmPi0")

        self.SkimLists = DstList


@fancy_skim_header
class EarlyData_DstToD0Pi_D0ToHpHmPi0(BaseSkim):
    """
    Cut criteria are not finally decided, and could be changed. Please check the
    code in the master branch to get up-to-date information.
    """

    __authors__ = []
    __description__ = "A special version of `DstToD0Pi_D0ToHpHmPi0` to deal with Early Data."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    def load_standard_lists(self, path):
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 0.5 and abs(z0) < 1.0"  # IP cut, tighter than previous skims
        mySel += " and 0.296706 < theta < 2.61799"  # CDC acceptance cut
        ma.fillParticleList("pi+:myhhp0", mySel, path=path)
        ma.fillParticleList("K+:myhhp0", mySel, path=path)

        ma.cutAndCopyList("pi0:myhhp0", "pi0:skim", "", path=path)  # see analysis/scripts/stdPi0s.py for cuts

        D0cuts = "1.70 < M < 2.10"
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"

        eventcuts = "nCleanedTracks(abs(d0) < 0.5 and abs(z0) < 1.0) >= 3"
        path = self.skim_event_cuts(eventcuts, path=path)

        D0_Channels = ["pi+:myhhp0 pi-:myhhp0 pi0:myhhp0",
                       "K+:myhhp0 K-:myhhp0 pi0:myhhp0",
                       ]

        DstList = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpHmPi0" + str(chID) + " -> " + channel, D0cuts, chID, path=path)
            ma.reconstructDecay("D*+:HpHmPi0" + str(chID) + " -> D0:HpHmPi0" + str(chID) + " pi+:myhhp0",
                                Dstcuts, chID, path=path)
            DstList.append("D*+:HpHmPi0" + str(chID))

        self.SkimLists = DstList
