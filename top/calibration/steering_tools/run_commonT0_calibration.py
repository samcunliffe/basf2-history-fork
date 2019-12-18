#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: common T0
# data type: cdst (bhabha or dimuon)
#
# usage: basf2 run_commonT0_calibration.py expNo runFirst runLast [sample method]
#        sample = bhabha/dimuon (D = bhabha)
#        method = BF/LL (D = BF)
#
# author: M. Staric
# ---------------------------------------------------------------------------------------

import sys
import os
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import commonT0_calibration_BF, commonT0_calibration_LL

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTags = ['data_reprocessing_prompt_rel4_patchb']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/'
bhabha_skim_dir = 'skim/hlt_bhabha/cdst/sub00'
dimuon_skim_dir = 'offskim/offskim_mumutop/cdst/sub00'
main_output_dir = 'top_calibration'
default_sample = 'bhabha'
default_method = 'BF'
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast [sample method]")
    print("       sample = bhabha/dimuon (D = bhabha)")
    print("       method = BF/LL (D = BF)")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])
sample = default_sample
method = default_method

if len(argvs) == 5:
    sample = argvs[4]
elif len(argvs) > 5:
    sample = argvs[4]
    method = argvs[5]

if sample == 'bhabha':
    skim_dir = bhabha_skim_dir
elif sample == 'dimuon':
    skim_dir = dimuon_skim_dir
else:
    B2ERROR("Invalid sample name: " + sample)
    sys.exit()

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/{expNo}/*/{runNo}/{skim_dir}/cdst*.root"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    B2ERROR('No cdst files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last) +
            ' (skim_dir=' + skim_dir + ')')
    sys.exit()

# Output folder name
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/commonT0-{sample}-{method}-{expNo}-{run_range}"

# Define calibration
if method == 'BF':
    cal = commonT0_calibration_BF(inputFiles, globalTags, localDBs)
elif method == 'LL':
    cal = commonT0_calibration_LL(inputFiles, sample, globalTags, localDBs)
else:
    B2ERROR('Invalid method name: ' + method)
    sys.exit()
cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
