#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import os
from simulation import add_simulation
import glob
import sys

# -----------------------------------------------------------------------------------
# This example is for preparing a root file for BG overlay using simulated BG samples
# Output file: BGforOverlay.root
# -----------------------------------------------------------------------------------

argvs = sys.argv
if len(argvs) > 1:
    if argvs[1] == 'phase2':
        phase = 2
        compression = 3
    elif argvs[1] == 'phase3':
        phase = 3
        compression = 4
    elif argvs[1] == 'phase31':
        phase = 31
        compression = 4
    else:
        B2ERROR('The argument can be either phase2, phase3 or phase31')
        sys.exit()
else:
    B2ERROR('No argument given specifying the running phase')
    B2INFO('Usage: basf2 ' + argvs[0] + ' phase2/phase3/phase31' + ' [scaleFactor=1]')
    sys.exit()

scaleFactor = 1.0
if len(argvs) > 2:
    scaleFactor = float(argvs[2])

if 'BELLE2_BACKGROUND_MIXING_DIR' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_MIXING_DIR variable is not set - it must contain the path to BG samples')
    sys.exit()

bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if len(bg) == 0:
    B2ERROR('No root files found in folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    sys.exit()

if phase == 2:
    for fileName in bg:
        if 'phase2' not in fileName:
            B2ERROR('BG mixing samples given in BELLE2_BACKGROUND_MIXING_DIR are not for phase 2')
            B2INFO('Try:\n export BELLE2_BACKGROUND_MIXING_DIR=/group/belle2/BGFile/OfficialBKG/15thCampaign/phase2/set0/')
            sys.exit()

B2INFO('Making BG overlay sample for ' + argvs[1] + ' with ECL compression = ' +
       str(compression))
B2INFO('Using background samples from folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
B2INFO('With scaling factor: ' + str(scaleFactor))

set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
if phase == 2:
    gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
elif phase == 31:
    gearbox.param('fileName', 'geometry/Belle2_earlyPhase3.xml')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('useDB', False)
main.add_module(geometry)

# Beam background mixer
bkgmixer = register_module('BeamBkgMixer')
bkgmixer.param('backgroundFiles', bg)
bkgmixer.param('overallScaleFactor', scaleFactor)
# PXD is sensitive to hits in intervall -20us to +20us
bkgmixer.param('minTimePXD', -20000.0)
bkgmixer.param('maxTimePXD', 20000.0)
main.add_module(bkgmixer)

# Emulate injection vetos for PXD
pxd_veto_emulator = register_module('PXDInjectionVetoEmulator')
main.add_module(pxd_veto_emulator)

# PXD digitizer (no data reduction!)
pxd_digitizer = register_module('PXDDigitizer')
main.add_module(pxd_digitizer)

# SVD digitizer
svd_digitizer = register_module('SVDDigitizer')
main.add_module(svd_digitizer)

# CDC digitization
cdc_digitizer = register_module('CDCDigitizer')
cdc_digitizer.param("Output2ndHit", False)
main.add_module(cdc_digitizer)

# TOP digitization
top_digitizer = register_module('TOPDigitizer')
main.add_module(top_digitizer)

# ARICH digitization
arich_digitizer = register_module('ARICHDigitizer')
main.add_module(arich_digitizer)

# ECL digitization
ecl_digitizer = register_module('ECLDigitizer')
main.add_module(ecl_digitizer, WaveformMaker=True, CompressionAlgorithm=compression)

# BKLM digitization
bklm_digitizer = register_module('BKLMDigitizer')
main.add_module(bklm_digitizer)

# EKLM digitization
eklm_digitizer = register_module('EKLMDigitizer')
main.add_module(eklm_digitizer)

# Output: digitized hits only
output = register_module('RootOutput')
output.param('outputFileName', 'BGforOverlay.root')
output.param('branchNames', ['PXDDigits', 'PXDInjectionBGTiming', 'SVDShaperDigits', 'CDCHits', 'TOPDigits',
                             'ARICHDigits', 'ECLWaveforms', 'BKLMDigits', 'EKLMDigits'])
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
