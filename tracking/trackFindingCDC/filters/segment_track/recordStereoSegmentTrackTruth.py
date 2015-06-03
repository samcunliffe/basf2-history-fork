#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import tracking.modules as modules
from tracking.run.event_generation import StandardEventGenerationRun
import logging
import sys


class StereoSegmentTrackTrackRecordingRun(StandardEventGenerationRun):
    n_events = 50
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(StereoSegmentTrackTrackRecordingRun, self).create_path()

        main_path.add_module(modules.CDCBackgroundHitFinder())
        main_path.add_module(modules.CDCLocalTrackFinder())
        main_path.add_module(modules.CDCBackgroundHitFinder())
        main_path.add_module(modules.CDCLegendreTrackFinder())
        main_path.add_module(modules.CDCStereoSegmentTrackMatcher(filter="recording"))
        return main_path


def main():
    run = StereoSegmentTrackTrackRecordingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
