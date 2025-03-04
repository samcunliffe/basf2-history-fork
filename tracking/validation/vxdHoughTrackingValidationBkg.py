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
  <input>EvtGenSim.root</input>
  <output>VXDHoughTrackingValidationBkg.root</output>
  <description>
  This module validates that the DATCON SVD only track finding is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

import tracking
from tracking.validation.run import TrackingValidationRun
import logging
import basf2
from vxdHoughTracking.vxdHoughTracking_functions import add_VXDHoughTracking

VALIDATION_OUTPUT_FILE = 'VXDHoughTrackingValidationBkg.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)


class VXDHoughTrackingValidationBkg(TrackingValidationRun):
    """
    Validation class for the DATCON tracking
    """
    #: the number of events to process
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: root input file to use, generated by central validation script
    root_input_file = '../EvtGenSim.root'
    #: use full detector for validation
    components = None

    @staticmethod
    def finder_module(path):
        """Add the VXDHoughTracking module and related modules to the basf2 path"""
        tracking.add_hit_preparation_modules(path, components=["SVD"])
        add_VXDHoughTracking(path, reco_tracks='RecoTracks', use_simple_roi_calculation=False)

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


def main():
    """
    create SVD validation class and execute
    """
    validation_run = VXDHoughTrackingValidationBkg()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
