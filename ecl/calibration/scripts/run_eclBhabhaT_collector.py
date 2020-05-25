#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------------------------------------------
# BASF2 script for the first (out of two) step of time shift calibration.
# using bhabha events.
# --------------------------------------------------------------------------
#
# There are two ways you can use it:
# 1. Provide parameters from command line:
#   basf2 EclBhabhaTCollector.py -i "/path/to/input/files/*.root" -o collector_output.root
#
# 2. Set parameters directly in steering file.
#   Change INPUT_LIST and OUTPUT variables.
#   (Multiple files can be easily added with glob.glob("/path/to/your/files/*.root"))
#   And then call
#   basf2 EclBhabhaTCollector.py

import basf2 as b2
from ROOT import Belle2
import glob
import tracking
import rawdata
import reconstruction

from basf2 import conditions as b2conditions
from reconstruction import prepare_cdst_analysis


env = Belle2.Environment.Instance()

###################################################################
# == Input/Output Parameters
###################################################################

# == List of input files
# NOTE: It is going to be sorted (alphabetic and length sorting, files with
#       shortest names are first)
INPUT_LIST = []
# = Processed data
INPUT_LIST += glob.glob("oneTestFile/*.root")

# == Output file
OUTPUT = "eclBhabhaTCollector.root"

########################
# Input/output overrides.

# Override input if "-i file.root" argument was sent to basf2.
input_arg = env.getInputFilesOverride()
if len(input_arg) > 0:
    INPUT_LIST = [str(x) for x in input_arg]
# Sort list of input files.
INPUT_LIST.sort(key=lambda item: (len(item), item))

# Override output if "-o file.root" argument was sent to basf2.
output_arg = env.getOutputFileOverride()
if len(output_arg) > 0:
    OUTPUT = output_arg

###################################################################
# == Collector parameters
###################################################################


# Events with abs(time_ECL-time_CDC) > TIME_ABS_MAX are excluded
TIME_ABS_MAX = 250

# If true, output file will contain TTree "tree" with detailed
# event information.
SAVE_TREE = False

# First ECL CellId to calibrate
MIN_CRYSTAL = 1
# Last ECL CellId to calibrate
MAX_CRYSTAL = 8736

# Bias of CDC event t0 in bhabha vs hadronic events (in ns)
CDC_T0_BIAS_CORRECTION_OFFSET = -0.9

###################################################################

components = ['CDC', 'ECL']

# == Create path
main = b2.create_path()

add_unpackers = False

# == SeqRoot/Root input
if INPUT_LIST[0].endswith('sroot'):
    main.add_module('SeqRootInput', inputFileNames=INPUT_LIST)
    add_unpackers = True
else:
    main.add_module('RootInput', inputFileNames=INPUT_LIST)

main.add_module("HistoManager", histoFileName=OUTPUT)

if 'Raw' in INPUT_LIST[0]:
    add_unpackers = True


main.add_module('Gearbox')

if add_unpackers:
    rawdata.add_unpackers(main, components=components)

    # = Get Tracks, RecoTracks, ECLClusters, add relations between them.
    tracking.add_tracking_reconstruction(main, components=components)
    reconstruction.add_ext_module(main, components)
    reconstruction.add_ecl_modules(main, components)
    reconstruction.add_ecl_track_matcher_module(main, components)

prepare_cdst_analysis(main)  # for new 2020 cdst format

# == Generate time calibration matrix from ECLDigit
ECLBhabhaTCollectorInfo = main.add_module('ECLBhabhaTCollector', timeAbsMax=TIME_ABS_MAX,
                                          minCrystal=MIN_CRYSTAL, maxCrystal=MAX_CRYSTAL,
                                          saveTree=SAVE_TREE,
                                          hadronEventT0_TO_bhabhaEventT0_correction=CDC_T0_BIAS_CORRECTION_OFFSET)

ECLBhabhaTCollectorInfo.set_log_level(b2.LogLevel.INFO)  # OR: LogLevel.DEBUG
ECLBhabhaTCollectorInfo.set_debug_level(36)


# == Show progress
main.add_module('Progress')

# set_log_level(LogLevel.DEBUG)
b2.set_log_level(b2.LogLevel.INFO)
b2.set_debug_level(100)

# == Configure database
# b2.reset_database()
# b2.use_database_chain()

b2conditions.reset()
b2conditions.override_globaltags()

B2INFO("Adding Local Database {} to head of chain of local databases.")
b2conditions.prepend_testing_payloads("localdb/database.txt")
B2INFO("Using Global Tag {}")
b2conditions.prepend_globaltag("ECL_testingNewPayload_RefCrystalPerCrate")
b2conditions.prepend_globaltag("master_2020-05-13")
b2conditions.prepend_globaltag("online_proc11")
b2conditions.prepend_globaltag("data_reprocessing_proc11")
b2conditions.prepend_globaltag("Reco_master_patch_rel5")


# old method
# use_central_database("ECL_testingNewPayload_RefCrystalPerCrate")
# use_central_database("master_2020-05-13")
# use_central_database("online_proc11")
# use_central_database("data_reprocessing_proc11")
# use_central_database("Reco_master_patch_rel5")
#

# Read in any required central databases

# b2.use_central_database("data_reprocessing_proc10")
# b2.use_central_database("data_reprocessing_prompt_rel4_patchb")

# Read in any required local databases.  This may be required when doing crystal/crate iterations
# b2.use_local_database("localdb/database.txt")


# == Process events
# process(main, max_event=350000)  # reasonable stats for one crate
# process(main, max_event=600000)  # reasonable stats for crystal calibs for proc10
# process(main, max_event=3000)    # reasonable stats and speed for a quick test
b2.process(main)                      # process all events

print(b2.statistics)
