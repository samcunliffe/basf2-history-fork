/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/ContinuumSuppressionBuilder/ContinuumSuppressionBuilderModule.h>

#include <analysis/ContinuumSuppression/ContinuumSuppression.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ContinuumSuppressionBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ContinuumSuppressionBuilderModule::ContinuumSuppressionBuilderModule() : Module()
{
  // Set module properties
  setDescription("Creates for each Particle in the given ParticleLists a ContinuumSuppression dataobject and makes BASF2 relation between them.");

  // Parameter definitions
  addParam("particleList", m_particleListName, "Name of the ParticleList", std::string(""));

  addParam("ROEMask", m_ROEMask, "ROE mask", std::string(""));

}

void ContinuumSuppressionBuilderModule::initialize()
{
  // Input
  m_plist.isRequired(m_particleListName);
  StoreArray<Particle>().isRequired();

  // Output
  m_csarray.registerInDataStore();
  StoreArray<Particle>().registerRelationTo(m_csarray);
}

void ContinuumSuppressionBuilderModule::event()
{
  for (unsigned i = 0; i < m_plist->getListSize(); i++) {
    addContinuumSuppression(m_plist->getParticle(i), m_ROEMask);      // pass the ROEMask to cs.cc here as a second argument.
  }
}

