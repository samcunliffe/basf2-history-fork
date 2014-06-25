/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCObservations2D.h"

using namespace std;
using namespace Eigen;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCObservations2D)


CDCObservations2D::CDCObservations2D()
{
}



CDCObservations2D::~CDCObservations2D()
{
}



size_t CDCObservations2D::getNObservationsWithDriftRadius() const
{

  // Obtain an iterator an advance it to the first drift radius
  std::vector<FloatType>::const_iterator itDriftRadius = m_observations.begin();
  std::advance(itDriftRadius, 2);

  size_t result = 0;

  //Every thrid element is a drift radius
  for (; itDriftRadius < m_observations.end(); std::advance(itDriftRadius, 3)) {

    bool hasDriftRadius = (*itDriftRadius != 0.0);
    result += hasDriftRadius ? 1 : 0;

  }
  return result;
}



CDCObservations2D::EigenObservationMatrix CDCObservations2D::getObservationMatrix()
{

  size_t nObservations = m_observations.size() / 3;
  FloatType* rawObservations = &(m_observations.front());
  Map< Matrix< FloatType, Dynamic, Dynamic, RowMajor > > eigenObservations(rawObservations, nObservations, 3);
  return eigenObservations;

}



void CDCObservations2D::centralize(const Vector2D& origin)
{
  RowVector2f eigenOrigin(origin.x(), origin.y());
  EigenObservationMatrix eigenObservations = getObservationMatrix();
  eigenObservations.leftCols<2>().rowwise() -= eigenOrigin;
}


Vector2D CDCObservations2D::centralize()
{
  size_t nObservations = size();
  if (nObservations == 0) return Vector2D(NAN, NAN);

  size_t iCentralObservation = nObservations / 2;

  FloatType centralX = m_observations[iCentralObservation * 3];
  FloatType centralY = m_observations[iCentralObservation * 3 + 1];
  Vector2D centralPoint(centralX, centralY);

  centralize(centralPoint);
  return centralPoint;

  // May refine somehow
  // EigenObservationMatrix eigenObservations = getObservationMatrix();
  // RowVector2f meanPoint = eigenObservations.leftCols<2>.colwise().mean();
  // Pick an observation at the center
}


