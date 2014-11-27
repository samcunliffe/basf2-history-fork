/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include "tracking/trackFindingVXD/FilterTools/TBranchLeafType.h"
#include <TBranch.h>
#include <TTree.h>

#include <typeinfo>

namespace Belle2 {

  /** Represents a range of arithmetic types.
   *
   * Range is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */

  template< typename MinType, typename MaxType>
  class ClosedRange {
    MinType m_min;
    MaxType m_max;
  public:

    /** Constructor */
    ClosedRange(MinType min, MaxType max): m_min(min), m_max(max) {};

    /** Method used by the filter tools to decide on the fate of the pair.
     *
     * @param x is the result of some SelectionVariable applied to a pair of objects.
     * The return value is true if x belongs to the open set ( m_min, m_max )
     */
    template< class VariableType >
    inline bool contains(const VariableType& x) const { return m_min <= x && x <= m_max ;};

    /** Creates and sets the addresses of the leaves to store the min and max values.
     *
     * @param t  the pointer to the TTree that will contain the TBranch of this range.
     * @param branchName the name of the TBranch that will host this range.
     * @param variableName the name of the selection variable this range is applied to.
     *
     * The leaves will be named as the selection variable name with the "_min"/"_max"
     * suffixes for the min/max value.
     */
    void persist(TTree* t, const string& branchName, const string& variableName) {

      string leafList;
      leafList += variableName;
      leafList += "_min/";
      leafList += TBranchLeafType(m_min);
      leafList += ":";
      leafList += variableName;
      leafList += "_max/";
      leafList += TBranchLeafType(m_max);
      TBranch* branch = new TBranch(t, branchName.c_str() , & m_min, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    /** Accessor to the inf of the set (which is also the min) */
    MinType getInf(void) const { return m_min; } ;

    /** Accessor to the sup of the set (which is alsto the max) */
    MaxType getSup(void) const { return m_max; } ;

  };


}
