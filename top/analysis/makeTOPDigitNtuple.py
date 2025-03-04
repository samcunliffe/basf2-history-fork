#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Unpack raw data with InterimFE and production FW to TOPDigits and convert into ntuple
# Usage: basf2 makeTOPDigitNtuple.py <input_file.sroot>
# ---------------------------------------------------------------------------------------

import basf2 as b2
import sys
import argparse
import re
from ROOT import TMath
from plotInterimFEDataNtuple import plotInterimFEDataNtupleSummary

parser = argparse.ArgumentParser(description="Create ntuple file and plots from interimFE data")
parser.add_argument("inputFile", nargs='?', default="NoInputFile",
                    help="input sroot file name")
parser.add_argument("--outputFile", default="NoOutputFile",
                    help="output root file name")
parser.add_argument("--calChannel", type=int, default=0,
                    help="asic channel number where calibration signals are injected [0-7]")
parser.add_argument(
    "--lookbackWindows",
    type=int,
    default=0,
    help="lookback windows to redefine rawTime so that t=0 is beginning of search windows."
    " Give 0 (default) not to allow the redefinition.")
parser.add_argument("--heightThreshold", type=int, default=40,
                    help="pulse height threshold in offline feature extraction in a unit of ADC counts")
parser.add_argument("--noOfflineFE", action="store_true", default=False,
                    help="Use only online FE hits, and do not use waveform data")
parser.add_argument("--saveWaveform", action="store_true", default=False,
                    help="Save waveform data in the output ntuple file")
parser.add_argument("--globalDAQ", action="store_true", default=False,
                    help="Force to assume global DAQ data.")
parser.add_argument("--pocketDAQ", action="store_true", default=False,
                    help="Force to assume global DAQ data.")
parser.add_argument("--skipPlot", action="store_true", default=False,
                    help="Skip making summary plot.")
parser.add_argument("--interim", action="store_true", default=False,
                    help="process data taken with interimFE FW")
args = parser.parse_args()

if args.inputFile == "NoInputFile":
    print("Create a flat ntuple file from sroot data file(s) taken with interimFE firmware and plots for quick data quality check.")
    print("usage:")
    print("basf2 makeTOPDigitNtuple.py (input_filename.sroot) [--arg --outputFile output_ntuple.root]")
    print("                            [--arg --calChannel asicCh]")
    print("                            [--arg --lookbackWindows windows]")
    print("                            [--arg --heightThreshold threshold]")
    print("                            [--arg --noOfflineFE] [--arg --saveWaveform]")
    print("                            [--arg --globalDAQ] [--arg --pocketDAQ]")
    print("                            [--arg --skipPlot] [--arg --interim]")
    print("*Switching of local/global run and output file name is automatically given as folows if it is not specified:")
    print("  runXXXXXX_slotYY_ntuple.root (local run with PocketDAQ)")
    print(" or top.XXXXX.YYYYYY_ntuple.root (global run with global DAQ)")
    print("*Deafult asic channel number with calibration signals is 0 (can be changed with \"--calChannel\" option.)")
    print("*Deafult the number of lookback windows is 0, with which timing correction is not applied.")
    print(" (rawTime is corrected so that it is measured with respect to the start of search windows when lookbackWindows>0.)")
    print("*Option \"--noOfflineFE\"  : disable offline FE from waveform data.")
    print("                             Calculation of reference timing is based on hit in calibration channel.")
    print("*       \"--saveWaveform\" : save waveform data in the output ntuple file")
    print("*       \"--globalDAQ\"    : force to assume global DAQ data as an input file")
    print("*       \"--pocketDAQ\"    : force to assume pocketDAQ data as an input file")
    print("*       \"--skipPlot\"     : only processing sroot data file, do not create summary plot")
    print("*       \"--interim\"      : process data taken with interimFE FW")
    sys.exit()

inputFile = args.inputFile
isGlobalDAQ = False
isOfflineFEDisabled = args.noOfflineFE
isGlobalDAQForced = args.globalDAQ
isPocketDAQForced = args.pocketDAQ
calCh = args.calChannel
lookbackWindows = args.lookbackWindows
heightThreshold = args.heightThreshold
isInterimFE = args.interim
if calCh < 0 or calCh > 7:
    print("ERROR : invalid calibration asic channel :" + str(calCh))
    print("        (should be [0-7])")
    sys.exit()

if re.search(r"run[0-9]+_slot[0-1][0-9]", inputFile):
    outputRoot = re.search(r"run[0-9]+_slot[0-1][0-9]", inputFile).group() + "_ntuple.root"
elif re.search(r"(top|cosmic|cdc|ecl|klm|test|debug|beam|physics|hlttest)\.[0-9]+\.[0-9]+", inputFile):
    isGlobalDAQ = True
    outputRoot = re.search(
        r"(top|cosmic|cdc|ecl|klm|test|debug|beam|physics|hlttest)\.[0-9]+\.[0-9]+",
        inputFile).group() + "_ntuple.root"
else:
    outputRoot = inputFile + "_ntuple.root"

if args.outputFile != "NoOutputFile":
    outputRoot = args.outputFile

if isGlobalDAQForced and (not isPocketDAQForced):
    isGlobalDAQ = True
elif (not isGlobalDAQForced) and isPocketDAQForced:
    isGlobalDAQ = False
elif isGlobalDAQForced and isPocketDAQForced:
    print("ERROR : both of --GlobalDAQ or --PocketDAQ can not be given.")
    sys.exit()

print(inputFile + " --> " + outputRoot)
print("Is global DAQ?        : " + str(isGlobalDAQ))
print("OfflineFE             : " + str(not isOfflineFEDisabled))
print("Save waveform?        : " + str(args.saveWaveform))
print("Cal. asic ch          : " + str(calCh))
print("# of lookback windows : " + str(lookbackWindows))
print()
print("start process...")

# Define a global tag (note: the one given bellow will become out-dated!)
b2.use_central_database('data_reprocessing_proc8')

# Create path
main = b2.create_path()

roinput = b2.register_module('SeqRootInput')
roinput.param('inputFileName', inputFile)
main.add_module(roinput)

# HistoManager
histoman = b2.register_module('HistoManager')
histoman.param('histoFileName', outputRoot)
main.add_module(histoman)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
if not isGlobalDAQ:
    converter = b2.register_module('Convert2RawDet')
    main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
if isInterimFE:  # need to be tested
    unpack.param('swapBytes', True)
    unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
if not isOfflineFEDisabled:
    featureExtractor = b2.register_module('TOPWaveformFeatureExtractor')
    featureExtractor.param('threshold', heightThreshold)
    featureExtractor.param('hysteresis', TMath.CeilNint(heightThreshold * 0.4 - 0.00001))
    main.add_module(featureExtractor)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('lookBackWindows', lookbackWindows)
converter.param('storageDepth', 508)
converter.param('calibrationChannel', calCh)  # if set, cal pulses will be flagged
# converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMin', 300)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 1.2)  # in [ns]
converter.param('calpulseWidthMax', 2.8)  # in [ns]
main.add_module(converter)

xtalk = b2.register_module('TOPXTalkChargeShareSetter')
main.add_module(xtalk)

ntuple = b2.register_module('TOPInterimFENtuple')
ntuple.param('saveWaveform', (args.saveWaveform))
if isInterimFE:
    ntuple.param('useDoublePulse', (not isOfflineFEDisabled))
ntuple.param('calibrationChannel', calCh)
# ntuple.param('minHeightFirstCalPulse', 450)  # in [ADC counts]
# ntuple.param('minHeightSecondCalPulse', 450)  # in [ADC counts]
ntuple.param('minHeightFirstCalPulse', 300)  # in [ADC counts]
ntuple.param('minHeightSecondCalPulse', 300)  # in [ADC counts]
ntuple.param('nominalDeltaT', 21.5)  # in [ns]
ntuple.param('nominalDeltaTRange', 2)  # in [ns]
ntuple.param('globalRefSlotNum', 1)
ntuple.param('globalRefAsicNum', 0)
ntuple.param('timePerWin', 23.581939)  # in [ns]
main.add_module(ntuple)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)

if isInterimFE and not args.skipPlot:
    plotInterimFEDataNtupleSummary(outputRoot, 2, isOfflineFEDisabled)
