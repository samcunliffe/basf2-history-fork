/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * structure containing the relevant informations
     * of each strip of the raw cluster
     */
    struct stripInRawCluster {
      int shaperDigitIndex; /**< index of the shaper digit*/
      int cellID; /**<strip cellID*/
      int maxSample; /** ADC max of the acquired samples*/
      int noise; /** ADC noise */
      Belle2::SVDShaperDigit::APVFloatSamples samples; /** ADC of the acquired samples*/
    };

    /**
     * Class representing a raw cluster candidate during clustering of the SVD
     */
    class RawCluster {

    public:

      /**
       * Default Constructor to create an empty RawCluster
       */
      RawCluster() {};

      /**
       * Constructor to create an empty RawCluster
       */
      RawCluster(VxdID vxdID, bool isUside, double cutSeed, double cutAdjacent);

      /**
       * You can specify the name of StoreArray<SVDShaperDigit>
       * which are needed to get clustered samples.
       */
      RawCluster(VxdID vxdID, bool isUside, double cutSeed, double cutAdjacent, const std::string& storeShaperDigitsName);

      /**
       * Add a Strip to the current cluster.
       * Update the cluster seed strip.
       * @param stripInRawCluster aStrip to add to the cluster
       * @return true if the strip is on the expected side and sensor and it's next to the last strip added to the cluster candidate
       */
      bool add(VxdID vxdID, bool isUside, struct  stripInRawCluster& aStrip);

      /**
       * @return true if the raw cluster candidate can be promoted to raw cluster (seedMaxSample > 0 and seedSNR > cutSeed)
       */
      bool isGoodRawCluster();

      /**
       * @return the VxdID of the cluster sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * @return true if the cluster is on the U/P side
       */
      bool isUSide() {return m_isUside;}

      /**
       * @return the APVFloatSamples obtained summing
       * sample-by-sample all the strips on the cluster
       */
      Belle2::SVDShaperDigit::APVFloatSamples getClsSamples() const;

      /**
       * @return the float vector of clustered 3-samples
       * selected by the MaxSum method
       * with First Frame of the selection
       */
      std::pair<int, std::vector<float>> getMaxSum3Samples() const;

      /**
       * @return the cluster size (number of strips of the cluster)
       */
      int getSize() const { return m_strips.size(); }

      /**
       * @return the vector of the strips in the cluster
       */
      const std::vector<stripInRawCluster> getStripsInRawCluster() const { return m_strips; };

      /**
       * @return the max sample (in ADC) of the seed strip
       */
      int getSeedMaxSample() const {return m_seedMaxSample;};

      /**
       * @return the internal index (in the stripsInRawCluster vector) of the seed strip
       */
      int getSeedInternalIndex() const {return m_seedInternalIndex;};

    protected:

      /** VxdID of the cluster */
      VxdID m_vxdID;

      /** side of the cluster */
      bool m_isUside;

      /** SNR above which the strip can be considered as seed*/
      double m_cutSeed = 5;

      /** SNR above which the strip can be considered for clustering*/
      double m_cutAdjacent = 3;

      /** ADC MaxSample of the seed strip */
      int m_seedMaxSample = -1;

      /** SNR (using MaxSample) of the seed strip */
      float m_seedSNR = -1;

      /** SVDShaperDigit index of the seed strip of the cluster */
      int m_seedIndex = -1;

      /** stripsInRawCluster index of the seed strip of the cluster */
      int m_seedInternalIndex = -1;

      /** vector containing the strips in the cluster */
      std::vector<stripInRawCluster> m_strips;

      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;

    };

  }

}

