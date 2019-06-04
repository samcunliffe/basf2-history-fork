#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import os
import tempfile
from basf2 import *
from modularAnalysis import *
from vertex import vertexTree
from ROOT import Belle2
from ROOT import TFile
from ROOT import TNtuple


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputMdst('default',  Belle2.FileSystem.findFile('1000_B_DstD0Kpi_skimmed.root', 'validation'), path=main)

        fillParticleList('pi+:a', 'pionID > 0.5', path=main)
        fillParticleList('K+:a', 'kaonID > 0.5', path=main)

        reconstructDecay('D0:rec -> K-:a pi+:a', '', 0, path=main)
        reconstructDecay('D*+:rec -> D0:rec pi+:a', '', 0, path=main)
        reconstructDecay('B0:rec -> D*+:rec pi-:a', ' InvM > 5', 0, path=main)
        matchMCTruth('B0:rec', path=main)

        conf = 0
        main.add_module('TreeFitter',
                        particleList='B0:rec',
                        confidenceLevel=conf,
                        massConstraintList=[],
                        massConstraintListParticlename=[],
                        expertUseReferencing=True,
                        ipConstraint=True,
                        updateAllDaughters=False)

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['chiProb', 'M', 'isSignal'])
        ntupler.param('particleList', 'B0:rec')
        main.add_module(ntupler)

        process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        truePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal > 0)")
        falsePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal == 0)")

        mustBeZero = ntuple.GetEntries("(chiProb < {})".format(conf))

        print("True fit survivors: {0} out of {1} true candidates".format(truePositives, allSig))
        print("False fit survivors: {0} out of {1} false candidates".format(falsePositives, allBkg))

        self.assertFalse(truePositives == 0, "No signal survived the fit.")

        self.assertTrue(falsePositives < 1521, f"Background rejection to high {falsePositives} out of {allBkg}")

        self.assertTrue(truePositives > 152, "Signal rejection too high {truePositives} out of {allSig}")
        self.assertFalse(mustBeZero, "We should have dropped all candidates with confidence level less than {}.".format(conf))

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
