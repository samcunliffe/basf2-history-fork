#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
import ROOT
from ROOT.Belle2 import IntervalOfValidity

set_log_level(LogLevel.DEBUG)

algo = ROOT.Belle2.TestCalibrationAlgorithm()
# Set the prefix manually if you want to use the algorithm for a specific collector
algo.setPrefix("TestHisto")

# Can use a Python list of input files/wildcards. It will resolve the existing files
inputFileNames = ["MyOutputFile_*.root"]
algo.setInputFileNames(inputFileNames)
# Python list object of files returned
d = algo.getInputFileNames()
for name in d:
    print("Resolved input file to algorithm:", name)

# Can use python lists/tuples to define the runs whose data you want to use [(exp,run), ...]
# IoV of the payloads will be calculated from this input list
print("Result of calibration =", algo.execute([(1, 1), (1, 2)]))
# localdb isn't updated until you call this
algo.commit()

# Can run over all collected data and auto define the IoV of the payloads
print("Result of calibration =", algo.execute())
# Can commit multiple times in one process since the payloads are cleared at the start
# of each execution
algo.commit()

# Could also define an IoV for your calibrations at the start of execution
unbound_iov = IntervalOfValidity(0, 0, 1, 1)
print("Result of calibration =", algo.execute([], 0, unbound_iov))
# IoV is applied to payloads during execution and gets used during commit
algo.commit()

# The algo.execute() method takes:
#
# execute(list runs, int iteration, IntervalOfValidity iov)
#
# if 'runs' is empty then all the input data will be used
# Don't use the 'runs' argument with granularity='all' collected data.
# It is meaningless to request speicifc data when all data was merged earlier
