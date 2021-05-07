#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Reconstruction with EKLM time calibration data collection.

import sys
import basf2
from reconstruction import add_reconstruction

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

input = basf2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

gearbox = basf2.register_module('Gearbox')

eklmtimecalibration = basf2.register_module('EKLMTimeCalibrationCollector')

# Create the main path and add the modules
main = basf2.create_path()
main.add_module(input)
main.add_module("HistoManager", histoFileName=sys.argv[2])
main.add_module(gearbox)
add_reconstruction(main)
main.add_module(eklmtimecalibration)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
