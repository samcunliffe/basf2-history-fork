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

VALIDATION_OUTPUT_FILE = 'toCDCfromEclCKFTrackingValidation_onlyEclSeed_expert.root'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking
from tracking.validation.run import TrackingValidationRun
import reconstruction


class toCDCfromEclCKF(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '/nfs/dust/belle2/user/kurzsimo/evtSamples/ElecGenSimNoBkg_highStats.root'
    # root_input_file = 'EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

        tracking.add_svd_reconstruction(path)
        tracking.add_vxd_track_finding_vxdtf2(path, reco_tracks="RecoTracksSVD", components=["SVD"])
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracksSVD")

        reconstruction.add_ecl_modules(path)

        # needed for truth matching
        # tracking.add_mc_track_finding(path)
        # same thing as below
        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName="MCRecoTracks",
                        # We are using primary as well as secondaries here!
                        WhichParticles=[],
                        UsePXDHits=False,
                        UseSVDHits=False,
                        UseCDCHits=True)

        # reconstruction.add_ecl_finalizer_module(path)
        # reconstruction.add_ecl_mc_matcher_module(path)

        path.add_module("TFCDC_WireHitPreparer",
                        wirePosition="aligned",
                        useSecondHits=False,
                        flightTimeEstimation="outwards")

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
                        outputRecoTrackStoreArrayName="RecoTracks",
                        outputRelationRecoTrackStoreArrayName="EclSeedRecoTracks",
                        writeOutDirection="forward",
                        # stateBasicFilter="rough_and_recording_eclSeed",
                        stateBasicFilterParameters={"maximalHitDistance": 7.5, "maximalHitDistanceEclSeed": 75.0},
                        # stateBasicFilterParameters={"maximalHitDistance": 7.5, "maximalHitDistanceEclSeed": 75.0,
                        #                            "returnWeight": 1.},
                        # stateBasicFilterParameters={"returnWeight": 1.},
                        stateExtrapolationFilterParameters={"extrapolationDirection": "backward"},
                        # stateFinalFilter="distance_and_recording_eclSeed",
                        # stateFinalFilterParameters={"returnWeight": 1.},
                        pathFilter="arc_length_fromEcl",
                        inputECLshowersStoreArrayName="ECLShowers",
                        trackFindingDirection="backward",
                        # filter="size_and_recording_fromEcl",
                        # filterParameters={"returnWeight": 1.},
                        # stateMaximalHitCandidates=10
                        #
                        # finalFilter="recording_fromEcl",
                        # finalFilterParameters={"returnWeight": 1.},
                        )

        # Do not combine tracks for testing
        # path.add_module("RelatedTracksCombiner",
        #                #                CDCRecoTracksStoreArrayName="CKFCDCRecoTracks",
        #                CDCRecoTracksStoreArrayName="CKFCDCfromEclRecoTracks",
        #                VXDRecoTracksStoreArrayName="RecoTracksSVD",
        #                recoTracksStoreArrayName="RecoTracks")

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracks")

        path.add_module('TrackCreator', recoTrackColName='RecoTracks')

        # path.add_module("PrintCollections", printForEvent=-1)

        path.add_module("MCRecoTracksMatcher",
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName="RecoTracks",
                        UseCDCHits=True,
                        UseSVDHits=False,
                        UsePXDHits=False)

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
