/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * Mapping of a boardstack number within a module to SCROD and COPPER/Finesse
   */
  class TOPFrontEndMap: public TObject {

  public:

    /**
     * Default constructor
     */
    TOPFrontEndMap()
    {}

    /**
     * Full constructor
     * @param barID TOP module ID
     * @param boardstack Boardstack number within TOP module
     * @param scrod SCROD ID
     * @param copper COPPER ID
     * @param finesse Finesse slot number
     * @param i index of this element in std::vector
     */
    TOPFrontEndMap(int barID,
                   int boardstack,
                   unsigned short scrod,
                   unsigned copper,
                   int finesse,
                   int i)
    {
      m_barID = barID;
      m_boardstack = boardstack;
      m_scrodID = scrod;
      m_copperID = copper;
      m_finesse = finesse;
      m_index = i;
    }

    /**
     * Return TOP module ID
     * @return bar ID
     */
    int getBarID() const {return m_barID;}

    /**
     * Return boardstack number
     * @return boardstack number
     */
    int getBoardstackNumber() const {return m_boardstack;}

    /**
     * Return SCROD ID
     * @return SCROD ID
     */
    unsigned short getScrodID() const {return m_scrodID;}

    /**
     * Return Copper ID
     * @return Copper ID
     */
    unsigned int getCopperID() const {return m_copperID;}

    /**
     * Return finesse slot number
     * @return finesse slot
     */
    int getFinesseSlot() const {return m_finesse;}

    /**
     * Return array index
     * @return array index
     */
    int getIndex() const {return m_index;}

  private:

    int m_barID = 0;              /**< module ID */
    int m_boardstack = 0;         /**< boardstack number within TOP module */
    unsigned short m_scrodID = 0; /**< SCROD ID */
    unsigned int m_copperID = 0;  /**< COPPER ID */
    int m_finesse = 0;          /**< Finesse slot number (0-based) */
    int m_index = 0;              /**< index of this element in std::vector */

    ClassDef(TOPFrontEndMap, 1); /**< ClassDef */

  };

} // end namespace Belle2
