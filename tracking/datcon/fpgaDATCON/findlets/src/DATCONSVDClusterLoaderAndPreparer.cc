/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterLoaderAndPreparer.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONSVDClusterLoaderAndPreparer::DATCONSVDClusterLoaderAndPreparer() : Super()
{
//   this->addProcessingSignalListener(&m_trackFitter);
}

void DATCONSVDClusterLoaderAndPreparer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
//   m_trackFitter.exposeParameters(moduleParamList, prefix);

//   Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "DATCONSVDClusterStoreArrayName"),
                                m_param_SVDClusterStoreArrayName,
                                "StoreArray name of the DATCONSVDCluster Store Array.",
                                m_param_SVDClusterStoreArrayName);

}

void DATCONSVDClusterLoaderAndPreparer::initialize()
{
  Super::initialize();

  m_SVDClusters.isRequired(m_param_SVDClusterStoreArrayName);
}

void DATCONSVDClusterLoaderAndPreparer::apply(std::vector<std::pair<VxdID, std::pair<long, long>>>& uHits,
                                              std::vector<std::pair<VxdID, std::pair<long, long>>>& vHits)
{
  if (m_SVDClusters.getEntries() == 0) {
    return;
  }

  for (uint i = 0; i < 8; i++) nClusterPerLayer.at(i) = 0;

  /* First convert to absolute hits and save into a map */
  for (auto& cluster : m_SVDClusters) {
    const VxdID& sensorID = cluster.getSensorID();
    const uint& layerNumber = sensorID.getLayerNumber();
    const uint& ladderNumber = sensorID.getLadderNumber();
    const uint& sensorNumber = sensorID.getSensorNumber();
    const long localPosition = convertToInt(cluster.getPosition(), 4); // convert the cluster position from cm to µm


    if (cluster.isUCluster()) {
      nClusterPerLayer.at(layerNumber - 3)++;
      const float rotangle = initialAngle[layerNumber - 3] + (ladderNumber - 1) * angleStep[layerNumber - 3];
      const float cosRotAngle = cos(rotangle);
      const float sinRotAngle = sin(rotangle);
      const int    sensorRadius = svdRadii[layerNumber - 3];
      const int    ytmp = localPosition + rPhiShiftsOfLayers[layerNumber - 3];

      // perform 2D rotation
      const long x = round(sensorRadius * cosRotAngle - ytmp * sinRotAngle); // x is in µm
      const long y = round(sensorRadius * sinRotAngle + ytmp * cosRotAngle); // y is in µm
      const long radiusSquared = x * x + y * y; // therefore radius is in µm as well, like all other length values
//       const float radius = sensorRadius;
      // x or y divided by r^2 can create values very close to 0. To cope for this and still
      // have non-zero numbers after conversion and rounding, the conversion to int is made with 10^7
      std::pair<long, long> pos2D = std::make_pair(convertToInt(2. * (float)x / (float)radiusSquared, 10),
                                                   convertToInt(2. * (float)y / (float)radiusSquared, 10));
      uHits.emplace_back(std::make_pair(sensorID, pos2D));
    } else {
      nClusterPerLayer.at(4 + layerNumber - 3)++;
      const int radius = svdRadii[layerNumber - 3];
      int z = 0;
      if (layerNumber == 3) {
        z = localPosition + zShiftL3[sensorNumber - 1];
      } else {
        switch (layerNumber) {
          case 4:
            if (sensorNumber == 1) {
              z = localPosition * cosSlantedAngles[layerNumber - 4] + zShiftL4[sensorNumber - 1];
            } else {
              z = localPosition + zShiftL4[sensorNumber - 1];
            }
            break;
          case 5:
            if (sensorNumber == 1) {
              z = localPosition * cosSlantedAngles[layerNumber - 4] + zShiftL5[sensorNumber - 1];
            } else {
              z = localPosition + zShiftL5[sensorNumber - 1];
            }
            break;
          case 6:
            if (sensorNumber == 1) {
              z = localPosition * cosSlantedAngles[layerNumber - 4] + zShiftL6[sensorNumber - 1];
            } else {
              z = localPosition + zShiftL6[sensorNumber - 1];
            }
            break;
        }
      }
      vHits.emplace_back(std::make_pair(sensorID, std::make_pair(z, radius)));
    }
  }

  if (std::any_of(nClusterPerLayer.begin(), nClusterPerLayer.end(), [](int i) {return i > 50;})) {
    B2WARNING("High occupancy in SVD, aborting DATCON...");
    uHits.clear();
    vHits.clear();
  }

}
