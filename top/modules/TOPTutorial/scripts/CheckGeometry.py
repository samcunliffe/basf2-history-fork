#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file creates the Belle II detector
# geometry and checks for overlaps
######################################################

from basf2 import *

# Geometry parameter loader
gearbox = register_module('Gearbox')
# Geometry builder
geometry = register_module('Geometry')
geometry.param('Components', ['TOP'])
# Overlap checker
overlapchecker = register_module('OverlapChecker')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(overlapchecker)

# Process one event
process(main)
