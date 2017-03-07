# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_cdc_trigger(path, minHits=None):
    """
    This function adds the CDC trigger modules to a path.
    """

    # standard CDC trigger
    trgcdc = register_module('TRGCDC')
    trgcdc_params = {
        'ConfigFile': Belle2.FileSystem.findFile("data/trg/cdc/TRGCDCConfig_0_20101111.dat"),
        'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
        'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
        'HoughFinderMappingFileMinus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingMinus20160223.dat"),
        'HoughFinderMappingFilePlus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingPlus20160223.dat"),
        '2DfinderCollection': 'TRGOld2DFinderTracks',
        '2DfitterCollection': 'TRG2DFitterTracks',
        '3DfitterCollection': 'TRG3DFitterTracks'}
    trgcdc.param(trgcdc_params)
    path.add_module(trgcdc)
    # new 2D finder
    hough = register_module('CDCTriggerHoughtracking')
    hough.param('outputCollection', 'TRG2DFinderTracks')
    if minHits:
        hough.param('minHits', minHits)
    path.add_module(hough)
    # neurotrigger
    neuro = register_module('NeuroTrigger')
    neuro.param('inputCollection', 'TRG2DFinderTracks')
    neuro.param('outputCollection', 'TRGNNTracks')
    neuro.param('filename', Belle2.FileSystem.findFile("data/trg/cdc/Neuro20170109.root"))
    path.add_module(neuro)
