/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMGEOMETRY_H
#define EKLMGEOMETRY_H

/* External headers. */
#include <TObject.h>
#include <CLHEP/Geometry/Point3D.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMElementNumbers.h>

namespace Belle2 {

  /**
   * Class to store EKLM geometry data in the database.
   */
  class EKLMGeometry : public EKLMElementNumbers {

  public:

    /** Mode of detector operation. */
    enum DetectorMode {
      /** Normal mode. */
      c_DetectorNormal = 0,
      /** Background study. */
      c_DetectorBackground = 1,
    };

    /**
     * @struct EndcapStructureGeometry
     * @brief EndcapStructure geometry parameters.
     *
     * @var EndcapStructureGeometry::Phi
     * The starting angle of the octagonal Endcap KLM shape.
     *
     * @var EndcapStructureGeometry::Dphi
     * The opening angle (shape is extended from phi to phi+dphi).
     *
     * @var EndcapStructureGeometry::Nsides
     * The number of sides (=8 : octagonal).
     *
     * @var EndcapStructureGeometry::Nboundary
     * The number of boundaries perpendicular to the z-axis.
     *
     * @var EndcapStructureGeometry::Z
     * The z coordinate of the section specified by input id (=boundary id).
     *
     * @var EndcapStructureGeometry::Rmin
     * The radius of the circle tangent to the sides of the inner polygon.
     *
     * @var EndcapStructureGeometry::Rmax
     * The radius of the circle tangent to the sides of the outer polygon.
     *
     * @var EndcapStructureGeometry::Zsub
     * The length of the tube.
     *
     * @var EndcapStructureGeometry::Rminsub
     * The inner radius of the tube.
     *
     * @var EndcapStructureGeometry::Rmaxsub
     * The outer radius of the tube.
     */
    struct EndcapStructureGeometry : public TObject {

      /**
       * Constructor.
       */
      EndcapStructureGeometry();

      /**
       * Copy constructor.
       */
      EndcapStructureGeometry(const EndcapStructureGeometry& geometry);

      /**
       * Operator =.
       */
      EndcapStructureGeometry& operator=(
        const EndcapStructureGeometry& geometry);

      /**
       * Destructor.
       */
      ~EndcapStructureGeometry();

      double Phi;
      double Dphi;
      int Nsides;
      int Nboundary;
      double* Z;    //[Nboundary]
      double* Rmin; //[Nboundary]
      double* Rmax; //[Nboundary]
      double Zsub;
      double Rminsub;
      double Rmaxsub;

    private:

      /** Makes objects storable. */
      ClassDef(EndcapStructureGeometry, 1);

    };

    /**
     * @struct ElementPosition
     * Position information for the elements of detector.
     *
     * @var ElementPosition::InnerR
     * Inner radius.
     *
     * @var ElementPosition::OuterR
     * Outer radius.
     *
     * @var ElementPosition::Length
     * Length.
     *
     * @var ElementPosition::X
     * X coordinate.
     *
     * @var ElementPosition::Y
     * Y coordinate.
     *
     * @var ElementPosition::Z
     * Z coordinate.
     *
     */
    struct ElementPosition : public TObject {

      /**
       * Constructor.
       */
      ElementPosition();

      double InnerR;
      double OuterR;
      double Length;
      double X;
      double Y;
      double Z;

    private:

      /** Makes objects storable. */
      ClassDef(ElementPosition, 1);

    };

    /**
     * @struct SectorSupportGeometry
     * Sector support geometry data.
     *
     * @var SectorSupportGeometry::Thickness
     * Thickness.
     *
     * @var SectorSupportGeometry::DeltaLY
     * outerR - Y of upper edge of BoxY.
     *
     * @var SectorSupportGeometry::CornerX
     * Coordinate X of corner 1.
     *
     * @var SectorSupportGeometry::Corner1LX
     * Corner 1 X length.
     *
     * @var SectorSupportGeometry::Corner1Width
     * Corner 1 width.
     *
     * @var SectorSupportGeometry::Corner1Thickness
     * Corner 1 thickness.
     *
     * @var SectorSupportGeometry::Corner1Z
     * Corner 1 Z coordinate.
     *
     * @var SectorSupportGeometry::Corner2LX
     * Corner 2 X length.
     *
     * @var SectorSupportGeometry::Corner2LY
     * Corner 2 Y length.
     *
     * @var SectorSupportGeometry::Corner2Thickness
     * Corner 2 thickness.
     *
     * @var SectorSupportGeometry::Corner2Z
     * Corner 2 Z coordinate.
     *
     * @var SectorSupportGeometry::Corner3LX
     * Corner 3 X length.
     *
     * @var SectorSupportGeometry::Corner3LY
     * Corner 3 Y length.
     *
     * @var SectorSupportGeometry::Corner3Thickness
     * Corner 3 thickness.
     *
     * @var SectorSupportGeometry::Corner3Z
     * Corner 3 Z coordinate.
     *
     * @var SectorSupportGeometry::Corner4LX
     * Corner 4 X length.
     *
     * @var SectorSupportGeometry::Corner4LY
     * Corner 4 Y length.
     *
     * @var SectorSupportGeometry::Corner4Thickness
     * Corner 4 thickness.
     *
     * @var SectorSupportGeometry::Corner4Z
     * Corner 4 Z coordinate.
     *
     * @var SectorSupportGeometry::CornerAngle
     * Corner 1 angle.
     *
     * @var SectorSupportGeometry::Corner1A
     * Corner 1A coordinates.
     *
     * @var SectorSupportGeometry::Corner1AInner
     * Corner 1A coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner1B
     * Corner 1B coordinates.
     *
     * @var SectorSupportGeometry::Corner1BInner
     * Corner 1B coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner2Inner
     * Corner 2 coordinates (inner side) (corner of prism, too).
     *
     * @var SectorSupportGeometry::Corner3
     * Corner 3 coordinates.
     *
     * @var SectorSupportGeometry::Corner3Inner
     * Corner 3 coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner3Prism
     * Coordinates of the corner of corner 3 prism.
     *
     * @var SectorSupportGeometry::Corner4
     * Corner 4 coordinates.
     *
     * @var SectorSupportGeometry::Corner4Inner
     * Corner 4 coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner4Prism
     * Coordinates of the corner of corner 4 prism.
     */
    struct SectorSupportGeometry : public TObject {

      /**
       * Constructor.
       */
      SectorSupportGeometry();

      double Thickness;
      double DeltaLY;
      double CornerX;
      double Corner1LX;
      double Corner1Width;
      double Corner1Thickness;
      double Corner1Z;
      double Corner2LX;
      double Corner2LY;
      double Corner2Thickness;
      double Corner2Z;
      double Corner3LX;
      double Corner3LY;
      double Corner3Thickness;
      double Corner3Z;
      double Corner4LX;
      double Corner4LY;
      double Corner4Thickness;
      double Corner4Z;
      /* The following parameters are not stored in the database. */
      double CornerAngle;                     //!
      HepGeom::Point3D<double> Corner1A;      //!
      HepGeom::Point3D<double> Corner1AInner; //!
      HepGeom::Point3D<double> Corner1B;      //!
      HepGeom::Point3D<double> Corner1BInner; //!
      HepGeom::Point3D<double> Corner2Inner;  //!
      HepGeom::Point3D<double> Corner3;       //!
      HepGeom::Point3D<double> Corner3Inner;  //!
      HepGeom::Point3D<double> Corner3Prism;  //!
      HepGeom::Point3D<double> Corner4;       //!
      HepGeom::Point3D<double> Corner4Inner;  //!
      HepGeom::Point3D<double> Corner4Prism;  //!

    private:

      /** Makes objects storable. */
      ClassDef(SectorSupportGeometry, 1);

    };

    /**
     * @struct PlasticSheetGeometry
     * Plastic sheet geometry data.
     *
     * @var PlasticSheetGeometry::Width
     * Width.
     *
     * @var PlasticSheetGeometry::DeltaL
     * Distance from edge of last strip to edge of plastic list.
     */
    struct PlasticSheetGeometry : public TObject {

      /**
       * Constructor.
       */
      PlasticSheetGeometry();

      double Width;
      double DeltaL;

    private:

      /** Makes objects storable. */
      ClassDef(PlasticSheetGeometry, 1);

    };

    /**
     * Segment support geometry data.
     */
    class SegmentSupportGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      SegmentSupportGeometry();

      /**
       * Get top part width.
       */
      double getTopWidth() const;

      /**
       * Set top part width.
       * @param[in] topWidth Top part width.
       */
      void setTopWidth(double topWidth);

      /**
       * Get top part thickness.
       */
      double getTopThickness() const;

      /**
       * Set top part thickness.
       * @param[in] topThickness Top part thickness.
       */
      void setTopThickness(double topThickness);

      /**
       * Get middle part width.
       */
      double getMiddleWidth() const;

      /**
       * Set middle part width.
       * @param[in] middleWidth Middle part width.
       */
      void setMiddleWidth(double middleWidth);

      /**
       * Get middle part thickness.
       */
      double getMiddleThickness() const;

      /**
       * Set middle part thickness.
       * @param[in] middleThickness Middle part thickness.
       */
      void setMiddleThickness(double middleThickness);

    private:

      /** Top box width. */
      double m_TopWidth;

      /** Top box thickness. */
      double m_TopThickness;

      /** Middle box width. */
      double m_MiddleWidth;

      /** Middle box thickness. */
      double m_MiddleThickness;

      /** Makes objects storable. */
      ClassDef(SegmentSupportGeometry, 1);

    };

    /**
     * Segment support position.
     */
    class SegmentSupportPosition : public TObject {

    public:

      /**
       * Constructor.
       */
      SegmentSupportPosition();

      /**
       * Get right Delta L.
       */
      double getDeltaLRight() const;

      /**
       * Set right Delta L.
       * @param[in] deltaLRight Right Delta L.
       */
      void setDeltaLRight(double deltaLRight);

      /**
       * Get left Delta L.
       */
      double getDeltaLLeft() const;

      /**
       * Set left DeltaL.
       * @param[in] deltaLLeft Left Delta L.
       */
      void setDeltaLLeft(double deltaLLeft);

      /**
       * Get length.
       */
      double getLength() const;

      /**
       * Set length.
       * @param[in] length Length.
       */
      void setLength(double length);

      /**
       * Get X coordinate.
       */
      double getX() const;

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x);

      /**
       * Get Y coordinate.
       */
      double getY() const;

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y);

      /**
       * Get Z coordinate.
       */
      double getZ() const;

      /**
       * Set Z coordinate.
       * @param[in] z Z coordinate.
       */
      void setZ(double z);

    private:

      /** Right (X-plane) delta L. */
      double m_DeltaLRight;

      /** Left (X-plane) delta L. */
      double m_DeltaLLeft;

      /** Length */
      double m_Length;

      /** X coordinate. */
      double m_X;

      /** Y coordinate. */
      double m_Y;

      /** Z coordinate. */
      double m_Z;

      /** Makes objects storable. */
      ClassDef(SegmentSupportPosition, 1);

    };

    /**
     * Strip geometry data.
     */
    class StripGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      StripGeometry();

      /**
       * Get width.
       */
      double getWidth() const;

      /**
       * Set width.
       * @param[in] width Width.
       */
      void setWidth(double width);

      /**
       * Get thickness.
       */
      double getThickness() const;

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness);

      /**
       * Get groove depth.
       */
      double getGrooveDepth() const;

      /**
       * Set groove depth.
       * @param[in] grooveDepth Groove depth.
       */
      void setGrooveDepth(double grooveDepth);

      /**
       * Get groove width.
       */
      double getGrooveWidth() const;

      /**
       * Set groove width.
       * @param[in] grooveWidth Groove width.
       */
      void setGrooveWidth(double grooveWidth);

      /**
       * Get nonscintillating layer thickness..
       */
      double getNoScintillationThickness() const;

      /**
       * Set nonscintillating layer thickness..
       * @param[in] thickness Nonscintillating layer thickness.
       */
      void setNoScintillationThickness(double thickness);

      /**
       * Get "SiPM" size.
       */
      double getRSSSize() const;

      /**
       * Set "SiPM" size.
       * @param[in]
       */
      void setRSSSize(double rssSize);

    private:

      /** Width. */
      double m_Width;

      /** Thickness. */
      double m_Thickness;

      /** Groove depth. */
      double m_GrooveDepth;

      /** Groove width. */
      double m_GrooveWidth;

      /** Non-scintillating layer thickness. */
      double m_NoScintillationThickness;

      /** Radiation study SiPM size. */
      double m_RSSSize;

      /** Makes objects storable. */
      ClassDef(StripGeometry, 1);

    };

    /**
     * 2D point.
     */
    class Point : public TObject {

    public:

      /**
       * Constructor.
       */
      Point();

      /**
       * Get X coordinate.
       */
      double getX() const;

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x);

      /**
       * Get Y coordinate.
       */
      double getY() const;

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y);

    private:

      /** X coordinate. */
      double m_X;

      /** Y coordinate. */
      double m_Y;

      /** Makes objects storable. */
      ClassDef(Point, 1);

    };

    /**
     * Shield layer detail geometry data.
     */
    class ShieldDetailGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      ShieldDetailGeometry();

      /**
       * Copy constructor.
       */
      ShieldDetailGeometry(const ShieldDetailGeometry& geometry);

      /**
       * Operator =.
       */
      ShieldDetailGeometry& operator=(const ShieldDetailGeometry& geometry);

      /**
       * Destructor.
       */
      ~ShieldDetailGeometry();

      /**
       * Get X length.
       */
      double getLengthX() const;

      /**
       * Set X length.
       * @param[in] lengthX X length.
       */
      void setLengthX(double lengthX);

      /**
       * Get Y length.
       */
      double getLengthY() const;

      /**
       * Set Y length.
       * @param[in] lengthY Y length.
       */
      void setLengthY(double lengthY);

      /**
       * Get number of points.
       */
      int getNPoints() const;

      /**
       * Set number of points.
       * @param[in] nPoints Number of points.
       */
      void setNPoints(int nPoints);

      /**
       * Get point.
       * @param[in] i Number of point (array index).
       */
      const Point* getPoint(int i) const;

      /**
       * Set point.
       * @param[in] i     Number of point (array index).
       * @param[in] point Point.
       */
      void setPoint(int i, const Point& point);

    private:

      /** X length. */
      double m_LengthX;

      /** Y length. */
      double m_LengthY;

      /** Number of points. */
      int m_NPoints;

      /** Points. */
      Point* m_Points; //[m_NPoints]

      /** Makes objects storable. */
      ClassDef(ShieldDetailGeometry, 1);

    };

    /**
     * Shield layer geometry data.
     */
    class ShieldGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      ShieldGeometry();

      /**
       * Get thickness.
       */
      double getThickness() const;

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness);

      /**
       * Get detail A geometry.
       */
      const ShieldDetailGeometry* getDetailA() const;

      /**
       * Set detail A geometry.
       * @param[in] geometry Detail A geometry.
       */
      void setDetailA(const ShieldDetailGeometry& geometry);

      /**
       * Get detail B geometry.
       */
      const ShieldDetailGeometry* getDetailB() const;

      /**
       * Set detail B geometry.
       * @param[in] geometry Detail B geometry.
       */
      void setDetailB(const ShieldDetailGeometry& geometry);

      /**
       * Get detail C geometry.
       */
      const ShieldDetailGeometry* getDetailC() const;

      /**
       * Set detail C geometry.
       * @param[in] geometry Detail C geometry.
       */
      void setDetailC(const ShieldDetailGeometry& geometry);

      /**
       * Get detail D geometry.
       */
      const ShieldDetailGeometry* getDetailD() const;

      /**
       * Set detail D geometry.
       * @param[in] geometry Detail D geometry.
       */
      void setDetailD(const ShieldDetailGeometry& geometry);

      /**
       * Get detail A center.
       */
      const Point* getDetailACenter() const;

      /**
       * Set detail A center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailACenter(double x, double y);

      /**
       * Get detail B center.
       */
      const Point* getDetailBCenter() const;

      /**
       * Set detail B center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailBCenter(double x, double y);

      /**
       * Get detail C center.
       */
      const Point* getDetailCCenter() const;

      /**
       * Set detail C center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailCCenter(double x, double y);

    private:

      /** Thickness. */
      double m_Thickness;

      /** Detail A. */
      ShieldDetailGeometry m_DetailA;

      /** Detail B. */
      ShieldDetailGeometry m_DetailB;

      /** Detail C. */
      ShieldDetailGeometry m_DetailC;

      /** Detail D. */
      ShieldDetailGeometry m_DetailD;

      /* The following data members are not stored in the database. */

      /** Detail A center. */
      Point m_DetailACenter; //!

      /** Detail B center. */
      Point m_DetailBCenter; //!

      /** Detail C center. */
      Point m_DetailCCenter; //!

      /** Makes objects storable. */
      ClassDef(ShieldGeometry, 1);

    };

    /**
     * Readout board geometry data.
     */
    class BoardGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      BoardGeometry();

      /**
       * Get length.
       */
      double getLength() const;

      /**
       * Set length.
       * @param[in] length Length.
       */
      void setLength(double length);

      /**
       * Get width.
       */
      double getWidth() const;

      /**
       * Set width.
       * @param[in] width Width.
       */
      void setWidth(double width);

      /**
       * Get height.
       */
      double getHeight() const;

      /**
       * Set height.
       * @param[in] height Height.
       */
      void setHeight(double height);

      /**
       * Get base board width.
       */
      double getBaseWidth() const;

      /**
       * Set base board width.
       * @param[in] baseWidth Base board width.
       */
      void setBaseWidth(double baseWidth);

      /**
       * Get base board height.
       */
      double getBaseHeight() const;

      /**
       * Set base board height.
       * @param[in] baseHeight Base board height.
       */
      void setBaseHeight(double baseHeight);

      /**
       * Get strip board length.
       */
      double getStripLength() const;

      /**
       * Set strip board length.
       * @param[in] stripLength Strip board length.
       */
      void setStripLength(double stripLength);

      /**
       * Get strip board width.
       */
      double getStripWidth() const;

      /**
       * Set strip board width.
       * @param[in] stripWidth Strip board width.
       */
      void setStripWidth(double stripWidth);

      /**
       * Get strip board height.
       */
      double getStripHeight() const;

      /**
       * Set strip board height.
       * @param[in] stripHeight Strip board height.
       */
      void setStripHeight(double stripHeight);

    private:

      /** Length. */
      double m_Length;

      /** Width. */
      double m_Width;

      /** Height. */
      double m_Height;

      /** Width of base board. */
      double m_BaseWidth;

      /** Height of base board. */
      double m_BaseHeight;

      /** Length of strip readout board. */
      double m_StripLength;

      /** Width of strip readout board. */
      double m_StripWidth;

      /** Height of strip readout board. */
      double m_StripHeight;

      /** Makes objects storable. */
      ClassDef(BoardGeometry, 1);

    };

    /**
     * Readout board position data.
     */
    class BoardPosition : public TObject {

    public:

      /**
       * Constructor.
       */
      BoardPosition();

      /**
       * Get radius.
       */
      double getR() const;

      /**
       * Set radius.
       * @param[in] r Radius.
       */
      void setR(double r);

      /**
       * Get angle.
       */
      double getPhi() const;

      /**
       * Set angle.
       * @param[in] phi Angle.
       */
      void setPhi(double phi);

    private:

      /** Radius of far edge of the board. */
      double m_R;

      /** Angle. */
      double m_Phi;

      /** Makes objects storable. */
      ClassDef(BoardPosition, 1);

    };

    /**
     * Strip readout board position data.
     */
    class StripBoardPosition : public TObject {

    public:

      /**
       * Constructor.
       */
      StripBoardPosition();

      /**
       * Get X coordinate.
       */
      double getX() const;

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x);

    private:

      /** X coordinate. */
      double m_X;

      /** Makes objects storable. */
      ClassDef(StripBoardPosition, 1);

    };

    /**
     * Constructor.
     */
    EKLMGeometry();

    /**
     * Copy constructor.
     */
    EKLMGeometry(const EKLMGeometry& geometry);

    /**
     * Destructor.
     */
    ~EKLMGeometry();

    /**
     * Operator =.
     */
    EKLMGeometry& operator=(const EKLMGeometry& geometry);

    /**
     * Get EKLM detector mode.
     */
    enum DetectorMode getDetectorMode() const;

    /**
     * Get number of endcaps.
     */
    int getNEndcaps() const;

    /**
     * Get number of layers.
     */
    int getNLayers() const;

    /**
     * Get number of detector layers.
     * @param[in] endcap Endcap number (1 - backward, 2 - forward).
     */
    int getNDetectorLayers(int endcap) const;

    /**
     * Get number of sectors.
     */
    int getNSectors() const;

    /**
     * Get number of planes.
     */
    int getNPlanes() const;

    /**
     * Get number of segments.
     */
    int getNSegments() const;

    /**
     * Get number of segment support elements *in one sector).
     */
    int getNSegmentSupportElementsSector() const;

    /**
     * Get number of strips in a segment.
     */
    int getNStripsSegment() const;

    /**
     * Get number of strips.
     */
    int getNStrips() const;

    /**
     * Get number of readout boards.
     */
    int getNBoards() const;

    /**
     * Get number of redout boards in one sector.
     */
    int getNBoardsSector() const;

    /**
     * Get number of strip readout boards.
     */
    int getNStripBoards() const;

    /**
     * Check if number of detector layers is correct (fatal error if not).
     * Endcap number must be checked separately.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     */
    void checkDetectorLayerNumber(int endcap, int layer) const;

    /**
     * Check if detector layer number is correct (fatal error if not).
     * Endcap number must be checked separately.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     */
    void checkDetectorLayer(int endcap, int layer) const;

    /**
     * Check if segment support number is correct (fatal error if not).
     * @param[in] support Segment support element number.
     */
    void checkSegmentSupport(int support) const;

    /**
     * Check if number of strip in a segment is correct (fatal error if not).
     * @param[in] strip Strip number.
     */
    void checkStripSegment(int strip) const;

    /**
     * Get solenoid center Z coordinate.
     */
    double getSolenoidZ() const;

    /**
     * Get endcap structure geometry data.
     */
    const struct EndcapStructureGeometry* getEndcapStructureGeometry() const;

    /**
     * Get position data for endcaps.
     */
    const struct ElementPosition* getEndcapPosition() const;

    /**
     * Get position data for layers.
     */
    const struct ElementPosition* getLayerPosition() const;

    /**
     * Get Z distance between two layers.
     */
    double getLayerShiftZ() const;

    /**
     * Get position data for sectors.
     */
    const struct ElementPosition* getSectorPosition() const;

    /**
     * Get position data for sector support structure.
     */
    const struct ElementPosition* getSectorSupportPosition() const;

    /**
     * Get sector support geometry data.
     */
    const struct SectorSupportGeometry* getSectorSupportGeometry() const;

    /**
     * Get position data for planes.
     */
    const struct ElementPosition* getPlanePosition() const;

    /**
     * Get plastic sheet geometry data.
     */
    const struct PlasticSheetGeometry* getPlasticSheetGeometry() const;

    /**
     * Get segment support geometry data.
     */
    const SegmentSupportGeometry* getSegmentSupportGeometry() const;

    /**
     * Get position data for segment support structure.
     * @param[in] plane   Plane number.
     * @param[in] support Segment support element number.
     */
    const SegmentSupportPosition*
    getSegmentSupportPosition(int plane, int support) const;

    /**
     * Get strip geometry data.
     */
    const StripGeometry* getStripGeometry() const;

    /**
     * Get position data for strips.
     * @param[in] strip Strip number.
     */
    const struct ElementPosition* getStripPosition(int strip) const;

    /**
     * Get shield layer details geometry data.
     */
    const ShieldGeometry* getShieldGeometry() const;

    /**
     * Get readout board geometry data.
     */
    const BoardGeometry* getBoardGeometry() const;

    /**
     * Get position data for readout boards.
     * @param[in] plane   Plane number.
     * @param[in] segment Segment number.
     */
    const BoardPosition* getBoardPosition(int plane, int segment) const;

    /**
     * Get position data for strip readout boards.
     * @param[in] board Number of board.
     */
    const StripBoardPosition* getStripBoardPosition(int board) const;

  protected:

    /** Detector mode. */
    enum DetectorMode m_Mode;

    /** Number of endcaps. */
    int m_NEndcaps;

    /** Number of layers in one endcap. */
    int m_NLayers;

    /** Number of detector layers. */
    int* m_NDetectorLayers; //[m_NEndcaps]

    /** Number of sectors in one layer. */
    int m_NSectors;

    /** Number of planes in one sector. */
    int m_NPlanes;

    /** Number of segments in one plane. */
    int m_NSegments;

    /** Number of segment support elements in one sector. */
    int m_NSegmentSupportElementsSector;

    /** Number of strips in one segment. */
    int m_NStripsSegment;

    /** Number of strips in one plane. */
    int m_NStrips;

    /** Number of readout boards corresponding to one plane. */
    int m_NBoards;

    /** Number of readout boards in one sector. */
    int m_NBoardsSector;

    /** Number of strip readout boards on one segment readout board. */
    int m_NStripBoards;

    /** Solenoid center Z coordinate. */
    double m_SolenoidZ;

    /** Endcap structure geometry data. */
    struct EndcapStructureGeometry m_EndcapStructureGeometry;

    /** Position data for endcaps. */
    struct ElementPosition m_EndcapPosition;

    /** Position data for layers. */
    struct ElementPosition m_LayerPosition;

    /** Z distance between two layers. */
    double m_LayerShiftZ;

    /** Position data for sectors. */
    struct ElementPosition m_SectorPosition;

    /** Position data for sector support structure. */
    struct ElementPosition m_SectorSupportPosition;

    /** Sector support geometry data. */
    struct SectorSupportGeometry m_SectorSupportGeometry;

    /** Position data for planes. */
    struct ElementPosition m_PlanePosition;

    /** Plastic sheet geometry data. */
    struct PlasticSheetGeometry m_PlasticSheetGeometry;

    /** Segment support geometry data. */
    SegmentSupportGeometry m_SegmentSupportGeometry;

    /** Position data for segment support structure. */
    SegmentSupportPosition* m_SegmentSupportPosition; //[m_NSegmentSupportElementsSector]

    /** Strip geometry data. */
    StripGeometry m_StripGeometry;

    /** Position data for strips. */
    struct ElementPosition* m_StripPosition; //[m_NStrips]

    /** Shield layer details geometry data. */
    ShieldGeometry m_ShieldGeometry;

    /** Readout board geometry data. */
    BoardGeometry m_BoardGeometry;

    /** Positions of readout boards. */
    BoardPosition* m_BoardPosition; //[m_NBoardsSector]

    /** Positions of strip readout boards. */
    StripBoardPosition* m_StripBoardPosition; //[m_NStripBoards]

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMGeometry, 1);

  };

}

#endif

