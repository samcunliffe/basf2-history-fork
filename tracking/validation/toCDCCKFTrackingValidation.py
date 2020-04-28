#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>glazov@mail.desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>toCDCCKFTrackingValidation.root</output>
  <description>This module validates ToCDCCKF module using Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'toCDCCKFTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking
from tracking.validation.run import TrackingValidationRun


class toCDCCKF(TrackingValidationRun):
    """Validate the ToCDCCKF Kalman finder/filter algorithm with Y(4S) events"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        """Add the ToCDCCKF module and related modules to the basf2 path"""

        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

        tracking.add_svd_reconstruction(path)
        tracking.add_vxd_track_finding_vxdtf2(path, reco_tracks="RecoTracksSVD", components=["SVD"])
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracksSVD")

        path.add_module("TFCDC_WireHitPreparer",
                        wirePosition="aligned",
                        useSecondHits=False,
                        flightTimeEstimation="outwards")
        path.add_module("ToCDCCKF",
                        inputWireHits="CDCWireHitVector",
                        inputRecoTrackStoreArrayName="RecoTracksSVD",
                        relatedRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        relationCheckForDirection="backward",
                        outputRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        outputRelationRecoTrackStoreArrayName="RecoTracksSVD",
                        writeOutDirection="backward",
                        stateBasicFilterParameters={"maximalHitDistance": 0.75},
                        stateExtrapolationFilterParameters={"direction": "forward"},
                        pathFilter="arc_length")

        path.add_module("RelatedTracksCombiner",
                        CDCRecoTracksStoreArrayName="CKFCDCRecoTracks",
                        VXDRecoTracksStoreArrayName="RecoTracksSVD",
                        recoTracksStoreArrayName="RecoTracks")

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracks")

        path.add_module('TrackCreator', recoTrackColName='RecoTracks')

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': True,
        'UseCDCHits': True,
        'WhichParticles': [],
    }

    #: Already fitted in the finder_module
    fit_tracks = False
    #: But we need to tell the validation module to use the fit information
    use_fit_information = True
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = toCDCCKF()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
