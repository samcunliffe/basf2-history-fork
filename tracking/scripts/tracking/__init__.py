#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from tracking.path_utils import *


def add_tracking_reconstruction(path, components=None, pruneTracks=False, skipGeometryAdding=False,
                                mcTrackFinding=False, trigger_mode="all", trackFitHypotheses=None,
                                reco_tracks="RecoTracks", prune_temporary_tracks=True, fit_tracks=True,
                                use_second_cdc_hits=False, skipHitPreparerAdding=False,
                                svd_ckf_mode="VXDTF2_after",
                                vxdtf2_mva_weight_file='tracking/data/VXDQE_weight_files/Default-CoG-noTime.xml'):
    """
    This function adds the standard reconstruction modules for tracking
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits except the first and the last in the found tracks.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used o just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param skipHitPreparerAdding: Advanced flag: do not add the hit preparation (esp. VXD cluster creation
        modules. This is useful if they have been added before already.
    :param mcTrackFinding: Use the MC track finders instead of the realistic ones.
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    :param fit_tracks: If false, the final track find and the TrackCreator module will no be executed
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param trackFitHypotheses: Which pdg hypothesis to fit. Defaults to [211, 321, 2212].
    """

    if not is_svd_used(components) and not is_cdc_used(components):
        return

    if not skipGeometryAdding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components=components)

    if not skipHitPreparerAdding and trigger_mode in ["all", "hlt"]:
        add_hit_preparation_modules(path, components=components)

    # Material effects for all track extrapolations
    if trigger_mode in ["all", "hlt"] and 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

    if mcTrackFinding:
        # Always add the MC finder in all trigger modes.
        add_mc_track_finding(path, components=components, reco_tracks=reco_tracks,
                             use_second_cdc_hits=use_second_cdc_hits)
    else:
        add_track_finding(path, components=components, trigger_mode=trigger_mode, reco_tracks=reco_tracks,
                          prune_temporary_tracks=prune_temporary_tracks,
                          use_second_cdc_hits=use_second_cdc_hits,
                          svd_ckf_mode=svd_ckf_mode,
                          vxdtf2_mva_weight_file=vxdtf2_mva_weight_file)

    if trigger_mode in ["hlt", "all"]:
        add_mc_matcher(path, components=components, reco_tracks=reco_tracks,
                       use_second_cdc_hits=use_second_cdc_hits)

        if fit_tracks:
            add_track_fit_and_track_creator(path, components=components, pruneTracks=pruneTracks,
                                            trackFitHypotheses=trackFitHypotheses,
                                            reco_tracks=reco_tracks)


def add_cr_tracking_reconstruction(path, components=None, prune_tracks=False,
                                   skip_geometry_adding=False, event_time_extraction=True,
                                   data_taking_period="gcr2017", top_in_counter=False,
                                   merge_tracks=False, use_second_cdc_hits=False):
    """
    This function adds the reconstruction modules for cr tracking to a path.

    :param path: The path to which to add the tracking reconstruction modules
    :param data_taking_period: The cosmics generation will be added using the
           parameters, that where used in this period of data taking. The periods can be found in cdc/cr/__init__.py.

    :param components: the list of geometry components in use or None for all components.
    :param prune_tracks: Delete all hits except the first and the last in the found tracks.

    :param skip_geometry_adding: Advanced flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it cannot
        determine if the geometry is already loaded. This flag can be used to just turn off the geometry adding
        (but you will have to add it on your own).
    :param event_time_extraction: extract time with either the TrackTimeExtraction or
        FullGridTrackTimeExtraction modules.
    :param merge_tracks: The upper and lower half of the tracks should be merged together in one track
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.

    :param top_in_counter: time of propagation from the hit point to the PMT in the trigger counter is subtracted
           (assuming PMT is put at -z of the counter).
    """
    # make sure CDC is used
    if not is_cdc_used(components):
        return

    if not skip_geometry_adding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components)

    add_hit_preparation_modules(path, components=components)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

    # track finding
    add_cr_track_finding(path, reco_tracks="RecoTracks", components=components, data_taking_period=data_taking_period,
                         merge_tracks=merge_tracks, use_second_cdc_hits=use_second_cdc_hits)

    # track fitting
    add_cr_track_fit_and_track_creator(path, components=components, prune_tracks=prune_tracks,
                                       event_timing_extraction=event_time_extraction,
                                       data_taking_period=data_taking_period, top_in_counter=top_in_counter)

    if merge_tracks:
        # Do also fit the not merged tracks
        add_cr_track_fit_and_track_creator(path, components=components, prune_tracks=prune_tracks,
                                           event_timing_extraction=False,
                                           data_taking_period=data_taking_period, top_in_counter=top_in_counter,
                                           reco_tracks="NonMergedRecoTracks", tracks="NonMergedTracks")


def add_mc_tracking_reconstruction(path, components=None, pruneTracks=False, use_second_cdc_hits=False):
    """
    This function adds the standard reconstruction modules for MC tracking
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=pruneTracks,
                                mcTrackFinding=True,
                                use_second_cdc_hits=use_second_cdc_hits)


def add_track_finding(path, components=None, trigger_mode="all", reco_tracks="RecoTracks",
                      prune_temporary_tracks=True, use_second_cdc_hits=False,
                      use_mc_truth=False, svd_ckf_mode="VXDTF2_after", add_both_directions=True,
                      vxdtf2_mva_weight_file=None):
    """
    Add the CKF to the path with all the track finding related to and needed for it.
    :param path: The path to add the tracking reconstruction modules to
    :param reco_tracks: The store array name where to output all tracks
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param use_mc_truth: Use the truth information in the CKF modules
    :param svd_ckf_mode: how to apply the CKF (with VXDTF2 or without). Defaults to "VXDTF2_after".
    :param add_both_directions: Curlers may be found in the wrong orientation by the CDC track finder, so try to
           extrapolate also in the other direction.
    :param use_second_cdc_hits: whether to use the secondary CDC hit during CDC track finding or not
    :param components: the list of geometry components in use or None for all components.
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    """
    if not is_svd_used(components) and not is_cdc_used(components):
        return

    cdc_reco_tracks = "CDCRecoTracks"
    if not is_pxd_used(components) and not is_svd_used(components):
        cdc_reco_tracks = reco_tracks

    svd_cdc_reco_tracks = "SVDCDCRecoTracks"
    if not is_pxd_used(components):
        svd_cdc_reco_tracks = reco_tracks

    svd_reco_tracks = "SVDRecoTracks"
    pxd_reco_tracks = "PXDRecoTracks"

    full_reco_tracks = reco_tracks

    latest_reco_tracks = None

    if trigger_mode in ["fast_reco", "all"] and is_cdc_used(components):
        add_cdc_track_finding(path, use_second_hits=use_second_cdc_hits, output_reco_tracks=cdc_reco_tracks)
        latest_reco_tracks = cdc_reco_tracks

    if trigger_mode in ["hlt", "all"] and is_svd_used(components):
        # in case the lastest_reco_tracks is not set and we are in hlt mode, a previous call to
        # this method using trigger_mode = "fast_reco" was done and we have the CDC-only RecoTrack
        # already
        if trigger_mode == "hlt" and latest_reco_tracks is None:
            latest_reco_tracks = cdc_reco_tracks

        add_svd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              output_reco_tracks=svd_cdc_reco_tracks, use_mc_truth=use_mc_truth,
                              temporary_reco_tracks=svd_reco_tracks,
                              svd_ckf_mode=svd_ckf_mode, add_both_directions=add_both_directions,
                              vxdtf2_mva_weight_file=vxdtf2_mva_weight_file)
        latest_reco_tracks = svd_cdc_reco_tracks

    if trigger_mode in ["all"] and is_pxd_used(components):
        add_pxd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              use_mc_truth=use_mc_truth, output_reco_tracks=full_reco_tracks,
                              temporary_reco_tracks=pxd_reco_tracks,
                              add_both_directions=add_both_directions)

    if trigger_mode in ["all"] and prune_temporary_tracks:
        for temporary_reco_track_name in [pxd_reco_tracks, svd_reco_tracks, cdc_reco_tracks, svd_cdc_reco_tracks]:
            if temporary_reco_track_name != reco_tracks:
                path.add_module('PruneRecoTracks', storeArrayName=temporary_reco_track_name)


def add_cr_track_finding(path, reco_tracks="RecoTracks", components=None, data_taking_period='gcr2017',
                         merge_tracks=True, use_second_cdc_hits=False,
                         vxdtf2_mva_weight_file=None):
    import cdc.cr as cosmics_setup

    if data_taking_period != "phase2":
        cosmics_setup.set_cdc_cr_parameters(data_taking_period)

        # track finding
        add_cdc_cr_track_finding(path, merge_tracks=merge_tracks, use_second_cdc_hits=use_second_cdc_hits,
                                 trigger_point=tuple(cosmics_setup.triggerPos))

    else:
        if not is_cdc_used(components):
            B2FATAL("CDC must be in components")

        reco_tracks_from_track_finding = reco_tracks
        if merge_tracks:
            reco_tracks_from_track_finding = "NonMergedRecoTracks"

        cdc_reco_tracks = "CDCRecoTracks"
        if not is_pxd_used(components) and not is_svd_used(components):
            cdc_reco_tracks = reco_tracks_from_track_finding

        svd_cdc_reco_tracks = "SVDCDCRecoTracks"
        if not is_pxd_used(components):
            svd_cdc_reco_tracks = reco_tracks_from_track_finding

        full_reco_tracks = reco_tracks_from_track_finding

        # CDC track finding with default settings
        add_cdc_cr_track_finding(path, merge_tracks=False, use_second_cdc_hits=use_second_cdc_hits,
                                 output_reco_tracks=cdc_reco_tracks)

        latest_reco_tracks = cdc_reco_tracks

        if is_svd_used(components):
            add_svd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                                  output_reco_tracks=svd_cdc_reco_tracks,
                                  svd_ckf_mode="only_ckf", add_both_directions=True,
                                  vxdtf2_mva_weight_file=vxdtf2_mva_weight_file)
            latest_reco_tracks = svd_cdc_reco_tracks

        if is_pxd_used(components):
            add_pxd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                                  output_reco_tracks=full_reco_tracks, add_both_directions=True,
                                  filter_cut=0.01)

        if merge_tracks:
            # merge the tracks together
            path.add_module("CosmicsTrackMerger", inputRecoTracks=reco_tracks_from_track_finding,
                            outputRecoTracks=reco_tracks)


def add_mc_track_finding(path, components=None, reco_tracks="RecoTracks", use_second_cdc_hits=False):
    """
    Add the MC based TrackFinder to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """
    if is_cdc_used(components) or is_pxd_used(components) or is_svd_used(components):
        # find MCTracks in CDC, SVD and PXD (or a subset of it)
        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName=reco_tracks,
                        UseSecondCDCHits=use_second_cdc_hits,
                        UsePXDHits=is_pxd_used(components),
                        UseSVDHits=is_svd_used(components),
                        UseCDCHits=is_cdc_used(components))


def add_tracking_for_PXDDataReduction_simulation(path, components, svd_cluster='__ROIsvdClusters'):
    """
    This function adds the standard reconstruction modules for tracking to be used for the simulation of PXD data reduction
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components, always exclude the PXD.
    """

    if not is_svd_used(components):
        return

    # Material effects
    if 'SetupGenfitExtrapolation' not in path:
        material_effects = register_module('SetupGenfitExtrapolation')
        material_effects.set_name(
            'SetupGenfitExtrapolationForPXDDataReduction')
        path.add_module(material_effects)

    # SET StoreArray names

    svd_reco_tracks = "__ROIsvdRecoTracks"

    # SVD ONLY TRACK FINDING
    add_vxd_track_finding_vxdtf2(path, components=['SVD'], reco_tracks=svd_reco_tracks, suffix="__ROI",
                                 svd_clusters=svd_cluster)

    # TRACK FITTING

    # track fitting
    dafRecoFitter = register_module("DAFRecoFitter")
    dafRecoFitter.set_name("SVD-only DAFRecoFitter")
    dafRecoFitter.param('recoTracksStoreArrayName', svd_reco_tracks)
    dafRecoFitter.param('svdHitsStoreArrayName', svd_cluster)
    path.add_module(dafRecoFitter)


def add_track_mva_quality_estimation(path, reco_tracks="RecoTracks", svd_cdc_reco_tracks="SVDCDCRecoTracks",
                                     cdc_reco_tracks="CDCRecoTracks", svd_reco_tracks="SVDRecoTracks",
                                     pxd_reco_tracks="PXDRecoTracks",
                                     file='tracking/data/TrackQE_weight_files/FullTrackQE-Default.xml',
                                     do_training=False):
    if(not do_training):
        trackQualityEstimatorMVA = register_module('TrackQualityEstimatorMVA', recoTracksStoreArrayName=reco_tracks,
                                                   SVDCDCRecoTracksStoreArrayName=svd_cdc_reco_tracks,
                                                   CDCRecoTracksStoreArrayName=cdc_reco_tracks,
                                                   SVDRecoTracksStoreArrayName=svd_reco_tracks,
                                                   PXDRecoTracksStoreArrayName=pxd_reco_tracks)
        trackQualityEstimatorMVA.param('WeightFileIdentifier', file)
        path.add_module(trackQualityEstimatorMVA)
    else:
        trackQETrainingDataCollector = register_module('TrackQETrainingDataCollector', recoTracksStoreArrayName=reco_tracks,
                                                       SVDCDCRecoTracksStoreArrayName=svd_cdc_reco_tracks,
                                                       CDCRecoTracksStoreArrayName=cdc_reco_tracks,
                                                       SVDRecoTracksStoreArrayName=svd_reco_tracks,
                                                       PXDRecoTracksStoreArrayName=pxd_reco_tracks)
        trackQETrainingDataCollector.param('TrainingDataOutputName', file)
        path.add_module(trackQETrainingDataCollector)
