#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <output>inclusiveBplusToKplusNuNu_Validation.root</output>
    <contact>trshillington@hep.physics.mcgill.ca</contact>
</header>
"""

# NOTE: This file is auto-generated by b2skim-generate-validation.
#       Do not make changes here, as your changes may be overwritten.
#       Instead, make changes to the validation_histograms method of
#       inclusiveBplusToKplusNuNu.

import basf2 as b2
import modularAnalysis as ma
from skim.WGs.ewp import inclusiveBplusToKplusNuNu

path = b2.Path()
skim = inclusiveBplusToKplusNuNu(validation=True, udstOutput=False)

ma.inputMdstList(
    "default",
    b2.find_file(skim.validation_sample, data_type="validation"),
    path=path,
)
skim(path)

b2.process(path)
