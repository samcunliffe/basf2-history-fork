/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * A local maximum is defined by its energy and the number and energy of  *
 * its neighbours. It holds a relation to to an ECLConnectedRegion and    *
 * a list of ECLCalDigits (set in SplitterN1).                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLLOCALMAXIMUM_H
#define ECLLOCALMAXIMUM_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /*! Class to store local maxima (LM)
   */

  class ECLLocalMaximum : public RelationsObject {
  public:

    /** Enumerator for LM types */
    enum {
      c_photon = 1, /**< photon */
      c_electron = 2, /**< electron */
      c_mergedpi0 = 3, /**< merged pi0 */
    };
    static const size_t c_nTypes = 3;

    /** default constructor for ROOT */
    ECLLocalMaximum()
    {
      m_LMId    = -1; /**< LM identifier */
      m_Type    = -1; /** LM type  (photon, electron, merged pi0...) */
      m_CellId  = -1; /** Cell Id */
    }

    // setters
    /*! Set LM identifier
     */
    void setLMId(int LMId) { m_LMId = LMId; }

    /*! Set CellId.
     */
    void setCellId(int cellid) { m_CellId = cellid; }

    /*! Set type.
     */
    void setType(int type) { m_Type = type; }

    // getters
    /*! Get LM identifier.
     * @return LMId
     */
    int getLMId() const { return m_LMId; }

    /*! Get type.
     * @return type
     */
    int getType() const { return m_Type; }

    /*! Get CellId.
     * @return cellid
     */
    int getCellId() const { return m_CellId; }

  private:
    int m_LMId;  /**< LM ID */
    int m_Type; /** LM type  (photon, electron, merged pi0...) */
    int m_CellId; /** Cell Id */

    // 1: Initial version.
    // 2: Added identifier for LM type (photon, electron, merged pi0...).
    // 2: Removed bulky extra information and added CellId.
    ClassDef(ECLLocalMaximum, 3); /**< ClassDef */

  };

} // end namespace Belle2

#endif
