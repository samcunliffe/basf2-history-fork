/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <stdio.h>

/* Belle2 headr4es. */
#include <eklm/dataobjects/EKLMHit2d.h>

using namespace Belle2;

ClassImp(Belle2::EKLMHit2d);

EKLMHit2d::EKLMHit2d()
{
  m_XStrip = NULL;
  m_YStrip = NULL;
}


EKLMHit2d::EKLMHit2d(EKLMDigit* xStrip, EKLMDigit* yStrip)
{

  m_XStrip = xStrip;
  m_YStrip = yStrip;
  setEndcap(xStrip->getEndcap());
  setLayer(xStrip->getLayer());
  setSector(xStrip->getSector());
}


const Belle2::EKLMDigit* EKLMHit2d::getXStripHit() const
{
  return m_XStrip;
}

const Belle2::EKLMDigit* EKLMHit2d::getYStripHit() const
{
  return m_YStrip;
}

void EKLMHit2d::setCrossPoint(HepGeom::Point3D<double> *point)
{
  m_crossPointX = point->x();
  m_crossPointY = point->y();
  m_crossPointZ = point->z();
}

HepGeom::Point3D<double> EKLMHit2d::getCrossPoint() const
{
  return HepGeom::Point3D<double>(m_crossPointX, m_crossPointY, m_crossPointZ);
}

double EKLMHit2d::getChiSq() const
{
  return m_ChiSq;
}

void EKLMHit2d::Print()
{
  printf("------------  Hit 2d  -------------- \n"
         "Endcap: %d Layer: %d Sector: %d\n",
         getEndcap(), getLayer(), getSector());
  printf("X: ");
  m_XStrip->Print();
  printf("Y: ");
  m_YStrip->Print();
  printf("Intersection: X:%f Y:%f Z:%f.", m_crossPointX, m_crossPointY,
         m_crossPointZ);
  printf("Chi squared: %f\n", m_ChiSq);
}

void EKLMHit2d::setChiSq(double chisq)
{
  m_ChiSq = chisq;
}

