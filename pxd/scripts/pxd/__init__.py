#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_pxd_reconstruction(path):

    pxd_clusterizer = register_module('PXDClusterizer')
    path.add_module(pxd_clusterizer)


def add_pxd_simulation(path):

    pxd_digitizer = register_module('PXDDigitizer')
    path.add_module(pxd_digitizer)
