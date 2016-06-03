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

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Marks all hits that are not on the first loop of the track by considering the mc truth information as background
    class WireHitMCMultiLoopBlocker : public Findlet<CDCWireHit> {

    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Marks all hits that were not reached after the specified number of loops as background based on MC information.";
      }

      /// Expose the parameters of the wire hit preparation
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override
      {
        moduleParamList->addParameter(prefixed(prefix, "UseNLoops"),
                                      m_param_useNLoops,
                                      "Maximal number of loops accepted as non background",
                                      m_param_useNLoops);
      }

      /// Signals the start of the event processing
      virtual void initialize() override final
      {
        Super::initialize();
        if (std::isnan(m_param_useNLoops)) return;
        CDCMCManager::getInstance().requireTruthInformation();

      }

      /// Prepare the Monte Carlo information at the start of the event
      virtual void beginEvent() override final
      {
        Super::beginEvent();
        if (std::isnan(m_param_useNLoops)) return;
        CDCMCManager::getInstance().fill();
      }

      /// Main algorithm marking the hit of higher loops as background
      virtual void apply(std::vector<CDCWireHit>& wireHits) override final
      {
        if (std::isnan(m_param_useNLoops)) return;
        const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

        auto isWithinMCLoops = [&mcHitLookUp, this](const CDCWireHit & wireHit) {
          const CDCSimHit* simHit = mcHitLookUp.getClosestPrimarySimHit(wireHit.getHit());
          if (not simHit) return false;
          // Reject hits with no assoziated CDCSimHit.

          const double tof = simHit->getFlightTime();

          const MCParticle* mcParticle = simHit->getRelated<MCParticle>();
          if (not mcParticle) return true;
          // Accept hits with no assoziated MCParticle (e.g. beam background.)

          const double speed = mcParticle->get4Vector().Beta() * Const::speedOfLight;

          const TVector3 mom3D = mcParticle->getMomentum();
          const float absMom2D = mom3D.Perp();
          const float absMom3D = mom3D.Mag();

          const Vector3D pos3D(0.0, 0.0, 0.0);
          const double bendRadius = absMom2DToBendRadius(absMom2D, pos3D);
          const double bendCircumfence =  2 * M_PI * bendRadius;
          const double loopLength = bendCircumfence * absMom3D / absMom2D;
          const double loopTOF =  loopLength / speed;
          if (tof > loopTOF * m_param_useNLoops) {
            return false;
          } else {
            return true;
          }
        };

        for (CDCWireHit& wireHit : wireHits) {
          if (not isWithinMCLoops(wireHit)) {
            wireHit.getAutomatonCell().setBackgroundFlag();
            wireHit.getAutomatonCell().setTakenFlag();
          }
        }
      }

    private:
      /// Parameter : Maximal fraction of loops of the mc particles trajectory needs to the hit to unblock it.
      double m_param_useNLoops = NAN;

    }; // end class WireHitMCMultiLoopBlocker

  } // end namespace TrackFindingCDC
} // end namespace Belle2
