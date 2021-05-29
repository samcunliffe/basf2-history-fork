#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>DATCONTrackingValidation.root</output>
  <description>
  This module validates that the DATCON SVD only track finding is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

import tracking
from tracking.validation.run import TrackingValidationRun
import logging
import basf2
from datcon.datcon_functions import add_datcon

VALIDATION_OUTPUT_FILE = 'DATCONTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)

rootFileName = 'trackCandAnalysisNoBkg.root'


class DATCONTrackingValidation(TrackingValidationRun):
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

    @staticmethod
    def finder_module(path):
        tracking.add_hit_preparation_modules(path, components=["SVD"])
        add_datcon(path, datcon_reco_tracks='RecoTracks', use_simple_roi_calculation=False)

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
    #: create expert-level histograms
    use_expert_folder = True
    #: Include resolution information in the validation output
    resolution = True
    #: Use the fit information in validation
    use_fit_information = True
    #: output file of plots
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    """
    create SVD validation class and execute
    """
    validation_run = DATCONTrackingValidation()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
