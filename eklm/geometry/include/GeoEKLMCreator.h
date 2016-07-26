/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Galina Pakhlova, Timofey Uglov, Kirill Chilikin          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_GEOEKLMCREATOR_H
#define EKLM_GEOEKLMCREATOR_H

/* C++ headers. */
#include <string>
#include <vector>

/* External headers. */
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4Transform3D.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4IntersectionSolid.hh>

/* Belle2 headers. */
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/G4TriangularPrism.h>
#include <eklm/geometry/GeoEKLMTypes.h>
#include <eklm/geometry/TransformData.h>

/**
 * @file
 * EKLM geometry.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * Sector support solids.
     */
    struct SectorSupportSolids {
      G4VSolid* itube; /**< Inner tube. */
      G4VSolid* otube; /**< Outer tube. */
    };

    /**
     * Sector support logical volumes.
     */
    struct SectorSupportLogicalVolumes {
      G4LogicalVolume* corn1; /**< Corner 1. */
      G4LogicalVolume* corn2; /**< Corner 2. */
      G4LogicalVolume* corn3; /**< Corner 3. */
      G4LogicalVolume* corn4; /**< Corner 4. */
      G4LogicalVolume* supp;  /**< Support. */
    };

    /**
     * Shield logical volumes.
     */
    struct ShieldLogicalVolumes {
      G4LogicalVolume* detailA; /**< Detail A. */
      G4LogicalVolume* detailB; /**< Detail B. */
      G4LogicalVolume* detailC; /**< Detail C. */
      G4LogicalVolume* detailD; /**< Detail D. */
    };

    /**
     * @struct Solids
     * All solids of EKLM.
     *
     * @var Solids::endcap
     * Endcap.
     *
     * @var Solids::layer
     * Layer.
     *
     * @var Solids::sector
     * Sector.
     *
     * @var Solids::secsupp
     * Sector support.
     *
     * @var Solids::plane
     * Plane.
     *
     * @var Solids::stripSegment
     * Strip segment.
     *
     * @var Solids::plasticSheetElement
     * Plastic sheet element.
     *
     * @var Solids::psheet
     * Plastic sheets (combined).
     *
     * @var Solids::stripvol
     * Strip volumes.
     *
     * @var Solids::strip
     * Strips.
     *
     * @var Solids::groove
     * Strip grooves.
     *
     * @var Solids::sipm
     * SiPM.
     *
     * @var Solids::board
     * Readout board.
     *
     * @var Solids::baseboard
     * Readout board base.
     *
     * @var Solids::stripboard
     * Readout board for 1 strip.
     *
     * @var Solids::subtractionBox
     * Box used for subtractions.
     *
     * @var Solids::sectorsup
     * Sector support.
     */
    struct Solids {
      G4VSolid* endcap;
      G4VSolid* layer;
      G4VSolid* sector;
      G4VSolid* secsupp;
      G4VSolid** plane;
      G4VSolid** stripSegment;
      G4VSolid** plasticSheetElement;
      G4VSolid** psheet;
      G4VSolid** stripvol;
      G4VSolid** strip;
      G4VSolid** groove;
      G4VSolid* sipm;
      G4VSolid* board;
      G4VSolid* baseboard;
      G4VSolid* stripboard;
      G4Box* subtractionBox;
      struct SectorSupportSolids sectorsup;
    };

    /**
     * @struct LogicalVolumes
     * Logical volumes of EKLM.
     *
     * @var LogicalVolumes::shieldLayer
     * Shield layer.
     *
     * @var LogicalVolumes::shieldLayerSector
     * Shield layer sector.
     *
     * @var LogicalVolumes::cover
     * Sector cover.
     *
     * @var LogicalVolumes::segment
     * Segments.
     *
     * @var LogicalVolumes::stripSegment
     * Strip segments.
     *
     * @var LogicalVolumes::stripvol
     * Strip volumes.
     *
     * @var LogicalVolumes::strip
     * Strips.
     *
     * @var LogicalVolumes::groove
     * Strip grooves.
     *
     * @var LogicalVolumes::scint
     * Scintillator.
     *
     * @var LogicalVolumes::psheet
     * Plastic sheet.
     *
     * @var LogicalVolumes::segmentsup
     * Segment support.
     *
     * @var LogicalVolumes::sectorsup
     * Sector support.
     *
     * @var LogicalVolumes::shield
     * Shield.
     */
    struct LogicalVolumes {
      G4LogicalVolume* shieldLayer;
      G4LogicalVolume* shieldLayerSector;
      G4LogicalVolume* cover;
      G4LogicalVolume** segment;
      G4LogicalVolume** stripSegment;
      G4LogicalVolume** stripvol;
      G4LogicalVolume** strip;
      G4LogicalVolume** groove;
      G4LogicalVolume** scint;
      G4LogicalVolume** psheet;
      G4LogicalVolume*** segmentsup;
      struct SectorSupportLogicalVolumes sectorsup;
      struct ShieldLogicalVolumes shield;
    };

    /**
     * Materials for EKLM.
     */
    struct Materials {
      G4Material* air;          /**< Air. */
      G4Material* polyethylene; /**< Polyethylene. */
      G4Material* polystyrene;  /**< Polystyrene. */
      G4Material* polystyrol;   /**< Polystyrol. */
      G4Material* iron;         /**< Iron. */
      G4Material* duralumin;    /**< Duralumin. */
      G4Material* silicon;      /**< Silicon. */
      G4Material* gel;          /**< Gel. */
    };

    /**
     * Volume numbers.
     */
    struct VolumeNumbers {
      int endcap;  /**< Endcap. */
      int layer;   /**< Layer. */
      int sector;  /**< Sector. */
      int plane;   /**< Plane. */
      int segment; /**< Segment. */
      int strip;   /**< Strip. */
      int board;   /**< Board. */
    };

    /**
     * Class GeoEKLMCreator.
     * The creator for the  EKLM geometry of the Belle II detector.
     */
    class GeoEKLMCreator : public geometry::CreatorBase {

    public:

      /**
       * Constructor.
       */
      GeoEKLMCreator();

      /**
       * Destructor.
       */
      ~GeoEKLMCreator();

      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      void create(const GearDir& content, G4LogicalVolume& topVolume,
                  geometry::GeometryTypes type);

      /**
       * Creation of the detector geometry from database.
       * @param[in] name      Name of the component in the database.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      void createFromDB(const std::string& name, G4LogicalVolume& topVolume,
                        geometry::GeometryTypes type);

      /**
       * Creation of payloads.
       * @param content XML data directory.
       * @param iov     Interval of validity.
       */
      void createPayloads(const GearDir& content,
                          const IntervalOfValidity& iov);

    private:

      /**
       * Creation of materials.
       */
      void createMaterials();

      /**
       * Read strip parameters from XML database.
       */
      void readXMLDataStrips();

      /**
       * Create endcap solid.
       */
      void createEndcapSolid();

      /**
       * Create layer logical volume.
       * @param[in] name Layer logical volume name.
       * @return Layer logical volume.
       */
      G4LogicalVolume* createLayerLogicalVolume(const char* name) const;

      /**
       * Create layer solid.
       */
      void createLayerSolid();

      /**
       * Create sector logical volume.
       * @param[in] name Sector logical volume name.
       * @return Sector logical volume.
       */
      G4LogicalVolume* createSectorLogicalVolume(const char* name) const;

      /**
       * Create sector solid.
       */
      void createSectorSolid();

      /**
       * Create sector cover solid.
       */
      void createSectorCoverLogicalVolume();

      /**
       * Create inner tube of sector support structure.
       */
      G4Tubs* createSectorSupportInnerTube();

      /**
       * Create outer tube of sector support structure.
       */
      G4Tubs* createSectorSupportOuterTube();

      /**
       * Create X side of sector support structure.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxX(G4Transform3D& t);

      /**
       * Create Y side of sector support structure.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxY(G4Transform3D& t);

      /**
       * Create box in the cutted corner of sector support structure.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxTop(G4Transform3D& t);

      /**
       * Create sector support logical volume.
       */
      void createSectorSupportLogicalVolume();

      /**
       * Create sector support corner 1 logical volume.
       */
      void createSectorSupportCorner1LogicalVolume();

      /**
       * Create sector support corner 2 logical volume.
       */
      void createSectorSupportCorner2LogicalVolume();

      /**
       * Create sector support corner 3 logical volume.
       */
      void createSectorSupportCorner3LogicalVolume();

      /**
       * Create sector support corner 4 logical volume.
       */
      void createSectorSupportCorner4LogicalVolume();

      /**
       * Create subtraction box solid.
       */
      void createSubtractionBoxSolid();

      /**
       * Cut corner of a solid.
       * @param[in] name           Name of resulting solid.
       * @param[in] solid          Solid.
       * @param[in] subtractionBox Box used for subtractions.
       * @param[in] transf         Additional transformation of subtraction box.
       * @param[in] largerAngles   If true then cut larger angles.
       * @param[in] x1             First point x coordinate.
       * @param[in] y1             First point y coordinate.
       * @param[in] x2             Second point x coordinate.
       * @param[in] y2             Second point y coordinate.
       */
      G4SubtractionSolid* cutSolidCorner(
        const char* name, G4VSolid* solid, G4Box* subtractionBox,
        HepGeom::Transform3D& transf, bool largerAngles,
        double x1, double y1, double x2, double y2);

      /**
       * Cut corner of a solid.
       * @param[in] name           Name of resulting solid.
       * @param[in] solid          Solid.
       * @param[in] subtractionBox Box used for subtractions.
       * @param[in] transf         Additional transformation of subtraction box.
       * @param[in] largerAngles   If true then cut larger angles.
       * @param[in] x              Initial point x coordinate.
       * @param[in] y              Initial point y coordinate.
       * @param[in] ang            Angle.
       */
      G4SubtractionSolid* cutSolidCorner(
        const char* name, G4VSolid* solid, G4Box* subtractionBox,
        HepGeom::Transform3D& transf, bool largerAngles,
        double x, double y, double ang);

      /**
       * Subtract board solids from planes.
       * @param[in] plane Plane solid without boards subtracted.
       * @param[in] n     Number of plane, from 0 to 1.
       */
      G4SubtractionSolid* subtractBoardSolids(G4SubtractionSolid* plane,
                                              int n);

      /**
       * Create plane solid.
       * @param[in] n Number of plane, from 0 to 1.
       */
      void createPlaneSolid(int n);

      /**
       * Create segment support logical volume.
       * @param[in] iPlane          Number of plane.
       * @param[in] iSegmentSupport Number of segment support.
       */
      void createSegmentSupportLogicalVolume(int iPlane, int iSegmentSupport);

      /**
       * Unify a group of solids.
       * @param[in] solids  Solids.
       * @param[in] transf  Transformations.
       * @param[in] nSolids Number of solids.
       * @param[in] name    First part of solid names.
       */
      G4VSolid* unifySolids(G4VSolid** solids, HepGeom::Transform3D* transf,
                            int nSolids, std::string name);

      /**
       * Create plastic sheet logical volume.
       * @param[in] iSegment Number of segment (0-based).
       */
      void createPlasticSheetLogicalVolume(int iSegment);

      /**
       * Create strip segment logical volume.
       * @param[in] iSegment Number of segment (0-based).
       */
      void createStripSegmentLogicalVolume(int iSegment);

      /**
       * Create segment logical volume (strips + plastic sheets).
       * @param[in] iSegment Number of segment (0-based).
       */
      void createSegmentLogicalVolume(int iSegment);

      /**
       * Create strip volume logical volume.
       * @param[in] iStrip Number of strip in length-based array.
       */
      void createStripVolumeLogicalVolume(int iStrip);

      /**
       * Create strip logical volume.
       * @param[in] iStrip Number of strip in length-based array.
       */
      void createStripLogicalVolume(int iStrip);

      /**
       * Create strip groove logical volume.
       * @param[in] iStrip Number of strip in length-based array.
       */
      void createStripGrooveLogicalVolume(int iStrip);

      /**
       * Create scintillator logical volume.
       * @param[in] iStrip Number of strip in length-based array.
       */
      void createScintillatorLogicalVolume(int iStrip);

      /**
       * Create shield detail A logical volume.
       */
      void createShieldDetailALogicalVolume();

      /**
       * Create shield detail B logical volume.
       */
      void createShieldDetailBLogicalVolume();

      /**
       * Create shield detail C logical volume.
       */
      void createShieldDetailCLogicalVolume();

      /**
       * Create shield detail D logical volume.
       */
      void createShieldDetailDLogicalVolume();

      /**
       * Create solids (or logical volumes which must be created only once).
       */
      void createSolids();

      /**
       * Check if a given layer is a detector layer.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @return True if this layer is a detector layer.
       */
      bool detectorLayer(int endcap, int layer) const;

      /**
       * Create endcap.
       * @param[in] topVolume Geant world volume.
       * @return Endcap logical volume.
       */
      G4LogicalVolume* createEndcap(G4LogicalVolume* topVolume) const;

      /**
       * Create layer.
       * @param[in] endcap Endcap logical volume.
       * @param[in] layer  Layer logical volume.
       * @return Layer logical volume.
       *
       * If layer == NULL, then new layer logical volume is created
       * (for detector layers). If layer != NULL, then the existing
       * logical volume is used (for shield layers).
       */
      G4LogicalVolume* createLayer(G4LogicalVolume* endcap,
                                   G4LogicalVolume* layer) const;

      /**
       * Create sector.
       * @param[in] layer  Layer logical volume.
       * @param[in] sector Sector logical volume.
       * @return Sector logical volume.
       *
       * If sector == NULL, then new sector logical volume is created
       * (for detector layers). If sector != NULL, then the existing
       * logical volume is used (for shield layers).
       */
      G4LogicalVolume* createSector(G4LogicalVolume* layer,
                                    G4LogicalVolume* sector) const;

      /**
       * Create sector cover.
       * @param[in] iCover Number of cover.
       * @param[in] sector Sector logical volume.
       */
      void createSectorCover(int iCover, G4LogicalVolume* sector) const;

      /**
       * Create sector support structure (main part without corners).
       * @param[in] sector Sector logical volume.
       */
      void createSectorSupport(G4LogicalVolume* mlv) const;

      /**
       * Create sector support corner 1.
       * @param[in] sector Sector logical volume.
       */
      void createSectorSupportCorner1(G4LogicalVolume* sector) const;

      /**
       * Create sector support corner 2.
       * @param[in] sector Sector logical volume.
       */
      void createSectorSupportCorner2(G4LogicalVolume* sector) const;

      /**
       * Create sector support corner 3.
       * @param[in] sector Sector logical volume.
       */
      void createSectorSupportCorner3(G4LogicalVolume* sector) const;

      /**
       * Create sector support corner 4.
       * @param[in] sector Sector logical volume.
       */
      void createSectorSupportCorner4(G4LogicalVolume* sector) const;

      /**
       * Create plane.
       * @param[in] sector Sector logical volume.
       * @return Plane logical volume.
       */
      G4LogicalVolume* createPlane(G4LogicalVolume* sector) const;

      /**
       * Create segment readout board.
       * @param[in] sector Sector logical volume.
       * @return Segment readout board logical volume.
       */
      G4LogicalVolume* createSegmentReadoutBoard(G4LogicalVolume* sector) const;

      /**
       * Create base board of segment readout board.
       * @param[in] segmentReadoutBoard Segment readout board logical volume.
       */
      void createBaseBoard(G4LogicalVolume* segmentReadoutBoard) const;

      /**
       * Create strip readout board.
       * @param[in] iBoard              Number of board.
       * @param[in] segmentReadoutBoard Segment readout board logical volume.
       */
      void createStripBoard(int iBoard,
                            G4LogicalVolume* segmentReadoutBoard) const;

      /**
       * Create segment support.
       * @param[in] iSegmentSupport Number of segment support.
       * @param[in] plane           Plane logical volume.
       */
      void createSegmentSupport(int iSegmentSupport,
                                G4LogicalVolume* plane) const;

      /**
       * Create plastic sheet.
       * @param[in] iSheetPlane Number of sheet plane.
       * @param[in] iSheet      Number of sheet (segment).
       */
      void createPlasticSheet(int iSheetPlane, int iSheet) const;

      /**
       * Create strip segment.
       * @param[in] iSegment Number of segment.
       */
      void createStripSegment(int iSegment) const;

      /**
       * Create segment (strips + plastic sheets).
       * @param[in] plane Plane logical volume.
       */
      void createSegment(G4LogicalVolume* plane) const;

      /**
       * Create strip volume.
       * @param[in] segment Segment logical volume.
       */
      void createStripVolume(G4LogicalVolume* segment) const;

      /**
       * Create strip (version for normal mode).
       * @param[in] segment Segment logical volume.
       */
      void createStrip(G4LogicalVolume* segment) const;

      /**
       * Create strip (version for background study mode).
       * @param[in] iStrip Number of strip in the length-based array.
       */
      void createStrip(int iStrip) const;

      /**
       * Create strip groove.
       * @param[in] iStrip Number of strip in the length-based array.
       */
      void createStripGroove(int iStrip) const;

      /**
       * Create scintillator.
       * @param[in] iStrip Number of strip in the length-based array.
       */
      void createScintillator(int iStrip) const;

      /**
       * Create silicon cube in the place of SiPM for radiation study.
       * @param[in] iStrip Number of strip in the length-based array.
       */
      void createSiPM(int iStrip) const;

      /**
       * Create shield.
       * @param[in] sector Sector logical volume.
       */
      void createShield(G4LogicalVolume* sector) const;

      /**
       * Create new volumes.
       */
      void newVolumes();

      /**
       * Create new transformations.
       */
      void newTransforms();

      /**
       * Create new sensitive detectors.
       */
      void newSensitive();

      /**
       * Delete volumes.
       */
      void deleteVolumes();

      /**
       * Delete transformations.
       */
      void deleteTransforms();

      /**
       * Delete sensitive detectors.
       */
      void deleteSensitive();

      /**
       * Get cutted corner angle.
       * @return Angle.
       */
      double getSectorSupportCornerAngle();

      /**
       * Calculate board transformations.
       */
      void calcBoardTransform();

      /**
       * Creation of the detector geometry.
       * @param[in] topVolume Geant world volume.
       */
      void create(G4LogicalVolume& topVolume);

      /** Solids. */
      struct Solids m_Solids;

      /** Logical volumes. */
      struct LogicalVolumes m_LogVol;

      /** Materials. */
      struct Materials m_Materials;

      /** Current volumes. */
      struct VolumeNumbers m_CurVol;

      /** Transformation data. */
      TransformData* m_TransformData;

      /** Geometry data. */
      const GeometryData* m_GeoDat;

      /** Transformations of boards from sector reference frame. */
      G4Transform3D* m_BoardTransform[2];

      /** Sensitive detectors. */
      EKLMSensitiveDetector* m_Sensitive[3];

    };

  }

}

#endif

