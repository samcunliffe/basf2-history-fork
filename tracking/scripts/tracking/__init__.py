#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_tracking_reconstruction(path, components=None, pruneTracks=False, skipGeometryAdding=False,
                                mcTrackFinding=False, trigger_mode="all", additionalTrackFitHypotheses=None,
                                reco_tracks="RecoTracks", prune_temporary_tracks=True, use_vxdtf2=False,
                                fit_tracks=True, use_second_cdc_hits=False):
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
    :param mcTrackFinding: Use the MC track finders instead of the realistic ones.
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    :param fit_tracks: If false, the final track find and the TrackCreator module will no be executed
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """

    if not is_svd_used(components) and not is_cdc_used(components):
        return

    if not skipGeometryAdding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components=components)

    # Material effects for all track extrapolations
    if trigger_mode in ["all", "hlt"] and 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    if mcTrackFinding:
        # Always add the MC finder in all trigger modes.
        add_mc_track_finding(path, components=components, reco_tracks=reco_tracks,
                             use_second_cdc_hits=use_second_cdc_hits)
    else:
        add_track_finding(path, components=components, trigger_mode=trigger_mode, reco_tracks=reco_tracks,
                          prune_temporary_tracks=prune_temporary_tracks, use_vxdtf2=use_vxdtf2,
                          use_second_cdc_hits=use_second_cdc_hits)

    if trigger_mode in ["hlt", "all"]:
        add_mc_matcher(path, components=components, reco_tracks=reco_tracks, use_second_cdc_hits=use_second_cdc_hits)

        if fit_tracks:
            add_track_fit_and_track_creator(path, components=components, pruneTracks=pruneTracks,
                                            additionalTrackFitHypotheses=additionalTrackFitHypotheses,
                                            reco_tracks=reco_tracks)


def add_cr_tracking_reconstruction(path, components=None, prune_tracks=False,
                                   skip_geometry_adding=False, event_time_extraction=True,
                                   data_taking_period="gcr2017", top_in_counter=False,
                                   merge_tracks=True, use_second_cdc_hits=False):
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
    import cdc.cr as cosmics_setup

    # make sure CDC is used
    if not is_cdc_used(components):
        return

    if not skip_geometry_adding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    cosmics_setup.set_cdc_cr_parameters(data_taking_period)

    # track finding
    add_cdc_cr_track_finding(path, merge_tracks=merge_tracks, use_second_cdc_hits=use_second_cdc_hits,
                             trigger_point=tuple(cosmics_setup.triggerPos))

    # track fitting
    add_cdc_cr_track_fit_and_track_creator(path, components, prune_tracks=prune_tracks,
                                           event_timing_extraction=event_time_extraction,
                                           data_taking_period=data_taking_period, top_in_counter=top_in_counter)

    if merge_tracks:
        # Do also fit the not merged tracks
        add_cdc_cr_track_fit_and_track_creator(path, components, prune_tracks=prune_tracks,
                                               event_timing_extraction=False,
                                               data_taking_period=data_taking_period, top_in_counter=top_in_counter,
                                               reco_tracks="NonMergedRecoTracks", tracks="NonMergedTracks")


def add_geometry_modules(path, components=None):
    """
    Helper function to add the geometry related modules needed for tracking
    to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    """
    # check for detector geometry, necessary for track extrapolation in genfit
    if 'Geometry' not in path:
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)


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


def add_track_fit_and_track_creator(path, components=None, pruneTracks=False, additionalTrackFitHypotheses=None,
                                    reco_tracks="RecoTracks"):
    """
    Helper function to add the modules performing the
    track fit, the V0 fit and the Belle2 track creation to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """
    # Correct time seed
    path.add_module("IPTrackTimeEstimator", recoTracksStoreArrayName=reco_tracks, useFittedInformation=False)
    # track fitting
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=reco_tracks).set_name("Combined_DAFRecoFitter")
    # create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator', defaultPDGCode=211, recoTrackColName=reco_tracks,
                    additionalPDGCodes=[13, 321, 2212] if additionalTrackFitHypotheses is None else additionalTrackFitHypotheses)
    # V0 finding
    path.add_module('V0Finder', RecoTrackColName=reco_tracks)

    # prune genfit tracks
    if pruneTracks:
        add_prune_tracks(path, components=components, reco_tracks=reco_tracks)


def add_cdc_cr_track_fit_and_track_creator(path, components=None,
                                           data_taking_period='gcr2017', top_in_counter=False,
                                           prune_tracks=False, event_timing_extraction=True,
                                           reco_tracks="RecoTracks", tracks=""):
    """
    Helper function to add the modules performing the cdc cr track fit
    and track creation to the path.

    :param path: The path to which to add the tracking reconstruction modules
    :param data_taking_period: The cosmics generation will be added using the
           parameters, that where used in this period of data taking. The periods can be found in cdc/cr/__init__.py.

    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: The name of the reco tracks to use
    :param tracks: the name of the output Belle tracks
    :param prune_tracks: Delete all hits expect the first and the last from the found tracks.
    :param event_timing_extraction: extract time with either the TrackTimeExtraction or
        FullGridTrackTimeExtraction modules.

    :param top_in_counter: time of propagation from the hit point to the PMT in the trigger counter is subtracted
           (assuming PMT is put at -z of the counter).
    """

    import cdc.cr as cosmics_setup

    cosmics_setup.set_cdc_cr_parameters(data_taking_period)

    # Time seed
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    recoTracksStoreArrayName=reco_tracks,
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=cosmics_setup.triggerPos,
                    triggerPlaneDirection=cosmics_setup.normTriggerPlaneDirection,
                    useFittedInformation=False)

    # Initial track fitting
    path.add_module("DAFRecoFitter",
                    recoTracksStoreArrayName=reco_tracks,
                    probCut=0.00001,
                    pdgCodesToUseForFitting=13,
                    )

    # Correct time seed
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    recoTracksStoreArrayName=reco_tracks,
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=cosmics_setup.triggerPos,
                    triggerPlaneDirection=cosmics_setup.normTriggerPlaneDirection,
                    useFittedInformation=True,
                    useReadoutPosition=top_in_counter,
                    readoutPosition=cosmics_setup.readOutPos,
                    readoutPositionPropagationSpeed=cosmics_setup.lightPropSpeed
                    )

    # Track fitting
    path.add_module("DAFRecoFitter",
                    recoTracksStoreArrayName=reco_tracks,
                    pdgCodesToUseForFitting=13,
                    )

    if event_timing_extraction:
        # Extract the time
        path.add_module("FullGridTrackTimeExtraction",
                        recoTracksStoreArrayName=reco_tracks,
                        maximalT0Shift=40,
                        minimalT0Shift=-40,
                        numberOfGrids=6
                        )

        # Track fitting
        path.add_module("DAFRecoFitter",
                        # probCut=0.00001,
                        recoTracksStoreArrayName=reco_tracks,
                        pdgCodesToUseForFitting=13,
                        )

    # Create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator',
                    recoTrackColName=reco_tracks,
                    trackColName=tracks,
                    defaultPDGCode=13,
                    useClosestHitToIP=True
                    )

    # Prune genfit tracks
    if prune_tracks:
        add_prune_tracks(path=path, components=components, reco_tracks=reco_tracks)


def add_mc_matcher(path, components=None, reco_tracks="RecoTracks", use_second_cdc_hits=False,
                   split_after_delta_t=-1.0):
    """
    Match the tracks to the MC truth. The matching works based on
    the output of the TrackFinderMCTruthRecoTracks.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """
    path.add_module('TrackFinderMCTruthRecoTracks',
                    RecoTracksStoreArrayName='MCRecoTracks',
                    WhichParticles=[],
                    UseSecondCDCHits=use_second_cdc_hits,
                    UsePXDHits=is_pxd_used(components),
                    UseSVDHits=is_svd_used(components),
                    UseCDCHits=is_cdc_used(components),
                    SplitAfterDeltaT=split_after_delta_t)

    path.add_module('MCRecoTracksMatcher',
                    mcRecoTracksStoreArrayName='MCRecoTracks',
                    prRecoTracksStoreArrayName=reco_tracks,
                    UsePXDHits=is_pxd_used(components),
                    UseSVDHits=is_svd_used(components),
                    UseCDCHits=is_cdc_used(components))


def add_prune_tracks(path, components=None, reco_tracks="RecoTracks"):
    """
    Adds removal of the intermediate states at each measurement from the fitted tracks.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """

    # do not add any pruning, if no tracking detectors are in the components
    if components and not ('SVD' in components or 'CDC' in components):
        return

    path.add_module('PruneRecoTracks', storeArrayName=reco_tracks)
    path.add_module("PruneGenfitTracks")


def add_track_finding(
        path,
        components=None,
        trigger_mode="all",
        reco_tracks="RecoTracks",
        prune_temporary_tracks=True,
        use_vxdtf2=False,
        use_second_cdc_hits=False):
    """
    Adds the realistic track finding to the path.
    The result is a StoreArray 'RecoTracks' full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    """
    if not is_svd_used(components) and not is_cdc_used(components):
        return

    use_svd = is_svd_used(components)
    use_cdc = is_cdc_used(components)

    # if only CDC or VXD are used, the track finding result
    # will be directly written to the final RecoTracks array
    # because no merging is required

    if use_cdc and use_svd:
        cdc_reco_tracks = "CDCRecoTracks"
        vxd_reco_tracks = "VXDRecoTracks"
    else:
        cdc_reco_tracks = reco_tracks
        vxd_reco_tracks = reco_tracks

    # CDC track finder
    if use_cdc and trigger_mode in ["fast_reco", "all"]:
        add_cdc_track_finding(path, reco_tracks=cdc_reco_tracks, use_second_hits=use_second_cdc_hits)

    # VXD track finder
    if use_svd and trigger_mode in ["hlt", "all"]:
        if use_vxdtf2:
            # version 2 of the track finder
            add_vxd_track_finding_vxdtf2(path, components=components, reco_tracks=vxd_reco_tracks)
        else:
            # version 1 of the track finder
            add_vxd_track_finding(path, components=components, reco_tracks=vxd_reco_tracks)

        # track merging
        if use_svd and use_cdc:
            # Merge CDC and CXD tracks
            path.add_module('VXDCDCTrackMerger',
                            CDCRecoTrackColName=cdc_reco_tracks,
                            VXDRecoTrackColName=vxd_reco_tracks)

            path.add_module("RelatedTracksCombiner", VXDRecoTracksStoreArrayName=vxd_reco_tracks,
                            CDCRecoTracksStoreArrayName=cdc_reco_tracks,
                            recoTracksStoreArrayName=reco_tracks)

            # Prune the temporary products if requested
            if prune_temporary_tracks:
                path.add_module('PruneRecoTracks', storeArrayName=cdc_reco_tracks)
                path.add_module('PruneRecoTracks', storeArrayName=vxd_reco_tracks)


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


def add_cdc_track_finding(path, reco_tracks="RecoTracks", with_ca=False, use_second_hits=False):
    """
    Convenience function for adding all cdc track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks. Defaults to RecoTracks.
    :param use_second_hits: If true, the second hit information will be used in the CDC track finding.
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("TFCDC_WireHitPreparer",
                    useSecondHits=use_second_hits,
                    flightTimeEstimation="outwards")

    # Constructs clusters and reduce background hits
    path.add_module("TFCDC_ClusterPreparer",
                    ClusterFilter="mva_bkg",
                    ClusterFilterParameters={"cut": 0.2})

    # Find segments within the clusters
    path.add_module("TFCDC_SegmentFinderFacetAutomaton")

    # Find axial tracks
    path.add_module("TFCDC_AxialTrackFinderLegendre")

    # Improve the quality of the axial tracks
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=["B2B"])

    # Find the stereo hits to those axial tracks
    path.add_module('TFCDC_StereoHitFinder')

    # Combine segments with axial tracks
    path.add_module('TFCDC_SegmentTrackCombiner',
                    segmentTrackFilter="mva",
                    segmentTrackFilterParameters={"cut": 0.74},
                    trackFilter="mva",
                    trackFilterParameters={"cut": 0.1})

    if with_ca:
        path.add_module("TFCDC_TrackFinderSegmentPairAutomaton",
                        tracks="CDCTrackVector2")

        # Overwrites the origin CDCTrackVector
        path.add_module("TFCDC_TrackCombiner",
                        inputTracks="CDCTrackVector",
                        secondaryInputTracks="CDCTrackVector2",
                        tracks="CDCTrackVector")

    # Improve the quality of all tracks and output
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=[
                        "LayerBreak",
                        "OneSuperlayer",
                        "Small",
                    ])

    if with_ca:
        # Add curlers in the axial inner most superlayer
        path.add_module("TFCDC_TrackCreatorSingleSegments",
                        MinimalHitsBySuperLayerId={0: 15})

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    RecoTracksStoreArrayName=reco_tracks)

    # Correct time seed (only necessary for the CDC tracks)
    path.add_module("IPTrackTimeEstimator",
                    useFittedInformation=False,
                    recoTracksStoreArrayName=reco_tracks)


def add_cdc_cr_track_finding(path, reco_tracks="RecoTracks", trigger_point=(0, 0, 0), merge_tracks=True,
                             use_second_cdc_hits=False):
    """
    Convenience function for adding all cdc track finder modules currently dedicated for the CDC-TOP testbeam
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).

    Arguments
    ---------
    path: basf2.Path
       The path to be filled
    reco_tracks: str
       Name of the output RecoTracks. Defaults to RecoTracks.
    merge_tracks: bool
       The upper and lower half of the tracks should be merged together in one track
    use_second_hits: bool
       If true, the second hit information will be used in the CDC track finding.
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("TFCDC_WireHitPreparer",
                    useSecondHits=use_second_cdc_hits,
                    flightTimeEstimation="downwards",
                    triggerPoint=trigger_point)

    # Constructs clusters and reduce background hits
    path.add_module("TFCDC_ClusterPreparer",
                    ClusterFilter="mva_bkg",
                    ClusterFilterParameters={"cut": 0.2})

    # Find segments within the clusters
    path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                    SegmentOrientation="downwards")

    # Find axial tracks
    path.add_module("TFCDC_AxialTrackFinderLegendre")

    # Improve the quality of the axial tracks
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=["B2B"])

    # Find the stereo hits to those axial tracks
    path.add_module('TFCDC_StereoHitFinder')

    # Combine segments with axial tracks
    path.add_module('TFCDC_SegmentTrackCombiner',
                    segmentTrackFilter="mva",
                    segmentTrackFilterParameters={"cut": 0.74},
                    trackFilter="mva",
                    trackFilterParameters={"cut": 0.1})

    # Improve the quality of all tracks and output
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=["LayerBreak", "OneSuperlayer", "Small"],
                    )

    # Flip track orientation to always point downwards
    path.add_module("TFCDC_TrackOrienter",
                    inputTracks="CDCTrackVector",
                    tracks="OrientedCDCTrackVector",
                    TrackOrientation="downwards",
                    )

    output_tracks = "OrientedCDCTrackVector"

    if merge_tracks:
        # Merge tracks together if needed
        path.add_module("TFCDC_TrackLinker",
                        inputTracks="OrientedCDCTrackVector",
                        tracks="MergedCDCTrackVector",
                        filter="phi",
                        )
        output_tracks = "MergedCDCTrackVector"

        # However, we also want to export the non merged tracks
        # Correct time seed - assumes velocity near light speed
        path.add_module("TFCDC_TrackFlightTimeAdjuster",
                        inputTracks="OrientedCDCTrackVector",
                        )

        # Export CDCTracks to RecoTracks representation
        path.add_module("TFCDC_TrackExporter",
                        inputTracks="OrientedCDCTrackVector",
                        RecoTracksStoreArrayName="NonMergedRecoTracks")

    # Correct time seed - assumes velocity near light speed
    path.add_module("TFCDC_TrackFlightTimeAdjuster",
                    inputTracks=output_tracks,
                    )

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    inputTracks=output_tracks,
                    RecoTracksStoreArrayName=reco_tracks)


def add_vxd_track_finding(path, reco_tracks="RecoTracks", components=None, suffix=""):
    """
    Convenience function for adding all vxd track finder modules
    to the path. This is for version1 of the trackfinder

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks, Defaults to RecoTracks.
    :param components: List of the detector components to be used in the reconstruction. Defaults to None which means all
                components.
    :param suffix: all names of intermediate Storearrays will have the suffix appended. Useful in cases someone needs to put several
                   instances of track finding in one path.
    """

    # Temporary array
    # add a suffix to be able to have different
    vxd_trackcands = '__VXDGFTrackCands' + suffix

    vxd_trackfinder = path.add_module('VXDTF', GFTrackCandidatesColName=vxd_trackcands)
    # WARNING: workaround for possible clashes between fitting and VXDTF
    # stays until the redesign of the VXDTF is finished.
    vxd_trackfinder.param('TESTERexpandedTestingRoutines', False)
    if is_pxd_used(components):
        vxd_trackfinder.param('sectorSetup',
                              ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
                               'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
                               'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
                               ])
        vxd_trackfinder.param('tuneCutoffs', 0.22)
    else:
        vxd_trackfinder.param('sectorSetup',
                              ['shiftedL3IssueTestSVDStd-moreThan400MeV_SVD',
                               'shiftedL3IssueTestSVDStd-100to400MeV_SVD',
                               'shiftedL3IssueTestSVDStd-25to100MeV_SVD'
                               ])
        vxd_trackfinder.param('tuneCutoffs', 0.06)

    # Convert VXD trackcands to reco tracks
    path.add_module("RecoTrackCreator", trackCandidatesStoreArrayName=vxd_trackcands,
                    recoTracksStoreArrayName=reco_tracks, recreateSortingParameters=True)


def add_vxd_track_finding_vxdtf2(path, reco_tracks="RecoTracks", components=None, suffix="",
                                 sectormap_file=None):
    """
    Convenience function for adding all vxd track finder Version 2 modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks, Defaults to RecoTracks.
    :param components: List of the detector components to be used in the reconstruction. Defaults to None which means all
                       components.
    :param suffix: all names of intermediate Storearrays will have the suffix appended. Useful in cases someone needs to
                   put several instances of track finding in one path.
    :param sectormap_file: if set to a finite value, a file will be used instead of the sectormap in the database.
    """
    ##########################
    # some setting for VXDTF2
    ##########################
    use_segment_network_filters = True
    filter_overlapping = True
    # the 'tripletFit' currently does not work with PXD
    quality_estimator = 'tripletFit'  # other option is 'circleFit'
    overlap_filter = 'greedy'  # other option is  'hopfield'
    # setting different for pxd and svd:
    if is_pxd_used(components):
        setup_name = "SVDPXDDefault"
        db_sec_map_file = "SVDPXDDefaultMap.root"
        use_pxd = True
    else:
        setup_name = "SVDOnlyDefault"
        db_sec_map_file = "SVDOnlyDefaultMap.root"
        use_pxd = False

    #################
    # VXDTF2 Step 0
    # Preparation
    #################
    nameSPs = 'SpacePoints' + suffix

    if 'SpacePointCreatorPXD' not in path:
        if use_pxd:
            spCreatorPXD = register_module('SpacePointCreatorPXD')
            spCreatorPXD.param('NameOfInstance', 'PXDSpacePoints')
            spCreatorPXD.param('SpacePoints', nameSPs)
            path.add_module(spCreatorPXD)

    if 'SpacePointCreatorSVD' not in path:
        # always use svd!
        spCreatorSVD = register_module('SpacePointCreatorSVD')
        spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
        spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
        spCreatorSVD.param('SpacePoints', nameSPs)
        path.add_module(spCreatorSVD)

    # SecMap Bootstrap
    secMapBootStrap = register_module('SectorMapBootstrap')
    secMapBootStrap.param('ReadSectorMap', sectormap_file is not None)  # read from file
    secMapBootStrap.param('ReadSecMapFromDB', sectormap_file is None)  # this will override ReadSectorMap
    secMapBootStrap.param('SectorMapsInputFile', sectormap_file or db_sec_map_file)
    secMapBootStrap.param('SetupToRead', setup_name)
    secMapBootStrap.param('WriteSectorMap', False)
    path.add_module(secMapBootStrap)

    ##################
    # VXDTF2 Step 1
    # SegmentNet
    ##################
    nameSegNet = 'SegmentNetwork' + suffix
    segNetProducer = register_module('SegmentNetworkProducer')
    segNetProducer.param('CreateNeworks', 3)
    segNetProducer.param('NetworkOutputName', nameSegNet)
    segNetProducer.param('allFiltersOff', not use_segment_network_filters)
    segNetProducer.param('SpacePointsArrayNames', [nameSPs])
    segNetProducer.param('printNetworks', False)
    segNetProducer.param('sectorMapName', setup_name)
    segNetProducer.param('addVirtualIP', False)
    segNetProducer.param('observerType', 0)
    path.add_module(segNetProducer)

    #################
    # VXDTF2 Step 2
    # TrackFinder
    #################

    # append a suffix to the storearray name
    nameSPTCs = 'SPTrackCands' + suffix

    cellOmat = register_module('TrackFinderVXDCellOMat')
    cellOmat.param('NetworkName', nameSegNet)
    cellOmat.param('SpacePointTrackCandArrayName', nameSPTCs)
    cellOmat.param('SpacePoints', nameSPs)
    cellOmat.param('printNetworks', False)
    cellOmat.param('strictSeeding', True)
    path.add_module(cellOmat)

    #################
    # VXDTF2 Step 3
    # Analyzer
    #################

    # Quality
    qualityEstimator = register_module('QualityEstimatorVXD')
    qualityEstimator.param('EstimationMethod', quality_estimator)
    qualityEstimator.param('SpacePointTrackCandsStoreArrayName', nameSPTCs)
    path.add_module(qualityEstimator)

    # will discard track candidates (with low quality estimators) if the number of TC is above threshold
    maxCandidateSelection = register_module('BestVXDTrackCandidatesSelector')
    maxCandidateSelection.param('NameSpacePointTrackCands', nameSPTCs)
    maxCandidateSelection.param('NewNameSpacePointTrackCands', nameSPTCs)
    maxCandidateSelection.param('SubsetCreation', False)
    path.add_module(maxCandidateSelection)

    # Properties
    vIPRemover = register_module('SPTCvirtualIPRemover')
    vIPRemover.param('tcArrayName', nameSPTCs)
    vIPRemover.param('maxTCLengthForVIPKeeping', 0)  # want to remove virtualIP for any track length
    path.add_module(vIPRemover)

    #################
    # VXDTF2 Step 4
    # OverlapFilter
    #################

    if filter_overlapping:
        overlapResolver = register_module('SVDOverlapResolver')
        overlapResolver.param('NameSpacePointTrackCands', nameSPTCs)
        overlapResolver.param('ResolveMethod', overlap_filter.lower())
        path.add_module(overlapResolver)
    #################
    # VXDTF2 Step 5
    # Converter
    #################
    momSeedRetriever = register_module('SPTCmomentumSeedRetriever')
    momSeedRetriever.param('tcArrayName', nameSPTCs)
    path.add_module(momSeedRetriever)

    converter = register_module('SPTC2RTConverter')
    converter.param('recoTracksStoreArrayName', reco_tracks)
    converter.param('spacePointsTCsStoreArrayName', nameSPTCs)
    path.add_module(converter)


def is_svd_used(components):
    """Return true, if the SVD is present in the components list"""
    return components is None or 'SVD' in components


def is_pxd_used(components):
    """Return true, if the PXD is present in the components list"""
    return components is None or 'PXD' in components


def is_cdc_used(components):
    """Return true, if the CDC is present in the components list"""
    return components is None or 'CDC' in components


def add_tracking_for_PXDDataReduction_simulation(path, components, use_vxdtf2=False):
    """
    This function adds the standard reconstruction modules for tracking to be used for the simulation of PXD data reduction
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components, always exclude the PXD.
    :param use_vxdtf2: true if VXDTF2 is used instead of VXDTF1
    """

    if not is_svd_used(components):
        return

    # Material effects
    if 'SetupGenfitExtrapolation' not in path:
        material_effects = register_module('SetupGenfitExtrapolation')
        material_effects.set_name('SetupGenfitExtrapolationForPXDDataReduction')
        path.add_module(material_effects)

    # SET StoreArray names
    svd_reco_tracks = "__ROIsvdRecoTracks"

    # SVD ONLY TRACK FINDING
    if(use_vxdtf2):
        add_vxd_track_finding_vxdtf2(path, components=['SVD'], reco_tracks=svd_reco_tracks, suffix="__ROI")
    else:
        add_vxd_track_finding(path, components=['SVD'], reco_tracks=svd_reco_tracks, suffix="__ROI")

    # TRACK FITTING

    # track fitting
    dafRecoFitter = register_module("DAFRecoFitter")
    dafRecoFitter.set_name("SVD-only DAFRecoFitter")
    dafRecoFitter.param('recoTracksStoreArrayName', svd_reco_tracks)
    path.add_module(dafRecoFitter)
