/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_SIMPLECLUSTERCANDIDATE_H
#define SVD_SIMPLECLUSTERCANDIDATE_H

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * structure containing the relevant informations
     * of eachstrip of the cluster
     */
    struct stripInCluster {
      int recoDigitIndex; /**< index of the reco digit*/
      float charge; /**< strip charge*/
      float noise; /**<strip noise*/
      int cellID; /**<strip cellID*/
      float time; /**<6-sample CoG strip time*/
    };

    /**
     * Class representing a cluster candidate during simple clustering of the SVD
     */
    class SimpleClusterCandidate {

    public:

      /** Constructor to create an empty Cluster */
      SimpleClusterCandidate(VxdID vxdID, bool isUside, int sizeHeadTail, double cutSeed, double cutAdjacent, double cutSNR,
                             int timeAlgorithm);

      /**
       * Add a Strip to the current cluster.
       * Update the cluster seed seed.
       * @param stripInCluster aStrip to add to the cluster
       * @return true if the strip is on the expected side and sensor and it's next to the last strip added to the cluster candidate
       */
      bool add(VxdID vxdID, bool isUside, struct  stripInCluster& aStrip);

      /**
       * compute the position, time and their error of the cluster
       */
      void finalizeCluster();

      /**
       * return true if the cluster candidate can be promoted to cluster
       */
      bool isGoodCluster();

      /**
       * return the VxdID of the cluster sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * return true if the cluster is on the U/P side
       */
      bool isUSide() {return m_isUside;}

      /**
       * return the charge of the cluster
       */
      float getCharge() const { return m_charge; }

      /**
       * return the error of the charge of the cluster
       */
      float getChargeError() const { return m_chargeError; }

      /**
       * return the seed charge of the cluster
       */
      float getSeedCharge() const { return m_seedCharge; }

      /**
       * return the time of the cluster depending on the m_timeAlgorithm
       */
      float getTime() const;
      /**
       * return the time of the cluster for the 6-sample CoG
       */
      float get6SampleCoGTime() const { return m_6SampleTime; }
      /**
       * return the time of the cluster for the 3-sample CoG
       */
      float get3SampleCoGTime() const;
      /**
       * return the time of the cluster for the 2-sample ELS
       */
      float get3SampleELSTime() const;

      /**
       * return the error on the time of the cluster depending on the m_timeAlgorithm, not implemented yet
       */
      float getTimeError() const;
      /**
       * return the time of the cluster for the 6-sample CoG
       */
      float get6SampleCoGTimeError() const  { return m_6SampleTime; }
      /**
       * return the time of the cluster for the 3-sample CoG
       */
      float get3SampleCoGTimeError() const;
      /**
       * return the time of the cluster for the 3-sample ELS
       */
      float get3SampleELSTimeError() const;

      /**
       * returns the APVFloatSamples obtained summing
       * sample-by-sample all the strips on the cluster
       */
      Belle2::SVDShaperDigit::APVFloatSamples getClsSamples() const;

      /**
       * return the position of the cluster
       */
      float getPosition() const { return m_position; }

      /**
       * return the error on the position of the cluster
       */
      float getPositionError() const { return m_positionError; }

      /**
       * return the signal over noise ratio of the cluster
       */
      float getSNR() const { return m_SNR; }

      /**
       * return the cluster size (number of strips of the cluster
       */
      int size() const { return m_strips.size(); }

      /** returns the vector of the strips in the cluster*/
      const std::vector<stripInCluster> getStripsInCluster() const { return m_strips; };

    protected:

      /** cluster is not good if something goes wrong */
      bool m_stopCreationCluster = false;

      /** VxdID of the cluster */
      VxdID m_vxdID;

      /** side of the cluster */
      bool m_isUside;

      /** number of strips after which we switch from COG to HeadTail estimation of the position*/
      int m_sizeHeadTail;

      /** SNR above which the strip can be considered as seed*/
      double m_cutSeed;

      /** SNR above which the strip can be considered for clustering*/
      double m_cutAdjacent;

      /** SNR above which the cluster is ok*/
      double m_cutCluster;

      /** selects the algorithm to compute the cluster tim
       *  0 = 6-sample CoG (default)
       *  1 = 3-sample CoG
       *  2 = 3-sample ELS
       */
      int m_timeAlgorithm = 0;

      /** Charge of the cluster */
      float m_charge;

      /** Error on Charge of the cluster */
      float m_chargeError;

      /** Seed Charge of the cluster */
      float m_seedCharge;

      /** Time of the cluster computed with the 6-sample CoG*/
      float m_6SampleTime;

      /** Error on Time of the cluster computed with the 6-sample CoG (not implemented yet)*/
      float m_6SampleTimeError;

      /** Position of the cluster */
      float m_position;

      /** Error on Position of the cluster */
      float m_positionError;

      /** SNR of the cluster */
      float m_SNR;

      /** SNR of the seed strip */
      float m_seedSNR;

      /** SVDRecoDigit index of the seed strip of the cluster */
      int m_seedIndex;

      /** vector containing the strips in the cluster */
      std::vector<stripInCluster> m_strips;

    };

  }

}

#endif //SVD_SIMPLECLUSTERCANDIDATE_H
