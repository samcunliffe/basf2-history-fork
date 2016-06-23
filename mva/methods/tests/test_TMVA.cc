/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/TMVA.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(TMVATest, TMVAOptions)
  {
    MVA::TMVAOptions specific_options;

    //EXPECT_EQ(specific_options.method, "FastBDT");
    //EXPECT_EQ(specific_options.type, "Plugins");
    //EXPECT_EQ(specific_options.config, "!H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3");
    EXPECT_EQ(specific_options.m_method, "BDT");
    EXPECT_EQ(specific_options.m_type, "BDT");
    EXPECT_EQ(specific_options.m_config,
              "!H:!V:CreateMVAPdfs:NTrees=400:BoostType=Grad:Shrinkage=0.1:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=1024:MaxDepth=3:IgnoreNegWeightsInTraining");
    EXPECT_EQ(specific_options.m_factoryOption, "!V:!Silent:Color:DrawProgressBar");
    EXPECT_EQ(specific_options.m_prepareOption, "SplitMode=random:!V");
    EXPECT_EQ(specific_options.m_workingDirectory, ".");
    EXPECT_EQ(specific_options.m_prefix, "TMVA");

    specific_options.m_method = "Method";
    specific_options.m_type = "Type";
    specific_options.m_config = "Config";
    specific_options.m_factoryOption = "FactoryOption";
    specific_options.m_prepareOption = "PrepareOption";
    specific_options.m_workingDirectory = "WorkingDirectory";
    specific_options.m_prefix = "Prefix";

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_EQ(pt.get<std::string>("TMVA_method"), "Method");
    EXPECT_EQ(pt.get<std::string>("TMVA_type"), "Type");
    EXPECT_EQ(pt.get<std::string>("TMVA_config"), "Config");
    EXPECT_EQ(pt.get<std::string>("TMVA_factoryOption"), "FactoryOption");
    EXPECT_EQ(pt.get<std::string>("TMVA_prepareOption"), "PrepareOption");
    EXPECT_EQ(pt.get<std::string>("TMVA_workingDirectory"), "WorkingDirectory");
    EXPECT_EQ(pt.get<std::string>("TMVA_prefix"), "Prefix");

    MVA::TMVAOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_method, "Method");
    EXPECT_EQ(specific_options2.m_type, "Type");
    EXPECT_EQ(specific_options2.m_config, "Config");
    EXPECT_EQ(specific_options2.m_factoryOption, "FactoryOption");
    EXPECT_EQ(specific_options2.m_prepareOption, "PrepareOption");
    EXPECT_EQ(specific_options2.m_workingDirectory, "WorkingDirectory");
    EXPECT_EQ(specific_options2.m_prefix, "Prefix");

    MVA::TMVAOptionsClassification specific_classification_options;
    EXPECT_EQ(specific_classification_options.transform2probability, true);
    EXPECT_EQ(specific_classification_options.m_factoryOption, "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification");

    specific_classification_options.transform2probability = false;
    boost::property_tree::ptree pt_classification;
    specific_classification_options.save(pt_classification);
    EXPECT_EQ(pt_classification.get<bool>("TMVA_transform2probability"), false);

    MVA::TMVAOptionsClassification specific_classification_options2;
    specific_classification_options2.load(pt_classification);
    EXPECT_EQ(specific_classification_options.transform2probability, false);

    MVA::TMVAOptionsRegression specific_regression_options;
    EXPECT_EQ(specific_regression_options.m_factoryOption, "!V:!Silent:Color:DrawProgressBar:AnalysisType=Regression");

    EXPECT_EQ(specific_classification_options.getMethod(), std::string("TMVAClassification"));
    EXPECT_EQ(specific_regression_options.getMethod(), std::string("TMVARegression"));

    // Test if po::options_description is created without crashing
    auto description = specific_options.getDescription();
    EXPECT_EQ(description.options().size(), 5);

    // Check for B2ERROR and throw if version is wrong
    // we try with version 100, surely we will never reach this!
    pt.put("TMVA_version", 100);
    try {
      EXPECT_B2ERROR(specific_options2.load(pt));
    } catch (...) {

    }
    EXPECT_THROW(specific_options2.load(pt), std::runtime_error);

  }

  class TestClassificationDataset : public MVA::Dataset {
  public:
    explicit TestClassificationDataset(const std::vector<float>& data) : MVA::Dataset(MVA::GeneralOptions()), m_data(data)
    {
      m_input = {0.0};
      m_target = 0.0;
      m_isSignal = false;
      m_weight = 1.0;
    }

    virtual unsigned int getNumberOfFeatures() const override { return 1; }
    virtual unsigned int getNumberOfEvents() const override { return m_data.size(); }
    virtual void loadEvent(unsigned int iEvent) override { m_input[0] = m_data[iEvent]; m_target = iEvent % 2; m_isSignal = m_target == 1; };
    virtual float getSignalFraction() override { return 0.1; };
    virtual std::vector<float> getFeature(unsigned int) override { return m_data; }

    std::vector<float> m_data;

  };


  TEST(TMVATest, TMVAClassificationInterface)
  {
    MVA::Interface<MVA::TMVAOptionsClassification, MVA::TMVATeacherClassification, MVA::TMVAExpertClassification>
    interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    general_options.m_target_variable = "Target";
    MVA::TMVAOptionsClassification specific_options;
    specific_options.m_prepareOption = "SplitMode=block:!V";
    specific_options.transform2probability = false;
    specific_options.m_config =
      "!H:!V:CreateMVAPdfs:NTrees=400:BoostType=Grad:Shrinkage=0.1:nCuts=10:MaxDepth=3:IgnoreNegWeightsInTraining:MinNodeSize=20";
    TestClassificationDataset dataset({1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0,
                                       1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0
                                      });

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < 24; ++i) {
      EXPECT_LE(probabilities[i], 0.1);
      EXPECT_GE(probabilities[i], -0.1);
    }
    for (unsigned int i = 24; i < 48; i += 2) {
      EXPECT_LE(probabilities[i], -0.8);
      EXPECT_GE(probabilities[i + 1], 0.8);
    }
    for (unsigned int i = 48; i < 72; ++i) {
      EXPECT_LE(probabilities[i], 0.1);
      EXPECT_GE(probabilities[i], -0.1);
    }
    for (unsigned int i = 72; i < 96; i += 2) {
      EXPECT_LE(probabilities[i], -0.8);
      EXPECT_GE(probabilities[i + 1], 0.8);
    }


  }

  TEST(TMVATest, TMVAClassificationPlugin)
  {
    MVA::Interface<MVA::TMVAOptionsClassification, MVA::TMVATeacherClassification, MVA::TMVAExpertClassification>
    interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    general_options.m_target_variable = "Target";
    MVA::TMVAOptionsClassification specific_options;
    specific_options.m_prepareOption = "SplitMode=block:!V";
    specific_options.transform2probability = false;
    specific_options.m_method = "FastBDT";
    specific_options.m_type = "Plugins";
    specific_options.m_config =
      "!H:!V:CreateMVAPdfs:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3";
    TestClassificationDataset dataset({1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0,
                                       1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0,
                                       2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0, 2.0, 3.0
                                      });

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < 24; ++i) {
      EXPECT_LE(probabilities[i], 0.55);
      EXPECT_GE(probabilities[i], 0.45);
    }
    for (unsigned int i = 24; i < 48; i += 2) {
      EXPECT_LE(probabilities[i], 0.1);
      EXPECT_GE(probabilities[i + 1], 0.9);
    }
    for (unsigned int i = 48; i < 72; ++i) {
      EXPECT_LE(probabilities[i], 0.55);
      EXPECT_GE(probabilities[i], 0.45);
    }
    for (unsigned int i = 72; i < 96; i += 2) {
      EXPECT_LE(probabilities[i], 0.1);
      EXPECT_GE(probabilities[i + 1], 0.9);
    }


  }


  class TestRegressionDataset : public MVA::Dataset {
  public:
    explicit TestRegressionDataset(const std::vector<float>& data) : MVA::Dataset(MVA::GeneralOptions()), m_data(data)
    {
      m_input = {0.0};
      m_target = 0.0;
      m_isSignal = false;
      m_weight = 1.0;
    }

    virtual unsigned int getNumberOfFeatures() const override { return 1; }
    virtual unsigned int getNumberOfEvents() const override { return m_data.size(); }
    virtual void loadEvent(unsigned int iEvent) override { m_input[0] = m_data[iEvent]; m_target = static_cast<float>((static_cast<int>(iEvent % 48) - 24) / 4) / 24.0;};
    virtual float getSignalFraction() override { return 0.0; };
    virtual std::vector<float> getFeature(unsigned int) override { return m_data; }

    std::vector<float> m_data;

  };

  TEST(TMVATest, TMVARegressionInterface)
  {
    MVA::Interface<MVA::TMVAOptionsRegression, MVA::TMVATeacherRegression, MVA::TMVAExpertRegression> interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    general_options.m_target_variable = "Target";
    MVA::TMVAOptionsRegression specific_options;
    specific_options.m_prepareOption = "SplitMode=block:!V";
    specific_options.m_config = "!H:!V:NTrees=200::BoostType=Grad:Shrinkage=0.1:nCuts=24:MaxDepth=3";
    //specific_options.config = "nCuts=120:NTrees=20:MaxDepth=4:BoostType=AdaBoostR2:SeparationType=RegressionVariance:MinNodeSize=10";
    TestRegressionDataset dataset({1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 3.0,
                                   4.0, 4.0, 4.0, 4.0, 5.0, 5.0, 5.0, 5.0, 6.0, 6.0, 6.0, 6.0,
                                   7.0, 7.0, 7.0, 7.0, 8.0, 8.0, 8.0, 8.0, 9.0, 9.0, 9.0, 9.0,
                                   10.0, 10.0, 10.0, 10.0, 11.0, 11.0, 11.0, 11.0, 12.0, 12.0, 12.0, 12.0,
                                   1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 3.0,
                                   4.0, 4.0, 4.0, 4.0, 5.0, 5.0, 5.0, 5.0, 6.0, 6.0, 6.0, 6.0,
                                   7.0, 7.0, 7.0, 7.0, 8.0, 8.0, 8.0, 8.0, 9.0, 9.0, 9.0, 9.0,
                                   10.0, 10.0, 10.0, 10.0, 11.0, 11.0, 11.0, 11.0, 12.0, 12.0, 12.0, 12.0
                                  });

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto values = expert->apply(dataset);
    EXPECT_EQ(values.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < 96; i += 4) {
      float r = static_cast<float>((static_cast<int>(i % 48) - 24) / 4) / 24.0;
      EXPECT_LE(values[i], r + 0.05);
      EXPECT_GE(values[i], r - 0.05);
      EXPECT_LE(values[i + 1], r + 0.05);
      EXPECT_GE(values[i + 1], r - 0.05);
      EXPECT_LE(values[i + 2], r + 0.05);
      EXPECT_GE(values[i + 2], r - 0.05);
      EXPECT_LE(values[i + 3], r + 0.05);
      EXPECT_GE(values[i + 3], r - 0.05);
    }

  }

}
