/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#ifndef GEOSVDBELLEII_H_
#define GEOSVDBELLEII_H_

namespace Belle2 {

  class GearDir;

  /** The GeoSVDBelleII class.
   *  The creator for the SVD geometry of the Belle II detector.
   */
  class GeoSVDBelleII : public CreatorBase {

  public:
    /** Constructor of the GeoSVDBelleII class. */
    GeoSVDBelleII();

    /** The destructor of the GeoSVDBelleII class. */
    virtual ~GeoSVDBelleII();

    /** Creates the ROOT Objects for the SVD geometry.
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    virtual void create(GearDir& content);
  };


}

#endif /* GEOSVDBELLEII_H_ */
