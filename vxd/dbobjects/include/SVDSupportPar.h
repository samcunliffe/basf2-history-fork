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

#include <vxd/dbobjects/SVDCoolingPipesPar.h>
#include <vxd/dbobjects/SVDSupportRibsPar.h>
#include <vxd/dbobjects/SVDEndringsPar.h>
#include <vxd/dbobjects/VXDRotationSolidPar.h>

#include <TObject.h>
#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class GearDir;

  class SVDHalfShellPar: public TObject {

  public:

    //! Default constructor
    SVDHalfShellPar() {}
    //! Constructor using Gearbox
    explicit SVDHalfShellPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDHalfShellPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
    //! Get half shell
    std::vector<VXDRotationSolidPar> getRotationSolid(void) const { return m_rotationSolid; }

  private:
    //! SVD Half Shell
    std::vector<VXDRotationSolidPar> m_rotationSolid;

    ClassDef(SVDHalfShellPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


  /**
  * The Class for SVD support
  */

  class SVDSupportPar: public TObject {

  public:

    //! Default constructor
    SVDSupportPar() {}
    //! Constructor using Gearbox
    explicit SVDSupportPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDSupportPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    SVDHalfShellPar m_halfShell;
    SVDEndringsPar m_endrings;
    SVDSupportRibsPar m_supportRibs;
    SVDCoolingPipesPar m_coolingPipes;

    ClassDef(SVDSupportPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

