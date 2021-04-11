#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>DATCONTrackingValidation.root</output>
  <description>
  This module validates that the svd only track finding is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

import tracking
from tracking.validation.run import TrackingValidationRun
import logging
import basf2
from datcon.datcon_functions import add_datcon

VALIDATION_OUTPUT_FILE = 'DATCONTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)

rootFileName = 'trackCandAnalysisNoBkg.root'


def setupFinderModule(path):
    tracking.add_hit_preparation_modules(path, components=["SVD"])
    add_datcon(path, rootFileName=rootFileName, datcon_reco_tracks='RecoTracks')
    # path.add_module('DATCON',
    # minimumUClusterTime=-10,
    # minimumVClusterTime=-10,
    # maximumUClusterTime=60,
    # maximumVClusterTime=60,

    # useSubHoughSpaces=False,

    # simpleVerticalHoughSpaceSize=0.25,
    # simpleNAngleSectors=256,
    # simpleNVerticalSectors=256,
    # simpleMinimumHSClusterSize=4,
    # simpleMaximumHSClusterSize=200,

    # maxRelations=1000000,
    # relationFilter='angleAndTime',
    # relationFilterParameters={'AngleAndTimeThetaCutDeltaL0': 0.05,
    # 'AngleAndTimeThetaCutDeltaL1': 0.10,
    # 'AngleAndTimeThetaCutDeltaL2': 0.25,
    # 'AngleAndTimeDeltaUTime': 50.,
    # 'AngleAndTimeDeltaVTime': 50., },
    # rootFileName=rootFileName,
    # maxRelationsCleaner=1000,
    # #  twoHitFilter='twoHitVirtualIPQI',
    # #  twoHitUseNBestHits=100,
    # twoHitFilter='all',
    # twoHitUseNBestHits=0,
    # threeHitUseNBestHits=50,
    # fourHitUseNBestHits=30,
    # fiveHitUseNBestHits=10,

    # #  trackQualityEstimationMethod='helixFit',
    # minQualitiyIndicatorSize3=0.50,
    # minQualitiyIndicatorSize4=0.001,
    # minQualitiyIndicatorSize5=0.001,
    # maxNumberOfHitsForEachPathLength=50,

    # RecoTracksStoreArrayName='RecoTracks',
    # )


class DATCONTrackingValidation(TrackingValidationRun):
    """
    Validation class for the four 4-SVD Layer tracking
    """
    #: the number of events to process
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: root input file to use, generated by central validation script
    root_input_file = '../EvtGenSimNoBkg.root'
    #: use full detector for validation
    components = None

    #: lambda method which is used by the validation to add the svd finder modules
    finder_module = staticmethod(setupFinderModule)

    #: use only the svd hits when computing efficiencies
    tracking_coverage = {
        'WhichParticles': ['SVD'],  # Include all particles seen in the SVD detector, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': True,
        'UseCDCHits': False,
    }

    #: perform fit after track finding
    fit_tracks = True
    #: plot pull distributions
    pulls = True
    #: create expert-level histograms
    use_expert_folder = True
    #: Include resolution information in the validation output
    resolution = True
    #: Use the fit information in validation
    use_fit_information = True
    #: output file of plots
    output_file_name = VALIDATION_OUTPUT_FILE

    #: Store additional information in output file (like the full trees)
    extended = True
    saveFullTrees = True


def main():
    """
    create SVD validation class and execute
    """
    validation_run = DATCONTrackingValidation()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
