/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDStateObject {
  public:
    static constexpr unsigned int N = 8;

    using SeedObject = RecoTrack;
    using HitObject = SpacePoint;

    CKFCDCToVXDStateObject() = default;

    void initialize(RecoTrack* seed)
    {
      m_seedRecoTrack = seed;

      m_measuredStateOnPlane = seed->getMeasuredStateOnPlaneFromFirstHit();
      m_cachedMeasuredStateOnPlane = seed->getMeasuredStateOnPlaneFromFirstHit();

      // Reset other state
      m_hitObject = nullptr;
      m_number = N;
      m_parent = nullptr;

      m_chi2 = 0;

      m_isFitted = false;
      m_isAdvanced = false;

      m_hasCache = false;
    }

    void set(CKFCDCToVXDStateObject* parent, const HitObject* hitObject)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_number = parent->getNumber() - 1;
      m_hitObject = hitObject;

      m_measuredStateOnPlane = parent->getMeasuredStateOnPlane();
      m_cachedMeasuredStateOnPlane = parent->getMeasuredStateOnPlane();

      // Reset other state
      m_chi2 = 0;

      m_isFitted = false;
      m_isAdvanced = false;

      m_hasCache = false;
    }


    std::pair<RecoTrack*, std::vector<const HitObject*>> finalize() const
    {
      std::vector<const HitObject*> hits;
      hits.reserve(N);

      const auto& hitAdder = [&hits](const CKFCDCToVXDStateObject * walkObject) {
        const HitObject* hitObject = walkObject->getHit();
        if (hitObject) {
          hits.push_back(hitObject);
        }
      };
      walk(hitAdder);

      return std::make_pair(getSeedRecoTrack(), hits);
    }

    // const Getters
    const CKFCDCToVXDStateObject* getParent() const
    {
      return m_parent;
    }

    RecoTrack* getSeedRecoTrack() const
    {
      return m_seedRecoTrack;
    }

    const HitObject* getHit() const
    {
      return m_hitObject;
    }

    unsigned int getNumber() const
    {
      return m_number;
    }

    unsigned int extractGeometryLayer() const
    {
      return static_cast<unsigned int>((static_cast<double>(m_number) / 2) + 3);
    }

    bool isOnOverlapLayer() const
    {
      return m_number % 2 == 0;
    }


    unsigned int getNumberOfHoles() const
    {
      unsigned int numberOfHoles = 0;

      walk([&numberOfHoles](const CKFCDCToVXDStateObject * walkObject) {
        if (not walkObject->getHit()) {
          numberOfHoles++;
        }
      });

      return numberOfHoles;
    }

    // mSoP handling
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane() const
    {
      return m_measuredStateOnPlane;
    }

    genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane()
    {
      return m_measuredStateOnPlane;
    }

    const genfit::MeasuredStateOnPlane& getParentsCachedMeasuredStateOnPlane() const
    {
      return m_parent->m_cachedMeasuredStateOnPlane;
    }

    genfit::MeasuredStateOnPlane& getParentsCachedMeasuredStateOnPlane()
    {
      return m_parent->m_cachedMeasuredStateOnPlane;
    }

    bool parentHasCache() const
    {
      return m_parent->m_hasCache;
    }

    void setParentHasCache()
    {
      m_parent->m_hasCache = true;
    }

    // chi2
    double getChi2() const
    {
      return m_chi2;
    }

    double& getChi2()
    {
      return m_chi2;
    }

    // State control
    bool isFitted() const
    {
      return m_isFitted;
    }

    void setFitted()
    {
      m_isFitted = true;
    }

    bool isAdvanced() const
    {
      return m_isAdvanced;
    }

    void setAdvanced()
    {
      m_isAdvanced = true;
    }

  private:
    RecoTrack* m_seedRecoTrack = nullptr;
    const HitObject* m_hitObject = nullptr;
    unsigned int m_number = N;
    CKFCDCToVXDStateObject* m_parent = nullptr;

    double m_chi2 = 0;

    bool m_isFitted = false;
    bool m_isAdvanced = false;

    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;

    bool m_hasCache = false;
    genfit::MeasuredStateOnPlane m_cachedMeasuredStateOnPlane;


    void walk(const std::function<void(const CKFCDCToVXDStateObject*)> f) const
    {
      const CKFCDCToVXDStateObject* walkObject = this;

      while (walkObject != nullptr) {
        f(walkObject);
        walkObject = walkObject->getParent();
      }
    }
  };
}
