/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <genfit/TrackCand.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <string>
#include <array>
#include <vector>
#include <tuple>
#include <algorithm> // for find_if

namespace Belle2 {

  /**
   * Module for testing if the converting Modules do their job as intened.
   * Takes the original genfit::TrackCand (e.g. from MCTruthFinder) that has been converted to a SpacePointTrackCand
   * and the genfit::TrackCand that was generated by the vice versa (from SpacePointTrackCand to genfit::TrackCand)
   * and compares these two. If the comparison fails, this module tries to find a reason why it failed and prints a
   * summary at the end (in terminate).
   */
  class TCConvertersTestModule : public Module {

  public:

    TCConvertersTestModule(); /**< constructor */

    void initialize() override; /*<< initialize: check if all required StoreArrays are present, initialize counters, etc. */

    void event() override; /**< event: event-wise jobs */

    void terminate() override; /** terminate: print some summary information */

    /** typedef to imitate a genfit::TrackCandHit */
    typedef std::tuple<int, int, int, double> trackCandHit;

  protected:

    std::string m_PXDClusterName; /**< Container name of PXDCluster */

    std::string m_SVDClusterName; /**< Container name of SVDCluster */

    std::vector<std::string> m_genfitTCNames; /**< Names of genfit::TrackCand Store Arrays */

    std::string m_SPTCName; /**< Container name of SpacePointTrackCands */

    std::vector<std::string> m_SpacePointArrayNames; /**< Names of SpacePoint StoreArrays */

    // counter variables

    int m_genfitTCCtr; /**< counter for presented genfit::TrackCands */

    int m_convertedTCCtr; /**< counter for genfit::TrackCands which where obtained by converting from a SpacePointTrackCand */

    int m_SpacePointTCCtr; /**< counter for presented SpacePointTrackCands */

    /** counter for conversions where no SpacePointTrackCand was created (i.e. no relation to the original genfit::TrackCand is
     * found) */
    int m_failedNoSPTC;

    /** counter for conversions where no genfit::TrackCand was created from a SpacePointTrackCand (i.e. if there is a
     * SpacePointTrackCand in the StoreArray but no genfit::TrackCand related from it, this counter will be increased) */
    int m_failedNoGFTC;

    int m_failedOther; /**< Counter for failed conversions for which none of the other stated coudl be assigned */

    /** Counter for failed conversions for which the genfit::TrackCandidates do not contain the same TrackCandHits */
    int m_failedNotSameHits;

    int m_failedWrongOrder; /**< Counter for failed conversions due to wrong ordering of TrackCandHits */

    int m_failedWrongSortingParam; /**< Counter for failed conversions due to one or more differing sorting parameters */

    int m_failedNoRelationOrig; /**< Counter for failed Relation to original genfit::TrackCand */

    int m_failedNoRelationConv; /**< Counter for failed Relation to converted genfit::TrackCand */

    unsigned int m_lessHitsCtr; /**< Counter for cases where the original GFTC has less hits than the converted */

    unsigned int m_moreHitsCtr; /**< Counter for cases where the original GFTC has more hits than the converted */

    unsigned int m_differButOKCtr; /**< Counter for differing GFTCs, where the difference can be assigned to a refereeStatus */


    void initializeCounters(); /**< initialize all counter variables to zero, to avoid indeterministic behaviour */

    /** analyze why the conversion failed, and check if it can be explained by the referee Status of the SPTC.
     * @param origTC original genfit::TrackCand from which a SpacePointTrackCand was created by conversion
     * @param convTC converted genfit::TrackCand that was created by conversion from a SpacePointTrackCand
     * @param spTC SpacePointTrackCand that was created by conversion from origTC and that was then converted to convTC
     * @returns true if the mismatch can be explained and false if it cannot be explained
     */
    bool analyzeMisMatch(const genfit::TrackCand* origTC, const genfit::TrackCand* convTC, const Belle2::SpacePointTrackCand* spTC);

    /**
     * get all TrackCandHits from a genfit::TrackCand (no such method in genfit)
     * @returns vector of tuples, where get<0> is the detID, get<1> is the hitId, get<2> is the planeId
     * and get<3> is the sorting parameter
     */
    std::vector<trackCandHit> getTrackCandHits(const genfit::TrackCand* trackCand);

    /**
     * check if the same trackCandHits are contained in both vectors
     * @param origHits hits that are used to be checked against
     * @param convHits hits that are compared to origHit
     * @returns array of bools: [0] all hits from convHits are contained in origHits,
     * [1] the hits in convHits appear in the same order as in origHits,
     * [3] the sortingParams of all hits in convHits agree with the ones in origHits, [2] same as [3] only with planeIds
     * NOTE: if [0] is true it is still possible that the sortingParams do not match!!
     * (They are not compared in genfit::TrackCandHit either!)
     * NOTE: [1] can be true although [0] is false, meaning that not all hits have been found,
     * but those that were found were in the right order
     */
    std::array<bool, 4> checkHits(const std::vector<trackCandHit>& origHits, const std::vector<trackCandHit>& convHits);

    /**
     * check if there is a match if only certain entries (p1, p2 and p3) of a trackCandHit are compared
     * @param hits the vector of trackCandHits
     * @param hit the trackCandHit that is compared with the trackCandHits in hits
     * @returns iterator to the (first) position where a match is found
     * NOTE: this just wraps std::find_if with a lambda-function that compares only certain entries of a trackCandHit
     * CAUTION: p3 defaults to 0!
     */
    template<int p1, int p2, int p3 = 0>
    std::vector<trackCandHit>::const_iterator checkEntries(const std::vector<trackCandHit>& hits, const trackCandHit& hit)
    {
      return std::find_if(hits.begin(), hits.end(),
      [&hit](const trackCandHit & oHit) {
        return std::get<p1>(hit) == std::get<p1>(oHit) &&
               std::get<p2>(hit) == std::get<p2>(oHit) &&
               std::get<p3>(hit) == std::get<p3>(oHit);
      }
                         );
    }
  };
}