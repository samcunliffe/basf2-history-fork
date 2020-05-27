/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <optional>

namespace Belle2 {
  /**
   * Class implementing SVD3SampleELSTimeCalibration calibration algorithm
   */
  class SVD3SampleELSTimeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor set the prefix to SVDCoGTimeCalibrationCollector*/
    explicit SVD3SampleELSTimeCalibrationAlgorithm(const std::string& str);

    /** Destructor*/
    virtual ~SVD3SampleELSTimeCalibrationAlgorithm() {}

    /** Setter for m_allowedTimeShift*/
    void setAllowedTimeShift(float value) {m_allowedTimeShift = value;}

    /** Getter for m_allowedTimeShift*/
    float getAllowedTimeShift() {return m_allowedTimeShift;}

    /** Set the minimum entries required in the histograms*/
    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

    /** Get the minimum entries required in the histograms*/
    int getMinEntries() {return m_minEntries;}

  protected:

    /** Run algo on data*/
    virtual EResult calibrate() override;

    /** If the event T0 changes significantly return true. This is run inside the findPayloadBoundaries member function
    in the base class.*/
    virtual bool isBoundaryRequired(const Calibration::ExpRun& currentRun) override;

    /** setup of the boundary finding*/
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /*iteration = 0*/) override
    {
      m_previousRawCoG.reset();
    }


  private:

    std::string m_id; /**< Parameter given to set the UniqueID of the payload*/
    std::optional<float> m_previousRawCoG; /**< RawCog of the previous run*/
    float m_allowedTimeShift = 2.; /**< Allowed EventT0 shift*/
    float m_minEntries = 10000; /**< Set the minimun number of entries required in the histograms of layer 3*/
  };
} // namespace Belle2


