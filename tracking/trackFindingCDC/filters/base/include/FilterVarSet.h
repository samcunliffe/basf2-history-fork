/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/algorithms/CellWeight.h>

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const filterVarNames[] = {
      "is_cell",
      "cell_weight",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment pair.
     */
    template<class Filter_>
    class FilterVarNames : public VarNames<typename Filter_::Object> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(filterVarNames);

      /// Getter for the name a the given index
      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return filterVarNames[iName];
      }
    };

    /** Class that computes floating point variables from segment pair.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    template<class Filter_>
    class  FilterVarSet : public VarSet<FilterVarNames<Filter_> > {

    private:
      /// Type of the base class
      typedef VarSet<FilterVarNames<Filter_> > Super;

    public:
      /// Type of the filter
      typedef Filter_ Filter;

      /// Type of the object from which the filter judgement should be extracted.
      typedef typename Filter::Object Object;

      /// Construct the varset and take an optional prefix.
      FilterVarSet(const std::string& prefix = "",
                   std::unique_ptr<Filter> ptrFilter = std::unique_ptr<Filter>(new Filter())) :
        Super(prefix),
        m_ptrFilter(std::move(ptrFilter))
      {
      }

      using Super::named;

      /// Generate filter weight variable from the object
      virtual bool extract(const Object* obj) override final
      {
        bool extracted = Super::extract(obj);
        if (m_ptrFilter and obj) {
          CellWeight cellWeight = (*m_ptrFilter)(*obj);
          this->template var<named("cell_weight")>() = cellWeight;
          this->template var<named("is_cell")>() = not isNotACell(cellWeight);
          // Forward the nested result.
          return extracted;
        } else {
          return false;
        }

      }

      /// Initialize the filter before event processing
      virtual void initialize() override final
      {
        Super::initialize();
        if (m_ptrFilter) {
          m_ptrFilter->initialize();
        }
      }

      /// Terminate the filter after event processing
      virtual void terminate() override final
      {
        if (m_ptrFilter) {
          m_ptrFilter->terminate();
        }
        Super::terminate();
      }

    public:
      /// Filter from which to generate weight as a variable set;
      std::unique_ptr<Filter> m_ptrFilter;

    };


    // template<class Filter_>
    // bool FilterVarSet<Filter_>::extract(const Object* obj)
    // {

    // }

  }
}
