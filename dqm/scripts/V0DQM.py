# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add the V0 DQM module.
"""

import basf2
import modularAnalysis as ma
from vertex import kFit


def add_V0_dqm(path):
    ma.fillParticleList("pi+:V0DQM", '', False, path)
    ma.reconstructDecay('K_S0:V0DQM -> pi-:V0DQM pi+:V0DQM', '', 1, True, path)
    kFit('K_S0:V0DQM', 0.001, path=path)

    dqm = basf2.register_module('V0ObjectsDQM')
    dqm.param('V0PListName', 'K_S0:V0DQM')
    path.add_module(dqm)
