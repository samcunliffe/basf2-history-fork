/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBStore.h>

#include <string>
#include <utility>

class TClass;

namespace Belle2 {

  /**
   * Base class for DBObjPtr and DBArray for easier common treatment.
   */
  class DBAccessorBase {
  public:

    /**
     * Constructor to access an object in the DBStore.
     * @param name       Name under which the object is stored in the database (and DBStore).
     * @param objClass   The type of the object.
     * @param isArray    Flag that indicates whether this is a single object or a TClonesArray.
     */
    DBAccessorBase(const std::string& package,
                   const std::string& module,
                   const TClass* objClass,
                   bool isArray)
    {
      m_entry = DBStore::Instance().getEntry(package, module, objClass, isArray);
      m_iov = m_entry->iov;
    };

    /**
     * Destructor.
     * Virtual because this is a base class.
     */
    virtual ~DBAccessorBase() {};

    /**
     * Return package name under which the object is saved in the DBStore.
     */
    const std::string& getPackage() const { return m_entry->package; }

    /**
     * Return module name under which the object is saved in the DBStore.
     */
    const std::string& getModule() const { return m_entry->module; }

    /**
     * Return package + module name under which the object is saved in the DBStore.
     */
    std::string getName() const { return getPackage() + "/" + getModule(); }

    /**
     * Check whether a valid object was obtained from the database.
     * @return          True if the object exists.
     **/
    inline bool isValid() const {return m_entry->object;}

    inline operator bool()  const {return isValid();}   /**< Imitate pointer functionality. */

    /**
     * Check if two store accessors point to the same object/array.
     */
    virtual bool operator==(const DBAccessorBase& other)
    {
      return getPackage() == other.getPackage() && getModule() == other.getModule();
    }

    /**
     * Check if two store accessors point to a different object/array.
     */
    virtual bool operator!=(const DBAccessorBase& other)
    {
      return !(*this == other);
    }

    /**
     * Check whether the object has changed since the last call to hasChanged  of the accessor).
     */
    bool hasChanged()
    {
      bool result = ((m_iov != m_entry->iov) || (m_ptr != m_entry->object));
      m_iov = m_entry->iov;
      m_ptr = m_entry->object;
      return result;
    }

  protected:
    /** Pointer to the entry in the DBStore. */
    DBEntry* m_entry;

    /** IoV at last call to hasChanged. */
    IntervalOfValidity m_iov;

    /** object pointer at last call to hasChanged. */
    TObject* m_ptr;

  };
}
