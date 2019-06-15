"""
Script demonstrating how to upload cuts to the database and also showing the
current HLT trigger menu.
"""

from softwaretrigger.db_access import upload_cut_to_db, upload_trigger_menu_to_db

# Definition of all filter cuts (by Chris Hearty)
FILTER_CUTS = [
    dict(cut_string="nTrkLoose >= 3 and nTrkTight >= 1 and ee2leg == 0",
         base_identifier="filter",
         cut_identifier="ge3_loose_tracks_inc_1_tight_not_ee2leg",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="nElow >= 3 and nEmedium >= 1 and nEhigh == 0",
         base_identifier="filter",
         cut_identifier="0.3ltEstar_max_clustlt2_GeV_plus_2_others_gt_0.2_GeV",
         prescale_factor=1,
         reject_cut=False),
    dict(
        cut_string="nTrkLoose == 2 and nTrkTight >= 1 and netChargeLoose == 0 and maximumPCMS < 0.8 and eexx == 0",
        base_identifier="filter",
        cut_identifier="2_loose_tracks_inc_1_tight_q==0_pstarmaxlt0.8_GeVc_not_eexx",
        prescale_factor=1,
        reject_cut=False),
    dict(
        cut_string="nTrkLoose == 2 and maximumPCMS > 0.8 and maximumPCMS < 4.5 and " +
                   "ee2leg == 0 and ee1leg1trk == 0 and eexx == 0",
        base_identifier="filter",
        cut_identifier="2_loose_tracks_0.8ltpstarmaxlt4.5_GeVc_not_ee2leg_ee1leg1trk_eexx",
        prescale_factor=1,
        reject_cut=False),
    dict(
        cut_string="nTrkLoose == 2 and maximumPCMS > 4.5 and ee2leg == 0 and ee1leg1trk == 0 and " +
                   "ee1leg1e == 0 and eeBrem == 0 and muonPairV == 0",
        base_identifier="filter",
        cut_identifier="2_loose_tracks_pstarmaxgt4.5_GeVc_not_ee2leg_ee1leg1trk_ee1leg1e_eeBrem_muonPairV",
        prescale_factor=1,
        reject_cut=False),
    dict(
        cut_string="n2GeVNeutBarrel >= 1 and gg2clst == 0 and ee1leg1clst == 0 and ee1leg1trk == 0 and eeBrem == 0",
        base_identifier="filter",
        cut_identifier="ge1_Estargt2_GeV_neutral_clst_32130_not_gg2clst_ee1leg1clst_ee1leg1trk_eeBrem",
        prescale_factor=1,
        reject_cut=False),
    dict(cut_string="n2GeVNeutEndcap >= 1 and gg2clst == 0 and ee2clst == 0 and ee1leg == 0 and eeBrem == 0",
         base_identifier="filter",
         cut_identifier="ge1_Estargt2_GeV_neutral_clst_2232_or_130145_not_gg2clst_ee2clst_ee1leg_eeBrem",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="nEsinglePhotonBarrel == 1 and nVetoClust <= 1",
         base_identifier="filter",
         cut_identifier="1_photon_Estargt1_GeV_clust_in_45115_and_no_other_clust_Estargt0.3_GeV",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="nEsingleElectronBarrel == 1 and nVetoClust <= 1",
         base_identifier="filter",
         cut_identifier="1_electron_Estargt1_GeV_clust_in_45115_and_no_other_clust_Estargt0.3_GeV",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="nEsinglePhotonEndcap == 1 and nVetoClust <= 1",
         base_identifier="filter",
         cut_identifier="1_photon_Estargt1_GeV_clust_not_low_not_45115_no_other_clust_Estargt0.3_GeV",
         prescale_factor=50,
         reject_cut=False),
    dict(cut_string="nEsingleClust == 1 and nEmedium == 1",
         base_identifier="filter",
         cut_identifier="1_Estargt1_GeV_cluster_no_other_cluster_Estargt0.3_GeV",
         prescale_factor=200,
         reject_cut=False),
    dict(cut_string="ee2leg",
         base_identifier="filter",
         cut_identifier="ee2leg",
         prescale_factor=1000,
         reject_cut=False),
    dict(cut_string="ee1leg1trk",
         base_identifier="filter",
         cut_identifier="ee1leg1trk",
         prescale_factor=1000,
         reject_cut=False),
    dict(cut_string="ee1leg1clst",
         base_identifier="filter",
         cut_identifier="ee1leg1clst",
         prescale_factor=1000,
         reject_cut=False),
    dict(cut_string="ee1leg",
         base_identifier="filter",
         cut_identifier="ee1leg",
         prescale_factor=1000,
         reject_cut=False),
    dict(cut_string="ee1leg1e",
         base_identifier="filter",
         cut_identifier="ee1leg1e",
         prescale_factor=1000,
         reject_cut=False),
    dict(cut_string="eexxSelect",
         base_identifier="filter",
         cut_identifier="eexxSelect",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="false",
         base_identifier="filter",
         cut_identifier="dummy",
         prescale_factor=9999,
         reject_cut=False),
    dict(cut_string="selectee1leg1trk",
         base_identifier="filter",
         cut_identifier="selee1leg1trk",
         prescale_factor=9999,
         reject_cut=False),
    dict(cut_string="selectee1leg1clst",
         base_identifier="filter",
         cut_identifier="selee1leg1clst",
         prescale_factor=9999,
         reject_cut=False),
    dict(cut_string="eeFlat0",
         base_identifier="filter",
         cut_identifier="ee_flat_0_19",
         prescale_factor=15,
         reject_cut=False),
    dict(cut_string="eeFlat1",
         base_identifier="filter",
         cut_identifier="ee_flat_19_22",
         prescale_factor=40,
         reject_cut=False),
    dict(cut_string="eeFlat2",
         base_identifier="filter",
         cut_identifier="ee_flat_22_25",
         prescale_factor=27,
         reject_cut=False),
    dict(cut_string="eeFlat3",
         base_identifier="filter",
         cut_identifier="ee_flat_25_30",
         prescale_factor=15,
         reject_cut=False),
    dict(cut_string="eeFlat4",
         base_identifier="filter",
         cut_identifier="ee_flat_30_35",
         prescale_factor=8,
         reject_cut=False),
    dict(cut_string="eeFlat5",
         base_identifier="filter",
         cut_identifier="ee_flat_35_45",
         prescale_factor=5,
         reject_cut=False),
    dict(cut_string="eeFlat6",
         base_identifier="filter",
         cut_identifier="ee_flat_45_60",
         prescale_factor=3,
         reject_cut=False),
    dict(cut_string="eeFlat7",
         base_identifier="filter",
         cut_identifier="ee_flat_60_90",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="eeFlat8",
         base_identifier="filter",
         cut_identifier="ee_flat_90_180",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="selectee",
         base_identifier="filter",
         cut_identifier="selectee",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="ee2clst",
         base_identifier="filter",
         cut_identifier="ee2clst",
         prescale_factor=1000,
         reject_cut=False),
    dict(cut_string="nEhigh >= 1",
         base_identifier="filter",
         cut_identifier="Estargt2_GeV_cluster",
         prescale_factor=100,
         reject_cut=False),
    dict(cut_string="gg2clst",
         base_identifier="filter",
         cut_identifier="gg2clst",
         prescale_factor=100,
         reject_cut=False),
    dict(cut_string="n2GeVChrg >= 1 and gg2clst == 0 and ee2clst == 0 and ee1leg == 0",
         base_identifier="filter",
         cut_identifier="ge1_Estargt2_GeV_chrg_clst_22145_not_gg2clst_ee2clst_ee1leg",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="ggBarrelLoose",
         base_identifier="filter",
         cut_identifier="ggBarrelLoose",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="ggEndcapLoose",
         base_identifier="filter",
         cut_identifier="ggEndcapLoose",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="n2GeVPhotonBarrel >= 1",
         base_identifier="filter",
         cut_identifier="n2GeVPhotonBarrelge1",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="n2GeVPhotonEndcap >= 1",
         base_identifier="filter",
         cut_identifier="n2GeVPhotonEndcapge1",
         prescale_factor=50,
         reject_cut=False),
    dict(cut_string="radBhabha",
         base_identifier="filter",
         cut_identifier="radiative_Bhabha",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="eexx",
         base_identifier="filter",
         cut_identifier="eexx",
         prescale_factor=9999,
         reject_cut=False),
    dict(cut_string="eeBrem",
         base_identifier="filter",
         cut_identifier="eeBrem",
         prescale_factor=9999,
         reject_cut=False),
    dict(cut_string="muonPairV",
         base_identifier="filter",
         cut_identifier="muonPairV",
         prescale_factor=9999,
         reject_cut=False),
    dict(cut_string="selectmumu",
         base_identifier="filter",
         cut_identifier="selectmumu",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="singleMuon",
         base_identifier="filter",
         cut_identifier="single_muon",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="nTrkTight >= 1",
         base_identifier="filter",
         cut_identifier="ge1_tight_track",
         prescale_factor=1000,
         reject_cut=False),
    dict(cut_string="nTrkLoose == 2 and maximumPCMS < 0.8 and eexx == 0",
         base_identifier="filter",
         cut_identifier="2_loose_tracks_pstarmaxlt0.8_GeVc",
         prescale_factor=100,
         reject_cut=False),
    dict(cut_string="nTrkLoose == 2 and maximumPCMS > 0.8 and maximumPCMS < 4.5",
         base_identifier="filter",
         cut_identifier="2_loose_tracks_0.8ltpstarmaxlt4.5_GeVc",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="nTrkLoose == 2 and maximumPCMS > 4.5",
         base_identifier="filter",
         cut_identifier="2_loose_tracks_pstarmaxgt4.5_GeVc",
         prescale_factor=500,
         reject_cut=False),
    dict(cut_string="true",
         base_identifier="filter",
         cut_identifier="L1_trigger",
         prescale_factor=10000,
         reject_cut=False),
    dict(cut_string="muonPairECL",
         base_identifier="filter",
         cut_identifier="ECLMuonPair",
         prescale_factor=10,
         reject_cut=False),
    dict(cut_string="l1_trigger_random",
         base_identifier="filter",
         cut_identifier="passthrough_l1_trigger_random",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="l1_trigger_delayed_bhabha",
         base_identifier="filter",
         cut_identifier="passthrough_l1_trigger_delayed_bhabha",
         prescale_factor=1,
         reject_cut=False),
]

# Definition of all skim cuts (copy from HLT before)
SKIM_CUTS = [
    dict(cut_string="[[nTracksLE >= 3] and [Bhabha2Trk == 0]]",
         base_identifier="skim",
         cut_identifier="accept_hadron",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[[5 >= nTracksLE] and [nTracksLE >= 2]] and [[P1CMSBhabhaLE < 5] and [P12CMSBhabhaLE < 9]]]",
         base_identifier="skim",
         cut_identifier="accept_2_tracks",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[[2 >= nTracksLE] and [nTracksLE >= 1]] and " +
                    "[[ENeutralLE > 1] and [[AngleGTLE > 0.785] and [nEidLE == 0]]]]",
         base_identifier="skim",
         cut_identifier="accept_1_track1_cluster",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[nTracksLE >= 2] and [[nEidLE == 0] and [[P1OEbeamCMSBhabhaLE > 0.35] and " +
                    "[[P2OEbeamCMSBhabhaLE > 0.2] and [[EtotLE < 7] and [[EC2CMSLE < 1] and " +
                    "[maxAngleTTLE > 0.785]]]]]]]",
         base_identifier="skim",
         cut_identifier="accept_mumu_2trk",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[nTracksLE == 1] and [[nEidLE == 0] and [[P1OEbeamCMSBhabhaLE > 0.1] and " +
                    "[[EC1CMSLE < 1] and [EtotLE < 7]]]]]",
         base_identifier="skim",
         cut_identifier="accept_mumu_1trk",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[nTracksLE >= 2] and [[P1CMSBhabhaLE < 5] and [[EtotLE < 9] and [VisibleEnergyLE < 9]]]]",
         base_identifier="skim",
         cut_identifier="accept_tau_tau",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[G1CMSBhabhaLE > 2] and [Bhabha2Trk == 0]]",
         base_identifier="skim",
         cut_identifier="accept_single_photon_2GeV_barrel",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[G1CMSBhabhaLE > 2] and [[Bhabha2Trk == 0] and [GG == 0]]]",
         base_identifier="skim",
         cut_identifier="accept_single_photon_2GeV_endcap",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[G1CMSBhabhaLE > 1] and [[Bhabha2Trk == 0] and [GG == 0]]]",
         base_identifier="skim",
         cut_identifier="accept_single_photon_1GeV",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[nB2BCCPhiHighLE >= 1] and [G1CMSBhabhaLE < 2]]",
         base_identifier="skim",
         cut_identifier="accept_b2bclusterhigh_phi",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[nB2BCCPhiLowLE >= 1] and [G1CMSBhabhaLE < 2]]",
         base_identifier="skim",
         cut_identifier="accept_b2bclusterlow_phi",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[nB2BCC3DLE >= 1] and [G1CMSBhabhaLE < 2]]",
         base_identifier="skim",
         cut_identifier="accept_b2bcluster_3D",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[[nTracksLE <= 1] and [[nEidLE == 0] and [[EC12CMSLE > 4] and [EC1CMSLE > 2]]]]",
         base_identifier="skim",
         cut_identifier="accept_gamma_gamma",
         prescale_factor=1,
         reject_cut=False),
    dict(cut_string="[Bhabha2Trk == 1]",
         base_identifier="skim",
         cut_identifier="accept_bhabha",
         prescale_factor=1,
         reject_cut=False),
]


def upload_cuts(cuts, accept_mode=True):
    """
    Helper function to upload all cuts and the trigger menu to the local database.
    :param cuts: A list of cuts (cut_string, base_identifier, cut_identifier, prescale_factor, reject_cut)
    :param accept_mode: Whether the trigger menu should be in accept mode (default) or not
    """
    for cut in cuts:
        upload_cut_to_db(**cut)

    upload_trigger_menu_to_db(cuts[0]["base_identifier"], [cut["cut_identifier"] for cut in cuts],
                              accept_mode=accept_mode)


if __name__ == '__main__':
    # Accept Mode = False means, the default is to reject the event (not to accept it)
    upload_cuts(FILTER_CUTS, accept_mode=False)
    # For skim cuts we do not care what the default is (there will be no selection anyways)
    upload_cuts(SKIM_CUTS)
