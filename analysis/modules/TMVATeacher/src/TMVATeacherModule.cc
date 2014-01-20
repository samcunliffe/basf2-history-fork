/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/logging/Logger.h>

#include <analysis/TMVAInterface/TMVATeacher.h>
#include <analysis/modules/TMVATeacher/TMVATeacherModule.h>

namespace Belle2 {


  REG_MODULE(TMVATeacher)

  TMVATeacherModule::TMVATeacherModule() : Module()
  {
    setDescription("Trains multivariate analysis method with given particle lists as training samples.");
    // We use root TFile and TMVA so, this is propably not usable for parallel processsing
    //setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::string> defaultList;
    addParam("listNames", m_listNames, "Input particle list names as list", defaultList);
    addParam("methods", m_methods, "Vector of Tuples with (Name, Type, Config) of the methods. Valid types are: BDT, KNN, NeuroBayes, Fisher. The Config is passed to the TMVA Method and is documented in the TMVA UserGuide.");
    addParam("identifier", m_identifier, "Identifier which is used by the TMVA method to write the files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml with additional information");
    addParam("variables", m_variables, "Input variables used by the TMVA method", defaultList);
    addParam("target", m_target, "Target used by the method");
    addParam("factoryOption", m_factoryOption, "Option passed to TMVA::Factory", std::string("!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification"));
    addParam("prepareOption", m_prepareOption, "Option passed to TMVA::Factory::PrepareTrainingAndTestTree", std::string("SplitMode=random:!V"));

    m_method = nullptr;
  }

  TMVATeacherModule::~TMVATeacherModule()
  {
  }

  void TMVATeacherModule::initialize()
  {
    for (auto & name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }
  }


  void TMVATeacherModule::beginRun()
  {
    std::vector<TMVAMethod> methods;
    for (auto & x : m_methods) {
      methods.push_back(TMVAMethod(std::get<0>(x), std::get<1>(x), std::get<2>(x)));
    }
    m_method = new TMVATeacher(m_identifier, m_variables, m_target, methods);
  }

  void TMVATeacherModule::endRun()
  {
    m_method->train(m_factoryOption, m_prepareOption);
    if (m_method !=  nullptr) {
      delete m_method;
      m_method = nullptr;
    }
  }

  void TMVATeacherModule::terminate()
  {

  }

  void TMVATeacherModule::event()
  {
    for (auto & listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles [0, N_PARTICLE] and AntiParticles [N_PARTICLE, N_PARTICLE + N_ANTIPARTICLE]
      for (unsigned i = 0; i < list->getNumofParticles() + list->getNumofAntiParticles(); ++i) {
        const Particle* particle = list->getParticle(i);
        m_method->addSample(particle);
      }
    }
  }

  void TMVATeacherModule::printModuleParams() const
  {
  }

} // Belle2 namespace

