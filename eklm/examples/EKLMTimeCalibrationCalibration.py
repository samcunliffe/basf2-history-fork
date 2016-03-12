#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM time calibration.

from basf2 import *
from reconstruction import *
from ROOT import Belle2
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

root_input = register_module('RootInput')
root_input.param('inputFileName', sys.argv[1])
root_input.initialize()

gearbox = register_module('Gearbox')
gearbox.initialize()

algo = Belle2.EKLMTimeCalibrationAlgorithm()
algo.execute()
algo.commit()
