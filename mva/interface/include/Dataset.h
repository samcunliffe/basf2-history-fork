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
#ifndef INCLUDE_GUARD_BELLE2_MVA_DATASET_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_DATASET_HEADER

#include <mva/interface/Options.h>

#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>

namespace Belle2 {
  namespace MVA {

    /**
     * Abstract base class of all Datasets given to the MVA interface
     * The current event can always be accessed via the public members of this class
     */
    class Dataset {

    public:
      /**
       * Constructs a new dataset given the general options
       * @param general_options which defines e.g. number of variables
       */
      explicit Dataset(const GeneralOptions& general_options);

      /**
       * Virtual default destructor
       */
      virtual ~Dataset() = default;

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const = 0;

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const = 0;

      /**
       * Load the event number iEvent
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int iEvent) = 0;

      /**
       * Returns the signal fraction of the whole sample
       */
      virtual float getSignalFraction();

      /**
       * Returns all values of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature);

      /**
       * Returns all weights
       */
      virtual std::vector<float> getWeights();

      /**
       * Returns all targets
       */
      virtual std::vector<float> getTargets();

      /**
       * Returns all is Signals
       */
      virtual std::vector<bool> getSignals();

      GeneralOptions m_general_options; /**< GeneralOptions passed to this dataset */
      std::vector<float> m_input; /**< Contains all feature values of the currently loaded event */
      float m_weight; /**< Contains the weight of the currently loaded event */
      float m_target; /**< Contains the target value of the currently loaded event */
      bool m_isSignal; /**< Defines if the currently loaded event is signal or background */
    };


    /**
     * Wraps the data of a single event into a Dataset.
     * Mostly useful if one wants to apply an Expert to a single feature vector
     */
    class SingleDataset : public Dataset {

    public:
      /**
       * Constructs a new SingleDataset
       * @param general_options which defines e.g. number of variables
       * @param input feature values of the single event
       * @param target target value of the single event (defaults to 1, because often this is not known if one wants to apply an expert)
       */
      SingleDataset(const GeneralOptions& general_options, const std::vector<float>& input, float target = 1.0);

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_input.size(); }

      /**
       * Returns the number of events in this dataset which is always one
       */
      virtual unsigned int getNumberOfEvents() const override { return 1; }

      /**
       * Does nothing in the case of a single dataset, because the only event is already loaded
       */
      virtual void loadEvent(unsigned int) override { };

      /**
       * Returns all values (in this case only one) of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override { return std::vector<float> {m_input[iFeature]}; }

    };

    /**
     * Wraps another Dataset and provides a view to a subset of its features and events.
     * Used by the Combination method which can combine multiple methods with possibly different variables
     */
    class SubDataset : public Dataset {

    public:
      /**
       * Constructs a new SubDataset holding a reference to the wrapped Dataset
       * @param general_options which defines e.g. a subset of variables of the original dataset
       * @param variables subset of variables which are provided by this Dataset
       * @param events subset of events which are provided by this Dataset
       * @param dataset reference to the wrapped Dataset
       */
      SubDataset(const GeneralOptions& general_options, const std::vector<bool>& events, Dataset& dataset);

      /**
       * Returns the number of features in this dataset, so the size of the given subset of the variables
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_feature_indices.size(); }

      /**
       * Returns the number of events in the wrapped dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return m_use_event_indices ? m_event_indices.size() : m_dataset.getNumberOfEvents(); }

      /**
       * Load the event number iEvent from the wrapped dataset
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int iEvent) override;

      /**
       * Returns all values of one feature in a std::vector<float> of the wrapped dataset
       * @param iFeature the position of the feature to return in the given subset
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override;

    private:
      bool m_use_event_indices = false;
      std::vector<unsigned int>
      m_feature_indices; /**< Mapping from the position of a feature in the given subset to its position in the wrapped dataset */
      std::vector<unsigned int>
      m_event_indices; /**< Mapping from the position of a event in the given subset to its position in the wrapped dataset */
      Dataset& m_dataset; /**< Reference to the wrapped dataset */

    };


    /**
     * Proivdes a dataset from a ROOT file
     * This is the usually used dataset providing training data to the mva methods
     */
    class ROOTDataset : public Dataset {

    public:
      /**
       * Creates a new ROOTDataset
       * @param _general_options defines the rootfile, treename, branches, ...
       */
      explicit ROOTDataset(const GeneralOptions& _general_options);

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_input.size(); }

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return m_tree->GetEntries(); }

      /**
       * Load the event number iEvent from the TTree
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int event) override;

      /**
       * Returns all values of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override;

      /**
       * Virtual destructor
       */
      virtual ~ROOTDataset();

    private:
      /**
       * Sets the branch addresses of all features, weight and target again
       */
      void setBranchAddresses();

    protected:
      TTree* m_tree = nullptr; /**< Pointer to the TTree containing the data */
      TFile* m_file = nullptr; /**< Pointer to the TFile containing the data */

    };

  }
}
#endif
