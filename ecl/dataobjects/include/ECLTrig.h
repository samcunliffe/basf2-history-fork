/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLTRIG_H
#define ECLTRIG_H

#include <framework/datastore/DataStore.h>


namespace Belle2 {

  //! Example Detector
  class ECLTrig : public TObject {
  public:


    //! The cell id of this hit.
    int m_cellId;

    //!  Fit time of this hit.
    double m_TimeTrig;

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to get Fitt time
    void setTimeTrig(double TimeTrig) { m_TimeTrig = TimeTrig; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get Fit time
    double getTimeTrig() const { return m_TimeTrig; }


    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLTrig() {;}

    ClassDef(ECLTrig, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
