#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import BKLM electronics mapping payloads.

import sys
import basf2
import ROOT
from ROOT.Belle2 import KLMDatabaseImporter, KLMElementNumbers, BKLMElementNumbers

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

# Database importer.
dbImporter = KLMDatabaseImporter()


def clear_electronics_map():
    """
    Clear electronics map (to load another version).
    """
    dbImporter.clearElectronicsMap()


def load_bklm_electronics_map(version, mc):
    """
    Load BKLM electronics map.
    Versions:
    1 = before experiment 10.
    2 = experiment 10 and later (mapping in chimney sector has changed).
    """
    if mc:
        dbImporter.loadBKLMElectronicsMap(1)
    else:
        dbImporter.loadBKLMElectronicsMap(version)
        # Switch lanes for real-data map.
        # The fibers of layer 1 and 2 are switched in BB6.
        dbImporter.setElectronicsMapLane(KLMElementNumbers.c_BKLM,
                                         BKLMElementNumbers.c_BackwardSection,
                                         7, 1, 2)
        dbImporter.setElectronicsMapLane(KLMElementNumbers.c_BKLM,
                                         BKLMElementNumbers.c_BackwardSection,
                                         7, 2, 1)


def load_eklm_electronics_map(version, mc):
    """
    Load EKLM electronics map.
    Versions:
    1 = phase 2 (wrong connection of cables for backward sectors 2 and 3).
    2 = phase 3
    """
    dbImporter.loadEKLMElectronicsMap(version, mc)

if mc:
    # MC map: a single version of map is used for all periods.

    load_bklm_electronics_map(1, True)
    load_eklm_electronics_map(1, True)

    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importElectronicsMap()

    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importElectronicsMap()

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importElectronicsMap()

else:
    # Data map.

    # Experiments 0 - 3.
    load_bklm_electronics_map(1, False)
    load_eklm_electronics_map(1, False)
    dbImporter.setIOV(1, 0, 3, -1)
    dbImporter.importElectronicsMap()

    # Experiments 4 - 9.
    clear_electronics_map()
    load_bklm_electronics_map(1, False)
    load_eklm_electronics_map(2, False)
    dbImporter.setIOV(4, 0, 9, -1)
    dbImporter.importElectronicsMap()

    # Experiment 10 and later.
    clear_electronics_map()
    load_bklm_electronics_map(2, False)
    load_eklm_electronics_map(2, False)
    dbImporter.setIOV(10, 0, -1, -1)
    dbImporter.importElectronicsMap()
