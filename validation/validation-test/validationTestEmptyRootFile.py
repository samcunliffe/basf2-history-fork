#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>validationTestEmptyRootFile.root</output>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>
</header>


This file will not generate an output root file to probe
the validation suite properly handles this case.
"""

from ROOT import TFile


VALIDATION_OUTPUT_FILE = 'validationTestEmptyRootFile.root'

tfile = TFile(VALIDATION_OUTPUT_FILE, "RECREATE")
tfile.Close()
