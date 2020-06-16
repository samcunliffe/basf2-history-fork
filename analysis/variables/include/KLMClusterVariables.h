/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Torben Ferber, Giacomo De Pietro            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>

namespace Belle2 {

  namespace Variable {

    /**
     * returns the KlId associated to the KLMCluster
     */
    double klmClusterKlId(const Particle* particle);

    /**
     * returns the Belle-style Track flag
     */
    int klmClusterBelleTrackFlag(const Particle* particle);

    /**
     * returns the Belle-style ECL flag
     */
    int klmClusterBelleECLFlag(const Particle* particle);

    /**
     * returns KLMCluster's timing
     */
    double klmClusterTiming(const Particle* particle);

    /**
     * returns KLMCluster's x position
     */
    double klmClusterPositionX(const Particle* particle);

    /**
     * returns KLMCluster's y position
     */
    double klmClusterPositionY(const Particle* particle);

    /**
     * returns KLMCluster's z position
     */
    double klmClusterPositionZ(const Particle* particle);

    /**
     * returns KLMCluster's number of the innermost layer with hits
     */
    double klmClusterInnermostLayer(const Particle* particle);

    /**
     * returns KLMCluster's number of layers with hits
     */
    double klmClusterLayers(const Particle* particle);

    /**
     * returns KLMCluster's energy (assuming the K_L0 hypothesis)
     */
    double klmClusterEnergy(const Particle* particle);

    /**
     * returns KLMCluster's momentum magnitude. N.B.: klmClusterMomentum is proportional to klmClusterLayers
     */
    double klmClusterMomentum(const Particle* particle);

    /**
     * returns 1 if the associated KLMCluster is in BKLM
     */
    double klmClusterIsBKLM(const Particle* particle);

    /**
     * returns 1 if the associated KLMCluster is in EKLM
     */
    double klmClusterIsEKLM(const Particle* particle);

    /**
     * returns 1 if the associated KLMCluster is in forward EKLM
     */
    double klmClusterIsForwardEKLM(const Particle* particle);

    /**
     * returns 1 if the associated KLMCluster is in backward EKLM
     */
    double klmClusterIsBackwardEKLM(const Particle* particle);

    /**
     * returns KLMCluster's theta
     */
    double klmClusterTheta(const Particle* particle);

    /**
     * returns KLMCluster's phi
     */
    double klmClusterPhi(const Particle* particle);

    /**
     * returns the maximum angle of a KLMCluster to this Particle in the CMS frame
     */
    double maximumKLMAngleCMS(const Particle* particle);

    /**
     * returns the number of Tracks matched to the KLMCluster associated to this Particle
     * (>0 for K_L0 and matched Tracks, NaN for not-matched Tracks).
     */
    double nKLMClusterTrackMatches(const Particle* particle);

    /**
     * returns the number of KLMClusters matched to the Track associated to this Particle.
     * It can return only 0, 1 or NaN (it returns NaN for K_L0 candidates with no Tracks associated).
     */
    double nMatchedKLMClusters(const Particle* particle);

    /**
     * returns the distance between the Track and the KLMCluster associated to this Particle.
     * This variable returns NaN if there is no Track-to-KLMCluster relationship.
     */
    double klmClusterTrackDistance(const Particle* particle);

  }
} // Belle2 namespace

