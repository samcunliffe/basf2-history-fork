/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMDigit.h>
#include <klm/dataobjects/eklm/EKLMHitBase.h>
#include <klm/dataobjects/eklm/EKLMHitGlobalCoord.h>
#include <klm/dataobjects/eklm/EKLMHitMCTime.h>

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class for 2d hits handling.
   */
  class EKLMHit2d : public RelationsObject, public EKLMHitBase,
    public EKLMHitGlobalCoord, public EKLMHitMCTime {

  public:

    /**
     * Constructor.
     */
    EKLMHit2d();

    /**
     * Constructor with two strips.
     * @param[in] s1 One of EKLMDigits.
     */
    explicit EKLMHit2d(EKLMDigit* s1);

    /**
     * Destructor.
     */
    ~EKLMHit2d();

    /**
     * Set Chi^2 of the crossing point.
     * @param[in] chi Chi^2.
     */
    void setChiSq(float chi);

    /**
     * Get Chi^2 of the crossing point.
     * @return Chi^2.
     */
    float getChiSq() const;

  private:

    /** Chi^2 of the hit. */
    float m_ChiSq;

    /** Class version. */
    ClassDef(Belle2::EKLMHit2d, 4);

  };

}
