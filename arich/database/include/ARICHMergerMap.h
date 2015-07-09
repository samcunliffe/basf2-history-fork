/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
class ARICHMergerInfo;
class ARICHB2LinkInfo;
class ARICHCableInfo;
namespace Belle2 {
  /**
  *   Mapping of the merger board to the detector
  */
  class ARICHMergerMap: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHMergerMap(): m_position(0), m_sextant(0) {};

    /**
     * Constructor
     */
    ARICHMergerMap(int position, int sextant)
    {
      m_position = position;
      m_sextant = sextant;
    }

    /**
     * Destructor
     */
    ~ARICHMergerMap() {};

    /** Return Merger Position
     * @return Merger Position
     */
    int getMergerPosition() const {return m_position; }

    /** Set Merger Position
     * @param Merger Position
     */
    void setMergerPosition(int position) {m_position = position; }

    /** Return Merger sextant
     * @return Merger sextant
     */
    int getMergerSextant() const {return m_sextant; }

    /** Set Merger sextant
     * @param Merger sextant
     */
    void setMergerSextant(int sextant) {m_sextant = sextant; }

    /** Return Merger board Identifier
     * @return Merger board Identifier
     */

    ARICHMergerInfo getMergerID() const {return m_merger; }

    /** Set Merger board Identifier
     * @param Merger board Identifier
     */
    void setMergerID(ARICHMergerInfo merger) {m_merger = merger; }


    /** Return B2Link Identifier
     * @return B2Link Identifier
     */

    ARICHB2LinkInfo getB2LinkID() const {return m_b2Link; }

    /** Set B2Link Identifier
     * @param B2Link Identifier
     */
    void setB2LinkID(ARICHB2LinkInfo b2Link) {m_b2Link = b2Link; }


    /** Return board Identifier
     * @return board Identifier
     */

    ARICHCableInfo getCableID() const {return m_cable; }

    /** Set cable Identifier
     * @param cable Identifier
     */
    void setCableID(ARICHCableInfo cable) {m_cable = cable; }



  private:
    int m_position;              /**< Position in the detector */
    int m_sextant;               /**< Sextant identification   */
    ARICHMergerInfo m_merger;   /**< Merger Board identifier */
    ARICHB2LinkInfo m_b2Link;   /**< B2Link Identifier */
    ARICHCableInfo  m_cable;    /**< Cable Identifier */

    ClassDef(ARICHMergerMap, 1);  /**< ClassDef */
  };
} // end namespace Belle2

