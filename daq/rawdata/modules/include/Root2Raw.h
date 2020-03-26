//+
// File : Root2Raw.h
// Description : Module to convert recorded ROOT raw data into raw data file
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Jun - 2014
//-

#ifndef ROOT2RAW_H
#define ROOT2RAW_H

#include <framework/core/Module.h>

#define MAXEVTSIZE 4000000


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Root2RawModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Root2RawModule();
    virtual ~Root2RawModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    //!
    virtual void endRun();

    //!
    virtual void terminate();

  protected :
    //!
    int m_file;
    std::string m_filename;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
