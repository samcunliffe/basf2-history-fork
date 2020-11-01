/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SVDOldDefaultPosition.h>
#include <svd/reconstruction/SVDPositionReconstruction.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDOldDefaultPosition::computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError)
    {

      if (rawCluster.getSize() < 3)
        applyCoGPosition(rawCluster, position, positionError);
      else
        applyAHTPosition(rawCluster, position, positionError);
    }

    double SVDOldDefaultPosition::getClusterPosition(const Belle2::SVD::RawCluster& rawCluster)
    {

      //as weighted average of the strip position with strip max sample

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      SVDPositionReconstruction positionReco(strips, rawCluster.getSensorID(), rawCluster.isUSide());

      double position = -99;

      if (strips.size() < 3)
        position = positionReco.getCoGPosition();
      else
        position = positionReco.getAHTPosition();

      return position;
    }

    double SVDOldDefaultPosition::getClusterPositionError(const Belle2::SVD::RawCluster& rawCluster)
    {

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      SVDPositionReconstruction positionReco(strips, rawCluster.getSensorID(), rawCluster.isUSide());

      double positionError = -99;

      if (strips.size() < 3)
        positionError = positionReco.getCoGPositionError();
      else
        positionError = positionReco.getAHTPositionError();

      //apply cluster position error scale factors
      positionError = m_ClusterCal.getCorrectedClusterPositionError(rawCluster.getSensorID(), rawCluster.isUSide(), strips.size(),
                      positionError);


      return positionError;
    }


  }  //SVD namespace
} //Belle2 namespace
