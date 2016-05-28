/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCCOSMICSELECTOR_H
#define CDCCOSMICSELECTOR_H

//basf2 framework headers
#include <framework/core/Module.h>

//C++/C standard lib elements.
//#include <string>

namespace Belle2 {

  /** The Class for.
   *
   */
  class CDCCosmicSelectorModule : public Module {

  public:
    /** Constructor.*/
    CDCCosmicSelectorModule();

    /** Initialize variables, print info, and start CPU clock. */
    void initialize();

    /** Actual digitization of all hits in the CDC.
     *
     *  The digitized hits are written into the DataStore.
     */
    void event();

  private:
    double m_xOfCounter;  /**< x-pos. of counter (cm) */
    double m_yOfCounter;  /**< y-pos. of counter (cm) */
    double m_zOfCounter;  /**< z-pos. of counter (cm) */
    double m_wOfCounter;  /**< full-width  of counter (cm) */
    double m_lOfCounter;  /**< full-length of counter (cm) */
    int    m_tof;         /**< tof option */
  };

} // end of Belle2 namespace

#endif // CDCCOSMICSELECTOR_H
