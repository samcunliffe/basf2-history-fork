#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

from daqdqm.commondqm import add_common_dqm


def add_cosmic_dqm(path, components=None, dqm_environment="expressreco", dqm_mode="dont_care"):
    """
    This function adds DQMs for CRT

    @param components: A list of the detector components which are available in this
                       run of basf2
    @param dqm_environment: The environment the DQM modules are running in
                            "expressreco" (default) if running on the ExpressReco system
                            "hlt" if running on the HLT online reconstructon nodes
                            If running on the hlt, you may want to output less or other DQM plots
                            due to the limited bandwith of the HLT nodes.
    @param dqm_mode: How to split up the path for online/HLT.
                     For dqm_mode == "dont_care" all the DQM modules should be added.
                     For dqm_mode == "all_events" only the DQM modules which should run on all events
                            (filtered and dismissed) should be added
                     For dqm_mode == "before_reco" only thw DQM modules which should run before
                            all reconstruction
                     For dqm_mode == "filtered"  only the DQM modules which should run on filtered
                            events should be added
    """
    assert dqm_mode in ["dont_care", "all_events", "filtered", "before_reco"]

    add_common_dqm(path, components=components, dqm_environment=dqm_environment, dqm_mode="dont_care")
