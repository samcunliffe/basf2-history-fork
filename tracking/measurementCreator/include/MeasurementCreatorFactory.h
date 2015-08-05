/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <memory>

namespace Belle2 {
  template <class BaseMeasurementCreatorType>
  class MeasurementCreatorFactory {
  public:
    /** Typedef for convenience */
    typedef BaseMeasurementCreatorType CreatorType;

    /** We need a virtual destr */
    virtual ~MeasurementCreatorFactory() { }

    /** Use the parameters given to the module and create the measurement creators from them */
    void initialize()
    {
      for (const auto& creatorWithParameterDictionary : m_creatorsWithParametersDictionary) {
        const std::string& creatorName = creatorWithParameterDictionary.first;
        const std::map<std::string, std::string>& parameterDictionary = creatorWithParameterDictionary.second;

        B2INFO("Creating measurement creator with name " << creatorName);

        BaseMeasurementCreatorType* creatorPointer = createMeasurementCreatorFromName(creatorName);
        if (creatorPointer == nullptr) {
          B2FATAL("Can not create a measurement creator with the name " << creatorName);
        }
        m_measurementCreators.push_back(std::move(std::unique_ptr<BaseMeasurementCreatorType>(creatorPointer)));

        for (const auto& parameterWithValue : parameterDictionary) {
          const std::string& parameterName = parameterWithValue.first;
          const std::string& parameterValue = parameterWithValue.second;
          creatorPointer->setParameter(parameterName, parameterValue);
        }
      }
    }

    /** Overload this method to create the measurement creators by their name */
    virtual BaseMeasurementCreatorType* createMeasurementCreatorFromName(const std::string& creatorName) const = 0;

    /** Return the creators to tht module */
    const std::vector<std::unique_ptr<BaseMeasurementCreatorType>>& getCreators() const
    {
      return m_measurementCreators;
    }

    std::map<std::string, std::map<std::string, std::string>>& getParameters()
    {
      return m_creatorsWithParametersDictionary;
    }

  private:
    /** A vector with the measurement creators. Is filled in initialize */
    std::vector<std::unique_ptr<BaseMeasurementCreatorType>> m_measurementCreators;

    /** The map of dictionaries of the parameters */
    std::map<std::string, std::map<std::string, std::string>> m_creatorsWithParametersDictionary;
  };
}
