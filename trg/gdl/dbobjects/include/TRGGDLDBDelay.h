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

#include <TObject.h>
#include <unordered_map>
#include <vector>

namespace Belle2 {



  /** The payload class for delay of GDL input bit
   *
   *  The payload class stores the number of input bit and their delay
   *  value of a missing delay is "-1"
   */


  class TRGGDLDBDelay: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBDelay(): m_ninbit{0}, m_delay{0} {}
    /** copy constructor */
    TRGGDLDBDelay(const TRGGDLDBDelay& b)
    {
      m_ninbit = b.m_ninbit;
      for (int i = 0; i < 320; i++) {
        m_delay[i] = b.m_delay[i];
      }
    }
    /** assignment operator */
    TRGGDLDBDelay& operator=(const TRGGDLDBDelay& b)
    {
      m_ninbit = b.m_ninbit;
      for (int i = 0; i < 320; i++) {
        m_delay[i] = b.m_delay[i];
      }
      return *this;
    }

    void setninbit(int i)
    {
      m_ninbit = i;
    }
    void setdelay(int i, const int j)
    {
      m_delay[i] = j;
    }

    int getninbit() const
    {
      return m_ninbit;
    }
    int getdelay(int i) const
    {
      return m_delay[i];
    }

  private:

    /** Number of bit */
    int m_ninbit;

    /** Number of bins per sensor along u side */
    int m_delay[320];

    ClassDef(TRGGDLDBDelay, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
