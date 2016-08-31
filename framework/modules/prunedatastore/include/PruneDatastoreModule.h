/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 **************************************************************************/

#ifndef PruneDatastoreModule_H
#define PruneDatastoreModule_H

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <boost/regex.hpp>

namespace Belle2 {
  /**
   * Clears the content of the DataStore while it keeps entries listed in the keepEntries option
   */
  class PruneDatastoreModule: public Module {

  public:

    /** Constructor */
    PruneDatastoreModule();

    /** Virtual Constructor to prevent memory leaks */
    virtual ~PruneDatastoreModule() = default;

    /** Prepare regex checks */
    void initialize() override;

    /** Prune datastore */
    void event() override;

  protected:
    /** Storing the option of branches to keep */
    std::vector<std::string> m_keepEntries;

    /** Caching the regex expression for the keep check */
    std::vector < boost::regex > m_compiled_regex;

  };
} // end namespace Belle2

#endif // PruneDatastoreModule_H
