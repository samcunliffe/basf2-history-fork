#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

import basf2
from ROOT.Belle2 import KLMTimeAlgorithm
from klm_calibration_utils import get_time_pre_collector_path
from prompt.calibrations.caf_klm_time import get_collector
from caf.framework import CAF, Calibration, Collection

basf2.set_log_level(basf2.LogLevel.INFO)

input_files = sys.argv[1:]

# Create KLM time calibration.
algorithm = KLMTimeAlgorithm()
cal_klm = Calibration(name='KLMTime', algorithms=algorithm)
coll_cdst = get_collector('hlt_mumu', 'klmTime')
rec_path_cdst = get_time_pre_collector_path(muon_list_name='klmTime', raw_format=False)
collection_cdst = Collection(collector=coll_cdst,
                             input_files=input_files,
                             pre_collector_path=rec_path_cdst)
cal_klm.add_collection(name='cdst', collection=collection_cdst)

# Create and run calibration framework.
framework = CAF()
framework.add_calibration(cal_klm)
framework.run()
