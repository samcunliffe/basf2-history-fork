/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// tracking:
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/MCVXDPurityInfo.h>

// stl:
#include <vector>
#include <utility> // std::pair




namespace Belle2 {
  /** simple class storing infos relevant for a TC for analizing it. */
  class AnalizerTCInfo {
  public:


    /** allows classifying TCs */
    enum TCType {
      Unclassified, // a TC which was not classified yet
      Lost, // reference TCs which were not found by test TF
      Ghost, // did not reach m_PARAMqiThreshold
      Clone, // reached threshold but for the same reference TC a better partner was already found
      Contaminated, // more than m_PARAMqiThreshold in purity
      Clean, // test TC has 100% purity for the particle type but not all hits of reference TC were found
      Perfect, // test TC is identical with a reference TC (have exactly the same hits and no extra ones)
      Reference, // reference TC
      NTypes // number of tcTypes available
    };


    /** carries the global coordinates of the position of the seed hit (typically the innermost hit) */
    TVector3 posSeed;

    /** carries the momentum vector at the position of the seed hit (typically the innermost hit) */
    TVector3 momSeed;

    /** a link to the TC itself */
    const SpacePointTrackCand* tC;

    /** for reference TC: best test TC found, for test TC, compatible reference TC found */
    AnalizerTCInfo* assignedTC;

    /** stores the iD of the particle and knows the purity for it */
    MCVXDPurityInfo assignedID;

    /** classifies attached TC */
    AnalizerTCInfo::TCType tcType;


    /** constructor, makes sure that pointers are on NULL until set */
    AnalizerTCInfo() : tC(NULL), assignedTC(NULL), tcType(Unclassified) {}


    /** static function for correctly creating TrackCandidates */
    static AnalizerTCInfo createTC(bool isReference, MCVXDPurityInfo& iD, SpacePointTrackCand& aTC)
    {
      AnalizerTCInfo newTC;

      newTC.posSeed = aTC.getPosSeed();
      newTC.momSeed = aTC.getMomSeed();
      newTC.tC = &aTC;
      newTC.assignedID = iD;
      newTC.tcType = isReference ? Reference : Unclassified;
      return newTC;
    }


    /** find unpaired tcs and mark them with given type */
    static void markUnused(std::vector<AnalizerTCInfo>& tcs, AnalizerTCInfo::TCType newType)
    {
      for (AnalizerTCInfo& aTC : tcs) {
        if (aTC.assignedTC == NULL) {
          aTC.tcType = newType;
        }
      }
    }


    /** for given pair of TCs their compatibility will be checked and the testTC classified, the given threshold will be used to draw the line between Contaminated and Ghost */
    static AnalizerTCInfo::TCType classifyTC(AnalizerTCInfo& referenceTC, AnalizerTCInfo& testTC, double threshold)
    {
      std::pair<int, float> testPurity = testTC.assignedID.getPurity();
      std::pair<int, float> refPurity = referenceTC.assignedID.getPurity();
      // catch ill case
      if (testPurity.first != refPurity.first) { return Unclassified; }

      if (testPurity.second < threshold) { return Ghost; }

      if (testPurity.second < 1.f) { return Contaminated; }

      if (testTC.assignedID.getNClustersTotal() < referenceTC.assignedID.getNClustersTotal()) { return Clean; }

      return Perfect;
    }


    /** function for discarding the old TC.
     *
     * the old TC attached to this will be marked as Clone and discarded
     */
    void discardTC()
    {
      assignedTC->tcType = Clone;
      assignedTC = NULL;
    }


    /** function for assigning TCs to each other */
    void assignTCs(AnalizerTCInfo* otherTC)
    {
      assignedTC = otherTC;
      otherTC->assignedTC = this;
    }


    /** links otherTC to this one */
    void pairUp(AnalizerTCInfo* otherTC)
    {
      // case: this TC was not assigned before
      if (assignedTC == NULL) {
        assignTCs(otherTC);
        return;
      }

      // case: was already assigned, but old one was not as good as the new one
      if (assignedTC->tcType < otherTC->tcType) {
        discardTC();
        assignTCs(otherTC);
        return;
      }

      otherTC->assignedTC = this;
      otherTC->tcType = Clone;
    }

  };
}
