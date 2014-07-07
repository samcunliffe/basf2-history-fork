/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Teacher.h>
#include <framework/logging/Logger.h>

#include <TMVA/Factory.h>
#include <TMVA/Tools.h>

#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

namespace Belle2 {

  namespace TMVAInterface {

    Teacher::Teacher(std::string prefix, std::string workingDirectory, std::string target, std::vector<Method> methods, bool useExistingData) : m_prefix(prefix), m_workingDirectory(workingDirectory), m_methods(methods), m_file(nullptr), m_tree(nullptr)
    {


      // Get Pointer to VariableManager::Var for the provided target name
      VariableManager& manager = VariableManager::Instance();
      m_target_var =  manager.getVariable(target);
      if (m_target_var == nullptr) {
        B2FATAL("Couldn't find target variable " << target << " via the VariableManager. Check the name!")
      }
      m_target = 0;

      // Create new tree which stores the input and target variable
      const auto& variables = m_methods[0].getVariables();
      m_input.resize(variables.size());

      // If we want to use existing data we open the file in UPDATE mode,
      // otherwise we recreate the file (overwrite it!)
      std::string mode = "RECREATE";
      if (useExistingData) {
        mode = "UPDATE";
      }

      m_file = TFile::Open((m_prefix + ".root").c_str(), mode.c_str());
      m_file->cd();
      std::string tree_name = m_prefix + "_tree";

      // Search for an existing tree in the file
      if (useExistingData) {
        //m_file->GetObject((tree_name + ";21").c_str(), m_tree);
        m_file->GetObject((tree_name).c_str(), m_tree);
      }

      if (m_tree == nullptr) {
        if (useExistingData)
          B2INFO("Couldn't find existing data, create new tree")
          m_tree = new TTree(tree_name.c_str(), tree_name.c_str());
      }

      for (unsigned int i = 0; i < variables.size(); ++i)
        m_tree->Branch(makeROOTCompatible(variables[i]->name).c_str(), &m_input[i]);
      m_tree->Branch(makeROOTCompatible(m_target_var->name).c_str(), &m_target);

      TBranch* targetBranch  = m_tree->GetBranch(makeROOTCompatible(m_target_var->name).c_str());
      targetBranch->SetAddress(&m_target);
      int nevent = m_tree->GetEntries();
      for (int i = 0; i < nevent; i++) {
        targetBranch->GetEvent(i);
        auto it = m_cluster_count.find(m_target);
        if (it == m_cluster_count.end())
          m_cluster_count[m_target] = 1;
        else
          it->second++;
      }

    }

    Teacher::~Teacher()
    {
      m_tree->Write();
      delete m_tree;
      m_file->Close();
    }

    void Teacher::addSample(const Particle* particle)
    {
      // Fill the tree with the input variables
      const auto& variables = m_methods[0].getVariables();
      for (unsigned int i = 0; i < variables.size(); ++i) {
        m_input[i] = variables[i]->function(particle);
      }

      // The target variable is converted to an integer
      m_target = int(m_target_var->function(particle) + 0.5);
      m_tree->Fill();

      // Now check if the target id was encountered before,
      // and update the cluster count accordingly
      auto it = m_cluster_count.find(m_target);
      if (it == m_cluster_count.end())
        m_cluster_count[m_target] = 1;
      else
        it->second++;
    }

    void Teacher::train(std::string factoryOption, std::string prepareOption, unsigned int maxEventsPerClass)
    {

      // Change the workling directory to the user defined working directory
      std::string oldDirectory = gSystem->WorkingDirectory();
      gSystem->ChangeDirectory(m_workingDirectory.c_str());

      m_file->cd();

      // Calculate the total number of events
      std::vector<int> classesWhichReachedMaximum;
      unsigned int total = 0;
      for (auto & x : m_cluster_count) {
        if (maxEventsPerClass != 0 and x.second > maxEventsPerClass) {
          classesWhichReachedMaximum.push_back(x.first);
        }
        total += x.second;
      }

      // Add to the output config xml file the different clusters and their fractions
      boost::property_tree::ptree pt;
      for (auto & x : m_cluster_count) {
        boost::property_tree::ptree node;
        node.put("ID", x.first);
        node.put("Count", x.second);
        node.put("MaxEventUsedInTraining", maxEventsPerClass);
        node.put("Fraction", static_cast<double>(x.second) / total);
        pt.add_child("Setup.Clusters.Cluster", node);
      }


      for (auto & x : m_methods[0].getVariables()) {
        boost::property_tree::ptree node;
        node.put("Name", x->name);
        pt.add_child("Setup.Variables.Variable", node);
      }

      if (m_cluster_count.size() <= 1) {
        B2ERROR("Found less than two clusters in sample, no training necessary!")
        return;
      }

      if (m_cluster_count.size() > 2) {
        B2FATAL("Found more than two clusters in sample. This isn't supported at the moment (the necessary logic is already implemented but the combination of the classifier outputs is missing")
        return;
      }

      // Train the found clusters against each other
      for (auto & x : m_cluster_count) {
        for (auto & y : m_cluster_count) {

          if (x.first == y.first)
            break;

          std::stringstream signal;
          std::stringstream bckgrd;
          signal << x.first;
          bckgrd << y.first;
          std::string jobName = m_prefix + "_" + signal.str() + "_vs_" + bckgrd.str();

          {

            boost::property_tree::ptree node;
            // Intitialize TMVA and ROOT stuff
            TMVA::Tools::Instance();
            TMVA::Factory factory(jobName, m_file, factoryOption);

            // Add variables to the factory
            for (auto & var : m_methods[0].getVariables()) {
              factory.AddVariable(makeROOTCompatible(var->name));
            }

            // Copy Events from original tree to new signal and background tree.
            // Unfortunatly this is necessary because TMVA internally uses vectors to store the data,
            // therefore TMVA looses its out-of-core capability.
            // The options nTrain_Background and nTest_Background (same for *_Signal) are applied
            // after this transformation to vectors, therefore they're too late to prevent a allocation of huge amount of memory
            // if one has many backgruond events.
            factory.AddSignalTree(m_tree->CopyTree(TCut((makeROOTCompatible(m_target_var->name) + " == " + signal.str()).c_str()))->CopyTree("", "", maxEventsPerClass == 0 ? x.second : maxEventsPerClass));
            factory.AddBackgroundTree(m_tree->CopyTree(TCut((makeROOTCompatible(m_target_var->name) + " == " + bckgrd.str()).c_str()))->CopyTree("", "", maxEventsPerClass == 0 ? y.second : maxEventsPerClass));
            factory.PrepareTrainingAndTestTree("", prepareOption);

            // Append the trained methods to the config xml file
            for (auto & method : m_methods) {
              boost::property_tree::ptree method_node;
              method_node.put("SignalID", x.first);
              method_node.put("BackgroundID", y.first);
              method_node.put("MethodName", method.getName());
              method_node.put("MethodType", method.getTypeAsString());
              method_node.put("Samplefile", m_prefix + ".root");
              method_node.put("Weightfile", std::string("weights/") + jobName + std::string("_") + method.getName() + std::string(".weights.xml"));
              factory.BookMethod(method.getType(), method.getName(), method.getConfig());
              node.add_child("Methods.Method", method_node);
            }

            factory.TrainAllMethods();
            factory.TestAllMethods();
            factory.EvaluateAllMethods();

            pt.add_child("Setup.Trainings.Training", node);
          }

        }
      }

      boost::property_tree::xml_writer_settings<char> settings('\t', 1);
      boost::property_tree::xml_parser::write_xml(m_prefix + ".config", pt, std::locale(), settings);
      gSystem->ChangeDirectory(oldDirectory.c_str());
    }
  }
}

