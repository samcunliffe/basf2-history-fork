/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Class to store slow data unpacked from raw data
   */

  class TOPSlowData : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPSlowData()
    {}

    /**
     * Full constructor
     */
    TOPSlowData(unsigned short scrodID, short type, short value):
      m_scrodID(scrodID), m_type(type), m_value(value)
    {}

    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned getScrodID() const {return m_scrodID;}

    /**
     * Returns slow data type
     * @return type
     */
    int getType() const {return m_type;}

    /**
     * Returns slow data value
     * @return value
     */
    int getValue() const {return m_value;}

  private:

    unsigned short m_scrodID = 0; /**< SCROD ID */
    short m_type = 0; /**< slow data type */
    short m_value = 0; /**< slow data value */

    ClassDef(TOPSlowData, 1); /**< ClassDef */


  };

}
