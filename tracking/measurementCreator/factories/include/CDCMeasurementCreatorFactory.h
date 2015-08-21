/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/measurementCreator/factories/MeasurementCreatorFactory.h>
#include <tracking/measurementCreator/creators/CoordinateMeasurementCreator.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /** Add all measurement creators related to CDC hits */
  class CDCMeasurementCreatorFactory : public
    MeasurementCreatorFactory<BaseMeasurementCreatorFromHit<RecoTrack::UsedCDCHit, Const::CDC>> {

  public:
    /** Initialize with a measurement factory */
    explicit CDCMeasurementCreatorFactory(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      MeasurementCreatorFactory<BaseMeasurementCreatorFromHit<RecoTrack::UsedCDCHit, Const::CDC>>(),
          m_measurementFactory(measurementFactory) {}

    /** Only a simple reco hit creator is implemented in the moment */
    BaseMeasurementCreatorFromHit<RecoTrack::UsedCDCHit, Const::CDC>* createMeasurementCreatorFromName(
      const std::string& creatorName) const
    {
      if (creatorName == "RecoHitCreator") {
        return new CoordinateMeasurementCreator<RecoTrack::UsedCDCHit, Const::CDC>(m_measurementFactory);
      }

      return nullptr;
    }
  private:
    /** A reference to the prefilled measurement factory */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;
  };
}
