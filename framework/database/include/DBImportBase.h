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

#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {

  /**
   * Base class for importing objects to the database.
   * Note that the object is NOT parked at DBStore, but allocated internally.
   */
  class DBImportBase {
  public:

    /**
     * Constructor
     * @param module  Name under which the object will be stored in the database
     * @param package Package name
     */
    DBImportBase(const std::string& module, const std::string& package):
      m_module(module),
      m_package(package)
    {}

    /**
     * Destructor
     */
    virtual ~DBImportBase()
    {
      for (auto& object : m_objects) delete object;
    }

    /**
     * Returns the name under which the object will be stored in the database
     * @return name
     */
    const std::string& getName() const {return m_module;}

    /**
     * Returns the package name under which the object will be stored in the database
     * @return package name
     */
    const std::string& getPackage() const {return m_package;}

    /**
     * add event dependency
     * @param eventNumber event number
     */
    virtual void addEventDependency(unsigned int eventNumber)
    {
      addIntraRunDependency(eventNumber, c_Event);
    }

    /**
     * add time stamp dependency
     * @param timeStamp time stamp
     */
    virtual void addTimeStampDependency(unsigned long long int timeStamp)
    {
      addIntraRunDependency(timeStamp, c_TimeStamp);
    }

    /**
     * add subrun dependency
     * @param subrun subrun number
     */
    virtual void addSubrunDependency(int subrun)
    {
      addIntraRunDependency(subrun, c_Subrun);
    }

    /**
     * Import the object to database
     * @param iov interval of validity
     */
    bool import(const IntervalOfValidity& iov);

    /**
     * Clear the content, e.g. destroy allocated objects and prepare for the new DB import.
     */
    virtual void clear();

  protected:

    /**
     * Intra run dependency types (used only internaly)
     */
    enum EIntraRunDependency { c_None = 0,
                               c_Event = 1,
                               c_TimeStamp = 2,
                               c_Subrun = 3
                             };

    /**
     * add intra run dependency
     * @param tag according to type: event number or time stamp or subrun number
     * @param type intra run dependency type
     */
    void addIntraRunDependency(unsigned long long int tag, EIntraRunDependency type);

    std::string m_module;      /**< object or array name in database */
    std::string m_package;     /**< package name */
    TObject* m_object = 0;     /**< pointer to allocated object or array */

  private:

    /**
     * Hidden copy constructor.
     * To prevent making copies, since the class contains pointer to allocated memory.
     */
    DBImportBase(const DBImportBase&);

    /**
     * Import intra run dependent object to database
     * @param iov interval of validity
     */
    template<class IntraRun> bool import(const IntervalOfValidity& iov)
    {
      if (m_objects.empty()) return false;
      IntraRun intraRun(m_objects[0], false); // IntraRun must not own the objects

      if (m_object) m_objects.push_back(m_object);
      for (unsigned i = 1; i < m_objects.size(); i++) {
        intraRun.add(m_tags[i - 1], m_objects[i]);
      }
      if (m_object) m_objects.pop_back(); // restore initial state (mandatory!)

      return Database::Instance().storeData(m_package, m_module, &intraRun, iov);
    }


    EIntraRunDependency m_dependency = c_None;  /**< dependency type */
    std::vector<TObject*> m_objects; /**< container for intra run dependency objects */
    std::vector<unsigned long long int> m_tags; /**< container for intra run dep. tags */

  };
}
