/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_TMVA_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_TMVA_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>

#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/utilities/WorkingDirectoryManager.h>
#include <framework/logging/LogSystem.h>
#include <framework/utilities/Utils.h>
#include <framework/logging/Logger.h>

#include <TMVA/Factory.h>
#include <TMVA/Tools.h>
#include <TMVA/Reader.h>
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
#include <TMVA/DataLoader.h>
#endif

#include <memory>

namespace Belle2 {
  namespace MVA {

    /**
     * Options for the TMVA MVA method
     */
    class TMVAOptions : public SpecificOptions {

    public:
      /**
       * Load mechanism to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override;

      /**
       * Save mechanism to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override;

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override;

      /**
       * Return method name
       */
      virtual std::string getMethod() const override { return "TMVA"; }

    public:
      std::string m_method = "BDT"; /**< tmva method name */
      std::string m_type = "BDT"; /**< tmva method type */

      /**
       * TMVA config string for the chosen method
       */
      std::string m_config =
        "!H:!V:CreateMVAPdfs:NTrees=400:BoostType=Grad:Shrinkage=0.1:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=1024:MaxDepth=3:IgnoreNegWeightsInTraining";
      //std::string method = "FastBDT";
      //std::string type = "Plugins";
      //std::string config = "!H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3";
      std::string m_factoryOption = "!V:!Silent:Color:DrawProgressBar"; /**< Factory options passed to tmva factory */
      std::string m_prepareOption = "SplitMode=random:!V"; /**< Prepare options passed to prepareTrainingAndTestTree method */
      std::string m_workingDirectory = ""; /**< Working directory of TMVA, if empty a temporary directory is used */
      std::string m_prefix = "TMVA"; /**< Prefix used for all files generated by TMVA */
    };

    /**
     * Options for the TMVA Classification MVA method
     */
    class TMVAOptionsClassification : public TMVAOptions {

    public:
      /**
       * Constructor
       * Adds Classification as AnalysisType to the factoryOptions
       */
      TMVAOptionsClassification()
      {
        m_factoryOption += ":AnalysisType=Classification";
      }

      /**
       * Load mechanism to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override;

      /**
       * Save mechanism to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override;

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override;

      /**
       * Return method name
       */
      virtual std::string getMethod() const override { return "TMVAClassification"; }

    public:
      bool transform2probability = true; /**< Transform output of method to a probability */
    };


    /**
     * Options for the TMVA Regression MVA method
     */
    class TMVAOptionsRegression : public TMVAOptions {

    public:
      /**
       * Constructor
       * Adds REgression as AnalysisType to the factoryOptions
       */
      TMVAOptionsRegression()
      {
        m_factoryOption += ":AnalysisType=Regression";
      }

      /**
       * Return method name
       */
      virtual std::string getMethod() const override { return "TMVARegression"; }
    };


    /**
     * Teacher for the TMVA MVA method
     */
    class TMVATeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defininf all method specific options
       */
      TMVATeacher(const GeneralOptions& general_options, const TMVAOptions& specific_options);

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      /**
       * Train a mva method using the given data loader returning a Weightfile
       * @param factory used to train the method
       * @param data_loader used to train the method
       * @param jobName name of the TMVA training
       */
      Weightfile trainFactory(TMVA::Factory& factory, TMVA::DataLoader& data_loader, std::string& jobName) const;
#else
      /**
       * Train a mva method using the given factory returning a Weightfile
       * @param factory used to train the method
       * @param jobName name of the TMVA training
       */
      Weightfile trainFactory(TMVA::Factory& factory, std::string& jobName) const;
#endif

    private:
      TMVAOptions specific_options; /**< Method specific options */

    };

    /**
     * Teacher for the TMVA Classification MVA method
     */
    class TMVATeacherClassification : public TMVATeacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defining all method specific options
       */
      TMVATeacherClassification(const GeneralOptions& general_options, const TMVAOptionsClassification& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const;

    protected:
      TMVAOptionsClassification specific_options; /**< Method specific options */
    };

    /**
     * Teacher for the TMVA Regression MVA method
     */
    class TMVATeacherRegression : public TMVATeacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defining all method specific options
       */
      TMVATeacherRegression(const GeneralOptions& general_options, const TMVAOptionsRegression& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const;

    protected:
      TMVAOptionsRegression specific_options; /**< Method specific options */
    };

    /**
     * Expert for the TMVA MVA method
     */
    class TMVAExpert : public MVA::Expert {

    public:
      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the expert
       */
      virtual void load(Weightfile& weightfile) override;

    protected:
      std::unique_ptr<TMVA::Reader> m_expert; /**< TMVA::Reader pointer */
      mutable std::vector<float>
      m_input_cache; /**< Input Cache for TMVA::Reader: Otherwise we would have to set the branch addresses in each apply call */
    };

    /**
     * Expert for the TMVA Classification MVA method
     */
    class TMVAExpertClassification : public TMVAExpert {

    public:
      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the m_expert
       */
      virtual void load(Weightfile& weightfile) override;

      /**
       * Apply this m_expert onto a dataset
       * @param test_data dataset
       */
      virtual std::vector<float> apply(Dataset& test_data) const override;

    protected:
      TMVAOptionsClassification specific_options; /**< Method specific options */
      float expert_signalFraction; /**< Signal fraction used to calculate the probability */

    };

    /**
     * Expert for the TMVA Regression MVA method
     */
    class TMVAExpertRegression : public TMVAExpert {

    public:
      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the m_expert
       */
      virtual void load(Weightfile& weightfile) override;

      /**
       * Apply this m_expert onto a dataset
       * @param test_data dataset
       */
      virtual std::vector<float> apply(Dataset& test_data) const override;

    protected:
      TMVAOptionsRegression specific_options; /**< Method specific options */

    };

  }
}
#endif
