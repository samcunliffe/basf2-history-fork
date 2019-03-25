#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Example of a steering file for producing summary ntuple of beam background hit rates
#
# usage: basf2 beamBkgHitRates.py expNo runNo globalTag
# ---------------------------------------------------------------------------------------

import basf2
import sys
import glob
from rawdata import add_unpackers

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "expNo runNo globalTag")
    sys.exit()

expNo = 'e' + '{:0=4d}'.format(int(argvs[1]))
runNo = 'r' + '{:0=5d}'.format(int(argvs[2]))
global_tag = argvs[3]

indir = '/hsm/belle2/bdata/Data/Raw/' + expNo + '/' + runNo + '/sub00'
files = sorted(glob.glob(indir + '/*.root'))
if len(files) == 0:
    B2ERROR('No files found in ' + indir)
    sys.exit()

outdir = '.'
outputFile = outdir + '/beamBkgHitRates-' + expNo + '-' + runNo + '.root'

# Define global tag
basf2.reset_database()
basf2.use_central_database(global_tag)

# Create path
main = basf2.create_path()

# Input (raw data)
main.add_module('RootInput', inputFileNames=files)

# Gearbox
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Unpacking
# some detectors are temporary excluded
# - EKLM because of missing payload in the above global tag
# - BKLM because of making segmentation violation
# - PXD because of too many warnings on exp 3 data
add_unpackers(path=main,
              components=['SVD', 'CDC', 'TOP', 'ARICH', 'ECL'])

# additional modules if needed for hit processing
main.add = module('ARICHFillHits')
main.add_module('TOPChannelMasker')

# Beam background rate monitor: output to flat ntuple
main.add_module('BeamBkgHitRateMonitor', outputFileName=outputFile)

# Show progress of processing
main.add_module('Progress')

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
