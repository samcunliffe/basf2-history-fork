/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PATH_H_
#define PATH_H_

#include <framework/core/PathElement.h>

#include <boost/shared_ptr.hpp>

#include <list>


namespace Belle2 {

  class Path;
  class Module;

  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  /** Defines a pointer to a path object as a boost shared pointer. */
  typedef boost::shared_ptr<Path> PathPtr;

  /** Implements a path consisting of Module and/or Path objects. The modules are arranged in a linear order.  */
  class Path : public PathElement {

  public:

    /**
     * Constructor.
     */
    Path();

    /**
     * Destructor.
     */
    ~Path();

    /**
     * Adds a module to the path.
     *
     * The module is added to the path by inserting it to the end
     * of the list of modules.
     *
     * See 'pydoc3 basf2.Path' for the complete documentation.
     *
     * @param module Module that should be added to the path.
     */
    void addModule(boost::shared_ptr<Module> module);

    /** Returns true if this Path doesn't contain any elements. */
    bool isEmpty() const;

    /**
     * Returns a list of the modules in this path. (Recursively searches sub-paths)
     */
    std::list<boost::shared_ptr<Module> > getModules() const override;

    /**
     * Builds a list of all modules which could be executed during the data processing.
     *
     * The method starts with the current path, iterates over the modules in the path and
     * follows recursively module conditions to make sure the final list contains all
     * modules which could be executed while preserving their correct order.
     *
     * @param unique If true, the list will be unique.
     * @return A list containing all modules which could be executed during the data processing.
     */
    std::list<boost::shared_ptr<Module> > buildModulePathList(bool unique = true) const;

    /**
     * Replaces all Modules and sub-Paths with the specified Module list
     */
    void putModules(const std::list<boost::shared_ptr<Module> >& mlist);

    /**
     * Does this Path contain a module of the given type?
     *
     * Useable in Python via '"ModuleType" in path' syntax.
     */
    bool contains(std::string moduleType) const;

    /** Create an independent copy of this path, recreating all contained modules with the same parameters.
     *
     * Note that parameters are shared, so changing them on a module in the cloned path will also affect
     * the module in the original path.
     */
    boost::shared_ptr<PathElement> clone() const override;


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /** See 'pydoc3 basf2.Path' */
    void addPath(PathPtr path);

    /** See 'pydoc3 basf2.Path' */
    void forEach(std::string loopObjectName, std::string arrayName, PathPtr path);

    /** See 'pydoc3 basf2.Path' */
    void addSkimPath(PathPtr skim_path, std::string ds_ID);

    /** return a string of the form [module a -> module b -> [another path]]
     *
     *  can be used to 'print' a path in a steering file.
     */
    std::string getPathString() const override;

    /** Exposes methods of the Path class to Python. */
    static void exposePythonAPI();


  private:

    std::list<boost::shared_ptr<PathElement> > m_elements; /**< The list of path elements (Modules and sub-Paths) */

    friend class PathIterator;
  };

} // end namespace Belle2

#endif /* PATH_H_ */
