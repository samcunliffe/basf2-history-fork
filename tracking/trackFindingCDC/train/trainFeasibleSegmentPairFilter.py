#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path

from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin


class FeasibleSegmentPairFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record segment pairs encountered at the SegmentPairCreator and retrain its mva method"""
    n_events = 10000
    generator_module = "generic"
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    truth = "truth_positive"

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()

        # In contrast to other training use only the first *half* loop for more aggressive training
        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards",
                        UseNLoops=0.5)

        if self.task == "train":
            varSets = [
                "feasible",
                "filter(truth)",
            ]

        elif self.task == "eval":
            varSets = [
                "filter(feasible)",
                "filter(truth)",
            ]

        elif self.task == "explore":
            varSets = [
                "basic",
                # "feasible",
                "fitless",
                "pre_fit",
                # "fit",
                "truth",  # for weighting
                # "old_fit",
                # "filter(fitless)",
                # "filter(simple)",
                # "filter(feasible)",
                "filter(truth)",
            ]

        else:
            raise ValueError("Unknown task " + self.task)

        # Also fix the segment orientation to outwards to make training additionally aggressive
        path.add_module("TFCDC_TrackFinderAutomaton",
                        SegmentOrientation="outwards",
                        SegmentPairFilter="unionrecording",
                        SegmentPairFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": varSets,
                        })

        return path


def main():
    run = FeasibleSegmentPairFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
