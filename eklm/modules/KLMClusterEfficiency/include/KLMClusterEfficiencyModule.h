/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMCLUSTEREFFICIENCYMODULE_H
#define KLMCLUSTEREFFICIENCYMODULE_H

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module for KLM cluster reconstruction efficiency studies.
   */
  class KLMClusterEfficiencyModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMClusterEfficiencyModule();

    /**
     * Destructor.
     */
    ~KLMClusterEfficiencyModule();

    /**
     * Initializer.
     */
    void initialize();

    /**
     * Called when entering a new run.
     */
    void beginRun();

    /**
     * This method is called for each event.
     */
    void event();

    /**
     * This method is called if the current run ends.
     */
    void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    void terminate();

  private:

    /** Output file name. */
    std::string m_OutputFileName;

    /** Draw cluster histograms for this number of events. */
    int m_EventsClusterHistograms;

    /** Output file. */
    TFile* m_OutputFile;

    /** Output tree. */
    TTree* m_OutputTree;

    /** MCParticle decay vertex X coordinate. */
    float m_DecayVertexX;

    /** MCParticle decay vertex Y coordinate. */
    float m_DecayVertexY;

    /** MCParticle decay vertex Z coordinate. */
    float m_DecayVertexZ;

    /** Maximal angle between MCParticle decay vertex and its hits. */
    float m_MaxDecayVertexHitAngle;

    /** Cluster X coordinate. */
    float m_ClusterX;

    /** Cluster Y coordinate. */
    float m_ClusterY;

    /** Cluster Z coordinate. */
    float m_ClusterZ;

    /** Maximal angle between KLM cluster and its hits. */
    float m_MaxClusterHitAngle;

    /** Number of clusters from a K_L0. */
    int m_KL0Clusters;

    /** Number of clusters from a K_L0 + other particles. */
    int m_PartlyKL0Clusters;

    /** Number of clusters from other particles. */
    int m_OtherClusters;

    /** Number of nonreconstructed K_L0. */
    int m_NonreconstructedKL0;

    /** Number of K_L0 reconstructed as 1, 2 clusters in EKLM. */
    int m_ReconstructedKL0EKLM[2];

    /** Number of K_L0 reconstructed as 1, 2 clusters in BKLM. */
    int m_ReconstructedKL0BKLM[2];

    /** Number of K_L0 reconstructed as 2 clusters, 1 in EKLM and 1 in BKLM. */
    int m_ReconstructedKL0EKLMBKLM;

    /** Number of K_L0 reconstructed as >= 2 clusters. */
    int m_ReconstructedKL03Clusters;

  };

}

#endif

