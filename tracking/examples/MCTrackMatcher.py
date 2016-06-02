#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ####### Imports #######
import sys
import os
import optparse

from basf2 import *
from ROOT import Belle2

import simulation
import tracking


class PrintMCMatchingRelation(Module):

    """Small helper module to present the relations and informations generated by the MCRecoTracksMatcherModule"""

    def initialize(self):
        """Initialize the track match lookup and internal arrays"""

        #: The track match lookup we use for convenienctly access the MC <-> PR relations.
        self.trackMatchLookUp = Belle2.TrackMatchLookUp('MCRecoTracks')

        #: An array to be filled with the flag whether the PR track is matched or not.
        self.isMatchedPRTracks = []

        #: An array to be filled with the flag whether the MC track is matched or not.
        self.isMatchedMCTracks = []

        #: An array holding the transverse momentum of the MC track candidates.
        self.mcAbsTransversMomentums = []

    def event(self):
        """Event method"""

        trackMatchLookUp = self.trackMatchLookUp

        trackCands = Belle2.PyStoreArray('RecoTracks')
        if trackCands:
            print('Number of pattern recognition tracks',
                  trackCands.getEntries())

            for (iTrackCand, trackCand) in enumerate(trackCands):
                isMatched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
                self.isMatchedPRTracks.append(isMatched)

        mcTrackCands = Belle2.PyStoreArray('MCTrackCands')
        if trackCands:
            print('Number of Monte Carlo tracks', mcTrackCands.getEntries())

            for mcTrackCand in mcTrackCands:
                isMatched = trackMatchLookUp.isMatchedMCTrackCand(mcTrackCand)
                self.isMatchedMCTracks.append(isMatched)

                mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
                momentum = mcParticle.getMomentum()
                absTransversMomentum = momentum.Perp()
                self.mcAbsTransversMomentums.append(absTransversMomentum)

    def terminate(self):
        """Print the results."""
        track_finding_efficiency = 1.0 * sum(self.isMatchedMCTracks) \
            / len(self.isMatchedMCTracks)
        print('Track finding efficiency: ', track_finding_efficiency)

        fake_rate = 1.0 - 1.0 * sum(self.isMatchedPRTracks) \
            / len(self.isMatchedPRTracks)
        print('Fake rate (% unmatched pattern recognition tracks) : ',
              fake_rate)

        import numpy as np
        mcAbsTransversMomentums = np.array(self.mcAbsTransversMomentums)
        isMatchedMCTracks = np.array(self.isMatchedMCTracks)
        (binned_total_counts, bins) = np.histogram(mcAbsTransversMomentums,
                                                   bins=50)

        (binned_match_counts, bins) = \
            np.histogram(mcAbsTransversMomentums[isMatchedMCTracks], bins=bins)
        binned_finding_efficiency = 1.0 * binned_match_counts \
            / binned_total_counts

        bin_centers = (bins[:-1] + bins[1:]) / 2.0
        upper_x_errors = bins[1:] - bin_centers
        lower_x_errors = bin_centers - bins[:-1]
        y_errors = np.sqrt(binned_finding_efficiency * (1 - binned_finding_efficiency) / binned_total_counts)

        import matplotlib.pyplot as plt
        plt.errorbar(bin_centers, binned_finding_efficiency, linestyle='None',
                     xerr=[lower_x_errors, upper_x_errors], yerr=y_errors)
        plt.xlabel('Transvers momentum (GeV)')
        plt.ylabel('Track finding efficiency')
        plt.show()


# ######## Register modules  ########
eventInfoSetterModule = register_module('EventInfoSetter')
eventInfoSetterModule.param({'evtNumList': [1000], 'runList': [1],
                             'expList': [1]})

printCollectionsModule = register_module('PrintCollections')

particleGunModule = register_module('ParticleGun')
particleGunModule.param({
    'pdgCodes': [13, -13],
    'nTracks': 6,
    'varyNTracks': False,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.2, 3.0],
    'thetaGeneration': 'uniform',
    'thetaParams': [17., 150.],
})
# A module to recreate genfit TrackCands.
trackCandidateConverter = register_module('GenfitTrackCandidatesCreator')

# Reference Monte Carlo tracks
trackFinderMCTruthModule = register_module('TrackFinderMCTruth')
trackFinderMCTruthModule.param({
    'UseCDCHits': True,
    'UseSVDHits': False,
    'UsePXDHits': False,
    'WhichParticles': ['primary'],
    'EnergyCut': 0.1,
    'recoTracksStoreArrayName': 'MCRecoTracks',
})

mcTrackMatcherModule = register_module('MCRecoTracksMatcher')
mcTrackMatcherModule.param({
    'UseCDCHits': True,
    'UseSVDHits': False,
    'UsePXDHits': False,
    'mcRecoTracksStoreArrayName': 'MCRecoTracks',
    'MinimalPurity': 0.66,
    'RelateClonesToMCParticles': True,
})

# ######## Create paths and add modules  ########
main = create_path()
main.add_module(eventInfoSetterModule)
main.add_module(particleGunModule)

components = ['MagneticFieldConstant4LimitedRCDC', 'BeamPipe', 'PXD', 'SVD',
              'CDC']
simulation.add_simulation(main, components=components)

tracking.add_cdc_track_finding(main)
main.add_module(trackCandidateConverter)

main.add_module(trackFinderMCTruthModule)
main.add_module(mcTrackMatcherModule)

# main.add_module(printCollectionsModule)
main.add_module(PrintMCMatchingRelation())

process(main)

# Print call statistics
print(statistics)
