/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 **************************************************************************/

#include <beast/pindiode/modules/PindiodeModule.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <boost/foreach.hpp>

using namespace std;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PINDIODE detector */
  namespace pindiode {

    //We have to register the module to the Framework. The "Module" part of the
    //class name will be appended automatically so every module hast to be named
    //XxxModule
    REG_MODULE(Pindiode)


    PindiodeModule::PindiodeModule() : Module(), m_intParameter(0), m_doubleParameter(0), m_stringParameter("")
    {
      setDescription("Creates PINDIODE crystals - sub-detector of BEASTII");

      //We can define parameters which can be set from the steering file. The arguments are:
      // name, reference to the veriable where the value will be stored, description, default value
      //If the default value is ommited the user has to specify this parameter, otherwise an error is produced
      addParam("intParameter", m_intParameter,
               "Useless parameter of type integer", 0);
      addParam("doubleParameter", m_doubleParameter,
               "Useless parameter of type double", 0.0);
      addParam("stringParameter", m_stringParameter,
               "Useless parameter of type string", string(""));
      addParam("doubleListParameter", m_doubleListParameter,
               "Useless parameter of type vector<double>", vector<double>(3, 0));


      //Valid parameter types are int, double, string, bool and vectors of any of those
    }

    void PindiodeModule::initialize()
    {
      B2INFO("Pindiode: Initialize");
      //Here you can do some stuff before processing starts. If you want to
      //write to some collections of the DataStore you have to register these
      //here by using StoreArray<T>::registerPersistent() for collections which
      //should be written to the output file by default or
      //StoreArray<T>::registerTransient() for collections which will not be
      //saved by default. If one just wants to access collections one should
      //check if they were registered by using the isRequired member

      StoreArray<MCParticle>   mcParticles;
      StoreArray<PindiodeSimHit>  simHits;
      RelationArray relMCSimHit(mcParticles, simHits);
      if (!(mcParticles.isRequired() && simHits.isRequired() && relMCSimHit.isRequired())) {
        //Fatal is not neccessary here as the storeArrays should just look
        //empty if not registered but let's make sure everything is present
        B2FATAL("Not all collections found, exiting processing");
      }
    }

    void PindiodeModule::beginRun()
    {
      B2INFO("Pindiode: Begin of new run");
      //Here comes the initialisation specific to each run
    }

    void PindiodeModule::event()
    {
      B2INFO("Pindiode: Event is being processed");
      //Here comes the actual event processing

      StoreArray<MCParticle>   mcParticles;
      StoreArray<PindiodeSimHit>  simHits;

      //RelationIndex is a readonly, bidirectional index for a Relation so that one
      //can easily use the RelationArray without looping over it manually.
      RelationIndex<MCParticle, PindiodeSimHit> relMCSimHit(mcParticles, simHits);

      //Lets loop over all created PindiodeSimHits:
      //int nSimHits = simHits.getEntries();
      //for (int i = 0; i < nSimHits; ++i) {
      //PindiodeSimHit& hit = *simHits[i];
      //Find all MCParticles which point to that SimHit and the corresponding weight
      //RelationIndex<MCParticle, PindiodeSimHit>::range_to range = relMCSimHit.getElementsTo(hit);
      //for (; range.first != range.second; ++range.first) {
      //And Print something about the relation
      //const RelationIndex<MCParticle, PindiodeSimHit>::Element& relation = *range.to;
      //B2INFO("PindiodeSimHit #" << i << " has an energy deposition of " << hit.getEnergyDep()
      //       << " and is related to MCParticle #" << relation.indexFrom
      //       << " which has an PDG code of " << relation.from->getPDG());
      //}
      //}

      //Now let's do it the other way round:
      int nMCParticles = mcParticles.getEntries();
      for (int i = 0; i < nMCParticles; ++i) {
        MCParticle& mcp = *mcParticles[i];
        //Find all PindiodeSimHits which point from that MCParticle using a typedef and BOOST_FOREACH
        //The typedef is needed as BOOST_FOREACH is a macro and cannot handle anything including a comma
        typedef RelationIndex<MCParticle, PindiodeSimHit>::Element relMCSimHit_Element;
        BOOST_FOREACH(const relMCSimHit_Element & relation, relMCSimHit.getElementsFrom(mcp)) {
          B2INFO("MCParticle #" << i << " created the AwesomSimHit #" << relation.indexTo
                 << " which has an energy deposition of " << relation.to->getEnergyDep());
        }
      }
    }

    void PindiodeModule::endRun()
    {
      B2INFO("Pindiode: End of run");
      //Here cleanup after each run
    }


    void PindiodeModule::terminate()
    {
      B2INFO("Pindiode: Terminate");
      //Here final cleanup
    }

  } //pindiode namespace
} //Belle2 namespace
