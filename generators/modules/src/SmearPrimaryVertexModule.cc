/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <generators/modules/SmearPrimaryVertexModule.h>

/* Belle II headers. */
#include <framework/dataobjects/MCInitialParticles.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SmearPrimaryVertex)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SmearPrimaryVertexModule::SmearPrimaryVertexModule() : Module(),
  m_Initial(BeamParameters::c_smearVertex)
{
  /* Module description. */
  setDescription("Smears primary vertex and all subsequent vertices of all the MCParticles using the informations stored in BeamParameters.");
  /* Parameters definition. */
  addParam("MCParticlesStoreArrayName", m_MCParticlesName, "Name of the MCParticles StoreArray.", std::string(""));
}

SmearPrimaryVertexModule::~SmearPrimaryVertexModule()
{
}

void SmearPrimaryVertexModule::initialize()
{
  StoreArray<MCParticle> mcParticles(m_MCParticlesName);
  mcParticles.isRequired(m_MCParticlesName);
  m_Initial.initialize();
}

void SmearPrimaryVertexModule::beginRun()
{
  if (not m_BeamParameters.isValid())
    B2FATAL("Beam Parameters data are not available.");
}

void SmearPrimaryVertexModule::event()
{
  StoreArray<MCParticle> mcParticles(m_MCParticlesName);
  /* Generate the primary beams. */
  MCInitialParticles& initial = m_Initial.generate();
  m_NewPrimaryVertex = initial.getVertex();
  bool primaryVertexFound = false;
  for (MCParticle& mcParticle : mcParticles) {
    /* Skip an MCParticle if it is flagged as c_Initial or c_IsVirtual. */
    if (not(mcParticle.hasStatus(MCParticle::c_Initial) or mcParticle.hasStatus(MCParticle::c_IsVirtual))
        continue;
    if (not primaryVertexFound) {
      /* Save the previous primary vertex. */
      m_OldPrimaryVertex = mcParticle.getProductionVertex();
        primaryVertexFound = true;
      }
    /* Shift the production vertex. */
    mcParticle.setProductionVertex(getShiftedVertex(mcParticle.getProductionVertex()));
    /* Shift also the decay vertex only if the MCParticle has a daughter. */
    if (mcParticle.getNDaughters() > 0)
    mcParticle.setDecayVertex(getShiftedVertex(mcParticle.getDecayVertex()));
  }
}
