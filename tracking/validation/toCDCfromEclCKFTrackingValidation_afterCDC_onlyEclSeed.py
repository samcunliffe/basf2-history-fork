#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>glazov@mail.desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>toCDCfromEclCKFTrackingValidation.root</output>
  <description>This module validates ToCDCCKF module using Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'toCDCfromEclCKFTrackingValidation_afterCDC_onlyEclSeed_expert.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

from basf2 import *
from tracking.path_utils import *
from tracking import *
import reconstruction

import logging
import tracking

from tracking.validation.run import TrackingValidationRun


class toCDCfromEclCKF(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '/nfs/dust/belle2/user/kurzsimo/evtSamples/ElecGenSimNoBkg_highStats.root'
    # root_input_file = 'EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        # tracking.add_tracking_reconstruction(path, components=["CDC"])

        # add all modules instead so other options can be chosen
        add_geometry_modules(path, components=["CDC"])
        add_hit_preparation_modules(path, components=["CDC"])

        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

        add_track_finding(path, components=["CDC"], trigger_mode="all", reco_tracks="CDCRecoTracks",
                          prune_temporary_tracks=True, use_second_cdc_hits=False)

        reconstruction.add_ecl_modules(path)

        # reconstruction.add_ecl_finalizer_module(path)
        # reconstruction.add_ecl_mc_matcher_module(path)

        cdcfromeclckf = basf2.register_module("ToCDCFromEclCKF")

        path.add_module(cdcfromeclckf,
                        inputWireHits="CDCWireHitVector",
                        minimalEnRequirementCluster=0.3,
                        seedDeadRegionPhi=-1,
                        seedDeadRegionTheta=-1,
                        #                relatedRecoTrackStoreArrayName="CKFCDCfromEclRecoTracks",
                        #                relationCheckForDirection="forward",
                        eclSeedRecoTrackStoreArrayName='EclSeedRecoTracks',
                        # outputRecoTrackStoreArrayName="CKFCDCfromEclRecoTracks",
                        hitFindingDirection="backward",
                        outputRecoTrackStoreArrayName="ECLRecoTracks",
                        outputRelationRecoTrackStoreArrayName="EclSeedRecoTracks",
                        writeOutDirection="forward",
                        # stateBasicFilter="rough_and_recording_eclSeed",
                        stateBasicFilterParameters={"maximalHitDistance": 7.5, "maximalHitDistanceEclSeed": 75.0},
                        # stateBasicFilterParameters={"maximalHitDistance": 7.5, "maximalHitDistanceEclSeed": 75.0,
                        #                             "returnWeight": 1.},
                        # stateBasicFilterParameters={"returnWeight": 1.},
                        # stateExtrapolationFilterParameters={"extrapolationDirection": "backward"},
                        # stateFinalFilter="distance_and_recording_eclSeed",
                        # stateFinalFilterParameters={"returnWeight": 1.},
                        pathFilter="arc_length_fromEcl",
                        inputECLshowersStoreArrayName="ECLShowers",
                        trackFindingDirection="backward",
                        # filter="size_and_recording_fromEcl",
                        # filterParameters={"returnWeight": 1.},
                        # stateMaximalHitCandidates=10
                        # finalFilter="recording_fromEcl",
                        # finalFilterParameters={"returnWeight": 1.},
                        )

        path.add_module("TracksCombiner",
                        Temp1RecoTracksStoreArrayName="CDCRecoTracks",
                        Temp2RecoTracksStoreArrayName="ECLRecoTracks",
                        recoTracksStoreArrayName="RecoTracks")

        add_time_extraction(path, components=["CDC"])

        add_mc_matcher(path, components=["CDC"], reco_tracks="RecoTracks",
                       use_second_cdc_hits=False)

        add_track_fit_and_track_creator(path, components=["CDC"], pruneTracks=False,
                                        trackFitHypotheses=None, reco_tracks="RecoTracks")

    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'WhichParticles': [],
    }

    # Already fitted in the finder_module
    fit_tracks = False
    use_fit_information = True
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE
    extended = True


def main():
    validation_run = toCDCfromEclCKF()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    if ACTIVE:
        main()
