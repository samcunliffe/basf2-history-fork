/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMDQMMODULE_H
#define BEAMDQMMODULE_H

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <string>

#include "TH1.h"
#include "TH2.h"
#include <TProfile2D.h>
#include <TVector3.h>

namespace Belle2 {
  /**

  /**
   * Scans beam parameters and produces data file for particle gun array
   *
   * For selected PXD sensor (by sensorID) uses truehits to analyze beam parameters.
   * Outputs histograms (don't forget to add HistoManager module before this in the path).
   * Outputs a data file with settings for array of particle guns to simulate beam parameters
   * recorder by this module.
   * Outputs a data file for drawing of vector field of average track direction.
   *
   */
  class BeamDQMModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BeamDQMModule();

    /** Module initialization */
    virtual void initialize();

    /** Loops over PXDTrueHits fill histograms */
    virtual void event();

    /** Performs analysis of histograms and outputs data files */
    virtual void endRun();

    /** Histogram definition */
    virtual void defineHisto();

  private:
    int m_hits; /**< Number of hits (for final normalization) */

    std::string m_sensorID;  /**< VxdID of sensor to be scanned */
    std::string m_dataFileName;  /**< Name of output file for particle gun array */
    std::string m_fieldFileName;  /**< Name of file for data outputName of output file for vector field drawing */
    int m_nBinX; /**< Histograms: number of bins in x */
    int m_nBinY; /**< Histograms: number of bins in y  */
    double m_minX; /**< Histograms: minimum value for x */
    double m_minY; /**< Histograms: minimum value for y */
    double m_maxX; /**< Histograms: maximum value for x */
    double m_maxY; /**< Histograms: maximum value for y */

    TProfile2D* m_prof[3];  /**< Profile 2D histogram of average x/y/z mo[]mentum */

    TProfile2D* m_phi;  /**< Profile 2D histogram of average phi angle */
    TProfile2D* m_theta;  /**< Profile 2D histogram of average theta angle */
    TProfile2D* m_mom;  /**< Profile 2D histogram of average momentum magnitude */
    TProfile2D* m_rmsY;  /**< Profile 2D histogram of average beam slopes divergence in Y */
    TProfile2D* m_rmsZ;  /**< Profile 2D histogram of average beam slopes divergence in Z */

    TH2F* m_hitmap;  /**< 2D histogram for hitmap */

  };
}

#endif /* BEAMDQMMODULE_H */
