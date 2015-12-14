/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/AlignmentChecker.h>
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/geometry/Polygon2D.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::AlignmentChecker::AlignmentChecker()
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  const SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  const ElementPosition* sectorSupportPosition =
    m_GeoDat->getSectorSupportPosition();
  m_LineCorner1 = new LineSegment2D(
    sectorSupportGeometry->Corner1AInner, sectorSupportGeometry->Corner1BInner);
  m_ArcOuter = new Arc2D(
    0, 0, sectorSupportPosition->OuterR - sectorSupportGeometry->Thickness,
    atan2(sectorSupportGeometry->Corner2Inner.y(),
          sectorSupportGeometry->Corner2Inner.x()),
    atan2(sectorSupportGeometry->Corner1BInner.y(),
          sectorSupportGeometry->Corner1BInner.x()));
  m_Line23 = new LineSegment2D(
    sectorSupportGeometry->Corner2Inner, sectorSupportGeometry->Corner3Inner);
  m_ArcInner = new Arc2D(
    0, 0, sectorSupportPosition->InnerR + sectorSupportGeometry->Thickness,
    atan2(sectorSupportGeometry->Corner3Inner.y(),
          sectorSupportGeometry->Corner3Inner.x()),
    atan2(sectorSupportGeometry->Corner4Inner.y(),
          sectorSupportGeometry->Corner4Inner.x()));
  m_Line41 = new LineSegment2D(
    sectorSupportGeometry->Corner4Inner, sectorSupportGeometry->Corner1AInner);
}

EKLM::AlignmentChecker::~AlignmentChecker()
{
  delete m_LineCorner1;
  delete m_ArcOuter;
  delete m_Line23;
  delete m_ArcInner;
  delete m_Line41;
}

bool EKLM::AlignmentChecker::
checkSegmentAlignment(int iPlane, int iSegment,
                      EKLMAlignmentData* alignment) const
{
  int i, j, iStrip;
  double lx, ly;
  HepGeom::Point3D<double> stripRectangle[4];
  HepGeom::Transform3D t;
  const struct ElementPosition* stripPosition;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  ly = 0.5 * stripGeometry->Width;
  for (i = 1; i <= 15; i++) {
    iStrip = 15 * (iSegment - 1) + i;
    stripPosition = m_GeoDat->getStripPosition(iStrip);
    lx = 0.5 * stripPosition->Length;
    stripRectangle[0].setX(lx);
    stripRectangle[0].setY(ly);
    stripRectangle[0].setZ(0);
    stripRectangle[1].setX(-lx);
    stripRectangle[1].setY(ly);
    stripRectangle[1].setZ(0);
    stripRectangle[2].setX(-lx);
    stripRectangle[2].setY(-ly);
    stripRectangle[2].setZ(0);
    stripRectangle[3].setX(lx);
    stripRectangle[3].setY(-ly);
    stripRectangle[3].setZ(0);
    t = HepGeom::Translate3D(alignment->getDx() * CLHEP::cm / Unit::cm,
                             alignment->getDy() * CLHEP::cm / Unit::cm, 0) *
        HepGeom::Translate3D(stripPosition->X, stripPosition->Y, 0) *
        HepGeom::RotateZ3D(alignment->getDalpha() * CLHEP::rad / Unit::rad);
    if (iPlane == 1)
      t = HepGeom::Rotate3D(180. * CLHEP::deg,
                            HepGeom::Vector3D<double>(1., 1., 0.)) * t;
    for (j = 0; j < 4; j++)
      stripRectangle[j] = t * stripRectangle[j];
    Polygon2D stripPolygon(stripRectangle, 4);
    if (stripPolygon.hasIntersection(*m_LineCorner1))
      return false;
    if (stripPolygon.hasIntersection(*m_ArcOuter))
      return false;
    if (stripPolygon.hasIntersection(*m_Line23))
      return false;
    if (stripPolygon.hasIntersection(*m_ArcInner))
      return false;
    if (stripPolygon.hasIntersection(*m_Line41))
      return false;
  }
  return true;
}

bool EKLM::AlignmentChecker::checkAlignment(EKLMAlignment* alignment) const
{
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment;
  EKLMAlignmentData* alignmentData;
  for (iEndcap = 1; iEndcap <= 2; iEndcap++) {
    for (iLayer = 1; iLayer <= EKLM::GeometryData::Instance().
         getNDetectorLayers(iEndcap); iLayer++) {
      for (iSector = 1; iSector <= 4; iSector++) {
        for (iPlane = 1; iPlane <= 2; iPlane++) {
          for (iSegment = 1; iSegment <= 5; iSegment++) {
            segment = EKLM::segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                          iSegment);
            alignmentData = alignment->getAlignmentData(segment);
            if (alignmentData == NULL)
              B2FATAL("Incomplete alignment data.");
            if (!checkSegmentAlignment(iPlane, iSegment, alignmentData))
              return false;
          }
        }
      }
    }
  }
  return true;
}

