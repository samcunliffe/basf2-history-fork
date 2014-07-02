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

  /** Implements a path consisting of Module and/or Path objects. The modules are arranged in a linear order.
   */
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
     * @param module Module that should be added to the path.
     */
    void addModule(boost::shared_ptr<Module> module);


    /** Insert another path at the end of this one.
     *
     * E.g.
     *
       \code
       main.add_module(a)
       main.add_path(otherPath)
       main.add_module(b)
       \endcode
     *
     * would create a path [ A -> [ contents of otherPath ] -> B ].
     */
    void addPath(PathPtr path);

    /** Returns true if this Path doesn't contain any elements. */
    bool isEmpty() const;

    /**
     * Returns a list of the modules in this path.
     *
     * @return A list of all modules of this path.
     */
    std::list<boost::shared_ptr<Module> > getModules() const;

    /**
     * Builds a list of all modules which could be executed during the data processing.
     *
     * The method starts with the current path, iterates over the modules in the path and
     * follows recursively module conditions to make sure the final list contains all
     * modules which could be executed while preserving their correct order.
     * Special care is taken to avoid that a module is added more than once to the list.
     *
     * @return A list containing all modules which could be executed during the data processing.
     */
    std::list<boost::shared_ptr<Module> > buildModulePathList() const;

    /**
     * Replaces all Modules and sub-Paths with the specified Module list
     */
    void putModules(const std::list<boost::shared_ptr<Module> >& mlist);

    /**
     * similar to addPath()/add_path(), this will execute path at the current position, but
     * will run it once for each object in the given array 'foreach', and set the loop variable
     * 'objectName' (a StoreObjPtr of same type as array) to the current object.
     *
       \code
       #read: for each  $objName   in $arrayName   run over $path
       path.for_each('MCParticle', 'MCParticles', subeventpath)
       \endcode
     *
     * Changes to existing arrays / objects will be available to all modules after the for_each(),
     * including those made to the loop variable (it will simply modify the i'th item in the array looped over.)
     * Arrays / objects created inside the loop will however be limited to the validity of the loop variable. That is,
     * creating a list of Particles matching the current MCParticle (loop object) will no longer exist after switching
     * to the next MCParticle or exiting the loop.
     */
    void forEach(std::string objectName, std::string foreach, PathPtr path);


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /** return a string of the form [module a -> module b -> [another path]]
     *
     *  can be used to 'print' a path in a steering file.
     */
    virtual std::string getPathString() const;

    /** Exposes methods of the Path class to Python. */
    static void exposePythonAPI();


  private:
    /**
     * Fills the module list with the modules of this path.
     *
     * Calls itself recursively for modules having a condition
     *
     * @param modList The list of modules.
     */
    void fillModulePathList(std::list<boost::shared_ptr<Module> >& modList) const;

    std::list<boost::shared_ptr<PathElement> > m_elements; /**< The list of path elements (Modules and sub-Paths) */

    friend class PathIterator;
  };

} // end namespace Belle2

#endif /* PATH_H_ */
