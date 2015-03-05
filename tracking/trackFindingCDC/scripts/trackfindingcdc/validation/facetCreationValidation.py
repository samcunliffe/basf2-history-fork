#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtrackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners
import tracking.metamodules as metamodules

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin


import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class FacetCreationValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    segment_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
    segment_finder_module.param({
        "WriteFacets": True,
        "FacetFilter": "all",
        "FacetNeighborChooser": "none",
    })

    py_profile = True
    output_file_name = "FacetCreationValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        argument_parser = super(FacetCreationValidationRun, self).create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(FacetCreationValidationRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        # main_path.add_module(AxialStereoPairFitterModule())
        validation_module = FacetCreationValidationModule(output_file_name=self.output_file_name)
        if self.py_profile:
            main_path.add_module(metamodules.PyProfilingModule(validation_module))
        else:
            main_path.add_module(validation_module)

        return main_path


class FacetCreationValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the facet creation cuts and compose validation plots on terminate."""

    cos_cut_value = math.cos(math.pi / 180.0 * 9)

    def __init__(self, output_file_name):
        super(FacetCreationValidationModule, self).__init__(foreach="CDCRecoFacetVector",
                                                            output_file_name=output_file_name)
        self.mc_facet_filter = None

    def initialize(self):
        super(FacetCreationValidationModule, self).initialize()
        self.mc_facet_filter = Belle2.TrackFindingCDC.MCFacetFilter()
        self.real_facet_filter = Belle2.TrackFindingCDC.SimpleFacetFilter()

    def prepare(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, facet):
        return True

    def peel(self, facet):
        crops = self.peel_target(facet)
        crops.update(self.peel_fit(facet))

        real_select = self.real_select(facet)
        crops["real_select"] = real_select

        return crops

    def peel_target(self, facet):
        mc_weight = self.mc_facet_filter.isGoodFacet(facet)
        mc_decision = np.isfinite(mc_weight)  # Filters for nan

        return dict(
            mc_weight=mc_weight,
            mc_decision=mc_decision,
        )

    def peel_fit(self, facet):
        # No fitless variables for now
        fitless_crops = self.peel_fitless(facet)

        # Fit the tangent lines
        self.fit(facet)

        start_to_middle_line = facet.getStartToMiddleLine()
        start_to_end_line = facet.getStartToEndLine()
        middle_to_end_line = facet.getMiddleToEndLine()

        start_cos = start_to_middle_line.tangential().cosWith(start_to_end_line.tangential())
        middle_cos = start_to_middle_line.tangential().cosWith(middle_to_end_line.tangential())
        end_cos = start_to_end_line.tangential().cosWith(middle_to_end_line.tangential())

        crops = dict(
            start_cos=start_cos,
            middle_cos=middle_cos,
            end_cos=end_cos,
            max_cos=max(start_cos, middle_cos, end_cos),
            min_cos=min(start_cos, middle_cos, end_cos),
            no_middle_max_cos=max(start_cos, end_cos),
            no_middle_min_cos=min(start_cos, end_cos),
        )

        select = self.select(crops)
        crops["select"] = select

        return crops

    def peel_fitless(self, facet):
        # There is some short cuts to investigate here. Find a bachelor to do it.
        return {}

    def fit(self, facet):
        facet.adjustLines()

    def select_fitless(self, fitless_crops):
        return True

    def select(self, crops):
        return crops["no_middle_min_cos"] > self.cos_cut_value

    def real_select(self, facet):
        real_weigth = self.real_facet_filter.isGoodFacet(facet)
        return np.isfinite(real_weigth)

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True, folder_name="histograms")
    save_tree = refiners.save_tree(folder_name="tree")

    # Investigate the preselection
    save_selection_variables_histograms = refiners.save_histograms(
        select=[
            "mc_decision",
            "start_cos",
            "middle_cos",
            "end_cos",
            "max_cos",
            "min_cos",
            "no_middle_max_cos",
            "no_middle_min_cos",
        ],
        stackby="mc_decision",
        lower_bound=cos_cut_value,
        upper_bound=1,
        folder_name="selection_variables",
    )

    save_select_quality_histograms = refiners.save_histograms(
        select=[
            "mc_decision",
            "select",
            "real_select",
        ],
        stackby="mc_decision",
        allow_discrete=True,
        folder_name="select_quality",
    )

    @refiners.context()
    def print_signal_number(self, crops, tdirectory, **kwds):
        info = get_logger().info

        mc_decisions = crops["mc_decision"]
        n = len(mc_decisions)
        n_signal = np.sum(mc_decisions)
        n_background = n - n_signal
        info("#Signal : %s", n_signal)
        info("#Background : %s", n_background)

        selects = np.nonzero(crops["select"])
        info("#Signal after selection : %s", np.sum(mc_decisions[selects]))
        info("#Background after selection : %s", np.sum(1 - mc_decisions[selects]))


def main():
    run = FacetCreationValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
