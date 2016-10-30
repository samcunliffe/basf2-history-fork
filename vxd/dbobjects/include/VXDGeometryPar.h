/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>
#include <map>
#include <vector>

#include <vxd/dbobjects/VXDGlobalPar.h>
#include <vxd/dbobjects/VXDAlignmentPar.h>
#include <vxd/dbobjects/VXDEnvelopePar.h>
#include <vxd/dbobjects/VXDHalfShellPar.h>

#include <vxd/dbobjects/VXDGeoComponentPar.h>
#include <vxd/dbobjects/VXDGeoSensorPar.h>
#include <vxd/dbobjects/VXDGeoLadderPar.h>
#include <vxd/dbobjects/VXDGeoRadiationSensorsPar.h>
#include <vxd/dbobjects/VXDSensorInfoBasePar.h>

namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class VXDGeometryPar: public TObject {
  public:
    //! Default constructor
    VXDGeometryPar() {}
    //! Constructor using Gearbox
    explicit VXDGeometryPar(const std::string& prefix, const GearDir& content) { read(prefix, content); }
    //! Destructor
    virtual ~VXDGeometryPar() {}
    //! Get geometry parameters from Gearbox
    void read(const std::string&, const GearDir&);

    /**
     * Return vector of VXDGeoPlacements with all the components defined inside a given path
     */
    std::vector<VXDGeoPlacementPar> getSubComponents(GearDir path);

    /**
     * Read the sensor definitions from the database
     * @param sensor Reference to the database containing the parameters
     */
    virtual VXDSensorInfoBasePar* createSensorInfo(const GearDir& sensor) = 0;

    /**
     * Create support structure for VXD Half Shell, that means everything
     * thagt does not depend on layer or sensor alignment
     * @param support Reference to the database containing the parameters
     */
    virtual void createHalfShellSupport(GearDir support) = 0;

    /**
     * Create support structure for a VXD Layer
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    virtual void createLayerSupport(int layer, GearDir support) = 0;

    /**
     * Create support structure for a VXD Ladder
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    virtual void createLadderSupport(int layer, GearDir support) = 0;

    /**
     * Read parameters for a ladder in layer with given ID from gearbox and layer
     * store them in chache m_ladders
     */
    virtual void cacheLadder(int layer, GearDir components);

    /**
     * Read parameters for ladder components and their alignment corresponding
     * to the given ladder id
     */
    virtual void readLadderInfo(int layerID, int ladderID, GearDir content);

    /**
     * Read parameters for component name from Gearbox into m_components cache.
     * The name is assumed to be unique and Volumes are cached.
     * @param name Name of the component
     * @param components Path to components
     */
    void cacheComponent(const std::string& name, GearDir components);

    /**
     * Read parameters for all components in placement container from Gearbox
     * into m_components cache.
     * @param placements container holding names of all components to be cached
     * @param componentDir Path to Gearbox where parameters are to be found
     */
    void cacheSubComponents(std::vector<VXDGeoPlacementPar> placements , GearDir componentsDir);

  private:

    /** Prefix to prepend to all volume names */
    std::string m_prefix {""};
    /** Container for a few general parameters */
    VXDGlobalPar m_globals;
    /** Alignment parameters for all components */
    std::map<std::string, VXDAlignmentPar> m_alignment;
    /** Detector envelope parameters */
    VXDEnvelopePar m_envelope;
    /** Cache for half shells  */
    std::map<std::string, VXDHalfShellPar> m_halfShells;
    /** Cache of all previously created components */
    std::map<std::string, VXDGeoComponentPar> m_componentCache;
    /** Map containing Information about all defined sensor types */
    std::map<std::string, VXDGeoSensorPar> m_sensorMap;
    /** Diamond radiation sensor "sub creator" */
    VXDGeoRadiationSensorsPar m_radiationsensors;
    /** Parameters of the detector ladders */
    std::map<int, VXDGeoLadderPar> m_ladders;
    /** Cache of all Bkg sensitive chip Ids*/
    std::map<std::string, int> m_sensitiveIDCache;


    ClassDef(VXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

