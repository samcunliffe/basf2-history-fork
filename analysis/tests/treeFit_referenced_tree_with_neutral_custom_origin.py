#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import os
import tempfile
import basf2
import b2test_utils
import modularAnalysis as ma
from vertex import vertexTree
from ROOT import Belle2
from ROOT import TFile
from ROOT import TNtuple


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file(
            'analysis/1000_B_DstD0Kpipi0_skimmed.root', 'validation', py_case=self)
        ma.inputMdst('default', inputfile, path=main)

        ma.fillParticleList('pi+:a', 'pionID > 0.5', path=main)
        ma.fillParticleList('K+:a', 'kaonID > 0.5', path=main)

        ma.fillParticleList('gamma:a', 'E > 0.08', path=main)
        ma.reconstructDecay('pi0:a -> gamma:a gamma:a', '0.125 < InvM < 0.145', 0, path=main)

        ma.reconstructDecay('D0:rec -> K-:a pi+:a pi0:a', '', 0, path=main)
        ma.reconstructDecay('D*+:rec -> D0:rec pi+:a', '', 0, path=main)
        ma.reconstructDecay('B0:rec -> D*+:rec pi-:a', ' InvM > 5', 0, path=main)
        ma.matchMCTruth('B0:rec', path=main)

        conf = 0
        main.add_module('TreeFitter',
                        particleList='B0:rec',
                        confidenceLevel=conf,
                        massConstraintList=[],
                        massConstraintListParticlename=[],
                        expertUseReferencing=True,
                        ipConstraint=False,
                        updateAllDaughters=False,
                        customOriginConstraint=True,
                        # just to test this doesnt crash
                        customOriginVertex=[0, 0, 0],
                        customOriginCovariance=[1, 0, 0, 0, 1, 0, 0, 0, 1]
                        )

        ntupler = basf2.register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['chiProb', 'M', 'isSignal'])
        ntupler.param('particleList', 'B0:rec')
        main.add_module(ntupler)

        basf2.process(main)

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

        self.assertTrue(falsePositives < 3032, f"Too many false positives: {falsePositives} out of {allBkg} total bkg events.")

        self.assertTrue(truePositives > 64, "Signal rejection too high")
        self.assertFalse(mustBeZero, "We should have dropped all candidates with confidence level less than {}.".format(conf))

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
