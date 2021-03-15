/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* MDST headers. */
#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>

/* Belle 2 headers. */
#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {

  class BeamParametersFitter {

  public:

    /**
     * Constructor.
     */
    BeamParametersFitter()
    {
    }

    /**
     * Destructor.
     */
    ~BeamParametersFitter()
    {
    }

    /**
     * Set interval of validity.
     */
    void setIntervalOfValidity(const IntervalOfValidity& iov)
    {
      m_IntervalOfValidity = iov;
    }

  protected:

    /** Interval of validity. */
    IntervalOfValidity m_IntervalOfValidity;

  };

}
