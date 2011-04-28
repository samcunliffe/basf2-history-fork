/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <list>
#include <string>

namespace Belle2 {

  /**
   * The Environment Class.
   *
   * This class stores all environment information required to run the
   * framework, such as module or data filepaths, number of processes to be used
   * in parallel processing etc. Usually, this information is set by the steering file.
   *
   * This class is designed as a singleton.
   */
  class Environment {

  public:

    /**
     * Static method to get a reference to the Environment instance.
     *
     * @return A reference to an instance of this class.
     */
    static Environment& Instance();

    /**
     * Returns a reference to the list of the modules search filepaths.
     *
     * @return A reference to the list of the modules search filepaths.
     */
    const std::list<std::string>& getModuleSearchPaths() const;

    /**
     * Sets the path which points to the data directory of the framework.
     *
     * @param dataPath Path in which the data files for the framework are located.
     */
    void setDataSearchPath(const std::string& dataPath) { m_dataSearchPath = dataPath; };

    /**
     * Returns the path which points to the data directory of the framework.
     *
     * @return The path in which the data files for the framework are located.
     */
    const std::string getDataSearchPath() { return m_dataSearchPath; };

    /**
     * Sets the number of processes which should be used for the parallel processing.
     * If the value is set to 0, no parallel processing will be used in the event loop.
     *
     * @param number The number of processes used for the parallel processing.
     */
    void setNumberProcesses(int number) { m_numberProcesses = number; }

    /**
     * Returns the number of processors which should be used for the parallel processing.
     *
     * @return  The number of processors used for the parallel processing.
     */
    int getNumberProcesses() { return m_numberProcesses; }


  private:

    std::string m_dataSearchPath; /**< The path in which the data files, such as geometry files, are stored. */
    int m_numberProcesses;        /**< The number of processes that should be used for the parallel processing. */

    /**
     * The constructor is hidden to avoid that someone creates an instance of this class.
     */
    Environment();

    /**
     * Disable/Hide the copy constructor.
     */
    Environment(const Environment&);

    /**
     * Disable/Hide the copy assignment operator.
     */
    Environment& operator=(const Environment&);

    /**
     * The Environment destructor.
     * Deletes the Environment.
     */
    ~Environment();

    static Environment* m_instance; /**< Pointer that saves the instance of this class. */

    /**
     * Destroyer class to delete the instance of the Environment class when the program terminates.
     */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (Environment::m_instance != NULL) delete Environment::m_instance;
      }
    };
    friend class SingletonDestroyer;

  };

} //end of namespace Belle2

#endif /* ENVIRONMENT_H */
