#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skims for leptonic analyses.
"""

__authors__ = [
    "Sophie Hollit",
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header
from variables import variables as vm
from validation_tools.metadata import create_validation_histograms


def LeptonicList(path):
    """
    Note:
        * **Skim description**: Skim for leptonic analyses,
          :math:`B_{\\text{sig}}^-\\to\\ell\\nu`, where :math:`\\ell=e,\\mu`.
        * **Skim LFN code**: 11130300
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)

    Build leptonic untagged skim lists, and supply the name of the
    lists. Uses the standard electron and muon particle lists, so
    these must be added to the path first.

    Example usage:

    >>> from stdCharged import stdE, stdMu
    >>> from skim.leptonic import LeptonicList
    >>> stdE('all', path=path)
    >>> stdMu('all', path=path)
    >>> LeptonicList(path)
    ['B-:L0', 'B-:L1']

    Reconstructed decays
        * :math:`B^- \\to e^-`
        * :math:`B^- \\to \\mu^-`

    Cuts applied
        * :math:`p_{\\ell}^{*} > 2\\,\\text{GeV}` in CMS Frame
        * :math:`\\text{electronID} > 0.5`
        * :math:`\\text{muonID} > 0.5`
        * :math:`n_{\\text{tracks}} \geq 3`

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``lepList``, a Python list containing the strings
        :code:`'B-:L0'` and :code:`'B-:L1'`, the names of the particle
        lists for leptonic :math:`B^-` skim candidates.
    """
    __authors__ = [
        "Phillip Urquijo"
    ]

    ma.cutAndCopyList('e-:highP', 'e-:all', 'useCMSFrame(p) > 2.0 and electronID > 0.5', True, path=path)
    ma.cutAndCopyList('mu-:highP', 'mu-:all', 'useCMSFrame(p) > 2.0 and muonID > 0.5', True, path=path)
    ma.reconstructDecay('B-:L0 -> e-:highP', '', 1, path=path)
    ma.reconstructDecay('B-:L1 -> mu-:highP', '', 2, path=path)
    ma.applyCuts('B-:L0', 'nTracks>=3', path=path)
    ma.applyCuts('B-:L1', 'nTracks>=3', path=path)
    lepList = ['B-:L0', 'B-:L1']
    return lepList


@fancy_skim_header
class LeptonicUntagged(BaseSkim):
    """
    Reconstructed decays
        * :math:`B^- \\to e^-`
        * :math:`B^- \\to \\mu^-`

    Cuts applied
        * :math:`p_{\\ell}^{*} > 2\\,\\text{GeV}` in CMS Frame
        * :math:`\\text{electronID} > 0.5`
        * :math:`\\text{muonID} > 0.5`
        * :math:`n_{\\text{tracks}} \\geq 3`
    """

    __authors__ = ["Phillip Urquijo"]
    __SkimDescription__ = (
        "Skim for leptonic analyses, "
        ":math:`B_{\\text{sig}}^-\\to\\ell\\nu`, where :math:`\\ell=e,\\mu`"
    )
    __WorkingGroup__ = "Semileptonic and Missing Energy Working Group (WG1)"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
            "stdMu": ["all"]
        }
    }

    def build_lists(self, path):
        ma.cutAndCopyList(
            "e-:highP",
            "e-:all",
            "useCMSFrame(p) > 2.0 and electronID > 0.5",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu-:highP",
            "mu-:all",
            "useCMSFrame(p) > 2.0 and muonID > 0.5",
            True,
            path=path,
        )
        ma.reconstructDecay("B-:L0 -> e-:highP", "", 1, path=path)
        ma.reconstructDecay("B-:L1 -> mu-:highP", "", 2, path=path)
        ma.applyCuts("B-:L0", "nTracks>=3", path=path)
        ma.applyCuts("B-:L1", "nTracks>=3", path=path)
        lepList = ["B-:L0", "B-:L1"]
        self.SkimLists = lepList

    def validation_histograms(self, path):
        ma.cutAndCopyLists("B-:LeptonicUntagged", ["B-:L0", "B-:L1"], "", path=path)

        ma.buildRestOfEvent("B-:LeptonicUntagged", path=path)
        ma.appendROEMask(
            "B-:LeptonicUntagged",
            "basic",
            "pt>0.05 and -2<dr<2 and -4.0<dz<4.0",
            "E>0.05",
            path=path,
        )
        ma.buildContinuumSuppression("B-:LeptonicUntagged", "basic", path=path)

        vm.addAlias("d0_p", "daughter(0,p)")
        vm.addAlias("d0_electronID", "daughter(0,electronID)")
        vm.addAlias("d0_muonID", "daughter(0,muonID)")
        vm.addAlias("MissP", "weMissP(basic,0)")

        histogramFilename = "LeptonicUntagged_Validation.root"
        contact = "Phil Grace <philip.grace@adelaide.edu.au>"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B-:LeptonicUntagged",
            variables_1d=[
                ("Mbc", 100, 4.0, 5.3, "Mbc", contact, "", ""),
                ("d0_p", 100, 0, 5.2, "Signal-side lepton momentum", contact, "", ""),
                ("d0_electronID", 100, 0, 1, "electronID of signal-side lepton",
                 contact, "", ""),
                ("d0_muonID", 100, 0, 1, "electronID of signal-side lepton", contact,
                 "", ""),
                ("R2", 100, 0, 1, "R2", contact, "", ""),
                ("MissP", 100, 0, 5.3, "Missing momentum of event (CMS frame)", contact,
                 "", ""),
            ],
            variables_2d=[
                ("deltaE", 100, -5, 5, "Mbc", 100, 4.0, 5.3, "Mbc vs deltaE", contact,
                 "", "")
            ],
            path=path,
        )
