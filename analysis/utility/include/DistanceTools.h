/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thibaud Humair                                           *
 *                                                                        *
 * Inspired from DistanceCalculatorModule by Sourav Dey & Abi Soffer      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <vector>
#include <string>
#include "TVector3.h"
#include "TMatrixDSym.h"

namespace Belle2 {

  /**
   * This namespace contains a collection of function that are useful to compute distances
   * between tracks and vertices.
   *
   * All tracks are assumed to be straight in the current implementation.
   */
  namespace DistanceTools {

    /**
     * Returns the 3D vector between a vertex and a track's point of closest approach to that vertex
     */
    TVector3 trackToVtxVec(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos);

    /**
     * Returns the distance between a vertex and a track's point of closest approach to that vertex
     */
    double trackToVtxDist(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos);

    /**
     * Returns the covariance (error) matrix of the 3D vector between a vertex and a track's point
     *  of closest approach to that vertex
     */
    TMatrixDSym trackToVtxCovmat(TVector3 const& trackP,
                                 TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat);

    /**
     * Returns the estimated uncertainty between a vertex and a track's point of closest approach
     * to that vertex
     */
    double trackToVtxDistErr(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos,
                             TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat);


    /**
     * Returns the 3D vector between two vertices, ie vtxPos2 - vtxPos1
     */
    TVector3 vtxToVtxVec(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos);

    /**
     * Returns the distance between two vertices
     */
    double vtxToVtxDist(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos);

    /**
     *  Returns the covariance (error) matrix related to the vector linking two vertices
     */
    TMatrixDSym vtxToVtxCovMat(TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat);

    /**
     * Returns the estimated uncertainty on the distance between two vertices
     */
    double vtxToVtxDistErr(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos,
                           TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat);
  }
}
