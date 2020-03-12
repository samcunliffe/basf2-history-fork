#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from b2biiConversion import convertBelleMdstToBelleIIMdst

os.environ['PGUSER'] = 'g0db'

main = basf2.create_path()

# add all modules necessary to read and convert the mdst file
inputfile = basf2.find_file('b2bii_input_evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst', 'examples', False)
convertBelleMdstToBelleIIMdst(inputfile, applySkim=True, path=main)

# Print out the contents of the DataStore
main.add_module('PrintCollections')

# progress
main.add_module('Progress')

basf2.process(main)

# Print call statistics
print(basf2.statistics)
