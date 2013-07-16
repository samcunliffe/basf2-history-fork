/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>

using namespace Belle2;
using namespace std;


Environment& Environment::Instance()
{
  static Environment instance;
  return instance;
}


const list<string>& Environment::getModuleSearchPaths() const
{
  return ModuleManager::Instance().getModuleSearchPaths();
}


//============================================================================
//                              Private methods
//============================================================================

Environment::Environment() :
  m_numberProcesses(0),
  m_steering(""),
  m_numberEventsOverride(0),
  m_inputFileOverride(""),
  m_outputFileOverride(""),
  m_numberProcessesOverride(-1),
  m_visualizeDataFlow(false),
  m_noStats(false)
{

}


Environment::~Environment()
{
}
