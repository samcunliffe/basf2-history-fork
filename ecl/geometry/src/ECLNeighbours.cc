/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Returns the list of neighbours for a given crystal ID.                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL GEOMETRY
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/geometry/ECLGeometryPar.h> // for geometry

// FRAMEWORK
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

// OTHER
#include "TMath.h" // for Nint
#include "TVector3.h"

using namespace Belle2;
using namespace ECL;

// Constructor.
ECLNeighbours::ECLNeighbours(const std::string neighbourDef, const float par)
{
  // resize the vector
  std::vector<short int> fakeneighbours;
  fakeneighbours.push_back(0); // insert one fake to avoid the "0" entry
  m_neighbourMap.push_back(fakeneighbours);

  int parToInt = TMath::Nint(par);

  // fixed number of neighbours:
  if (neighbourDef == "N") {
    B2INFO("ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", n x n: " << parToInt * 2 + 1 << " x " << parToInt * 2 + 1);
    if ((parToInt >= 0) && (parToInt < 6)) initializeN(parToInt);
    else B2FATAL("ECLNeighbours::ECLNeighbours: " << parToInt << " is an invalid parameter (must be between 0 and 5)!");
  }
  // or neighbours depend on the distance:
  else if (neighbourDef == "R") {
    B2INFO("ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", radius: " << par << " cm");
    if ((par > 0.0) && (par < 30.0 * Belle2::Unit::cm)) initializeR(par);
    else B2FATAL("ECLNeighbours::ECLNeighbours: " << par << " is an invalid parameter (must be between 0 cm and 30 cm)!");
  }
  // or wrong type:
  else {
    B2FATAL("ECLNeighbours::ECLNeighbours (constructor via std::string): Invalid option: " << neighbourDef <<
            " (valid: N1, N2, N3, N4, N5 or R");
  }

}

ECLNeighbours::~ECLNeighbours()
{
  ;
}

void ECLNeighbours::initializeR(float radius)
{
  // resize the vector
  std::vector<short int> fakeneighbours;
  fakeneighbours.push_back(0); // insert one fake to avoid the "0" entry
  m_neighbourMapTemp.push_back(fakeneighbours);

  // distance calculations take a long time, so reduce the number of candidates first:
  initializeN(6);

  // ECL geometry
  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  for (int i = 0; i < 8736; i++) {
    // get the central one
    TVector3 direction = geom->GetCrystalVec(i);
    TVector3 position  = geom->GetCrystalPos(i);

    // get all nearby crystals
    std::vector<short int> neighbours = getNeighbours(i + 1);
    std::vector<short int> neighboursTemp;

    // ... and calculate the shortest distance between the central one and all possible neighbours (of the reduced set)
    for (auto const& id : neighbours) {
      TVector3 directionNeighbour = geom->GetCrystalVec(id - 1);
      const float alpha = direction.Angle(directionNeighbour);
      const float R = position.Mag();
      const float distance = getDistance(alpha, R);

//      B2INFO(i << " " << id << " " << distance << " (radius = " << radius << ")");

      if (distance <= radius) neighboursTemp.push_back(id);
    }

    m_neighbourMapTemp.push_back(neighboursTemp);
  }

  // all done, reoplace the original map
  m_neighbourMap = m_neighbourMapTemp;

}

void ECLNeighbours::initializeN(int n)
{
  // ECL geometry
  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  // This is the "NxN-edges" case (in the barrel)
  // in the endcaps we project the neighbours to the outer and inner rings.
  for (int i = 0; i < 8736; i++) {

//    B2INFO(i);

    // this vector will hold all neighbours for the i-th crystal
    std::vector<short int> neighbours;

    // phi and theta coordinates of the central crystal
    geom->Mapping(i);
    //const short cid = i;
    const short tid = geom->GetThetaID();
    const short pid = geom->GetPhiID();

    // 'left' and 'right' extremal neighbours in the same theta ring
    const short int phiInc = increasePhiId(pid, tid, n);
    const short int phiDec = decreasePhiId(pid, tid, n);
    const float fractionalPhiInc = static_cast < float >(phiInc) / m_crystalsPerRing [ tid ];
    const float fractionalPhiDec = static_cast < float >(phiDec) / m_crystalsPerRing [ tid ];
//    B2INFO("   phiInc = " << phiInc << " phiDec = " << phiDec);
//    B2INFO("   fractionalPhiInc = " << fractionalPhiInc << " fractionalPhiDec = " << fractionalPhiDec);

    // loop over all possible theta rings and add neighbours
    for (int theta = tid - n; theta <= tid + n; theta++) {
      if ((theta > 68) || (theta < 0)) continue;     // valid theta ids are 0..68 (69 in total)
//      B2INFO("   " << theta);

      short int thisPhiInc = TMath::Nint(fractionalPhiInc * m_crystalsPerRing [ theta ]);

      short int thisPhiDec = TMath::Nint(fractionalPhiDec * m_crystalsPerRing [ theta ]);
      if (thisPhiDec == m_crystalsPerRing [ theta ]) thisPhiDec = 0;

//      B2INFO("          thisPhiInc = " << thisPhiInc << " thisPhiDec = " << thisPhiDec);

      const std::vector<short int> phiList = getPhiIdsInBetween(thisPhiInc, thisPhiDec, theta);

      for (unsigned int k = 0; k < phiList.size(); ++k) {
        neighbours.push_back(geom->GetCellID(theta , phiList.at(k)) + 1);
      }

    }

    // push back the final vector of IDs
    m_neighbourMap.push_back(neighbours);

  }
}

std::vector<short int> ECLNeighbours::getNeighbours(const short int cid)
{
  return m_neighbourMap.at(cid);
}

// decrease the phi id by "n" integers numbers (valid ids range from 0 to m_crystalsPerRing[thetaid] - 1)
short int ECLNeighbours::decreasePhiId(const short int phiid, const short int thetaid, const short int n)
{
  short int previousPhiId = ((phiid - n < 0) ? m_crystalsPerRing[thetaid] + phiid - n : phiid - n);     // "underflow"
  return previousPhiId;
}

// increase the phi id by "n" integers numbers (valid ids range from 0 to m_crystalsPerRing[thetaid] - 1)
short int ECLNeighbours::increasePhiId(const short int phiid, const short int thetaid, const short int n)
{
  short int nextPhiId = (((phiid + n) > (m_crystalsPerRing[thetaid] - 1)) ? phiid + n - m_crystalsPerRing[thetaid] : phiid +
                         n);       // "overflow"
  return nextPhiId;
}

std::vector<short int> ECLNeighbours::getPhiIdsInBetween(const short int phi0, const short int phi1, const short int theta)
{
  std::vector<short int> phiList;
  short int loop = phi0;

  while (1) {
    phiList.push_back(loop);
    if (loop == phi1) break;
    loop = decreasePhiId(loop, theta, 1);
  }

  return phiList;
}

float ECLNeighbours::getDistance(float alpha, float R)
{
  return 2.0 * R * TMath::Sin(alpha / 2.0);
}

