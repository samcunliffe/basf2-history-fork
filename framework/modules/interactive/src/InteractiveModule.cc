/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <boost/python.hpp>

#include <framework/modules/interactive/InteractiveModule.h>

#include <framework/logging/Logger.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Interactive)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

InteractiveModule::InteractiveModule() : Module()
{
  setDescription("Start an interactive python shell in each call of event().  Also imports the ROOT.Belle2 namespace for convenience, allowing you to use Belle2.PyStoreArray etc. directly.");
}

InteractiveModule::~InteractiveModule()
{
}

void InteractiveModule::initialize()
{
  if (PyRun_SimpleString("import interactive") == -1) {
    B2FATAL("'import interactive' failed.");
  }
  if (PyRun_SimpleString("from ROOT import Belle2") == -1) {
    B2FATAL("'from ROOT import Belle2' failed.");
  }
}
void InteractiveModule::event()
{
  if (PyRun_SimpleString("interactive.embed()") == -1) {
    B2ERROR("embed() failed!");
  }
}
