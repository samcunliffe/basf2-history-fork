#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import collections
import numpy as np

# Need for B2WARNING for some reason
import inspect

from tracking.validation.pull import PullAnalysis
from tracking.validation.fom import ValidationFiguresOfMerit, \
    ValidationManyFiguresOfMerit
from tracking.validation.module import AlwaysPassFilter, \
    getHelixFromMCParticle, TrackingValidationModule

import basf2

import ROOT

ROOT.gSystem.Load('libtracking')
from ROOT import Belle2

# FIXME: define hash function for TrackCand to be able to add it to set. This is
# not really correct, it just checks for the address and normally if a==b also
# hash(a) == hash(b) is required
ROOT.genfit.TrackCand.__hash__ = lambda x: id(x)


class ExpertTrackingValidationModule(TrackingValidationModule):

    """Module to collect more matching information about the found particles and to generate validation
    plots and figures of merit on the performance of track finding. This module gives information on the
    number of hits etc. """

    def __init__(
            self,
            name,
            contact,
            fit=False,
            pulls=False,
            output_file_name=None,
            track_filter_object=AlwaysPassFilter(),
            plot_name_postfix='',
            plot_title_postfix='',
            exclude_profile_mc_parameter='',
            exclude_profile_pr_parameter='',
            use_expert_folder=True,
            trackCandidatesColumnName='RecoTracks',
            mcTrackCandidatesColumnName='MCRecoTracks',
            cdcHitsColumnName='CDCHits',
            write_tables=False):

        TrackingValidationModule.__init__(
            self,
            name,
            contact,
            fit,
            pulls,
            output_file_name,
            track_filter_object,
            plot_name_postfix,
            plot_title_postfix,
            exclude_profile_mc_parameter,
            exclude_profile_pr_parameter,
            use_expert_folder,
            trackCandidatesColumnName,
            mcTrackCandidatesColumnName)

        self.cdcHitsColumnname = cdcHitsColumnName
        self.write_tables = write_tables

    def initialize(self):
        TrackingValidationModule.initialize(self)
        # Use deques in favour of lists to prevent repeated memory allocation of cost O(n)

        self.number_of_total_hits = collections.deque()
        self.number_of_mc_hits = collections.deque()
        self.number_of_pr_hits = collections.deque()
        self.is_hit_found = collections.deque()
        self.is_hit_matched = collections.deque()

        # MC information
        self.mc_missing = collections.deque()
        self.ratio_hits_in_mc_tracks_and_not_in_pr_tracks = collections.deque()
        self.ratio_hits_in_mc_tracks_and_in_pr_tracks = collections.deque()
        self.ratio_hits_in_missing_mc_tracks_and_in_pr_tracks = collections.deque()
        self.ratio_hits_in_mc_tracks_and_in_fake_pr_tracks = \
            collections.deque()
        self.ratio_hits_in_mc_tracks_and_in_good_pr_tracks = \
            collections.deque()
        self.mc_is_primary = collections.deque()
        self.mc_number_of_hits = collections.deque()

        # PT information
        # This is the number of mcTrackCands sharing a hit with the track cand.
        self.number_of_connected_tracks = collections.deque()
        self.number_of_wrong_hits = collections.deque()  # This number gives information about the "badness" of the fake.
        # It is calculated by going through all hits of the fake track and the connected mc track cands and counting the number.
        # These numbers are than summed up and substracted by the biggest number
        # of hits this candidates shares with the mc track cands.
        self.pr_number_of_hits = collections.deque()
        self.pr_number_of_matched_hits = collections.deque()

    def event(self):
        """Event method"""

        TrackingValidationModule.event(self)
        self.examine_hits_in_event()

    def examine_hits_in_event(self):

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)
        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)
        if self.cdcHitsColumnname not in Belle2.PyStoreArray.list():
            # No CDC hits available, hit analysis incomplete, don't perform
            # hit analysis
            return
        cdcHits = Belle2.PyStoreArray(self.cdcHitsColumnname)

        # # CDC Hits in MC tracks
        totalHitListMC = []
        for mcTrackCand in mcTrackCands:
            cdcHitIDs = [cdcHit.getArrayIndex() for cdcHit in mcTrackCand.getCDCHitList()]  # Checked
            # Working around a bug in ROOT where you should not access empty std::vectors
            if len(cdcHitIDs) == 0:
                cdcHitIDs = set()
            else:
                cdcHitIDs = set(cdcHitIDs)
            totalHitListMC.extend(cdcHitIDs)

        # Make the ids unqiue
        totalHitListMC = set(totalHitListMC)

        # # CDC Hits in PR tracks
        totalHitListPR = []
        totalHitListPRGood = []
        totalHitListPRClone = []
        totalHitListPRFake = []
        for trackCand in trackCands:
            if trackCand.getNumberOfTotalHits() == 0:
                basf2.B2WARNING("Encountered a pattern recognition track with no hits")
                continue

            cdcHitIDs = [cdcHit.getArrayIndex() for cdcHit in mcTrackCand.getCDCHitList()]  # Checked
            # Working around a bug in ROOT where you should not access empty std::vectors
            if len(cdcHitIDs) == 0:
                cdcHitIDs = set()
            else:
                cdcHitIDs = set(cdcHitIDs)

            totalHitListPR.extend(cdcHitIDs)
            if self.trackMatchLookUp.isMatchedPRTrackCand(trackCand):
                totalHitListPRGood.extend(cdcHitIDs)

            if self.trackMatchLookUp.isClonePRTrackCand(trackCand):
                totalHitListPRClone.extend(cdcHitIDs)

            if (self.trackMatchLookUp.isBackgroundPRTrackCand(trackCand) or
                    self.trackMatchLookUp.isBackgroundPRTrackCand(trackCand)):
                totalHitListPRFake.extend(cdcHitIDs)

        # Make the ids unqiue
        totalHitListPR = set(totalHitListPR)
        totalHitListPRGood = set(totalHitListPRGood)
        totalHitListPRClone = set(totalHitListPRClone)
        totalHitListPRFake = set(totalHitListPRFake)

        # # All CDC Hits
        totalHitList = set([cdcHit.getArrayIndex() for cdcHit in cdcHits])

        number_of_mc_hits = len(totalHitListMC)
        number_of_pr_hits = len(totalHitListPR)
        number_of_all_hits = len(totalHitList)

        is_hit_matched = 0
        is_hit_found = len(totalHitListMC & totalHitListPR)

        for trackCand in trackCands:

            is_matched = self.trackMatchLookUp.isMatchedPRTrackCand(trackCand)
            is_clone = self.trackMatchLookUp.isClonePRTrackCand(trackCand)

            trackCandHits = [cdcHit.getArrayIndex() for cdcHit in trackCand.getCDCHitList()]
            # Working around a bug in ROOT where you should not access empty std::vectors
            if len(trackCandHits) == 0:
                trackCandHits = set()
            else:
                trackCandHits = set(trackCandHits)

            # this is not very efficient...
            list_of_connected_mc_tracks = set()
            list_of_numbers_of_hits_for_connected_tracks = collections.deque()
            number_of_connected_tracks = 0
            number_of_wrong_hits = 0

            for mcTrackCand in mcTrackCands:
                mcTrackCandHits = [cdcHit.getArrayIndex() for cdcHit in mcTrackCand.getCDCHitList()]
                # Working around a bug in ROOT where you should not access empty std::vectors
                if len(mcTrackCandHits) == 0:
                    mcTrackCandHits = set()
                else:
                    mcTrackCandHits = set(mcTrackCandHits)

                length_of_intersection = len(mcTrackCandHits & trackCandHits)
                if length_of_intersection > 0:
                    list_of_connected_mc_tracks.add(mcTrackCand)
                    list_of_numbers_of_hits_for_connected_tracks.append(length_of_intersection)

            if len(list_of_numbers_of_hits_for_connected_tracks) == 0:
                self.number_of_wrong_hits.append(0)
                self.pr_number_of_matched_hits.append(0)
            else:
                maximum_intersection = \
                    max(list_of_numbers_of_hits_for_connected_tracks)
                self.pr_number_of_matched_hits.append(sum(list_of_numbers_of_hits_for_connected_tracks))
                self.number_of_wrong_hits.append(sum(list_of_numbers_of_hits_for_connected_tracks) -
                                                 maximum_intersection)

            self.number_of_connected_tracks.append(len(list_of_connected_mc_tracks))

            if is_matched or is_clone:
                mcTrackCand = \
                    self.trackMatchLookUp.getRelatedMCTrackCand(trackCand)
                mcTrackCandHits = [cdcHit.getArrayIndex() for cdcHit in mcTrackCand.getCDCHitList()]  # Checked
                # Working around a bug in ROOT where you should not access empty std::vectors
                if len(mcTrackCandHits) == 0:
                    mcTrackCandHits = set()
                else:
                    mcTrackCandHits = set(mcTrackCandHits)

                is_hit_matched += len(trackCandHits & mcTrackCandHits)

            self.pr_number_of_hits.append(len(trackCandHits))

        for mcTrackCand in mcTrackCands:
            is_missing = \
                self.trackMatchLookUp.isMissingMCTrackCand(mcTrackCand)

            mcTrackCandHits = [cdcHit.getArrayIndex() for cdcHit in mcTrackCand.getCDCHitList()]  # Checked

            # Working around a bug in ROOT where you should not access empty std::vectors
            if len(mcTrackCandHits) == 0:
                continue
            else:
                mcTrackCandHits = set(mcTrackCandHits)

            ratio = 1.0 * len(mcTrackCandHits & totalHitListPR) / len(mcTrackCandHits)

            self.ratio_hits_in_mc_tracks_and_not_in_pr_tracks.append(1.0 - ratio)
            self.ratio_hits_in_mc_tracks_and_in_pr_tracks.append(ratio)
            if is_missing:
                self.ratio_hits_in_missing_mc_tracks_and_in_pr_tracks.append(ratio)
            self.ratio_hits_in_mc_tracks_and_in_good_pr_tracks.append(
                1.0 * len(mcTrackCandHits & totalHitListPRGood) / len(mcTrackCandHits))
            self.ratio_hits_in_mc_tracks_and_in_fake_pr_tracks.append(
                1.0 * len(mcTrackCandHits & totalHitListPRFake) / len(mcTrackCandHits))

            mcParticle = \
                self.trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
            is_primary = \
                mcParticle.hasStatus(Belle2.MCParticle.c_PrimaryParticle)
            self.mc_is_primary.append(is_primary)
            self.mc_number_of_hits.append(len(mcTrackCandHits))

            self.mc_missing.append(is_missing)

        self.number_of_total_hits.append(number_of_all_hits)
        self.number_of_mc_hits.append(number_of_mc_hits)
        self.number_of_pr_hits.append(number_of_pr_hits)

        self.is_hit_found.append(is_hit_found)
        self.is_hit_matched.append(is_hit_matched)

    def terminate(self):
        TrackingValidationModule.terminate(self)

        output_tfile = ROOT.TFile(self.output_file_name, 'update')

        validation_plots = []

        # Hit ratios #
        ######################
        all_tracks_plot = self.profiles_by_parameters_base(
            xs=self.ratio_hits_in_mc_tracks_and_in_pr_tracks,
            quantity_name="ratio of hits in MCTracks found by the track finder",
            make_hist=True,
            parameter_names=[],
            profile_parameters={},
            unit=None)

        validation_plots.extend(all_tracks_plot)

        missing_tracks_plot = self.profiles_by_parameters_base(
            xs=self.ratio_hits_in_missing_mc_tracks_and_in_pr_tracks,
            quantity_name="ratio of hits in missing MCTracks found by the track finder",
            make_hist=True,
            parameter_names=[],
            profile_parameters={},
            unit=None)

        validation_plots.extend(missing_tracks_plot)

        for validation_plot in validation_plots:
            validation_plot.write()

        if self.write_tables:
            # MC Figures of merit
            mc_figures_of_merit = \
                ValidationManyFiguresOfMerit('%s_mc_figures_of_merit' % self.name)

            mc_figures_of_merit['mc_pts'] = self.mc_pts
            mc_figures_of_merit['mc_d0s'] = self.mc_d0s
            mc_figures_of_merit['mc_matches'] = self.mc_matches
            mc_figures_of_merit['mc_hit_efficiencies'] = self.mc_hit_efficiencies
            mc_figures_of_merit['mc_multiplicities'] = self.mc_multiplicities
            mc_figures_of_merit['mc_phis'] = self.mc_phi
            mc_figures_of_merit['mc_tan_lambdas'] = self.mc_tan_lambdas
            mc_figures_of_merit['mc_thetas'] = self.mc_theta
            mc_figures_of_merit['mc_missing'] = self.mc_missing
            mc_figures_of_merit['mc_is_primary'] = self.mc_is_primary
            mc_figures_of_merit['mc_number_of_hits'] = self.mc_number_of_hits
            mc_figures_of_merit['ratio_hits_in_mc_tracks_and_in_good_pr_tracks'] = \
                self.ratio_hits_in_mc_tracks_and_in_good_pr_tracks
            mc_figures_of_merit['ratio_hits_in_mc_tracks_and_in_fake_pr_tracks'] = \
                self.ratio_hits_in_mc_tracks_and_in_fake_pr_tracks
            mc_figures_of_merit['ratio_hits_in_mc_tracks_and_not_in_pr_tracks'] = \
                self.ratio_hits_in_mc_tracks_and_not_in_pr_tracks

            mc_figures_of_merit.write()

            # PR Figures of merit
            pr_figures_of_merit = \
                ValidationManyFiguresOfMerit('%s_pr_figures_of_merit' % self.name)

            pr_figures_of_merit['pr_clones_and_matches'] = \
                self.pr_clones_and_matches
            pr_figures_of_merit['pr_matches'] = self.pr_matches
            pr_figures_of_merit['pr_fakes'] = self.pr_fakes
            pr_figures_of_merit['pr_number_of_hits'] = self.pr_number_of_hits
            pr_figures_of_merit['pr_number_of_matched_hits'] = \
                self.pr_number_of_matched_hits
            pr_figures_of_merit['pr_seed_tan_lambdas'] = self.pr_seed_tan_lambdas
            pr_figures_of_merit['pr_seed_phi'] = self.pr_seed_phi
            pr_figures_of_merit['pr_seed_theta'] = self.pr_seed_theta

            pr_figures_of_merit['number_of_connected_tracks'] = \
                self.number_of_connected_tracks
            pr_figures_of_merit['number_of_wrong_hits'] = self.number_of_wrong_hits

            pr_figures_of_merit.write()

            # Hit Figures of merit
            hit_figures_of_merit = \
                ValidationFiguresOfMerit('%s_hit_figures_of_merit' % self.name)

            hit_figures_of_merit['number_of_total_hits'] = \
                np.sum(self.number_of_total_hits)
            hit_figures_of_merit['number_of_mc_hits'] = \
                np.sum(self.number_of_mc_hits)
            hit_figures_of_merit['number_of_pr_hits'] = \
                np.sum(self.number_of_pr_hits)
            hit_figures_of_merit['is_hit_found'] = np.sum(self.is_hit_found)
            hit_figures_of_merit['is_hit_matched'] = np.sum(self.is_hit_matched)

            print(hit_figures_of_merit)
            hit_figures_of_merit.write()

        output_tfile.Close()
