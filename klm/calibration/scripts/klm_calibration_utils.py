# -*- coding: utf-8 -*-

"""Implements some extra utilities for doing KLM calibration with the CAF."""

import basf2

from rawdata import add_unpackers
from reconstruction import add_cosmics_reconstruction, add_reconstruction, prepare_cdst_analysis
import modularAnalysis as ma


def get_alignment_pre_collector_path_cosmic(entry_sequence=""):
    """
    Parameters:
        entry_sequence  (str): A single entry sequence e.g. '0:100' to help limit processed events.

    Returns:
        basf2.Path:  A reconstruction path to run before the collector. Used for raw cosmic input files.
    """
    main = basf2.create_path()
    if entry_sequence:
        main.add_module('RootInput',
                        entrySequences=[entry_sequence])

    main.add_module('Gearbox')
    main.add_module('Geometry')

    # Unpackers and reconstruction.
    add_unpackers(main)
    add_cosmics_reconstruction(main, pruneTracks=False, add_muid_hits=True,
                               merge_tracks=False)
    # Disable the time window in muid module by setting it to 1 second.
    # This is necessary because the  alignment needs to be performed before
    # the time calibration; if the time window is not disabled, then all
    # scintillator hits are rejected.
    basf2.set_module_parameters(main, 'Muid', MaxDt=1e9)

    main.add_module('DAFRecoFitter',
                    pdgCodesToUseForFitting=[13],
                    resortHits=True)

    main.add_module('SetupGenfitExtrapolation',
                    noiseBetheBloch=False,
                    noiseCoulomb=False,
                    noiseBrems=False)

    return main


def get_alignment_pre_collector_path_physics(entry_sequence=""):
    """
    Parameters:
        entry_sequence  (str): A single entry sequence e.g. '0:100' to help limit processed events.

    Returns:
        basf2.Path:  A reconstruction path to run before the collector. Used for raw physics input files.
    """
    main = basf2.create_path()
    if entry_sequence:
        main.add_module('RootInput',
                        entrySequences=[entry_sequence])

    main.add_module('Gearbox')
    main.add_module('Geometry')

    # Unpackers and reconstruction.
    add_unpackers(main)
    add_reconstruction(main, pruneTracks=False, add_muid_hits=True)
    # Disable the time window in muid module by setting it to 1 second.
    # This is necessary because the  alignment needs to be performed before
    # the time calibration; if the time window is not disabled, then all
    # scintillator hits are rejected.
    basf2.set_module_parameters(main, 'Muid', MaxDt=1e9)

    main.add_module('DAFRecoFitter', resortHits=True)

    main.add_module('SetupGenfitExtrapolation',
                    noiseBetheBloch=False,
                    noiseCoulomb=False,
                    noiseBrems=False)

    return main


def get_strip_efficiency_pre_collector_path(entry_sequence="", raw_format=True):
    """
    Parameters:
        entry_sequence  (str): A single entry sequence e.g. '0:100' to help limit processed events.
        raw_format  (bool): True if cDST input files are in the raw+tracking format.

    Returns:
        basf2.Path:  A reconstruction path to run before the collector. Used for cDST input files.
    """
    main = basf2.create_path()
    if entry_sequence:
        main.add_module('RootInput',
                        entrySequences=[entry_sequence])
    if raw_format:
        prepare_cdst_analysis(main)
    else:
        main.add_module('Gearbox')
        main.add_module('Geometry')

    # Fill muon particle list
    ma.fillParticleList('mu+:all',
                        '1 < p and p < 11 and abs(d0) < 2 and abs(z0) < 5',
                        path=main)

    return main
