/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef pxdClusterShapeCalibrationModule_H
#define pxdClusterShapeCalibrationModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/ClusterCache.h>
#include <pxd/reconstruction/ClusterProjection.h>
#include <pxd/reconstruction/NoiseMap.h>
#include <string>
#include <memory>

#include <calibration/CalibrationCollectorModule.h>

namespace Belle2 {

  /** The PXDClusterShapeCalibration module.
   *
   * This module is responsible to clibrate cluster position and error estimation
   * base on information from cluster only (shape, signal, seed), or with tracking
   * information (angle of track, in-pixel position).
   *
   * Output of module is source for calculation of matrix of corrections
   * for positiom and error estimation
   * colelcted on TTree root file
   * Need to add details of module after his finalyzing
   *
   * @see PXDClusterShape
   *
   */
  class pxdClusterShapeCalibrationModule : public CalibrationCollectorModule {


  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    pxdClusterShapeCalibrationModule();

    /** Initialize the module */
    virtual void prepare();
    /** Extract parameters for pxd cluster shape calibration of cluster reconstruction using true position */
    virtual void collect();

  private:
    /** Region close edge where remove cluster shape corrections */
    int m_EdgeClose = 3;

    /** Current event id */
    int m_evt = -1;
    /** Current run id */
    int m_run = -1;
    /** Current experiment id */
    int m_exp = -1;
    /** Current process id */
    int m_procId = -1;

    /** Spread of gaussian (mean=42) filling test histogram (range=<0,100>) - probability of algo iterations depend on it */
//    int m_spread;

    /** Name of the collection to use for the PXDClusters */
    std::string m_storeClustersName;
    /** Name of the collection to use for the PXDTrueHits */
    std::string m_storeTrueHitsName;
    /** Name of the collection to use for the MCParticles */
    std::string m_storeMCParticlesName;
    /** Name of the relation between PXDClusters and MCParticles */
    std::string m_relClusterMCParticleName;
    /** Name of the relation between PXDClusters and PXDTrueHits */
    std::string m_relClusterTrueHitName;

    /** Name of variable for layer ID (1, 2) */
    short m_layer = 0;
    /** Name of variable for sensor ID (1, 2) */
    short m_sensor = 0;
    /** Name of variable for segment ID (1, 2) */
    short m_segment = 0;
    /** Name of variable for pixel kind ID (1..8) */
    short m_pixelKind = 0;
    /** Name of variable for mark, if cluster is touch edge or masked pixel  */
    short m_closeEdge = 0;
    /** Name of variable for ID of Cluster Shape */
    short m_shape = 0;
    /** Name of variable for theta angle of track direction via sensor */
    float m_theta = 0.0;
    /** Name of variable for phi angle of track direction via sensor */
    float m_phi = 0.0;
    /** Name of variable for collected charge - signal */
    double m_signal = 0.0;
    /** Name of variable for seed - maximum of signal */
    double m_seed = 0.0;
    /** Name of variable for in-pixel position U from track or simulation true hit */
    double m_InPixUTrue = 0.0;
    /** Name of variable for in-pixel position V from track or simulation true hit */
    double m_InPixVTrue = 0.0;
    /** Name of variable for in-pixel reco position U */
    double m_InPixUReco = 0.0;
    /** Name of variable for in-pixel reco position V */
    double m_InPixVReco = 0.0;
    /** Name of variable for defference in U between true and reco position */
    double m_CorrU = 0.0;
    /** Name of variable for defference in V between true and reco position */
    double m_CorrV = 0.0;

  };
}

#endif /* pxdClusterShapeCalibrationModule_H */

