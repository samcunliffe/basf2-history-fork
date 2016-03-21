/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBArray.h>

namespace Belle2 {

  std::string emptyString = "";

  /**
   * Class for pointing to an element in an array stored in the database.
   */
  template<class T, typename KEY, KEY(T::*METHOD)() const, std::string& NAME = emptyString> class DBPointer {
  public:

    /**
     * Constructor of pointer to an array element in the database.
     * @param key       The key value to identify the element.
     */
    explicit DBPointer(KEY key):
      m_key(key), m_object(0) {};

    /** Accessor for key value */
    inline KEY key() const {return m_key;}

    /** Accessor for key value */
    inline operator KEY() const {return m_key;}

    /** Setter for key value */
    inline KEY operator = (KEY key) {m_key = key; m_object = 0; return m_key;}

    /**
     * Check whether we point to a valid object.
     * @return          True if the object exists.
     **/
    inline bool isValid() const {getPointer(); return m_object;}

    /** Is this pointer's data safe to access? */
    inline operator bool() const {return isValid();}

    inline T& operator *()  const {getPointer(); return *m_object;}  /**< Imitate pointer functionality. */
    inline T* operator ->() const {getPointer(); return m_object;}   /**< Imitate pointer functionality. */

  private:
    /** Find the referred object and set m_object to it */
    inline void getPointer() const
    {
      if (!m_object) {
        DBArray<T> array(NAME);
        m_object = array.getByKey(METHOD, m_key);
      }
    }

    /** Key value of the referred array element. */
    KEY m_key;

    /** Pointer to actual object. */
    mutable T* m_object;   //!
  };
}
