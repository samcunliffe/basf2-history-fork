/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Marco Staric, Nils Braun                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <algorithm>

namespace Belle2 {
  class TrackFitResult;
  /** Class that bundles various TrackFitResults.
   *
   *  As the RecoTrack has a lot of information that is only necessary during the fit,
   *  this is a much stripped down version containing only indices of TrackFitResults
   *  (which is what remains of the various TrackReps, that Genfit might use during the fit).
   */
  class Track : public RelationsObject {
  public:

    /**
    * Pair to hold the particle hypothesis used for the fit as first entry and
    * the result of the track fit as second.
    */
    typedef std::pair< Const::ChargedStable, const TrackFitResult*> ChargedStableTrackFitResultPair;

    /** Constructor without arguments; needed for I/O.
     *
     *  The array with the indices for the TrackFitResults is initialized with -1,
     *  which is an invalid index.
     */
    Track()
    {
      std::fill(m_trackFitIndices, m_trackFitIndices + Const::chargedStableSet.size(), -1);
    }

    /** Access to TrackFitResults.
     *
     * This tries to return the TrackFitResult for the requested track hypothesis. If the requested track hypothesis is
     * not available, we return a nullptr.
     *
     * TODO: Do something special if we did not even try to fit!
     *
     *  @param chargedStable   Determines the particle for which you want to get the best available fit hypothesis.
     *  @return TrackFitResult for fit with particle hypothesis given by ParticleCode or a nullptr, if no result is
     *          available.
     *  @sa TrackFitResult
     */
    const TrackFitResult* getTrackFitResult(const Const::ChargedStable& chargedStable) const;

    /** Return the track fit for a fit hypothesis with the closest mass
     *
     * Multiple particle hypothesis are used for fitting during the reconstruction and stored with
     * this Track class. Not all hypothesis are available for all tracks because either a specific hypothesis
     * was not fitted or because the fit failed.
     * This method returns the track fit result of a successful fit with the hypothesis of a mass closest
     * to the requested particle type. If the requested type's hypothesis is available it will be returned
     * otherwise the next closest hypothesis in terms of the absolute mass difference will be returned.
     *
     * For example, if a pion is requested (mass 140 MeV) and only a muon fit (mass 106 MeV) and an
     * electron fit (mass 511 kEV) is available, the muon fit result will be returned.
     *
     * @param requestedType The particle type for which the fit result should be returned.
     * @return pair of the particle type used for the fit and a pointer to the TrackFitResult object.
     *
     */
    ChargedStableTrackFitResultPair getFitResultWithClosestMass(const Const::ChargedStable& requestedType) const;

    /** Access to all track fit results at the same time
     *
     * Returns a vector of pair of all track fit results which have been set and the respective particle
     * hypothesis they have been fitted with.
     */
    std::vector<ChargedStableTrackFitResultPair> getTrackFitResults() const;

    /** Set an index (for positive values) or unavailability-code (with negative values) for a specific mass hypothesis.
     *
     *  The TrackFitResult itself should be saved separately in the DataStore.
     *
     *  @param chargedStable  Determines the hypothesis for which you want to store the index or unavailability-code.
     */
    void setTrackFitResultIndex(const Const::ChargedStable& chargedStable, short index)
    {
      m_trackFitIndices[chargedStable.getIndex()] = index;
    }

    /** Returns the number of fitted hypothesis which are stored in this track. */
    unsigned int getNumberOfFittedHypotheses() const;

  private:
    /** Index list of the TrackFitResults associated with this Track. */
    short int m_trackFitIndices[Const::ChargedStable::c_SetSize];

    /**
     * Returns a vector of all fit hypothesis indices in m_trackFitIndices
     * which have been set (meaning are not -1)
     */
    std::vector < short int > getValidIndices() const;

    ClassDef(Track, 3); // Class that bundles various TrackFitResults.
  };
}
