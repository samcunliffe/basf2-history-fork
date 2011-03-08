/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/PrintMCParticlesModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <TDatabasePDG.h>

using namespace std;
using namespace Belle2;
using namespace Generators;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintMCParticles)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintMCParticlesModule::PrintMCParticlesModule() : Module()
{
  //Set module properties
  setDescription("Print an MCParticle Collection");

  //Parameter definition
  addParam("collectionName", m_particleList, "Collection to print", string(DEFAULT_MCPARTICLES));
  addParam("onlyPrimaries", m_onlyPrimaries, "Show only primary particles", true);
  addParam("maxLevel", m_maxLevel, "Show only up to specified depth level, -1 means no limit", -1);
}


void PrintMCParticlesModule::event()
{
  StoreArray<MCParticle> MCParticles(m_particleList);
  m_seen.clear();
  m_seen.resize(MCParticles.GetEntries() + 1, false);
  B2INFO("Content from MCParticle Collection '" + m_particleList + "':");

  //Loop over the primary particles (no mother particle exists)
  for (int i = 0; i < MCParticles.GetEntries(); i++) {
    MCParticle& mc = *MCParticles[i];
    if (mc.getMother() != NULL) continue;
    printTree(mc, 0);
  }
}


void PrintMCParticlesModule::printTree(const MCParticle& mc, int level)
{
  if (m_onlyPrimaries && !mc.hasStatus(MCParticle::c_PrimaryParticle)) return;

  //Only show up to max level
  if (m_maxLevel >= 0 && level > m_maxLevel) return;
  ++level;
  string indent = "";
  for (int i = 0; i < 2*level; i++) indent += " ";
  TDatabasePDG* pdb = TDatabasePDG::Instance();
  TParticlePDG* pdef = pdb->GetParticle(mc.getPDG());
  string name = pdef ? (string(" (") + pdef->GetTitle() + ")") : "";

  if (m_seen[mc.getIndex()]) {
    B2INFO(boost::format("%4d %s%10d%s*") % mc.getIndex() % indent % mc.getPDG() % name);
    return;
  }
  const TVector3& p = mc.getMomentum();
  const TVector3& v = mc.getVertex();

  B2INFO(boost::format("%4d %s%10d%s%40tp:(%10.3e, %10.3e, %10.3e) v:(%10.3e, %10.3e, %10.3e), t:%10.3e,%10.3e, s:%d")
         % mc.getIndex() % indent % mc.getPDG() % name
         % p.X() % p.Y() % p.Z()
         % v.X() % v.Y() % v.Z()
         % mc.getProductionTime() % mc.getDecayTime()
         % mc.getStatus()
        );

  const vector<MCParticle*> daughters = mc.getDaughters();
  BOOST_FOREACH(MCParticle* daughter, daughters) {
    printTree(*daughter, level);
  }
  m_seen[mc.getIndex()] = true;
}
