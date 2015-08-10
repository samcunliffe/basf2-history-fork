#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
from basf2 import *
from ROOT import Belle2
import modularCalibration as cal

set_random_seed(100)
set_log_level(LogLevel.INFO)
use_local_database('db/database.txt', '', LogLevel.WARNING)

# Makes list of 2-tuples [(exp, runs[0]), (exp, runs[1]), ...]


def exp_runs(exp, runs):
    if not isinstance(runs, list):
        runs = [runs]
    expruns = []
    for run in runs:
        expruns.append((exp, run))
    return expruns

cal.setDstStoragePattern('alignment/examples/DST_Exp{experiment}_Run{run}.root')

# Set path and format of DST files and runs, where magnet is off
# If you don't, you should see plenty of failed extrapolations
cal.setMagnetOffRuns(exp_runs(1, range(6, 15 + 1)))

# createCalibration(name='tadeasTest1.root')#, importCalibration='database.root')

cal.selectRange(1, 1, 1, 20)

cal.selectSample(exp_runs(1, [1, 2, 4]) +            # Exp 1, runs 1,2,4
                 exp_runs(1, range(6, 11 + 1)) +     # Exp 1, runs 6,7,8,9,10,11
                 [(1, 18), (1, 19)] +                # Exp 1, runs 18, 19
                 exp_runs(1, 20))                    # Exp 1, run  20

cal.loadGeometry()

# Generate misalignment in first pass
if not os.path.isfile('calibration_cache.txt'):
    misalignment = cal.VXDMisalignment('VXDMisalignment')
    # misalign everything
    misalignment.genSensorU('0.0.0', 0.01)
    misalignment.genSensorV('0.0.0', 0.01)
    misalignment.genSensorW('0.0.0', 0.01)
    misalignment.genSensorAlpha('0.0.0', 0.001)
    misalignment.genSensorBeta('0.0.0', 0.001)
    misalignment.genSensorGamma('0.0.0', 0.001)

    # Reset misalignment to zero for sensor we fix
    # in PedeSteering (see below)
    misalignment.setSensorAll('4.0.1', 0.0)
    misalignment.setSensorAll('5.0.1', 0.0)
    misalignment.setSensorAll('6.0.0', 0.0)

    cal.generateMisalignment(misalignment)

cal.useMisalignment('VXDMisalignment')
cal.refitGBL()

cal.createPedeSteering(name='PedeSteeringFine',
                       commands=['method diagonalization 3 0.1',
                                 'chiscut 30. 6.',
                                 'outlierdownweighting 3',
                                 'dwfractioncut 0.1'],
                       fix=['4.0.1.0', '5.0.1.0', '6.0.0.0'])  # fix slanted SVD's + whole 6th layer

cal.addMillepedeCalibration(name='Millepede',
                            granularity='data',
                            steering='PedeSteeringFine')
"""

cal.addMillepedeCalibration(name='MP2_perrun',
                            granularity='1.1.1.1,1.2.1.2')
cal.addMillepedeCalibration(name='MP2_perdata',
                            granularity='data',
                            dependency='MP2_perrun')
cal.addMillepedeCalibration(name='MP2_perdata_fine',
                            granularity='data',
                            dependency='MP2_perdata',
                            steering='PedeSteeringFine')
"""

cal.calibrate()
print statistics
