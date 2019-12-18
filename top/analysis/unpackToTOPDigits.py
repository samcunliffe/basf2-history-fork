#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Unpack raw data to TOPDigits
# Usage: basf2 unpackToTOPDigits.py -i <input_file.sroot> -o <output_file.root>
# ---------------------------------------------------------------------------------------

from basf2 import *

# Define a global tag (note: the one given bellow will become out-dated!)
use_central_database('data_reprocessing_proc8')

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = register_module('TOPUnpacker')
main.add_module(unpack)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
main.add_module(converter)

# output
output = register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'TOPRawDigits', 'TOPInterimFEInfos',
                             'TOPRawDigitsToTOPInterimFEInfos',
                             # 'TOPRawWaveforms', 'TOPRawWaveformsToTOPInterimFEInfos',
                             # 'TOPRawDigitsToTOPRawWaveforms',
                             ])
main.add_module(output)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
