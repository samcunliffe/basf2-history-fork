#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Study of alignment limits.

import basf2
from ROOT.Belle2 import KLMDisplacementGenerator

# Create main path
main = basf2.create_path()
basf2.set_log_level(basf2.LogLevel.INFO)

# EventInfoSetter
main.add_module('EventInfoSetter')

# Gearbox
main.add_module('Gearbox')

# Process the main path
basf2.process(main)

displacementGenerator = KLMDisplacementGenerator()
displacementGenerator.studyAlignmentLimits('EKLMAlignmentLimits.root')
