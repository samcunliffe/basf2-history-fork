#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2
from tracking.run.event_generation import StandardEventGenerationRun

import logging


def get_logger():
    return logging.getLogger(__name__)


class CDCSegmentPairTruthRecordingRun(StandardEventGenerationRun):
    n_events = 100
    generator_name = "simple_gun"
    # bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")
    segment_finder_module = basf2.register_module("TrackFinderCDCAutomatonDev")
    segment_finder_module.param({
        "SegmentPairFilter": "unionrecording",
        "SegmentPairFilterParameters": {
            "root_file_name": "CDCSegmentPairTruthRecords.root",
            "varsets": "fitless,truth",
        },
        "SegmentPairRelationFilter": "none"
    })

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(CDCSegmentPairTruthRecordingRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)
        return main_path


def main():
    run = CDCSegmentPairTruthRecordingRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
