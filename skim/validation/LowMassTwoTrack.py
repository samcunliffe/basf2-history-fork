#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>LowMassTwoTrack_Validation.root</output>
    <contact>xing-yu.zhou@desy.de</contact>
</header>
"""

# NOTE: This file is auto-generated by b2skim-generate-validation.
#       Do not make changes here, as your changes may be overwritten.
#       Instead, make changes to the validation_histograms method of
#       LowMassTwoTrack.

import basf2 as b2
import modularAnalysis as ma
from skim.lowMulti import LowMassTwoTrack

path = b2.Path()
skim = LowMassTwoTrack(validation=True, udstOutput=False)

ma.inputMdstList(
    "default",
    b2.find_file(skim.validation_sample, data_type="validation"),
    path=path,
)
skim(path)

b2.process(path)
