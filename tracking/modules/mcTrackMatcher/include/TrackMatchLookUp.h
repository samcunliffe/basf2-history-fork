/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKMATCHLOOKUP_H
#define TRACKMATCHLOOKUP_H


#include <mdst/dataobjects/MCParticle.h>

#include <genfit/TrackCand.h>

#include <TObject.h>

namespace Belle2 {

  /// Class to provide conventient methods to look up matching information between pattern recognition and Monte Carlo tracks.
  class TrackMatchLookUp : public TObject {

  public:
    /// Container struct to encapsual the matching categories of the Monte Carlo tracks.
    struct MCToPR {
    public:
      /// Matching categories for the Monte Carlo tracks.
      enum MatchInfo { INVALID, MATCHED, MERGED, MISSING };
    };

    /// Container struct to encapsual the matching categories of the pattern recognition tracks.
    struct PRToMC {
    public:
      /// Matching categories for the pattern recognition tracks.
      enum MatchInfo { INVALID, MATCHED, CLONE, BACKGROUND, GHOST };
    };

  public:
    /// Constructor taking the names StoreArrays containing the Monte Carlo track candidates and the pattern recognition track candidates respectivelly.
    TrackMatchLookUp(const std::string& mcTrackCandStoreArrayName,
                     const std::string& prTrackCandStoreArrayName = "");

    /// Empty deconstructor
    ~TrackMatchLookUp();

  public:
    /// Checks if the given track candidate is in the Monte Carlo track candidate StoreArray.
    bool isMCTrackCand(const genfit::TrackCand& trackCand);

    /// Checks if the given track candidate is in the pattern recognition StoreArray.
    bool isPRTrackCand(const genfit::TrackCand& trackCand);

    /// Helper function looking for a related Monte Carlo track to the given pattern recognition track in the purity relation. Also returns to found purity as output parameter.
    const genfit::TrackCand* getRelatedMCTrackCand(const genfit::TrackCand& prTrackCand, float& purity);

    /// Helper function looking for a related Monte Carlo track to the given pattern recognition track in the efficiency relation. Also returns to found efficiency as output parameter.
    const genfit::TrackCand* getRelatedPRTrackCand(const genfit::TrackCand& mcTrackCand, float& efficiency);

  private:
    /// Helper function to assume the correct matching category for the Monte Carlo tracks from the information efficiency relation.
    MCToPR::MatchInfo extractMCToPRMatchInfo(const genfit::TrackCand* prTrackCand, const float& efficiency);

    /// Helper function to assume the correct matching category for the pattern recognition tracks from the information purity relation.
    PRToMC::MatchInfo extractPRToMCMatchInfo(const genfit::TrackCand& prTrackCand, const genfit::TrackCand* mcTrackCand, const float& purity);

  public:
    /// Looks for a registered relation of the given track candidate to a Monte Carlo particle. Works for both pattern recognition and Monte Carlo track candidates. Returns nullptr if not found, If clones have a related MCParticle depends on the settings of the MCTrackMatcher module what filled the relations. (Default is that clones are related to the MCParticles.)
    const MCParticle* getRelatedMCParticle(const genfit::TrackCand& trackCand);

    /// Looks for a related Monte Carlo track for the given pattern recognition track candidate and return it if found. Return nullptr if not found.
    const genfit::TrackCand* getRelatedMCTrackCand(const genfit::TrackCand& prTrackCand);

    /// Looks for a related pattern recognition track for the given Monte Carlo track candidate and return it if found. Return nullptr if not found.
    const genfit::TrackCand* getRelatedPRTrackCand(const genfit::TrackCand& mcTrackCand);

  public:
    /// Getter for the purity that is stored in the purity relation from pattern recognition tracks.
    /** This can be negative if the patter recognition track is a clone. Returns NAN if no relation to a Monte Carlo track exists.
     */
    float getRelatedPurity(const genfit::TrackCand& prTrackCand);

    /// Getter for the efficiency that is stored in the efficiency relation from pattern recognition to Monte Carlo tracks.
    /** This can be negative if the Monte Carlo track is a merged. Returns NAN if no relation to a pattern recognition track extists.
     */
    float getRelatedEfficiency(const genfit::TrackCand& mcTrackCand);

  public:
    /// Looks up the matched Monte Carlo track for the given pattern recognition track candidate and return it if found.
    /// If there is no match or the pattern recognition track candidate is a clone return nullptr.
    const genfit::TrackCand* getMatchedMCTrackCand(const genfit::TrackCand& prTrackCand);

    /// Looks up the matched pattern recognition track candidate for the given Monte Carlo track and return it if found.
    /// If there is no match or the monte carlo track is merged into another pattern recognition track return nullptr.
    const genfit::TrackCand* getMatchedPRTrackCand(const genfit::TrackCand& mcTrackCand);

  public:
    /// Get the hit purity of the matched track candidate.
    /** The given track candidate can be both Monte Carlo track or pattern recognition track since the match is one to one relation.
     *  The returned purity is positive.
     *  If no exact match is assoziated with the track return NAN.
     */
    float getMatchedPurity(const genfit::TrackCand& trackCand);

    /// Get the hit efficiency of the matched track candidate.
    /** The given track candidate can be both Monte Carlo track or pattern recognition track since the match is one to one relation.
     *  The returned efficiency is positive.
     *  If no exact match is assoziated with the track return NAN.
     */
    float getMatchedEfficiency(const genfit::TrackCand& trackCand);

    /// Gets the matching category of Monte Carlo track. Is one of PRToMC::MATCHED, MERGED, MISSING, INVALID.
    MCToPR::MatchInfo getMCToPRMatchInfo(const genfit::TrackCand& mcTrackCand) {
      float efficiency = NAN;
      const genfit::TrackCand* prTrackCand = getRelatedPRTrackCand(mcTrackCand, efficiency);
      return extractMCToPRMatchInfo(prTrackCand, efficiency);
    }

    /// Gets the matching category of pattern recognition track. Is one of PRToMC::MATCHED, CLONE, BACKGROUND, GHOST.
    PRToMC::MatchInfo getPRToMCMatchInfo(const genfit::TrackCand& prTrackCand) {
      float purity = NAN;
      const genfit::TrackCand* mcTrackCand = getRelatedMCTrackCand(prTrackCand, purity);
      return extractPRToMCMatchInfo(prTrackCand, mcTrackCand, purity);
    }

    /// Checks, if the pattern recognition track was matched to a Monte Carlo track.
    bool isMatchedPRTrackCand(const genfit::TrackCand& prTrackCand)
    { return getPRToMCMatchInfo(prTrackCand) == PRToMC::MatchInfo::MATCHED; }

    /// Checks, if the pattern recognition track is a clone of an other pattern recognition track.
    bool isClonePRTrackCand(const genfit::TrackCand& prTrackCand)
    { return getPRToMCMatchInfo(prTrackCand) == PRToMC::MatchInfo::CLONE; }

    /// Checks, if the pattern recognition track is mostly made from background hits
    bool isBackgroundPRTrackCand(const genfit::TrackCand& prTrackCand)
    { return getPRToMCMatchInfo(prTrackCand) == PRToMC::MatchInfo::BACKGROUND; }

    /// Checks, if the pattern recognition track has contributions of different Monte Carlo tracks and/or background hits, such that a match is not possible
    bool isGhostPRTrackCand(const genfit::TrackCand& prTrackCand)
    { return getPRToMCMatchInfo(prTrackCand) == PRToMC::MatchInfo::GHOST; }



    /// Checks, if the Monte Carlo Track was matched to a pattern recognition track.
    bool isMatchedMCTrackCand(const genfit::TrackCand& mcTrackCand)
    { return getMCToPRMatchInfo(mcTrackCand) == MCToPR::MatchInfo::MATCHED; }

    /// Checks, if the Monte Carlo Track has been merged into pattern recognition track.
    bool isMergedMCTrackCand(const genfit::TrackCand& mcTrackCand)
    { return getMCToPRMatchInfo(mcTrackCand) == MCToPR::MatchInfo::MERGED; }

    /// Checks, if the Monte Carlo Track has now corresponding pattern recognition track.
    bool isMissingMCTrackCand(const genfit::TrackCand& mcTrackCand)
    { return getMCToPRMatchInfo(mcTrackCand) == MCToPR::MatchInfo::MISSING; }



  public:
    /// Getter for the name of the StoreArray of the Monte Carlo tracks.
    const std::string& getMCTracksStoreArrayName()
    { return m_mcTracksStoreArrayName; }

    /// Getter for the name of the StoreArray of the pattern recognition tracks.
    const std::string& getPRTracksStoreArrayName()
    { return m_prTracksStoreArrayName; }


  private:
    /// Name of the StoreArray of Monte Carlo tracks
    std::string m_mcTracksStoreArrayName;

    /// Name of the StoreArray of Pattern recognition tracks
    std::string m_prTracksStoreArrayName;

    /// ROOT Macro to make TrackMatchLookUp a ROOT class.
    ClassDef(TrackMatchLookUp, 1);

  }; //class
} // namespace Belle2
#endif // TRACKMATCHLOOKUP
