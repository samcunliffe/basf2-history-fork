#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM time conversion payloads.

import sys
from ROOT.Belle2 import KLMDatabaseImporter, KLMTimeConversion

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

dbImporter = KLMDatabaseImporter()

timeConversion = KLMTimeConversion()
# TDC frequency in GHz.
timeConversion.setTDCFrequency(1.017728000)
# Time offset in ns.
timeConversion.setTimeOffset(0)
# CTIME shift in bits.
timeConversion.setCTimeShift(2)

if (mc):
    dbImporter.setIOV(1002, 0, 1002, -1)
    timeConversion.setCTimeShift(2)
    dbImporter.importTimeConversion(timeConversion)

    dbImporter.setIOV(1003, 0, 1003, -1)
    timeConversion.setCTimeShift(0)
    dbImporter.importTimeConversion(timeConversion)

    dbImporter.setIOV(0, 0, 0, -1)
    timeConversion.setCTimeShift(0)
    dbImporter.importTimeConversion(timeConversion)

else:
    dbImporter.setIOV(0, 0, 3, -1)
    timeConversion.setCTimeShift(2)
    dbImporter.importTimeConversion(timeConversion)

    dbImporter.setIOV(4, 0, -1, -1)
    timeConversion.setCTimeShift(0)
    dbImporter.importTimeConversion(timeConversion)
