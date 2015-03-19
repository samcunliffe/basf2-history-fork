#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.validation.module import SeparatedTrackingValidationModule

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from trackfindingcdc.cdcLegendreTrackingValidation import ReassignHits

import logging

import root_pandas
import pandas
import matplotlib.pyplot as plt
import seaborn as sb


def get_logger():
    return logging.getLogger(__name__)

CONTACT = "ucddn@student.kit.edu"


class AddValidationMethod:

    def create_validation(self, main_path, track_candidates_store_array_name, output_file_name):
        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': track_candidates_store_array_name,
        })

        validation_module = SeparatedTrackingValidationModule(
            name="",
            contact="",
            output_file_name=output_file_name,
            trackCandidatesColumnName=track_candidates_store_array_name,
            expert_level=2)

        main_path.add_module(mc_track_matcher_module)
        main_path.add_module(validation_module)


class MCTrackFinderRun(StandardEventGenerationRun):
    # output track cands
    mc_track_cands_store_array_name = "MCTrackCands"

    def create_path(self):
        main_path = super(MCTrackFinderRun, self).create_path()

        track_finder_mc_truth_module = basf2.register_module('TrackFinderMCTruth')
        track_finder_mc_truth_module.param({
            'UseCDCHits': True,
            'WhichParticles': [],
            'GFTrackCandidatesColName': self.mc_track_cands_store_array_name,
        })

        main_path.add_module(track_finder_mc_truth_module)

        return main_path


class LegendreTrackFinderRun(MCTrackFinderRun):
    # output (splitted) track cands
    legendre_track_cands_store_array_name = "LegendreTrackCands"
    # output not assigned cdc hits
    not_assigned_cdc_hits_store_array_name = "NotAssignedCDCHits"
    # input tmva cut
    tmva_cut = 0.1
    # input flag if to split
    splitting = True

    def create_argument_parser(self, **kwds):
        argument_parser = super(LegendreTrackFinderRun, self).create_argument_parser(**kwds)
        argument_parser.add_argument(
            '-t',
            '--tmva-cut',
            dest='tmva_cut',
            default=self.tmva_cut,
            type=float,
            help='Cut for the TMVA in the module.'
        )

        argument_parser.add_argument(
            '-s',
            '--splitting',
            dest='splitting',
            default=self.splitting,
            help='Split the tracks before searching for not assigned hits.'
        )

        return argument_parser

    def create_path(self):
        main_path = super(LegendreTrackFinderRun, self).create_path()

        good_cdc_hits_store_array_name = "GoodCDCHits"
        temp_track_cands_store_array_name = "TempTrackCands"

        background_hit_finder_module = basf2.register_module("BackgroundHitFinder")
        background_hit_finder_module.param("TMVACut", float(self.tmva_cut))
        if self.tmva_cut > 0:
            background_hit_finder_module.param("GoodCDCHitsStoreObjName", good_cdc_hits_store_array_name)

        cdctracking = basf2.register_module('CDCLegendreTracking')
        if self.tmva_cut > 0:
            cdctracking.param('CDCHitsColName', good_cdc_hits_store_array_name)
        cdctracking.param('GFTrackCandidatesColName', temp_track_cands_store_array_name)
        cdctracking.set_log_level(basf2.LogLevel.WARNING)

        cdc_stereo_combiner = basf2.register_module('CDCLegendreHistogramming')
        if self.tmva_cut > 0:
            cdc_stereo_combiner.param('CDCHitsColName', good_cdc_hits_store_array_name)
        cdc_stereo_combiner.param('GFTrackCandidatesColName', temp_track_cands_store_array_name)
        cdc_stereo_combiner.set_log_level(basf2.LogLevel.WARNING)

        not_assigned_hits_searcher_module = basf2.register_module("NotAssignedHitsSearcher")
        not_assigned_hits_searcher_module.param({"TracksFromFinder": temp_track_cands_store_array_name,
                                                 "SplittedTracks": self.legendre_track_cands_store_array_name,
                                                 "NotAssignedCDCHits": self.not_assigned_cdc_hits_store_array_name,
                                                 })

        if self.tmva_cut > 0:
            not_assigned_hits_searcher_module.param("CDCHits", good_cdc_hits_store_array_name)

        if self.splitting:
            not_assigned_hits_searcher_module.param("MinimumDistanceToSplit", 0.2)
            not_assigned_hits_searcher_module.param("MinimalHits", 17)
        else:
            not_assigned_hits_searcher_module.param("MinimumDistanceToSplit", 1.1)

        if self.tmva_cut > 0:
            main_path.add_module(background_hit_finder_module)

        main_path.add_module(cdctracking)
        main_path.add_module(cdc_stereo_combiner)
        main_path.add_module(not_assigned_hits_searcher_module)

        if self.tmva_cut > 0:
            main_path.add_module(ReassignHits(
                old_cdc_hits_store_array_name=good_cdc_hits_store_array_name,
                new_cdc_hits_store_array_name="CDCHits",
                track_cands_store_array_name=self.legendre_track_cands_store_array_name))

        return main_path


class CombinerTrackFinderRun(LegendreTrackFinderRun):

    local_track_cands_store_array_name = "LocalTrackCands"

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(CombinerTrackFinderRun, self).create_path()

        local_track_finder = basf2.register_module('SegmentFinderCDCFacetAutomaton')
        local_track_finder.param({
            "GFTrackCandsStoreArrayName": self.local_track_cands_store_array_name,
            "UseOnlyCDCHitsRelatedFrom": self.not_assigned_cdc_hits_store_array_name,
            "SegmentsStoreObjName": "RecoSegments",
            "CreateGFTrackCands": True
        })

        segment_quality_check = basf2.register_module("SegmentQualityCheck")
        segment_quality_check.param("RecoSegments", "RecoSegments")

        not_assigned_hits_combiner = basf2.register_module("NotAssignedHitsCombiner")
        not_assigned_hits_combiner.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                          "ResultTrackCands": "ResultTrackCands",
                                          "BadTrackCands": "BadTrackCands",
                                          "RecoSegments": "RecoSegments",
                                          "MinimalChi2": 0.8,
                                          "MinimalThetaDifference": 0.3,
                                          "MinimalZDifference": 10,
                                          "MinimalChi2Stereo": 0.000001})

        track_finder_output_combiner_naive = basf2.register_module("NaiveCombiner")
        track_finder_output_combiner_naive.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                                  "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                                  "ResultTrackCands": "NaiveResultTrackCands",
                                                  "UseMCInformation": False
                                                  })

        track_finder_output_combiner_mc = basf2.register_module("NaiveCombiner")
        track_finder_output_combiner_mc.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                               "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                               "ResultTrackCands": "BestResultTrackCands",
                                               "UseMCInformation": True
                                               })

        main_path.add_module(local_track_finder)
        main_path.add_module(segment_quality_check)
        main_path.add_module(not_assigned_hits_combiner)
        main_path.add_module(track_finder_output_combiner_naive)
        # main_path.add_module(track_finder_output_combiner_mc)

        return main_path


class CombinerValidationRun(CombinerTrackFinderRun, AddValidationMethod):

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(CombinerValidationRun, self).create_path()

        self.create_validation(
            main_path,
            track_candidates_store_array_name="ResultTrackCands",
            output_file_name="evaluation/result_%.2f.root" %
            self.tmva_cut)
        self.create_validation(
            main_path,
            track_candidates_store_array_name="NaiveResultTrackCands",
            output_file_name="evaluation/naive_%.2f.root" %
            self.tmva_cut)
        self.create_validation(
            main_path,
            track_candidates_store_array_name=self.legendre_track_cands_store_array_name,
            output_file_name="evaluation/legendre_%.2f.root" %
            self.tmva_cut)

        print main_path

        return main_path


def main():
    run = CombinerValidationRun()
    run.configure_and_execute_from_commandline()


def plot(tmva_cut):
    def catch_rates(prefix):
        fom = root_pandas.read_root(
            "evaluation/" +
            prefix +
            "_%.2f.root" %
            tmva_cut,
            tree_key="ExpertMCSideTrackingValidationModule_overview_figures_of_merit")
        rates = root_pandas.read_root(
            "evaluation/" +
            prefix +
            "_%.2f.root" %
            tmva_cut,
            tree_key="ExpertPRSideTrackingValidationModule_overview_figures_of_merit")

        return {"tmva_cut": tmva_cut,
                "finding_efficiency": fom.finding_efficiency[0],
                "hit_efficiency": fom.hit_efficiency[0],
                "fake_rate": rates.fake_rate[0],
                "clone_rate": rates.clone_rate[0]}

    print "naive", catch_rates("naive")
    print "legendre", catch_rates("legendre")
    print "results", catch_rates("result")

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
    plot(0.0)
