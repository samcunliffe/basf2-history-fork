/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/modules/TMVAExpert/TMVAExpertModule.h>

#include <analysis/TMVAInterface/Config.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/VariableManager/Manager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <fstream>

namespace Belle2 {


  REG_MODULE(TMVAExpert)

  TMVAExpertModule::TMVAExpertModule() : Module()
  {
    setDescription("Adds an ExtraInfo to the Particle objects in given ParticleLists. The ExtraInfo is calculated by a TMVA method "
                   "and represents the SignalProbability of the Particle with respect to the training. "
                   "Requires existing training for the specified method via TMVATeacher. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");
    setPropertyFlags(c_ParallelProcessingCertified);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the expert is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("method", m_methodName, "Method name specified in the training via TMVATeacher.");
    addParam("prefix", m_methodPrefix, "Common prefix for the methods trained by TMVATeacher. "
             "The prefix is used by the TMVAInterface to read its config-file $prefix.config "
             "and by TMVA method itself to read the files weights/$prefix_$method.class.C "
             "and weights/$prefix_$method.weights.xml with additional information", std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory,
             "Working directory in which the expert finds the config file and the weight file directory", std::string("."));
    addParam("expertOutputName", m_expertOutputName,
             "Name under which the output of the expert is stored in the ExtraInfo of the Particle object.");
    addParam("signalClass", m_signalClass,
             "Class which is considered as signal. e.g. the pdg of the Particle which is considered signal if "
             "you trained the method with pdg as target variable. Or 1 if you trained with isSignal as target.", 1);
    addParam("signalFraction", m_signalFraction,
             "signalFraction to calculate probability (if -1 the signalFraction of the training data is used)", -1.0f);
    addParam("transformToProbability", m_transformToProbability,
             "Transform classifier output to a probability using given signalFraction."
             "WARNING: If you want to use this feature you have to set the options createMVAPDFs and NbinsMVAPdf in the method config string provided to TMVATeacher to reasonable values!"
             "The default values of TMVA will result in a unusable classifier output!", true);

  }

  void TMVAExpertModule::initialize()
  {
    // All specified ParticleLists are required to exist
    for (auto& name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    if (m_listNames.empty()) {
      StoreObjPtr<EventExtraInfo>::registerPersistent("", DataStore::c_Event, false);
    } else {
      StoreObjPtr<ParticleExtraInfoMap>::registerPersistent("", DataStore::c_Event, false); //allow re-registration
    }

    TMVAInterface::ExpertConfig config(m_methodPrefix, m_workingDirectory, m_methodName, m_signalClass, m_signalFraction);
    m_method = std::unique_ptr<TMVAInterface::Expert>(new TMVAInterface::Expert(config, m_transformToProbability));

  }

  void TMVAExpertModule::terminate()
  {
    m_method.reset();
  }

  void TMVAExpertModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate target Value for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        Particle* particle = list->getParticle(i);
        float targetValue = m_method->analyse(particle);
        if (particle->hasExtraInfo(m_expertOutputName)) {
          B2WARNING("Extra Info with given name is already set! Overwriting old value!")
          particle->setExtraInfo(m_expertOutputName, targetValue);
        } else {
          particle->addExtraInfo(m_expertOutputName, targetValue);
        }
      }
    }
    if (m_listNames.empty()) {
      StoreObjPtr<EventExtraInfo> eventExtraInfo;
      if (not eventExtraInfo.isValid())
        eventExtraInfo.create();
      if (eventExtraInfo->hasExtraInfo(m_expertOutputName)) {
        B2WARNING("Extra Info with given name is already set! I won't set it again!")
      } else {
        float targetValue = m_method->analyse(nullptr);
        eventExtraInfo->addExtraInfo(m_expertOutputName, targetValue);
      }
    }
  }

} // Belle2 namespace

