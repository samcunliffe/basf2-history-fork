#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>SVDTrackingValidation.root</output>
  <description>
  This module validates that the svd only track finding is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'SVDTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

from tracking.validation.run import TrackingValidationRun
import tracking


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
    finder_module = staticmethod(lambda path: tracking.add_vxd_track_finding(path, components=["SVD"]))

    #: use only the svd hits when computing efficiencies
    tracking_coverage = {
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
    create SVD validation classa and execute
    """
    validation_run = SVD4Layer()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
