#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>SVDTrackingValidation.root</output>
  <description>
  This module validates that the svd only track finding is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

import tracking
from tracking.validation.run import TrackingValidationRun
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'SVDTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)


def setupFinderModule(path):
    tracking.add_hit_preparation_modules(path, components=["SVD"])
    tracking.add_vxd_track_finding_vxdtf2(path, components=["SVD"])


class SVD4Layer(TrackingValidationRun):
    """
    Validation class for the four 4-SVD Layer tracking
    """
    #: the number of events to process
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: root input file to use, generated by central validation script
    root_input_file = '../EvtGenSimNoBkg.root'
    #: use full detector for validation
    components = None

    #: lambda method which is used by the validation to add the svd finder modules
    finder_module = staticmethod(setupFinderModule)

    #: use only the svd hits when computing efficiencies
    tracking_coverage = {
        'WhichParticles': ['SVD'],  # Include all particles seen in the SVD detector, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': True,
        'UseCDCHits': False,
    }

    #: perform fit after track finding
    fit_tracks = True
    #: plot pull distributions
    pulls = True
    #: output file of plots
    output_file_name = VALIDATION_OUTPUT_FILE

    # tweak sectormap
    # def adjust_path(self, path):
    #     basf2.set_module_parameters( path, "SectorMapBootstrap", ReadSecMapFromDB=False)
    #     basf2.set_module_parameters( path, "SectorMapBootstrap", ReadSectorMap=True)
    #     basf2.set_module_parameters( path, "SectorMapBootstrap", SectorMapsInputFile="mymap.root")


def main():
    """
    create SVD validation class and execute
    """
    validation_run = SVD4Layer()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
