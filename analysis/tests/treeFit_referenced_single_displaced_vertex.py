#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import unittest
import tempfile
import basf2
import modularAnalysis as ma
import b2test_utils
from ROOT import TFile


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file(
            'analysis/1000_B_Jpsi_ks_pipi.root', 'validation', py_case=self)
        ma.inputMdst('default', inputfile, path=main)

        ma.fillParticleList('pi+:a', 'pidProbabilityExpert(211, ALL) > 0.5', path=main)

        ma.reconstructDecay('K_S0:all -> pi+:a pi-:a', '', 0, path=main)
        ma.matchMCTruth('K_S0:all', path=main)

        conf = 0
        main.add_module('TreeFitter',
                        particleList='K_S0:all',
                        confidenceLevel=conf,
                        massConstraintList=[],
                        massConstraintListParticlename=[],
                        expertUseReferencing=True,
                        ipConstraint=False,
                        updateAllDaughters=False)

        ntupler = basf2.register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['chiProb', 'M', 'isSignal'])
        ntupler.param('particleList', 'K_S0:all')
        main.add_module(ntupler)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        truePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal > 0)")
        falsePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal == 0)")

        mustBeZero = ntuple.GetEntries(f"(chiProb < {conf})")

        print(f"True fit survivors: {truePositives} out of {allSig} true candidates")
        print(f"False fit survivors: {falsePositives} out of {allBkg} false candidates")

        self.assertFalse(truePositives == 0, "No signal survived the fit.")

        self.assertTrue(falsePositives < 2586, f"Too many false positives: {falsePositives} out of {allBkg} total bkg events.")

        self.assertTrue(truePositives > 540, "Signal rejection too high")
        self.assertFalse(mustBeZero, f"We should have dropped all candidates with confidence level less than {conf}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
