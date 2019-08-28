/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// needed to build variables here
#include <reconstruction/variables/DedxVariables.h>
#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

namespace Belle2 {

  /**
  * dEdx value from particle
  */
  CDCDedxTrack const* getDedxFromParticle(Particle const* particle)
  {
    const Track* track = particle->getTrack();
    if (!track) {
      return nullptr;
    }

    const CDCDedxTrack* dedxTrack = track->getRelatedTo<CDCDedxTrack>();
    if (!dedxTrack) {
      return nullptr;
    }

    return dedxTrack;
  }

  namespace Variable {

    double dedx(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getDedx();
      }
    }

    double dedxnosat(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getDedxNoSat();
      }
    }

    double pCDC(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getMomentum();
      }
    }

    double chiE(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(0);
      }
    }

    double chiMu(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(1);
      }
    }

    double chiPi(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(2);
      }
    }

    double chiK(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(3);
      }
    }

    double chiP(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(4);
      }
    }


    double chiD(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(5);
      }
    }


    VARIABLE_GROUP("Dedx");
    REGISTER_VARIABLE("dedx", dedx, "dE/dx truncated mean");
    REGISTER_VARIABLE("dedxnosat", dedxnosat, "dE/dx truncated mean without saturation correction");
    REGISTER_VARIABLE("pCDC", pCDC, "Momentum valid in the CDC");
    REGISTER_VARIABLE("chiE", chiE, "Chi value of electrons from CDC dEdx");
    REGISTER_VARIABLE("chiMu", chiMu, "Chi value of muons from CDC dEdx");
    REGISTER_VARIABLE("chiPi", chiPi, "Chi value of pions from CDC dEdx");
    REGISTER_VARIABLE("chiK", chiK, "Chi value of kaons from CDC dEdx");
    REGISTER_VARIABLE("chiP", chiP, "Chi value of protons from CDC dEdx");
    REGISTER_VARIABLE("chiD", chiD, "Chi value of duetrons from CDC dEdx");
  }
}
