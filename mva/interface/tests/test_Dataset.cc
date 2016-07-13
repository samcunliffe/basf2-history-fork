/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Dataset.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <boost/filesystem/operations.hpp>

#include <gtest/gtest.h>

#include <fstream>

using namespace Belle2;

namespace {

  class TestDataset : public MVA::Dataset {
  public:
    explicit TestDataset(MVA::GeneralOptions& general_options) : MVA::Dataset(general_options)
    {
      m_input = {1.0, 2.0, 3.0, 4.0, 5.0};
      m_target = 3.0;
      m_isSignal = true;
      m_weight = -3.0;
    }

    virtual unsigned int getNumberOfFeatures() const override { return 5; }
    virtual unsigned int getNumberOfEvents() const override { return 20; }
    virtual void loadEvent(unsigned int iEvent) override { float f = static_cast<float>(iEvent); m_input = {f + 1, f + 2, f + 3, f + 4, f + 5}; };
    virtual float getSignalFraction() override { return 0.1; };
    virtual std::vector<float> getFeature(unsigned int iFeature) override
    {
      std::vector<float> a(20, 0.0);
      std::iota(a.begin(), a.end(), iFeature + 1);
      return a;
    }

  };

  TEST(DatasetTest, SingleDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"a", "b", "c"};
    general_options.m_signal_class = 2;
    std::vector<float> input = {1.0, 2.0, 3.0};

    MVA::SingleDataset x(general_options, input, 2.0);

    EXPECT_EQ(x.getNumberOfFeatures(), 3);
    EXPECT_EQ(x.getNumberOfEvents(), 1);

    // Should just work
    x.loadEvent(0);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.0);

    EXPECT_FLOAT_EQ(x.m_weight, 1.0);
    EXPECT_FLOAT_EQ(x.m_target, 2.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 1.0);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 1);
    EXPECT_FLOAT_EQ(feature[0], 2.0);

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 1);
    EXPECT_FLOAT_EQ(feature[0], 2.0);

  }

  TEST(DatasetTest, MultiDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"a", "b", "c"};
    general_options.m_signal_class = 2;
    std::vector<std::vector<float>> matrix = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    std::vector<float> targets = {2.0, 0.0, 2.0};
    std::vector<float> weights = {1.0, 2.0, 3.0};

    EXPECT_B2ERROR(MVA::MultiDataset(general_options, matrix, {1.0}, weights));

    EXPECT_B2ERROR(MVA::MultiDataset(general_options, matrix, targets, {1.0}));

    MVA::MultiDataset x(general_options, matrix, targets, weights);

    EXPECT_EQ(x.getNumberOfFeatures(), 3);
    EXPECT_EQ(x.getNumberOfEvents(), 3);

    // Should just work
    x.loadEvent(0);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.0);

    EXPECT_FLOAT_EQ(x.m_weight, 1.0);
    EXPECT_FLOAT_EQ(x.m_target, 2.0);
    EXPECT_EQ(x.m_isSignal, true);

    // Should just work
    x.loadEvent(1);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 5.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 6.0);

    EXPECT_FLOAT_EQ(x.m_weight, 2.0);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    // Should just work
    x.loadEvent(2);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 7.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 8.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 9.0);

    EXPECT_FLOAT_EQ(x.m_weight, 3.0);
    EXPECT_FLOAT_EQ(x.m_target, 2.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 4.0 / 6.0);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 3);
    EXPECT_FLOAT_EQ(feature[0], 2.0);
    EXPECT_FLOAT_EQ(feature[1], 5.0);
    EXPECT_FLOAT_EQ(feature[2], 8.0);


  }

  TEST(DatasetTest, SubDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_signal_class = 3;
    general_options.m_variables = {"a", "b", "c", "d", "e"};
    TestDataset test_dataset(general_options);

    general_options.m_variables = {"a", "d", "e"};
    std::vector<bool> events = {true, false, true, false, true, false, true, false, true, false,
                                true, false, true, false, true, false, true, false, true, false
                               };
    MVA::SubDataset x(general_options, events, test_dataset);

    EXPECT_EQ(x.getNumberOfFeatures(), 3);
    EXPECT_EQ(x.getNumberOfEvents(), 10);

    // Should just work
    x.loadEvent(0);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 5.0);

    EXPECT_FLOAT_EQ(x.m_weight, -3.0);
    EXPECT_FLOAT_EQ(x.m_target, 3.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 1);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 10);
    for (unsigned int iEvent = 0; iEvent < 10; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent * 2 + 4);
    };

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 10);
    for (unsigned int iEvent = 0; iEvent < 10; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent * 2 + 4);
    };

    // Test without event indices
    MVA::SubDataset y(general_options, {}, test_dataset);
    feature = y.getFeature(1);
    EXPECT_EQ(feature.size(), 20);
    for (unsigned int iEvent = 0; iEvent < 20; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent + 4);
    };

    // Same result for mother class implementation
    feature = y.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 20);
    for (unsigned int iEvent = 0; iEvent < 20; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent + 4);
    };

    general_options.m_variables = {"a", "d", "e", "DOESNOTEXIST"};
    try {
      EXPECT_B2ERROR(MVA::SubDataset(general_options, events, test_dataset));
    } catch (...) {

    }
    EXPECT_THROW(MVA::SubDataset(general_options, events, test_dataset), std::runtime_error);

  }

  TEST(DatasetTest, ROOTDataset)
  {

    TestHelpers::TempDirCreator tmp_dir;
    TFile file("datafile.root", "RECREATE");
    file.cd();
    TTree tree("tree", "TreeTitle");
    float a, b, c, d, e, f, g = 0;
    tree.Branch("a", &a);
    tree.Branch("b", &b);
    tree.Branch("c", &c);
    tree.Branch("d", &d);
    tree.Branch("e__bo__bc", &e);
    tree.Branch("f__bo__bc", &f);
    tree.Branch("g", &g);
    tree.Branch("__weight__", &c);

    for (unsigned int i = 0; i < 5; ++i) {
      a = i + 1.0;
      b = i + 1.1;
      c = i + 1.2;
      d = i + 1.3;
      e = i + 1.4;
      f = i + 1.5;
      g = i % 2 == 0;
      tree.Fill();
    }

    file.Write("tree");

    MVA::GeneralOptions general_options;
    // Both names with and without makeROOTCompatible should work
    general_options.m_variables = {"a", "b", "e__bo__bc", "f()"};
    general_options.m_signal_class = 1;
    general_options.m_datafile = "datafile.root";
    general_options.m_treename = "tree";
    general_options.m_target_variable = "g";
    general_options.m_weight_variable = "c";
    MVA::ROOTDataset x(general_options);

    EXPECT_EQ(x.getNumberOfFeatures(), 4);
    EXPECT_EQ(x.getNumberOfEvents(), 5);

    // Should just work
    x.loadEvent(0);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 1.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 1.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 1.5);
    EXPECT_FLOAT_EQ(x.m_weight, 1.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(1);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 2.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 2.5);
    EXPECT_FLOAT_EQ(x.m_weight, 2.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(2);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 3.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 3.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 3.5);
    EXPECT_FLOAT_EQ(x.m_weight, 3.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(3);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 4.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 4.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 4.5);
    EXPECT_FLOAT_EQ(x.m_weight, 4.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(4);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 5.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 5.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 5.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 5.5);
    EXPECT_FLOAT_EQ(x.m_weight, 5.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 0.6);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 5);
    EXPECT_FLOAT_EQ(feature[0], 1.1);
    EXPECT_FLOAT_EQ(feature[1], 2.1);
    EXPECT_FLOAT_EQ(feature[2], 3.1);
    EXPECT_FLOAT_EQ(feature[3], 4.1);
    EXPECT_FLOAT_EQ(feature[4], 5.1);

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 5);
    EXPECT_FLOAT_EQ(feature[0], 1.1);
    EXPECT_FLOAT_EQ(feature[1], 2.1);
    EXPECT_FLOAT_EQ(feature[2], 3.1);
    EXPECT_FLOAT_EQ(feature[3], 4.1);
    EXPECT_FLOAT_EQ(feature[4], 5.1);

    auto weights = x.getWeights();
    EXPECT_EQ(weights.size(), 5);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);

    auto targets = x.getTargets();
    EXPECT_EQ(targets.size(), 5);
    EXPECT_FLOAT_EQ(targets[0], 1.0);
    EXPECT_FLOAT_EQ(targets[1], 0.0);
    EXPECT_FLOAT_EQ(targets[2], 1.0);
    EXPECT_FLOAT_EQ(targets[3], 0.0);
    EXPECT_FLOAT_EQ(targets[4], 1.0);

    auto signals = x.getSignals();
    EXPECT_EQ(signals.size(), 5);
    EXPECT_EQ(signals[0], true);
    EXPECT_EQ(signals[1], false);
    EXPECT_EQ(signals[2], true);
    EXPECT_EQ(signals[3], false);
    EXPECT_EQ(signals[4], true);

    // Using __weight__ should work as well,
    // the only difference to using _weight__ instead of g is
    // in setBranchAddresses which avoids calling makeROOTCompatible
    // So we have to check the behaviour using __weight__ as well
    general_options.m_weight_variable = "__weight__";
    MVA::ROOTDataset y(general_options);

    weights = y.getWeights();
    EXPECT_EQ(weights.size(), 5);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);

    // Check for missing tree
    general_options.m_treename = "missing tree";
    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);

    // Check for missing branch
    general_options.m_treename = "tree";
    general_options.m_variables = {"a", "b", "e", "f", "missing branch"};
    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);

    // Check for missing file
    general_options.m_datafile = "DOESNOTEXIST.root";
    general_options.m_treename = "tree";
    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);

    // Check for invalid file
    general_options.m_datafile = "ISNotAValidROOTFile";
    general_options.m_treename = "tree";

    {
      std::ofstream(general_options.m_datafile);
    }
    EXPECT_TRUE(boost::filesystem::exists(general_options.m_datafile));

    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);


  }


}
