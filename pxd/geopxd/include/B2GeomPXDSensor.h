/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2GEOMPXDSENSOR_H_
#define B2GEOMPXDSENSOR_H_

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>
#include <pxd/geopxd/B2GeomVolume.h>

#include <string>
#include "TGeoMaterial.h"
#include "TROOT.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TGeoTrd2.h"
#include "TGeoCompositeShape.h"


using namespace std;

namespace Belle2 {
  class GearDir;

  /** Class for building the active silicon of the PXD sensor. */
  class B2GeomPXDSensorActive : public B2GeomVXDVolume {
  public:
    /** Constructor for the active sensor */
    B2GeomPXDSensorActive() { }
    /** Destructor for the active sensor */
    virtual ~B2GeomPXDSensorActive() { }
    /** Reads parameters for PXD active silicon (inclusive UserInfo) from GearBox. */
    Bool_t init(GearDir& content);
    /** Build the geometry and adds PXDUserInfo for the active silicon. */
    Bool_t make();
  };

  /** Class to build the silicon of the PXD Sensor. */
  class B2GeomPXDSensorSilicon: public B2GeomVXDVolume {
  private:
    B2GeomPXDSensorActive* volActive; /** < Object for building the active volume */
    B2GeomVXDVolume* volThinned; /** < Object for building the thinning. */
  public:
    /** Constructor for the Silicon of the PXD. */
    B2GeomPXDSensorSilicon();
    /** Destructor for the Silicon of the PXD. */
    virtual ~B2GeomPXDSensorSilicon() {}
    /** Reads Parameters for PXD Silicon from GearBox. */
    Bool_t init(GearDir& content);
    /** Builds the geometry of the PXD sensor silicon. */
    Bool_t make();
  };

  /** Class to build the PXD Switchers (DCDs). */
  class B2GeomPXDSensorSwitchers1 : public B2GeomVXDVolume {
  private:
    B2GeomVXDVolume* volDCD1; /** < Object for building the switcher including DCD1. */
    B2GeomVXDVolume* volDCD2; /** < Object for building the switcher including DCD2. */
    B2GeomVXDVolume* volDCD3; /** < Object for building the switcher including DCD3. */
    B2GeomVXDVolume* volDCD4; /** < Object for building the switcher including DCD4. */
    B2GeomVXDVolume* volDCD5; /** < Object for building the switcher including DCD5. */
    B2GeomVXDVolume* volDCD6; /** < Object for building the switcher including DCD6. */

    B2GeomVXDVolume* volDHP1; /** < Object for building the switcher including DHP1. */
    B2GeomVXDVolume* volDHP2; /** < Object for building the switcher including DHP2. */
    B2GeomVXDVolume* volDHP3; /** < Object for building the switcher including DHP3. */
    B2GeomVXDVolume* volDHP4; /** < Object for building the switcher including DHP4. */
    B2GeomVXDVolume* volDHP5; /** < Object for building the switcher including DHP5. */
    B2GeomVXDVolume* volDHP6; /** < Object for building the switcher including DHP6. */

  public:
    /** Constructor for PXD Sensor. */
    B2GeomPXDSensorSwitchers1();
    /** Destructor for PXD Sensor. */
    virtual ~B2GeomPXDSensorSwitchers1() { }
    /** Reads parameters from GearBox and creates objects for sub components. */
    Bool_t init(GearDir& content);
    /** Builds the PXD sensor geometry. */
    Bool_t make();
  };

  /** Class to build the PXD Switchers (DCDs). */
  class B2GeomPXDSensorSwitchers2 : public B2GeomVXDVolume {
  private:
    B2GeomVXDVolume* volSwitcher1; /** < Object for building the switcher including DCD1. */
    B2GeomVXDVolume* volSwitcher2; /** < Object for building the switcher including DCD2. */
    B2GeomVXDVolume* volSwitcher3; /** < Object for building the switcher including DCD3. */
    B2GeomVXDVolume* volSwitcher4; /** < Object for building the switcher including DCD4. */
    B2GeomVXDVolume* volSwitcher5; /** < Object for building the switcher including DCD5. */
    B2GeomVXDVolume* volSwitcher6; /** < Object for building the switcher including DCD6. */

  public:
    /** Constructor for PXD Sensor. */
    B2GeomPXDSensorSwitchers2();
    /** Destructor for PXD Sensor. */
    virtual ~B2GeomPXDSensorSwitchers2() { }
    /** Reads parameters from GearBox and creates objects for sub components. */
    Bool_t init(GearDir& content);
    /** Builds the PXD sensor geometry. */
    Bool_t make();
  };

  /** Class to build the PXD Sensor (silicon, active silicon, thinning, switchers). */
  class B2GeomPXDSensor : public B2GeomVXDStructVolume<B2GeomPXDSensor> {
  private:
    B2GeomPXDSensorSilicon* volSilicon; /** < Object for building the silicon including thinning and active volume. */
    B2GeomPXDSensorSwitchers1* volSwitchers1; /** < Object for building the switchers. */
    B2GeomPXDSensorSwitchers2* volSwitchers2; /** < Object for building the switchers. */

  public:
    /** Constructor for PXD Sensor. */
    B2GeomPXDSensor();
    /** Destructor for PXD Sensor. */
    virtual ~B2GeomPXDSensor() { }
    /** Reads parameters from GearBox and creates objects for sub components. */
    Bool_t init(GearDir& content);
    /** Builds the PXD sensor geometry. */
    Bool_t make();
  };

}
#endif
