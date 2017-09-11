/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <boost/variant/variant.hpp>
#include <string>
#include <vector>
#include <map>

namespace Belle2 {
  namespace TrackFindingCDC {
    class ParamList;

    /// Type variant with allowed parameters types
    using ParameterVariant =
      boost::variant<bool, int, double, std::string, std::vector<double>, std::vector<std::string>>;

    /// Map of names to the variant parameter values
    using ParameterVariantMap = std::map<std::string, ParameterVariant>;

    /// A helper class to unpack a boost::variant parameter value and set it in the parameter list.
    struct AssignParameterVisitor : public boost::static_visitor<> {

      /// Transfer all the parameters from the map boost:variant values to the module parmeter list.
      template <class ... T>
      static void update(ParamList* paramList,
                         const std::map<std::string, boost::variant<T...> >& valuesByName)
      {
        for (auto& nameAndValue : valuesByName) {
          const std::string& name = nameAndValue.first;
          const boost::variant<T...>& value = nameAndValue.second;
          AssignParameterVisitor visitor(paramList, name);
          boost::apply_visitor(visitor, value);
        }
      }

      /// Constructor taking the module parameter list and the name of the parameter to be set from the boost::variant.
      AssignParameterVisitor(ParamList* paramList, const std::string& paramName);

      /// Function call that receives the parameter value from the boost::variant with the correct type.
      template <class T>
      void operator()(const T& t) const;

    private:
      /// Parameter list which contains the parameter to be set
      ParamList* m_paramList;

      /// Name of the parameter to be set.
      std::string m_paramName;
    };
  }
}
