#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>FullTrackingValidation.root</output>
  <description>This script validates the full track finding chain in Y(4S) runs.</description>
</header>
"""

import basf2
basf2.set_random_seed(1337)

import logging
import tracking

from tracking.validation.run import TrackingValidationRun

test_cuts = [0.0, 0.25, 0.5, 0.6, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95]
with open("fullVal_cut_iterator.txt", "r") as cut_file:
    iterator = 9 - int(cut_file.read())
cut = test_cuts[iterator]
print("The cut value of the CDC QI is:", cut)
print(str(int(cut*100)).zfill(3))
VALIDATION_OUTPUT_FILE = 'ftv_reco_qi_N1000_wNEWvxd_withCA_cdcQi' + str(int(cut*100)).zfill(3) + '.root'


# cut = 0.0
# VALIDATION_OUTPUT_FILE = 'ftv_cdc_qi_N1000_cdcQi' + str(int(cut*100)).zfill(3) + '.root'
N_EVENTS = 1000
ACTIVE = True


class Full(TrackingValidationRun):
    """Validate the full track-finding chain"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    # root_input_file = '../EvtGenSim_1k_exp1003_run2_new.root'
    root_input_file = 'generated_mc_N1000_BBBAR_test.root'
    #: use the complete track-reconstruction chain
    #: finder_module = staticmethod(tracking.add_tracking_reconstruction)

    def finder_module(self, path):
        tracking.add_tracking_reconstruction(path, add_cdcTrack_QI=True, add_vxdTrack_QI=True, add_recoTrack_QI=True)

        # Replace weightfile identifiers from defaults (CDB payloads) to new
        # weightfiles created by the b2luigi script
        cdc_qe_mva_filter_parameters = {
            "identifier": 'cdc_mva_qe_nTrees350_nCuts6_nLevels5_shrin10_withCA.weights.xml',
            "cut": cut}
        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filterParameters=cdc_qe_mva_filter_parameters,
            )
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier='vxdtf2_mva_qe_nTrees350_nCuts6_nLevels5_shrin10.weights.xml'
            )
        basf2.set_module_parameters(
            path,
            name="TrackQualityEstimatorMVA",
            WeightFileIdentifier="recotrack_mva_qe_nTrees350_nCuts6_nLevels5_shrin10_useVXD_deleteCDCQI"
            + str(int(cut*100)).zfill(3) + ".weights.xml",
            )

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseNLoops': 1
    }
    #: tracks will be already fitted by
    #: add_tracking_reconstruction finder module set above
    fit_tracks = False
    #: But we need to tell the validation module to use the fit information
    use_fit_information = True
    #: do not create expert-level output histograms
    use_expert_folder = False
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    # save trees such that manual studies are possible
    extended = True
    saveFullTrees = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = Full()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
