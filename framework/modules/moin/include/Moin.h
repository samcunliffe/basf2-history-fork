// This is a template header file module for the basf2 framework
// it contains a class that should be derived from some basf2 module base class

#ifndef MODULEMOIN_H
#define MODULEMOIN_H

#include <fwcore/Module.h>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <string>
#include <vector>
#include <TRandom3.h>

#include <iostream>
#include <fstream>

#include <datastore/StoreDefs.h>
#include <datastore/StoreObjPtr.h>
#include <datastore/StoreArray.h>
#include <datastore/DataStore.h>
#include <datastore/SimpleVec.h>
#include <datastore/Relation.h>
#include <hitcdc/HitCDC.h>

#include <TDatabasePDG.h>
#include <TParticlePDG.h>


namespace Belle2 {

  //! Module Moin
  /*!
  */
  class ModuleMoin : public Module {

  public:

    //! Macro which adds a method to return a new instance of the module.
    NEW_MODULE(ModuleMoin)

    //! Constructor
    ModuleMoin();

    //! Destructor
    virtual ~ModuleMoin();

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.

        This method has to be implemented by subclasses.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    virtual void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        This method has to be implemented by subclasses.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.

        This method has to be implemented by subclasses.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    virtual void terminate();


  protected:


  private:

    int m_evtNum;

    TRandom3 random;

    std::fstream outputstream;



  };

} // end namespace Belle2

#endif // MODULEMOIN_H
