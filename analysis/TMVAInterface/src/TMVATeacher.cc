/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/TMVATeacher.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/DataStore.h>

#include <TMVA/Factory.h>
#include <TMVA/Tools.h>

#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TRandom.h>
#include <TPluginManager.h>

#include <algorithm>
#include <string>

namespace Belle2 {

  TMVAMethod::TMVAMethod(std::string name, std::string type, std::string config) : m_name(name), m_config(config)
  {

    // For plugin methods we load the TMVA::PluginName interface at runtime as via the TPluginManager of ROOT
    size_t pos = type.find(':');
    if (pos < std::string::npos) {
      std::string plg_type = type.substr(0, pos);
      std::string name = type.substr(pos + 2, std::string::npos);
      if (plg_type == "Plugins") {
        gPluginMgr->AddHandler("TMVA@@MethodBase", (std::string(".*_") + name + std::string(".*")).c_str(), (std::string("TMVA::Method") + name).c_str(),
                               (std::string("TMVA") + name).c_str(), (std::string("Method") + name + std::string("(DataSetInfo&,TString)")).c_str());
        gPluginMgr->AddHandler("TMVA@@MethodBase", (std::string(".*") + name + std::string(".*")).c_str(), (std::string("TMVA::Method") + name).c_str(),
                               (std::string("TMVA") + name).c_str(), (std::string("Method") + name + std::string("(TString&,TString&,DataSetInfo&,TString&)")).c_str());
      }
      B2INFO("Loaded plugin " << name)
      m_type = TMVA::Types::kPlugins;
    } else {
      m_type = TMVA::Types::Instance().GetMethodType(type);
    }
  }

  TMVATeacher::TMVATeacher(std::string identifier, std::vector<std::string> variables, std::string target, std::vector<TMVAMethod> methods) : m_identifier(identifier), m_methods(methods)
  {

    // Get Pointers to VariableManager::Var for every provided variable name
    VariableManager& manager = VariableManager::Instance();
    for (auto & variable : variables) {
      const VariableManager::Var* x =  manager.getVariable(variable);
      if (x == nullptr) {
        B2ERROR("Couldn't find variable " << variable << " via the VariableManager. Check the name!")
        continue;
      }
      m_input.insert(std::make_pair(x, 0));
    }

    // Get Pointer to VariableManager::Var for the provided target name
    const VariableManager::Var* targetVar =  manager.getVariable(target);
    if (targetVar == nullptr) {
      B2ERROR("Couldn't find target variable " << target << " via the VariableManager. Check the name!")
    }
    m_target = {targetVar, 0};

    m_signal_tree = new TTree((m_identifier + "_signal_tree").c_str(), (m_identifier + "_signal_tree").c_str());
    m_bckgrd_tree = new TTree((m_identifier + "_bckgrd_tree").c_str(), (m_identifier + "_bckgrd_tree").c_str());

    for (auto & pair : m_input) {
      m_signal_tree->Branch(pair.first->name.c_str(), &pair.second, "F");
      m_bckgrd_tree->Branch(pair.first->name.c_str(), &pair.second, "F");
    }
    m_signal_tree->Branch(m_target.first->name.c_str(), &m_target.second, "F");
    m_bckgrd_tree->Branch(m_target.first->name.c_str(), &m_target.second, "F");

  }

  TMVATeacher::~TMVATeacher()
  {
    delete m_signal_tree;
    delete m_bckgrd_tree;
  }

  void TMVATeacher::addSample(const Particle* particle)
  {

    for (auto & pair : m_input) {
      pair.second = pair.first->function(particle);
    }
    m_target.second = m_target.first->function(particle);

    if (m_target.second > 0.5) {
      m_signal_tree->Fill();
    } else {
      m_bckgrd_tree->Fill();
    }
  }

  void TMVATeacher::train(std::string factoryOption, std::string prepareOption)
  {

    TFile* file = TFile::Open((m_identifier + ".root").c_str(), "RECREATE");
    {
      // Intitialize TMVA and ROOT stuff
      TMVA::Tools::Instance();
      TMVA::Factory factory(m_identifier, file, factoryOption);

      // Add variables to the factory
      for (auto & pair : m_input) {
        factory.AddVariable(pair.first->name);
      }

      factory.AddSignalTree(m_signal_tree);
      factory.AddBackgroundTree(m_bckgrd_tree);
      factory.PrepareTrainingAndTestTree(TCut(""), prepareOption);

      for (auto & method : m_methods) {
        factory.BookMethod(method.getType(), method.getName(), method.getConfig());
      }

      factory.TrainAllMethods();
      factory.TestAllMethods();
      factory.EvaluateAllMethods();
    }

    file->Close();
  }

}

