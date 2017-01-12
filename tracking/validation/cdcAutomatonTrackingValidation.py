#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>CDCAutomatonTrackingValidation.root</output>
  <description>
  This module validates that cdc cellular automaton track finding
  is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CDCAutomatonTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

from tracking.validation.run import TrackingValidationRun


class CDCAutomaton(TrackingValidationRun):
    n_events = N_EVENTS
    root_input_file = '../EvtGenSimNoBkg.root'

    def finder_module(self, path):
        path.add_module('TrackFinderCDCAutomaton',
                        # UseNLoops = 1,
                        )

    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        # 'WhichParticles': ['CDC'], # Uncomment to count also secondary tracks
        # "UseNLoops" : 1,
        # 'EnergyCut': 0.1,
    }
    pulls = True
    contact = CONTACT
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCAutomaton()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
