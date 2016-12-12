/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {


  /** This module compares tracks generated by some pattern recognition algorithm for PXD, SVD and CDC to ideal Monte Carlo tracks
   *  and performs a matching from the former to the underlying MCParticles.
   *
   *  To achieve this it evaluates and saves the purities and efficiencies from the pattern recognition track to Monte-Carlo tracks,
   *  which can than also be used by a subsequent evaluation modules.
   *
   *  In order to match the tracks the module takes two StoreArrays of GFTrackCands, which should be compared.
   *
   *  One of them contains RecoTracks composed by the patter recognition algorithm to be assessed. They are refered to as PRTracks.
   *
   *  The second StoreArray holds the reference tracks, which should ideally be reconstructed. These are refered to as MCTracks and
   *  should generally be composed by the MCTrackingModule.
   *  (Design note : We use the tracks composed by the MCTrackFinder as reference, because the mere definition of
   *   what a trackable particle and what the best achievable track is, lies within the implementation of the MCTrackFinder.
   *   If we did not use the tracks from the MCTrackFinder as input, it would mean a double implementation of great parts of that logic.)
   *
   *  If the pattern recognition only covers a part of the tracking detectors, the matching can be constrained
   *  to specific subdetectors by switching of the appropriate usePXDHits, useSVDHits or useCDCHits parameters.
   *
   *  As a result the module
   *   -# creates a RelationArray from the PRTracks to the MCTracks, which will be called the purity relation,
   *   -# creates a RelationArray from the MCTracks to the PRTracks, which will be called the efficiency relation,
   *   -# assigns the McTrackId property of the PRTracks and
   *   -# creates a RelationArray from the PRTracks to the MCParticles.
   *   .
   *
   *  The RelationArray for purity and efficiency generally store only the single highest purity and
   *  the single highest efficiency for a given PRTrack, MCTrack respectivelly. However these values are stored with
   *  a minus sign if the PRTrack is a clone, or the MCTrack is merged into another PRTrack. The McTrackId is either set to the
   *  index of the MCParticle or to some negative value indicating the severity of the mismatch. (Classification details below).
   *
   *  Moreover, only PRTracks that exceed the minimal purity requirement and a minimal efficiency requirement
   *  will have their purity/efficiency stored and will take part in the matching.
   *  The minimal purity can be choosen by the minimalPurity parameter (default 2.0/3.0).
   *  The minimal efficiency can be choosen by the minimalEfficiency parameter (default 0.05).
   *
   *  Last but not least a RelationArray from matched PRTracks to MCParticles is build and
   *  the McTrackId property of the PRTrack is set to the StoreArray index of the MCParticle
   *  (similar as the MCTrackFinder does it for the MCTracks).
   *  By default clone tracks are also assigned to their MCParticle.
   *  This behaviour can be switched off by the relateClonesToMCParticles.
   *
   *  In the following a more detailed explaination is given for the matching and the classification of PRTracks and MCTracks.
   *
   *  The PRTracks can be classified into four categories, which are described in the following
   *  - MATCHED
   *      - The highest efficiency PRTrack of the highest purity MCTrack to this PRTrack is the same as this PRTrack.
   *      - This means the PRTrack contains a high contribution of only one MCTrack and
   *        is also the best of all PRTracks describing this MCTrack.
   *      - The McTrackId property of matched PRTrack is set to the MCTrackId property of the MCTrack,
   *        which is usually the index of the MCParticle in its corresponding StoreArray.
   *      - Also the relation from PRTrack to MCParticle is added.
   *      - The purity relation is setup from the PRTrack to the MCTrack with the (positive) purity as weight.
   *
   *  - CLONE
   *      - The highest purity MCTrack as a different highest efficiency PRTrack than this track.
   *      - This means the PRTrack contains high contributions of only one MCTrack but a different other PRTrack contains an even higher contribution to this MCTrack.
   *      - Only if the relateClonesToMCParticles parameter is active, the McTrackId property of cloned PRTracks is set to the MCTrackId property of the MCTrack.
   *        Else it will be set to -9.
   *      - Also the relation from PRTrack to MCParticle is added, only if the relateClonesToMCParticles parameter is active.
   *      - The purity relation is always setup from the PRTrack to the MCTrack with the _negative_ purity as weight,
   *        to be able to distinguish them from the matched tracks.
   *
   *  - BACKGROUND
   *      - The PRTrack contains mostly hits, which are not part of any MCTrack.
   *      - This normally means, that this PRTracks is made of beam background hits.
   *      - If one constrains the MCTracks in the MCTrackFinder to some specific particles, say the tag side,
   *        also all signal side tracks end up in this category (in case of reasonable tracking performance).
   *        In this case the background rate is somewhat less meaningful.
   *      - For background tracks the McTrackId of the PRTrack is set to -99.
   *      - No relation from the PRTrack to the MCParticle is inserted.
   *      - PRTracks classified as background are not entered in the purity RelationArray.
   *
   *  - GHOST
   *      - The highest purity MCTrack to this PRTrack has a purity lower than the minimal purity given in the parameter minimalPurity or
   *      - has an efficiency lower than the efficiency given in the parameter minimalEfficiency.
   *      - This means that the PRTrack does not contain a significat number of a specific MCTrack nor can it considered only made of background.
   *      - For ghost tracks the McTrackId of the GFTrackCand is set to -999.
   *      - No relation from the PRTrack to the MCParticle is inserted.
   *      - PRTracks classified as ghost are not entered in the purity RelationArray.
   *  .
   *
   *  MCTracks are classified into three categories:
   *  - MATCHED
   *      - The highest purity MCTrack of the highest efficiency PRTrack of this MCTrack is the same as this MCTrack.
   *      - This means the MCTrack is well described by a PRTrack and this PRTrack has only a significant contribution from this MCTrack.
   *      - The efficiency relation is setup from the MCTrack to the PRTrack with the (positive) efficiency as weight.
   *
   *  - MERGED
   *      - The highest purity MCTrack of the highest efficiency PRTrack of this MCTrack is not the same as this MCTrack.
   *      - This means this MCTrack is mostly contained in a PRTrack, which in turn however better describes a MCTrack different form this.
   *      - The efficiency relation is setup from the MCTrack to the PRTrack with the _negative_ efficiency as weight,
   *        to be able to distinguish them from the matched tracks.
   *
   *  - MISSING
   *      - There is no highest efficiency PRTrack to this MCTrack, which also fullfills the minimal purity requirement.
   *      - For this category no efficiency relation is inserted.
   *  .
   *
   */

  class MCRecoTracksMatcherModule : public Module {

  public:

    MCRecoTracksMatcherModule();

    virtual void initialize();

    virtual void event();

  private:
    //Parameters
    std::string
    m_param_prRecoTracksStoreArrayName;                        /**< RecoTracks store array name from the patter recognition*/
    std::string m_param_mcRecoTracksStoreArrayName;                        /**< RecoTracks store array name from the mc recognition*/

    bool m_param_usePXDHits;                                          /**< Boolean to select if PXDHits should be used*/
    bool m_param_useSVDHits;                                          /**< Boolean to select if SVDHits should be used*/
    bool m_param_useCDCHits;                                          /**< Boolean to select if CDCHits should be used*/
    bool m_param_useOnlyAxialCDCHits;                                 /**< Boolean to select if only axial CDCHits should be used*/
    bool m_param_abandonDiscardedCDCHits;               /**< Boolean to select if discarded (in MCTrack) CDC hits are taken into account*/

    double m_param_minimalPurity;                                     /**< Minimal purity of a PRTrack to be considered matchable to a MCTrack.
                   This number encodes how many correct hits are minimally need to compensate for a false hits.
                   The default 0.66 suggests that for each background hit can be compensated by two correct hits.*/

    double m_param_minimalEfficiency;                                  /**< Minimal efficiency of a MCTrack to be considered matchable to a PRTrack.
                    This number encodes which fraction of the true hits must at least be in the reconstructed track.
                    The default 0.05 suggests that at least 5% of the true hits should have been picked up.*/


    /*int m_param_minimalExcessNdf;                                     < Minimum number of degrees of freedom in the PRtrack. 2D hits count as 2.
                   The number can be given to insure that a minimum of correct degress of freedom
                   is in the track for the fit as well as the rest of the correct hits is able to
                   compensate for false hits. Essentially this number is subtracted from the number
                   of signal hits before calculating the purity. */


    bool m_param_relateClonesToMCParticles;                                    /**< Boolean to indicate if a matching to MCParticles should be performed for ghosts.*/

    //Other variables

    /// Descriptive type defintion for a number of degrees of freedom.
    typedef int NDF;
    std::map<int, NDF> m_ndf_by_detId = {{Const::PXD, 2}, {Const::SVD, 2}, {Const::CDC, 1}}; /**< Map storing the standard number degrees of freedom for a single hit by detector */

    bool m_mcParticlesPresent =
      false; /**< This flag is set to false if there are no MC Particles in the data store (probably data run?) and we can not create MC Reco tracks. */

  }; // end class
} // end namespace Belle2

