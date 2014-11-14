#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <output>cosmicsTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'cosmicsTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 10000

import basf2
basf2.set_random_seed(1337)

import os
import sys
import logging

import numpy as np

from cdclocaltracking import event_generation

import ROOT
from ROOT import Belle2


class ValidationPlot(object):

    def __init__(
        self,
        name,
        lower_bound=None,
        upper_bound=None,
        n_bins=None,
        description='',
        check='',
        contact='',
        ):

        self.name = name

        self.upper_bound = upper_bound
        self.lower_bound = lower_bound
        self.n_bins = n_bins

        self._description = description
        self._check = check
        self._contact = contact

        self._xlabel = ''
        self._ylabel = ''
        self._title = ''

        self.histogram = None

    @staticmethod
    def is_binary(xs):
        is_boolean = all(isinstance(x, bool) for x in xs)
        is_one_or_zero = all(x == 0 or x == 1 for x in xs)
        return is_boolean or is_one_or_zero

    def fill(
        self,
        xs,
        ys=None,
        weights=None,
        ):

        name = self.name

        # Handle data content of the histgram
        ## Figure out the the binning if not externally set already
        if self.is_binary(xs):
            # if xs is a binary variable setup the histogram to
            # have only two bins with the right boundaries.
            if self.lower_bound is None:
                self.lower_bound = 0

            if self.upper_bound is None:
                self.upper_bound = np.nextafter(1.0, np.inf)

            if self.n_bins is None:
                self.n_bins = 2
        else:
            if self.lower_bound is None:
                self.lower_bound = np.min(xs)

            if self.upper_bound is None:
                self.upper_bound = np.max(xs)

            if self.n_bins is None:
                n_data = len(x)
                rice_n_bins = ceil(2.0 * pow(n_data, 1.0 / 3.0))
                self.n_bins = rice_n_bins

        lower_bound = self.lower_bound
        upper_bound = self.upper_bound
        n_bins = self.n_bins

        ## Setup the histogram and fill it with values
        if ys is not None:
            histogram = ROOT.TProfile(name, '', self.n_bins, lower_bound,
                                      upper_bound)
            Fill = histogram.Fill
            if weights is None:
                for (x, y) in zip(xs, ys):
                    Fill(float(x), float(y))
            else:
                for (x, y, weight) in zip(xs, ys, weights):
                    Fill(float(x), float(y), float(weight))

            max_y = max(ys)
            min_y = min(ys)
            y_range = max_y - min_y

            histogram.SetMaximum(max_y + 0.1 * y_range)
            histogram.SetMinimum(min_y)
        else:

            histogram = ROOT.TH1F(name, '', self.n_bins, lower_bound,
                                  upper_bound)
            Fill = histogram.Fill
            if weights is None:
                for x in xs:
                    Fill(float(x))
            else:
                for (x, weight) in zip(xs, weights):
                    Fill(float(x), float(weight))

        # Now attach the additional information
        root_description = ROOT.TNamed('Descrition', self.description)
        root_check = ROOT.TNamed('Check', self.check)
        root_contact = ROOT.TNamed('Contact', self.contact)

        for root_attribute in (root_description, root_check, root_contact):
            histogram.GetListOfFunctions().Add(root_attribute)

        title = (self.title if self.title else self.name)
        histogram.SetTitle(title)

        xlabel = self.xlabel
        histogram.SetXTitle(xlabel)

        if self.ylabel is None:
            if ys is None:
                # Default ylabel for a histogram plot
                self.ylabel = 'Count'

        ylabel = self.ylabel
        histogram.SetYTitle(ylabel)

        self.histogram = histogram

    @property
    def title(self):
        return self._title

    @title.setter
    def _(self, title):
        self._title = title
        if self.histogram is not None:
            self.histogram.SetTitle(title)

    @property
    def xlabel(self):
        return self._xlabel

    @xlabel.setter
    def xlabel(self, xlabel):
        self._xlabel = xlabel
        if self.histogram is not None:
            self.histogram.SetXTitle(xlabel)

    @property
    def ylabel(self):
        return self._ylabel

    @ylabel.setter
    def ylabel(self, ylabel):
        self._ylabel = ylabel
        if self.histogram is not None:
            self.histogram.SetYTitle(ylabel)

    @property
    def contact(self):
        return self._contact

    @contact.setter
    def contact(self, contact):
        self._contact = contact
        if self.histogram is not None:
            found_obj = self.histogram.FindObject('Contact')
            if found_obj:
                found_obj.SetTitle(contact)

    @property
    def description(self):
        return self._description

    @description.setter
    def description(self, description):
        self._description = description
        if self.histogram is not None:
            found_obj = self.histogram.FindObject('Description')
            if found_obj:
                found_obj.SetTitle(description)

    @property
    def check(self):
        return self._check

    @check.setter
    def check(self, check):
        self._check = check
        if self.histogram is not None:
            found_obj = self.histogram.FindObject('Check')
            if found_obj:
                found_obj.SetTitle(check)

    def show(self):
        self.histogram.Draw()

    def write(self):
        self.histogram.Write()


class ValidationFiguresOfMerit(dict):

    def __init__(
        self,
        name,
        description='',
        check='',
        contact='',
        ):

        self.name = name
        self.description = description
        self.check = check
        self.contact = contact

        self.figures_by_name = {}

    def __str__(self):
        """Informal sting output listing the assigned figures of merit."""

        return '\n'.join('%s : %s' % (key, value) for (key, value) in
                         self.figures_by_name.items())

    def write(self):
        """Writes the figures of merit as a TNtuple to the currently open TFile in the format complient with the validation frame work."""

        name = self.name
        figure_names = list(self.figures_by_name.keys())
        values = list(self.figures_by_name.values())

        leaf_specification = ':'.join(figure_names)
        title = ''
        ntuple = ROOT.TNtuple(name, title, leaf_specification)
        ntuple.Fill(*values)

        ntuple.SetAlias('Description', self.description)
        ntuple.SetAlias('Check', self.check)
        ntuple.SetAlias('Contact', self.contact)

        ntuple.Write()

    def __setitem__(self, figure_name, value):
        """Braketed item assignement for figures of merit"""

        self.figures_by_name[figure_name] = value

    def __getitem__(self, figure_name):
        """Braketed item lookup for figures of merit"""

        return self.figures_by_name[figure_name]

    def __delitem__(self, figure_name):
        """Braketed item deletion for figures of merit"""

        del self.figures_by_name[figure_name]

    def __iter__(self):
        """Implements the iter() hook as if it was a dictionary."""

        return iter(self.figures_by_name)

    def __len__(self):
        """Returns the number of figures of merit assigned. Implements the len() hook."""

        return len(self.figures_by_name)


# Analysis module #
###################

class CosmicsTrackingValidationModule(basf2.Module):

    """Module to collect matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding."""

    def initialize(self):
        self.trackMatchLookUp = Belle2.TrackMatchLookUp('MCTrackCands')

        self.pr_clones_and_matches = []
        self.pr_matches = []
        self.pr_seen_in_vxds = []

        self.mc_transverse_momenta = []
        self.mc_matches = []
        self.mc_impacts = []
        self.mc_cos_thetas = []
        self.mc_hit_efficiencies = []

    def event(self):
        """Event method"""

        trackMatchLookUp = self.trackMatchLookUp

        # Analyse from the pattern recognition side
        trackCands = Belle2.PyStoreArray('TrackCands')
        if trackCands:
            # print 'Number of pattern recognition tracks', \
            #    trackCands.getEntries()

            for (iTrackCand, trackCand) in enumerate(trackCands):
                is_matched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
                is_clone = trackMatchLookUp.isClonePRTrackCand(trackCand)

                seen_in_vxd = False
                if is_matched or is_clone:
                    # Only matched and clone tracks have a related MCParticle
                    mcParticle = \
                        trackMatchLookUp.getRelatedMCParticle(trackCand)
                    if mcParticle:
                        seenInDetector = mcParticle.getSeenInDetector()

                        pxd_detector_set = \
                            Belle2.Const.DetectorSet(Belle2.Const.PXD)
                        svd_detector_set = \
                            Belle2.Const.DetectorSet(Belle2.Const.SVD)

                        seen_in_pxd = seenInDetector.contains(pxd_detector_set)
                        seen_in_svd = seenInDetector.contains(svd_detector_set)
                        seen_in_vxd = seen_in_pxd | seen_in_vxd

                self.pr_clones_and_matches.append(is_matched or is_clone)
                self.pr_matches.append(is_matched)
                self.pr_seen_in_vxds.append(seen_in_vxd)

        # Analyse from the Monte carlo reference side
        mcTrackCands = Belle2.PyStoreArray('MCTrackCands')
        if trackCands:
            # print 'Number of Monte Carlo tracks', mcTrackCands.getEntries()

            for mcTrackCand in mcTrackCands:
                is_matched = trackMatchLookUp.isMatchedMCTrackCand(mcTrackCand)

                hit_efficiency = \
                    trackMatchLookUp.getRelatedEfficiency(mcTrackCand)
                if np.isnan(hit_efficiency):
                    hit_efficiency = 0

                mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
                momentum = mcParticle.getMomentum()
                transverse_momentum = momentum.Perp()
                cos_theta = momentum.CosTheta()
                impact = 0

                self.mc_matches.append(is_matched)
                self.mc_hit_efficiencies.append(hit_efficiency)
                self.mc_transverse_momenta.append(transverse_momentum)
                self.mc_impacts.append(impact)
                self.mc_cos_thetas.append(cos_theta)

    def terminate(self):
        # Overall figures of merit #
        ############################
        track_finding_efficiency = np.mean(self.mc_matches)
        fake_rate = 1.0 - np.mean(self.pr_clones_and_matches)
        clone_rate = 1.0 - np.average(self.pr_matches,
                                      weights=self.pr_clones_and_matches)
        hit_efficiency = np.mean(self.mc_hit_efficiencies)

        cosmics_figures_of_merit = \
            ValidationFiguresOfMerit('Cosmics_figures_of_merit')
        cosmics_figures_of_merit['finding_efficiency'] = \
            track_finding_efficiency
        cosmics_figures_of_merit['fake_rate'] = fake_rate
        cosmics_figures_of_merit['clone_rate'] = clone_rate
        cosmics_figures_of_merit['hit_efficiency'] = hit_efficiency

        cosmics_figures_of_merit.description = \
            """
finding_efficiency - the ratio of matched Monte Carlo tracks to all Monte Carlo tracks <br/>
fake_rate - ratio of pattern recognition tracks that are not related to a particle (background, ghost) to all pattern recognition tracks <br/>
clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches) <br/>
"""
        cosmics_figures_of_merit.check = ''
        cosmics_figures_of_merit.contact = CONTACT

        print cosmics_figures_of_merit

        # Validation plots #
        ####################
        validation_plots = []

        # A try out efficiency plot
        efficiency_profile = \
            ValidationPlot('Cosmics_track_finding_efficiency_by_momentum',
                           n_bins=50)
        efficiency_profile.fill(self.mc_transverse_momenta, self.mc_matches)

        efficiency_profile.xlabel = 'Momentum (GeV)'
        efficiency_profile.ylabel = 'Efficiency'
        efficiency_profile.description = 'Not a serious plot yet.'
        efficiency_profile.check = ''
        efficiency_profile.contact = CONTACT

        validation_plots.append(efficiency_profile)
        efficiency_profile.show()

        # A tryout impact parameter plot, yet using the seen in vxd as an impact parameter.
        clone_rate_by_impact = \
            ValidationPlot('Cosmics_clone_rate_by_seen_in_vxd')
        clone_rate_by_impact.fill(self.pr_seen_in_vxds, [not x for x in
                                  self.pr_matches],
                                  weights=self.pr_clones_and_matches)

        clone_rate_by_impact.xlabel = 'Seen in vxd (bool)'
        clone_rate_by_impact.ylabel = 'Clone rate'

        clone_rate_by_impact.description = 'Not a serious plot yet.'
        clone_rate_by_impact.check = ''
        clone_rate_by_impact.contact = CONTACT

        validation_plots.append(clone_rate_by_impact)
        clone_rate_by_impact.show()

        # Finding efficiency by impact parameter
        finding_efficiency_by_impact = \
            ValidationPlot('Cosmics_finding_efficiency_by_impact')
        finding_efficiency_by_impact.n_bins = 50
        finding_efficiency_by_impact.fill(self.mc_impacts, self.mc_matches)

        finding_efficiency_by_impact.xlabel = 'Impact (cm)'
        finding_efficiency_by_impact.ylabel = 'Finding efficiency'

        finding_efficiency_by_impact.description = 'Not a serious plot yet.'
        finding_efficiency_by_impact.check = ''
        finding_efficiency_by_impact.contact = CONTACT

        validation_plots.append(finding_efficiency_by_impact)

        # Finding efficiency by cos theta
        finding_efficiency_by_cos_theta = \
            ValidationPlot('Cosmics_finding_efficiency_by_cos_theta')
        finding_efficiency_by_cos_theta.n_bins = 50
        finding_efficiency_by_cos_theta.fill(self.mc_cos_thetas,
                self.mc_matches)

        finding_efficiency_by_cos_theta.xlabel = '#cos #theta'
        finding_efficiency_by_cos_theta.ylabel = 'Finding efficiency'

        finding_efficiency_by_cos_theta.description = 'Not a serious plot yet.'
        finding_efficiency_by_cos_theta.check = ''
        finding_efficiency_by_cos_theta.contact = CONTACT

        validation_plots.append(finding_efficiency_by_cos_theta)

        # Histogram of the hit efficiency
        hit_efficiency_histogram = ValidationPlot('Cosmics_hit_efficiency')
        hit_efficiency_histogram.n_bins = 50
        hit_efficiency_histogram.fill(self.mc_hit_efficiencies)

        hit_efficiency_histogram.xlabel = 'Hit efficiency'
        hit_efficiency_histogram.description = 'Not a serious plot yet.'
        hit_efficiency_histogram.check = ''
        hit_efficiency_histogram.contact = CONTACT

        validation_plots.append(hit_efficiency_histogram)

        # Hit efficiency by impact parameter
        hit_efficiency_by_impact = \
            ValidationPlot('Cosmics_hit_efficiency_by_impact')
        hit_efficiency_by_impact.n_bins = 50
        hit_efficiency_by_impact.fill(self.mc_impacts,
                                      self.mc_hit_efficiencies)

        hit_efficiency_by_impact.xlabel = 'Impact (cm)'
        hit_efficiency_by_impact.ylabel = 'Hit efficiency'

        hit_efficiency_by_impact.description = 'Not a serious plot yet.'
        hit_efficiency_by_impact.check = ''
        hit_efficiency_by_impact.contact = CONTACT

        validation_plots.append(hit_efficiency_by_impact)

        # Hit efficiency by cos_theta parameter
        hit_efficiency_by_cos_theta = \
            ValidationPlot('Cosmics_hit_efficiency_by_cos_theta')
        hit_efficiency_by_cos_theta.n_bins = 50
        hit_efficiency_by_cos_theta.fill(self.mc_cos_thetas,
                self.mc_hit_efficiencies)

        hit_efficiency_by_cos_theta.xlabel = '#cos #theta (cm)'
        hit_efficiency_by_cos_theta.ylabel = 'Hit efficiency'

        hit_efficiency_by_cos_theta.description = 'Not a serious plot yet.'
        hit_efficiency_by_cos_theta.check = ''
        hit_efficiency_by_cos_theta.contact = CONTACT

        validation_plots.append(hit_efficiency_by_cos_theta)

        # Save everything to a ROOT file
        output_file = ROOT.TFile(VALIDATION_OUTPUT_FILE, 'recreate')

        cosmics_figures_of_merit.write()

        efficiency_profile.write()
        for validation_plot in validation_plots:
            validation_plot.write()

        output_file.Close()


def main():
    """Function to be executed during the validation run"""

    main_path = event_generation.create_simulate_events_path('cosmics',
            n_events=N_EVENTS)

    # Setup track finder
    trackFinderModule = basf2.register_module('CDCLocalTracking')
    main_path.add_module(trackFinderModule)

    # Reference Monte Carlo tracks
    trackFinderMCTruthModule = basf2.register_module('TrackFinderMCTruth')
    trackFinderMCTruthModule.param({
        'UseCDCHits': True,
        'UseSVDHits': False,
        'UsePXDHits': False,
        'WhichParticles': ['primary'],
        'EnergyCut': 0.1,
        'GFTrackCandidatesColName': 'MCTrackCands',
        })
    main_path.add_module(trackFinderMCTruthModule)

    # Track matcher
    mcTrackMatcherModule = basf2.register_module('MCTrackMatcher')
    mcTrackMatcherModule.param({
        'UseCDCHits': True,
        'UseSVDHits': False,
        'UsePXDHits': False,
        'MCGFTrackCandsColName': 'MCTrackCands',
        'MinimalPurity': 0.66,
        'RelateClonesToMCParticles': True,
        })

    main_path.add_module(mcTrackMatcherModule)
    main_path.add_module(CosmicsTrackingValidationModule())

    print 'Start processing'

    basf2.process(main_path)
    print basf2.statistics


if __name__ == '__main__':
    main()

