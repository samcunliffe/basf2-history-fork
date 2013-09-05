/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergey Yashchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/CosmicsModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace boost::assign;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Cosmics)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CosmicsModule::CosmicsModule() : Module()
{
  // Set module properties
  setDescription("cosmics generator to generate cosmic ray tracks");
  setPropertyFlags(c_Input);
  // Set default values for parameters
  m_parameters.level = 1;
  m_parameters.ipRequirement = 0;
  m_parameters.ipdr = 3.; // Only relevant for ipRequirement = 1
  m_parameters.ipdz = 3.; // Only relevant for ipRequirement = 1
  m_parameters.ptmin = 0.7;
  // Parameter definition
  addParam("level", m_parameters.level,
           "level of the cosmic generator (1 or 2), default is 1", m_parameters.level);
  addParam("ipRequirement", m_parameters.ipRequirement,
           "ipRequirement, default is 0", m_parameters.ipRequirement);
  addParam("ipdr", m_parameters.ipdr,
           "ipdr, default is 3.", m_parameters.ipdr);
  addParam("ipdz", m_parameters.ipdz,
           "ipdz, default is 3.", m_parameters.ipdz);
  addParam("ptmin", m_parameters.ptmin,
           "ptmin, default is 0.7", m_parameters.ptmin);
}

void CosmicsModule::initialize()
{
  // Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();
  m_cosmics.setParameters(m_parameters);
}


void CosmicsModule::event()
{
  try {
    m_particleGraph.clear();
    m_cosmics.generateEvent(m_particleGraph);
    m_particleGraph.generateList();
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }
}

