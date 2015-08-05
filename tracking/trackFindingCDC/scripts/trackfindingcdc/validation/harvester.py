from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners

from ROOT import Belle2

import numpy as np


class ReconstructionPositionHarvester(HarvestingModule):
    #: Harvester module to check for the reconstructed positions

    def __init__(self, output_file_name, tracks_store_vector_name="CDCTrackVector"):
        super(
            ReconstructionPositionHarvester,
            self).__init__(
            foreach=tracks_store_vector_name,
            output_file_name=output_file_name)

        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp()

    def peel(self, track_cand):

        mc_hit_lookup = self.mc_hit_lookup
        mc_hit_lookup.fill()

        sum_of_difference_norms_stereo = 0
        num_norms_stereo = 0
        sum_of_difference_norms_axial = 0
        num_norms_axial = 0

        number_of_wrong_rl_infos = 0

        for reco_hit in track_cand.items():
            underlaying_cdc_hit = reco_hit.getWireHit().getHit()
            hit_difference = mc_hit_lookup.getRecoPos3D(underlaying_cdc_hit) - reco_hit.getRecoPos3D()
            sum_of_difference_norms_axial += hit_difference.xy().norm()
            num_norms_axial += 1

            if reco_hit.getStereoType() != 0:  # AXIAL
                sum_of_difference_norms_stereo += abs(hit_difference.z())
                num_norms_stereo += 1

                correct_rl_info = mc_hit_lookup.getRLInfo(underlaying_cdc_hit)

                if correct_rl_info != reco_hit.getRLInfo():
                    number_of_wrong_rl_infos += 1

        return dict(sum_of_difference_norms_axial=sum_of_difference_norms_axial,
                    num_norms_axial=num_norms_axial,
                    mean_of_difference_norms_axial=np.true_divide(sum_of_difference_norms_axial, num_norms_axial),
                    sum_of_difference_norms_stereo=sum_of_difference_norms_stereo,
                    num_norms_stereo=num_norms_stereo,
                    mean_of_difference_norms_stereo=np.true_divide(sum_of_difference_norms_stereo, num_norms_stereo),
                    number_of_wrong_rl_infos=number_of_wrong_rl_infos,
                    mean_wrong_rl_infos=np.true_divide(number_of_wrong_rl_infos, num_norms_stereo))

    save_tree = refiners.save_tree(folder_name="tree")


class WrongRLInfoCounter(HarvestingModule):
    #: Harvester module to check for the reconstructed positions

    def __init__(self, output_file_name, tracks_store_vector_name="TrackCands"):
        HarvestingModule.__init__(self,
                                  foreach=tracks_store_vector_name,
                                  output_file_name=output_file_name)

        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp()

    def peel(self, track_cand):
        cdc_hits = Belle2.PyStoreArray("CDCHits")

        mc_hit_lookup = self.mc_hit_lookup
        mc_hit_lookup.fill()

        number_of_wrong_rl_infos = 0
        number_of_wrong_rl_infos_stereo_only = 0

        number_of_hits = 0
        number_of_stereo = 0

        for hitID in xrange(track_cand.getNHits()):
            hit = track_cand.getHit(hitID)
            if not hit.__class__.__name__ == "genfit::WireTrackCandHit":
                continue

            underlaying_cdc_hit = cdc_hits[hit.getHitId()]

            correct_rl_info = mc_hit_lookup.getRLInfo(underlaying_cdc_hit)

            number_of_hits += 1

            if underlaying_cdc_hit.getISuperLayer() % 2 != 0:
                number_of_stereo += 1

            if correct_rl_info != hit.getLeftRightResolution():
                if underlaying_cdc_hit.getISuperLayer() % 2 != 0:
                    number_of_wrong_rl_infos_stereo_only += 1
                number_of_wrong_rl_infos += 1

        return dict(number_of_wrong_rl_infos=number_of_wrong_rl_infos,
                    number_of_wrong_rl_infos_stereo_only=number_of_wrong_rl_infos_stereo_only,
                    number_of_hits=number_of_hits,
                    number_of_stereo=number_of_stereo,
                    mean_wrong_rl=np.true_divide(number_of_wrong_rl_infos, number_of_hits),
                    mean_wrong_rl_stereo=np.true_divide(number_of_wrong_rl_infos_stereo_only, number_of_stereo))

    save_tree = refiners.save_tree(folder_name="tree")


class SegmentFakeRatesModule(HarvestingModule):
    #: Harvesting module to check for basic matching information of the found segments

    def __init__(
            self,
            output_file_name,
            local_track_cands_store_array_name="LocalTrackCands",
            mc_track_cands_store_array_name="MCTrackCands",
            legendre_track_cand_store_array_name="LegendreTrackCands"):
        super(
            SegmentFakeRatesModule,
            self).__init__(
            foreach=local_track_cands_store_array_name,
            output_file_name=output_file_name)

        self.mc_track_cands_store_array_name = mc_track_cands_store_array_name
        self.legendre_track_cand_store_array_name = legendre_track_cand_store_array_name

        self.mc_track_matcher_local = Belle2.TrackMatchLookUp(self.mc_track_cands_store_array_name, self.foreach)
        self.mc_track_matcher_legendre = Belle2.TrackMatchLookUp(
            self.mc_track_cands_store_array_name,
            legendre_track_cand_store_array_name)

    def prepare(self):
        self.cdcHits = Belle2.PyStoreArray("CDCHits")
        return HarvestingModule.prepare(self)

    def peel(self, local_track_cand):
        mc_track_matcher_local = self.mc_track_matcher_local
        mc_track_matcher_legendre = self.mc_track_matcher_legendre

        cdc_hits = self.cdcHits

        is_background = mc_track_matcher_local.isBackgroundPRTrackCand(local_track_cand)
        is_ghost = mc_track_matcher_local.isGhostPRTrackCand(local_track_cand)
        is_matched = mc_track_matcher_local.isMatchedPRTrackCand(local_track_cand)
        is_clone = mc_track_matcher_local.isClonePRTrackCand(local_track_cand)
        is_clone_or_matched = is_matched or is_clone
        hit_purity = abs(mc_track_matcher_local.getRelatedPurity(local_track_cand))

        # Stereo Track?
        first_cdc_hit_id = local_track_cand.getHitIDs(Belle2.Const.CDC)[0]
        first_cdc_hit = cdc_hits[first_cdc_hit_id]
        is_stereo = first_cdc_hit.getISuperLayer() % 2 == 1
        superlayer_of_segment = first_cdc_hit.getISuperLayer()

        has_partner = np.NaN
        hit_purity_of_partner = np.NaN
        hit_efficiency_of_partner = np.NaN
        mc_track_pt = np.NaN
        mc_track_dist = np.NaN
        number_of_new_hits = local_track_cand.getNHits()
        number_of_hits = local_track_cand.getNHits()
        number_of_hits_in_same_superlayer = np.NaN
        partner_has_stereo_information = np.NaN

        if is_clone_or_matched:
            related_mc_track_cand = mc_track_matcher_local.getRelatedMCTrackCand(local_track_cand)
            has_partner = (mc_track_matcher_legendre.isMatchedMCTrackCand(related_mc_track_cand) or
                           mc_track_matcher_legendre.isMergedMCTrackCand(related_mc_track_cand))
            mc_track_pt = related_mc_track_cand.getMomSeed().Pt()
            mc_track_dist = related_mc_track_cand.getPosSeed().Mag()
            if has_partner:
                partner_legendre_track_cand = mc_track_matcher_legendre.getRelatedPRTrackCand(related_mc_track_cand)
                hit_purity_of_partner = abs(mc_track_matcher_legendre.getRelatedPurity(partner_legendre_track_cand))
                hit_efficiency_of_partner = abs(mc_track_matcher_legendre.getRelatedEfficiency(related_mc_track_cand))

                # Count number of new hits
                legendre_hits = set(list(partner_legendre_track_cand.getHitIDs()))
                local_hits = set(list(local_track_cand.getHitIDs()))

                local_hits_new = local_hits - legendre_hits
                number_of_new_hits = len(local_hits_new)

                # Count number of hits in this superlayer
                partner_has_stereo_information = 0
                number_of_hits_in_same_superlayer = 0
                for cdc_hit_ID in legendre_hits:
                    cdc_hit = cdc_hits[cdc_hit_ID]
                    if cdc_hit.getISuperLayer() == superlayer_of_segment:
                        number_of_hits_in_same_superlayer += 1

                    if cdc_hit.getISuperLayer() % 2 == 1:
                        partner_has_stereo_information = 1

        return dict(superlayer_of_segment=superlayer_of_segment,
                    mc_track_dist=mc_track_dist,
                    is_background=is_background,
                    is_ghost=is_ghost,
                    is_matched=is_matched,
                    is_clone=is_clone,
                    is_clone_or_matched=is_clone_or_matched,
                    is_stereo=is_stereo,
                    mc_track_pt=mc_track_pt,
                    hit_purity=hit_purity,
                    has_partner=has_partner,
                    hit_purity_of_partner=hit_purity_of_partner,
                    hit_efficiency_of_partner=hit_efficiency_of_partner,
                    number_of_new_hits=number_of_new_hits,
                    number_of_hits=number_of_hits,
                    number_of_hits_in_same_superlayer=number_of_hits_in_same_superlayer,
                    partner_has_stereo_information=partner_has_stereo_information)

    save_tree = refiners.save_tree(folder_name="tree")


class SegmentFinderParameterExtractorModule(HarvestingModule):
    #: Harvester module to extract momentum and position information from the found segments and tracks

    def __init__(self, local_track_cands_store_array_name, mc_track_cands_store_array_name, output_file_name):
        super(
            SegmentFinderParameterExtractorModule,
            self).__init__(
            foreach=local_track_cands_store_array_name,
            output_file_name=output_file_name)

        self.mc_track_cands_store_array_name = mc_track_cands_store_array_name

        self.mc_track_matcher = Belle2.TrackMatchLookUp(self.mc_track_cands_store_array_name,
                                                        self.foreach)

    def peel(self, local_track_cand):
        mc_track_matcher = self.mc_track_matcher

        is_matched = mc_track_matcher.isMatchedPRTrackCand(local_track_cand)
        is_background = mc_track_matcher.isBackgroundPRTrackCand(local_track_cand)
        is_ghost = mc_track_matcher.isGhostPRTrackCand(local_track_cand)

        related_mc_track_cand = mc_track_matcher.getRelatedMCTrackCand(local_track_cand)

        track_momentum = np.NaN
        track_position = np.NaN
        track_pt = np.NaN
        track_phi = np.NaN

        segment_momentum = np.NaN
        segment_position = np.NaN
        segment_pt = np.NaN
        segment_phi = np.NaN

        if is_matched:
            track_momentum = related_mc_track_cand.getPosSeed()
            track_position = related_mc_track_cand.getPosSeed()
            track_pt = track_momentum.Pt()
            track_phi = track_momentum.Phi()
            trajectory_track = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(track_position),
                                                                      Belle2.TrackFindingCDC.Vector3D(track_momentum),
                                                                      related_mc_track_cand.getChargeSeed())

            segment_momentum = local_track_cand.getMomSeed()
            segment_position = local_track_cand.getPosSeed()
            segment_pt = segment_momentum.Pt()
            segment_phi = segment_momentum.Phi()
            trajectory_segment = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(segment_position),
                                                                        Belle2.TrackFindingCDC.Vector3D(segment_momentum),
                                                                        local_track_cand.getChargeSeed())

        return dict(is_matched=is_matched,
                    is_background=is_background,
                    is_ghost=is_ghost,
                    segment_pt=segment_pt,
                    track_pt=track_pt,
                    difference_pt=segment_pt - track_pt,
                    segment_phi=segment_phi,
                    track_phi=track_phi,
                    difference_phi=segment_phi - track_phi)

    save_tree = refiners.save_tree(folder_name="tree")


class SeedsAnalyser(HarvestingModule):

    def __init__(self, output_file_name, track_cands):
        HarvestingModule.__init__(self, foreach=track_cands, output_file_name=output_file_name)

    def peel(self, legendre_track_cand):
        # Reconstruct the z0 of the origin
        Vector3D = Belle2.TrackFindingCDC.Vector3D
        Vector2D = Belle2.TrackFindingCDC.Vector2D

        position = Vector3D(legendre_track_cand.getPosSeed())
        momentum = Vector3D(legendre_track_cand.getMomSeed())
        trajectory3D = Belle2.TrackFindingCDC.CDCTrajectory3D(position, momentum, legendre_track_cand.getChargeSeed())
        trajectory3D.setLocalOrigin(Vector3D())
        trajectorySZ = trajectory3D.getTrajectorySZ()

        perpSOrigin = trajectory3D.calcPerpS(Vector3D(0, 0, 0))

        if legendre_track_cand.getChargeSeed() > 0:
            helix = Belle2.Helix(legendre_track_cand.getPosSeed(), legendre_track_cand.getMomSeed(), 1, 1.5)
        if legendre_track_cand.getChargeSeed() < 0:
            helix = Belle2.Helix(legendre_track_cand.getPosSeed(), legendre_track_cand.getMomSeed(), -1, 1.5)

        matcher = Belle2.TrackMatchLookUp("MCTrackCands", self.foreach)
        mc_track_cand = matcher.getMatchedMCTrackCand(legendre_track_cand)

        if mc_track_cand:
            mc_x = mc_track_cand.getPosSeed().X()
            mc_y = mc_track_cand.getPosSeed().Y()
            mc_z = mc_track_cand.getPosSeed().Z()
            mc_mom_x = mc_track_cand.getMomSeed().X()
            mc_mom_y = mc_track_cand.getMomSeed().Y()
            mc_mom_z = mc_track_cand.getMomSeed().Z()
        else:
            mc_x = np.NaN
            mc_y = np.NaN
            mc_z = np.NaN
            mc_mom_x = np.NaN
            mc_mom_y = np.NaN
            mc_mom_z = np.NaN

        return dict(helix_z=helix.getZ0(), helix_x=helix.getPerigeeX(), helix_y=helix.getPerigeeY(),
                    helix_mom_z=helix.getMomentumZ(1.5), helix_mom_x=helix.getMomentumX(1.5), helix_mom_y=helix.getMomentumY(1.5),
                    mc_x=mc_x, mc_y=mc_y, mc_z=mc_z,
                    mc_mom_x=mc_mom_x, mc_mom_y=mc_mom_y, mc_mom_z=mc_mom_z)

    save_tree = refiners.SaveTreeRefiner()
