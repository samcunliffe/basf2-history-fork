#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToNtuple module saves variables from the VariableManager to a candidate-based TNtuple

from basf2 import *
from modularAnalysis import *

import os
if os.path.isfile('mdst.root'):
    filename = 'mdst.root'
elif os.path.isfile('/storage/jbod/tkeck/MC7/evtgen-charged/sub00/mdst_000240_prod00000788_task00000685.root'):
    filename = '/storage/jbod/tkeck/MC7/evtgen-charged/sub00/mdst_000240_prod00000788_task00000685.root'
elif os.path.isfile('/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000788/s00/e0000/4S/r00000/'
                    'charged/sub00/mdst_000240_prod00000788_task00000685.root'):
    filename = '/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000788/s00/e0000/4S/r00000/'\
               'charged/sub00/mdst_000240_prod00000788_task00000685.root'
else:
    raise RuntimeError("Please copy an mdst file from KEKCC into this directory named mdst.root")

inputMdstList('MC7', [filename])

fillParticleLists([('K-', 'kaonID > 0.2'), ('pi+', 'pionID > 0.2')])
reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95')
matchMCTruth('D0')

# This will write out one row per candidate in the D0 list
analysis_main.add_module('VariablesToExtraInfo',
                         particleList='D0',
                         variables={'M': 'M_before_vertex_fit'})

fitVertex('D0', -2.0)

analysis_main.add_module('VariablesToNtuple',
                         particleList='D0',
                         variables=['M', 'extraInfo(M_before_vertex_fit)'])

process(analysis_main)
print(statistics)
