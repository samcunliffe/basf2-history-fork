#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

from ROOT import Belle2

from geometry import check_components

from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction

from tracking import (
    add_mc_tracking_reconstruction,
    add_tracking_reconstruction,
    add_cr_tracking_reconstruction,
    add_mc_track_finding,
    add_track_finding,
    add_prune_tracks,
)

from softwaretrigger.path_utils import (
    add_filter_software_trigger,
    add_skim_software_trigger
)

import mdst


def add_reconstruction(path, components=None, pruneTracks=True, add_trigger_calculation=True, skipGeometryAdding=False,
                       trackFitHypotheses=None, addClusterExpertModules=True,
                       use_second_cdc_hits=False, add_muid_hits=False,
                       use_second_cdc_hits=False, add_muid_hits=False, reconstruct_cdst=False,
                       nCDCHitsMax=4000, nSVDShaperDigitsMax=70000):
    """
    This function adds the standard reconstruction modules to a path.
    Consists of tracking and the functionality provided by :func:`add_posttracking_reconstruction()`,
    plus the modules to calculate the software trigger cuts.

    :param path: Add the modules to this path.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last of the tracks after the dEdX modules.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used to just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param trackFitHypotheses: Change the additional fitted track fit hypotheses. If no argument is given,
        the fitted hypotheses are pion, muon and proton, i.e. [211, 321, 2212].
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.
    :param add_trigger_calculation: add the software trigger modules for monitoring (do not make any cut)
    :param reconstruct_cdst: run only the minimal reconstruction needed to produce the cdsts (raw+tracking+dE/dx)
    :param nCDCHitsMax: the max number of CDC hits for an event to be reconstructed.
    :param nSVDShaperDigitsMax: the max number of SVD shaper digits for an event to be reconstructed.
    """

    # Check components.
    check_components(components)

    # Do not even attempt at reconstructing events w/ abnormally large occupancy.
    empty_path = create_path()
    doom = path.add_module(EventsOfDoomBuster(nCDCHitsMax, nSVDShaperDigitsMax))
    doom.if_true(empty_path, AfterConditionPath.END)

    # Add modules that have to be run BEFORE track reconstruction
    add_pretracking_reconstruction(path,
                                   components=components)

    # Add tracking reconstruction modules
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=False,
                                mcTrackFinding=False,
                                skipGeometryAdding=skipGeometryAdding,
                                trackFitHypotheses=trackFitHypotheses,
                                use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    # Add only the dE/dx calculation and prune the tracks
    if reconstruct_cdst:
        add_dedx_modules(main_path)
        add_prune_tracks(main_path, components=components)

    else:
        # Add further reconstruction modules
        add_posttracking_reconstruction(path,
                                        components=components,
                                        pruneTracks=pruneTracks,
                                        add_muid_hits=add_muid_hits,
                                        addClusterExpertModules=addClusterExpertModules)

        # Add the modules calculating the software trigger cuts (but not performing them)
        if add_trigger_calculation and (not components or (
                "CDC" in components and "ECL" in components and "EKLM" in components and "BKLM" in components)):
            add_filter_software_trigger(path)
            add_skim_software_trigger(path)


def add_cosmics_reconstruction(
        path,
        components=None,
        pruneTracks=True,
        skipGeometryAdding=False,
        eventTimingExtraction=True,
        addClusterExpertModules=True,
        merge_tracks=True,
        top_in_counter=False,
        data_taking_period='early_phase3',
        use_second_cdc_hits=False,
        add_muid_hits=False,
        reconstruct_cdst=False):
    """
    This function adds the standard reconstruction modules for cosmic data to a path.
    Consists of tracking and the functionality provided by :func:`add_posttracking_reconstruction()`,
    plus the modules to calculate the software trigger cuts.

    :param path: Add the modules to this path.
    :param data_taking_period: The cosmics generation will be added using the
           parameters, that where used in this period of data taking. The periods can be found in cdc/cr/__init__.py.

    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last of the tracks after the dEdX modules.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used to just turn off the geometry adding at
        all (but you will have to add it on your own then).

    :param eventTimingExtraction: extract the event time
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.

    :param merge_tracks: The upper and lower half of the tracks should be merged together in one track
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.

    :param top_in_counter: time of propagation from the hit point to the PMT in the trigger counter is subtracted
           (assuming PMT is put at -z of the counter).

    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.

    :param reconstruct_cdst: run only the minimal reconstruction needed to produce the cdsts (raw+tracking+dE/dx)
    """

    # Check components.
    check_components(components)

    # Add modules that have to be run before track reconstruction
    add_pretracking_reconstruction(path,
                                   components=components)

    # Add cdc tracking reconstruction modules
    add_cr_tracking_reconstruction(path,
                                   components=components,
                                   prune_tracks=False,
                                   skip_geometry_adding=skipGeometryAdding,
                                   event_time_extraction=eventTimingExtraction,
                                   merge_tracks=merge_tracks,
                                   data_taking_period=data_taking_period,
                                   top_in_counter=top_in_counter,
                                   use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    # Add only the dE/dx calculation and prune the tracks
    if reconstruct_cdst:
        add_dedx_modules(main_path)
        add_prune_tracks(main_path, components=components)

    else:
        # Add further reconstruction modules
        add_posttracking_reconstruction(path,
                                        components=components,
                                        pruneTracks=pruneTracks,
                                        addClusterExpertModules=addClusterExpertModules,
                                        add_muid_hits=add_muid_hits,
                                        cosmics=True)


def add_mc_reconstruction(path, components=None, pruneTracks=True, addClusterExpertModules=True,
                          use_second_cdc_hits=False, add_muid_hits=False):
    """
    This function adds the standard reconstruction modules with MC tracking
    to a path.

    @param components list of geometry components to include reconstruction for, or None for all components.
    @param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.
    """

    # Add modules that have to be run before track reconstruction
    add_pretracking_reconstruction(path,
                                   components=components)

    # tracking
    add_mc_tracking_reconstruction(path,
                                   components=components,
                                   pruneTracks=False,
                                   use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    # add further reconstruction modules
    add_posttracking_reconstruction(path,
                                    components=components,
                                    pruneTracks=pruneTracks,
                                    add_muid_hits=add_muid_hits,
                                    addClusterExpertModules=addClusterExpertModules)


def add_pretracking_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules BEFORE tracking
    to a path.

    :param path: The path to add the modules to.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    """

    add_ecl_modules(path, components)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Clustering')


def add_posttracking_reconstruction(path, components=None, pruneTracks=True, addClusterExpertModules=True,
                                    add_muid_hits=False, cosmics=False):
    """
    This function adds the standard reconstruction modules after tracking
    to a path.

    :param path: The path to add the modules to.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last after the post-tracking modules.
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.
    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.
    :param cosmics: if True, steer TOP for cosmic reconstruction
    """

    add_dedx_modules(path, components)
    add_ext_module(path, components)
    add_top_modules(path, components, cosmics=cosmics)
    add_arich_modules(path, components)

    path.add_module('StatisticsSummary').set_name('Sum_PID')

    path.add_module("EventT0Combiner")

    add_ecl_finalizer_module(path, components)

    add_ecl_mc_matcher_module(path, components)

    add_klm_modules(path, components)

    add_klm_mc_matcher_module(path, components)

    add_muid_module(path, add_hits_to_reco_track=add_muid_hits, components=components)
    add_ecl_track_cluster_modules(path, components)
    add_ecl_cluster_properties_modules(path, components)
    add_ecl_eip_module(path, components)
    add_pid_module(path, components)

    if addClusterExpertModules:
        # FIXME: Disabled for HLT until execution time bug is fixed
        add_cluster_expert_modules(path, components)

    add_ecl_track_brem_finder(path, components)

    # Prune tracks as soon as the post-tracking steps are complete
    if pruneTracks:
        add_prune_tracks(path, components)

    path.add_module('StatisticsSummary').set_name('Sum_Clustering')


def add_mdst_output(
    path,
    mc=True,
    filename='mdst.root',
    additionalBranches=[],
    dataDescription=None,
):
    """
    This function adds the MDST output modules to a path, saving only objects defined as part of the MDST data format.

    @param path Path to add modules to
    @param mc Save Monte Carlo quantities? (MCParticles and corresponding relations)
    @param filename Output file name.
    @param additionalBranches Additional objects/arrays of event durability to save
    @param dataDescription Additional key->value pairs to be added as data description
           fields to the output FileMetaData
    """

    return mdst.add_mdst_output(path, mc, filename, additionalBranches, dataDescription)


def add_cdst_output(
    path,
    mc=True,
    filename='cdst.root',
    additionalBranches=[],
    dataDescription=None,
):
    """
    This function adds the cDST output modules (mDST + calibration objects) to a path,
    saving only objects defined as part of the cDST data format.

    @param path Path to add modules to
    @param mc Save Monte Carlo quantities? (MCParticles and corresponding relations)
    @param filename Output file name.
    @param additionalBranches Additional objects/arrays of event durability to save
    @param dataDescription Additional key->value pairs to be added as data description
           fields to the output FileMetaData
    """

    calibrationBranches = [
        'RecoTracks',
        'EventT0',
        'SVDShaperDigits',
        'SVDRecoDigits',
        'SVDClusters',
        'CDCDedxTracks',
        'TOPDigits',
        'ExtHits',
        'TOPLikelihoods',
        'TOPRecBunch',
        'TOPTimeZeros',
        'TOPAsicMask',
        'ECLDigits',
        'ECLCalDigits',
        'TRGECLClusters',
        'TRGECLUnpackerStores',
        'TRGECLUnpackerEvtStores',
        'BKLMHit2ds',
        'TRGGRLUnpackerStore',
        'TracksToBKLMHit2ds',
        'RecoHitInformations',
        'RecoHitInformationsToBKLMHit2ds',
        'EKLMAlignmentHits',
        'TracksToEKLMAlignmentHits',
        'EKLMHit2ds',
        'EKLMDigits',
        'Muids',
        'TracksToMuids',
        'TracksToARICHLikelihoods',
        'TracksToExtHits',
        'ARICHDigits',
        'ARICHInfo',
        'ARICHTracks',
        'ARICHLikelihoods',
        'ARICHTracksToExtHits',
        'SoftwareTriggerVariables',
        'BKLMDigits',
        'BKLMHit1ds',
        'BKLMHit2dsToBKLMHit1ds',
        'BKLMHit1dsToBKLMDigits'
    ]
    if dataDescription is None:
        dataDescription = {}
    dataDescription.setdefault("dataLevel", "cdst")
    return mdst.add_mdst_output(path, mc, filename, additionalBranches + calibrationBranches, dataDescription)


def add_arich_modules(path, components=None):
    """
    Add the ARICH reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ARICH' in components:
        arich_fillHits = register_module('ARICHFillHits')
        path.add_module(arich_fillHits)
        arich_rec = register_module('ARICHReconstructor')
        # enabled for ARICH DQM plots
        arich_rec.param('storePhotons', 1)
        path.add_module(arich_rec)


def add_top_modules(path, components=None, cosmics=False):
    """
    Add the TOP reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    :param cosmics: if True, steer TOP for cosmic reconstruction
    """
    # TOP reconstruction
    if components is None or 'TOP' in components:
        top_cm = register_module('TOPChannelMasker')
        path.add_module(top_cm)
        if cosmics:
            top_finder = register_module('TOPCosmicT0Finder')
            path.add_module(top_finder)
        else:
            top_finder = register_module('TOPBunchFinder')
            path.add_module(top_finder)
        top_rec = register_module('TOPReconstructor')
        path.add_module(top_rec)


def add_cluster_expert_modules(path, components=None):
    """
    Add the cluster expert modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # klong id and cluster matcher, whcih also builds "cluster"
    if components is None or ('EKLM' in components and 'BKLM' in components and 'ECL' in components):
        KLMClassifier = register_module('KLMExpert')
        path.add_module(KLMClassifier)
        ClusterMatch = register_module('ClusterMatcher')
        path.add_module(ClusterMatch)


def add_pid_module(path, components=None):
    """
    Add the PID modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # charged particle PID
    if components is None or 'SVD' in components or 'CDC' in components:
        mdstPID = register_module('MdstPID')
        path.add_module(mdstPID)


def add_klm_modules(path, components=None):
    """
    Add the (E/B)KLM reconstruction modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'EKLM' in components:
        eklm_rec = register_module('EKLMReconstructor')
        path.add_module(eklm_rec)

    # BKLM reconstruction
    if components is None or 'BKLM' in components:
        bklm_rec = register_module('BKLMReconstructor')
        path.add_module(bklm_rec)

    # K0L reconstruction
    if components is None or ('BKLM' in components and 'EKLM' in components):
        klm_k0l_rec = register_module('KLMK0LReconstructor')
        path.add_module(klm_k0l_rec)


def add_klm_mc_matcher_module(path, components=None):
    """
    Add the (E/B)KLM mc matcher module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # MC matching
    if components is None or 'BKLM' in components or 'EKLM' in components:
        klm_mc = register_module('MCMatcherKLMClusters')
        path.add_module(klm_mc)


def add_muid_module(path, add_hits_to_reco_track=False, components=None):
    """
    Add the MuID module to the path.

    :param path: The path to add the modules to.
    :param add_hits_to_reco_track: Add the found KLM hits also to the RecoTrack. Make sure to refit the track afterwards.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'BKLM' in components and 'EKLM' in components:
        muid = register_module('Muid', addHitsToRecoTrack=add_hits_to_reco_track)
        path.add_module(muid)


def add_ecl_modules(path, components=None):
    """
    Add the ECL reconstruction modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # ECL calibration and reconstruction
    if components is None or 'ECL' in components:

        # ECL offline waveform fitting
        ecl_waveform_fit = register_module('ECLWaveformFit')
        path.add_module(ecl_waveform_fit)

        # ECL digit calibration
        ecl_digit_calibration = register_module('ECLDigitCalibrator')
        path.add_module(ecl_digit_calibration)

        # ECL T0 extraction
        path.add_module('ECLEventT0')

        # ECL connected region finder
        ecl_crfinder = register_module('ECLCRFinder')
        path.add_module(ecl_crfinder)

        # ECL local maximum finder
        ecl_lmfinder = register_module('ECLLocalMaximumFinder')
        path.add_module(ecl_lmfinder)

        # ECL splitter N1
        ecl_splitterN1 = register_module('ECLSplitterN1')
        path.add_module(ecl_splitterN1)

        # ECL splitter N2
        ecl_splitterN2 = register_module('ECLSplitterN2')
        path.add_module(ecl_splitterN2)

        # ECL Shower Correction
        ecl_showercorrection = register_module('ECLShowerCorrector')
        path.add_module(ecl_showercorrection)

        # ECL Shower Calibration
        ecl_showercalibration = register_module('ECLShowerCalibrator')
        path.add_module(ecl_showercalibration)

        # ECL Shower Shape
        ecl_showershape = register_module('ECLShowerShape')
        path.add_module(ecl_showershape)

        # ECL Pulse Shape Discrimination
        ecl_clusterPSD = register_module('ECLClusterPSD')
        path.add_module(ecl_clusterPSD)

        # ECL covariance matrix
        ecl_covariance = register_module('ECLCovarianceMatrix')
        path.add_module(ecl_covariance)

        # ECL finalize -> must run after eventT0Combiner


def add_ecl_finalizer_module(path, components=None):
    """
        Add the ECL finalizer module to the path.

        :param path: The path to add the modules to.
        :param components: The components to use or None to use all standard components.
        """

    if components is None or 'ECL' in components:
        # ECL finalize
        ecl_finalize = register_module('ECLFinalizer')
        path.add_module(ecl_finalize)


def add_ecl_track_cluster_modules(path, components=None):
    """
    Add the ECL track cluster matching module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or ('ECL' in components and ('PXD' in components or 'SVD' in components or 'CDC' in components)):
        path.add_module('ECLTrackClusterMatching')


def add_ecl_cluster_properties_modules(path, components=None):
    """
    Add the ECL cluster properties module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or ('ECL' in components and ('PXD' in components or 'SVD' in components or 'CDC' in components)):
        path.add_module('ECLClusterProperties')


def add_ecl_track_brem_finder(path, components=None):
    """
    Add the bremsstrahlung finding module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or ('ECL' in components and ('PXD' in components or 'SVD' in components)):
        brem_finder = register_module('ECLTrackBremFinder')
        path.add_module(brem_finder)


def add_ecl_eip_module(path, components=None):
    """
    Add the ECL charged PID module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        # charged PID
        charged_id = register_module('ECLChargedPID')
        path.add_module(charged_id)


def add_ecl_mc_matcher_module(path, components=None):
    """
    Add the ECL MC matcher module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        # MC matching
        ecl_mc = register_module('MCMatcherECLClusters')
        path.add_module(ecl_mc)


def add_ext_module(path, components=None):
    """
    Add the extrapolation module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'CDC' in components:
        ext = register_module('Ext')
        path.add_module(ext)


def add_dedx_modules(path, components=None):
    """
    Add the dEdX reconstruction modules to the path
    and prune the tracks afterwards if wanted.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # CDC dE/dx PID
    if components is None or 'CDC' in components:
        CDCdEdxPID = register_module('CDCDedxPID')
        path.add_module(CDCdEdxPID)

    # VXD dE/dx PID
    # only run this if the SVD is enabled - PXD is disabled by default
    if components is None or 'SVD' in components:
        VXDdEdxPID = register_module('VXDDedxPID')
        path.add_module(VXDdEdxPID)


<< << << < HEAD


class EventsOfDoomBuster(Module):
    """
    Module that flags an event destined for doom at reconstruction,
    based on the size of selected hits/digits containers after the unpacking.

    This is meant to be registered in the path *after* the unpacking, but *before* reconstruction.
    """

    def __init__(self, nCDCHitsMax=int(1e9), nSVDShaperDigitsMax=int(1e9)):
        """
        Module constructor.

        Args:
            nCDCHitsMax (Optional[int]): the max number of CDC hits
                for an event to be kept for reconstruction.
                By default, no events are skipped based upon this requirement.
            nSVDShaperDigitsMax (Optional[int]): the max number of SVD shaper digits
                for an event to be kept for reconstruction.
                By default, no events are skipped based upon this requirement.
        """

        super().__init__()

        self.nCDCHitsMax = nCDCHitsMax
        self.nSVDShaperDigitsMax = nSVDShaperDigitsMax

    def initialize(self):
        """
        Module initializer.
        """

        self.eventinfo = Belle2.PyStoreObj("EventMetaData")
        self.cdchits = Belle2.PyStoreArray("CDCHits")
        self.svdshaperdigits = Belle2.PyStoreArray("SVDShaperDigits")

    def event(self):
        """
        Flag each event.

        Returns:
            bool: True if event exceeds `nCDCHitsMax or nSVDShaperDigitsMax`.
                  In that case, the event should be skipped for reco.
        """

        ncdchits = len(self.cdchits)
        nsvdshaperdigits = len(self.svdshaperdigits)

        B2DEBUG(20, f"Event: {self.eventinfo.getEvent()} - nCDCHits: {ncdchits}, nSVDShaperDigits: {nsvdshaperdigits}")

        doom_cdc = ncdchits > self.nCDCHitsMax
        doom_svd = nsvdshaperdigits > self.nSVDShaperDigitsMax

        if doom_cdc:
            B2WARNING("Skip event --> Too much occupancy for reco!",
                      event=self.eventinfo.getEvent(),
                      run=self.eventinfo.getRun(),
                      exp=self.eventinfo.getExperiment(),
                      nCDCHits=ncdchits,
                      nCDCHitsMax=self.nCDCHitsMax)
        if doom_svd:
            B2WARNING("Skip event --> Too much occupancy for reco!",
                      event=self.eventinfo.getEvent(),
                      run=self.eventinfo.getRun(),
                      exp=self.eventinfo.getExperiment(),
                      nSVDShaperDigits=nsvdshaperdigits,
                      nSVDShaperDigitsMax=self.nSVDShaperDigitsMax)

        self.return_value(doom_cdc or doom_svd)
== == == =


def prepare_cdst_analysis(path, components=None):
    """
    Adds to a (analysis) path all the modules needed to
    analyse a cdsts file in the raw+tracking format.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # unpackers
    add_unpackers(path, components=components)

    # this is currently just calls add_ecl_modules
    add_pretracking_reconstruction(path, components=components)

    # needed to retrieve the PXD and SVD clusters out of the raws
    if components is None or 'SVD' in components:
        add_svd_reconstruction(path)
    if components is None or 'PXD' in components:
        add_pxd_reconstruction(path)

    # check, this one may not be needed...
    path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    # from here on mostly a replica of add_posttracking_reconstruction without dE/dx, prunetracks and eventT0 modules
    add_ext_module(path, components)
    add_top_modules(path, components)
    add_arich_modules(path, components)
    add_ecl_finalizer_module(path, components)
    add_ecl_mc_matcher_module(path, components)
    add_klm_modules(path, components)
    add_klm_mc_matcher_module(path, components)
    add_muid_module(path, components=components)
    add_ecl_track_cluster_modules(path, components)
    add_ecl_cluster_properties_modules(path, components)
    add_ecl_eip_module(path, components)
    add_pid_module(path, components)
    add_ecl_track_brem_finder(path, components)
>>>>>> > 57d98769f2... Add the prepare_cdst_analysis function
