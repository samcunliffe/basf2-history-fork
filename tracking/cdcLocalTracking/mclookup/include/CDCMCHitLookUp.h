/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCMCHITLOOKUP_H
#define CDCMCHITLOOKUP_H

#include <tracking/cdcLocalTracking/typedefs/InfoTypes.h>
#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Interface class to the Monte Carlo information for individual hits
    /** This class provides a stable interface for the underlying implementation for look ups
     *  into the Monte Carlo informations for the first stage of the algorithm.*/
    class CDCMCHitLookUp : public UsedTObject {

    public:
      /// Empty constructor
      CDCMCHitLookUp();

      /// Empty deconstructor
      ~CDCMCHitLookUp();

    public:
      /// Getter for the singletone instance
      static CDCMCHitLookUp& getInstance();

    public:
      /// Getter for the CDCSimHit which is related to the CDCHit contained in the given wire hit
      const Belle2::CDCSimHit* getSimHit(const CDCHit* ptrHit) const;

      /// Getter for the MCParticle which is related to the CDCHit contained in the given wire hit
      const Belle2::MCParticle* getMCParticle(const CDCHit* ptrHit) const;

    public:
      /// Indicates if the hit was reassigned to a different mc particle because it was caused by a secondary.
      bool isReassignedSecondary(const CDCHit* ptrHit) const;

      /// Getter for the closest simulated hit of a primary particle to the given hit - may return nullptr of no closest is found
      const CDCSimHit* getClosestPrimarySimHit(const CDCHit* ptrHit) const;

      /// Returns the track id for the hit
      ITrackType getMCTrackId(const CDCHit* ptrHit) const;

      /// Returns if this hit is considered background
      bool isBackground(const CDCHit* ptrHit) const;

      /// Returns the position if the wire hit in the track along the travel direction
      int getInTrackId(const CDCHit* ptrHit) const;

      /// Returns the id of the segment in the track.
      int getInTrackSegmentId(const CDCHit* ptrHit) const;

      /// Returns the number of superlayers the track traversed until this hit.
      int getNPassedSuperLayers(const CDCHit* ptrHit) const;

      /// Returns the true right left passage information
      RightLeftInfo getRLInfo(const CDCHit* ptrHit) const;

      /// ROOT Macro to make CDCMCHitLookUp a ROOT class.
      ClassDefInCDCLocalTracking(CDCMCHitLookUp, 1);

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCMCHITLOOKUP
