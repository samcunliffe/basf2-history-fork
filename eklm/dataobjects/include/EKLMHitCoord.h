/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITCOORD_H
#define EKLMHITCOORD_H

/* Extrenal headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <TObject.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitGlobalCoord.h>

namespace Belle2 {

  /**
   * Hit coordinates.
   */
  class EKLMHitCoord : public EKLMHitGlobalCoord {

  public:

    /**
     * Constructor.
     */
    EKLMHitCoord();

    /**
     * Destructor.
     */
    virtual ~EKLMHitCoord();

    /**
     * Get local position of the particle hit.
     * @return Hit coordinates.
     */
    HepGeom::Point3D<double> getLocalPosition() const;

    /**
     * Set local position of the particle hit.
     * @param[in] lpos Hit coordinates.
     */
    void setLocalPosition(HepGeom::Point3D<double> lpos);

  protected:

    /** Local position X coordinate. */
    float m_localX;

    /** Local position Y coordinate. */
    float m_localY;

    /** Local position Z coordinate. */
    float m_localZ;

  private:

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMHitCoord, 1);

  };

}

#endif

