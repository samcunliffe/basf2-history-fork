/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <framework/logging/Logger.h>
#include <TH1F.h>
#include <top/reconstruction/TOPreco.h>


namespace Belle2 {
  namespace TOP {

    /**
     * Binned one dimensional PDF (a projection of PDF to time axis)
     */
    class TOP1Dpdf {

    public:

      /**
       * Full constructor
       * @param reco reconstruction object
       * @param moduleID slot number
       * @param binSize approximate bin size (use getBinSize() to get the actual one)
       */
      TOP1Dpdf(TOPreco& reco, int moduleID, double binSize);

      /**
       * Returns actual bin size used
       * @return bin size
       */
      double getBinSize() const {return m_binSize;}

      /**
       * Returns slot number
       * @return slot number
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns binned one dimensional PDF (projection to time axis)
       * @param name histogram name
       * @param title histogram title
       * @return PDF (signal + background)
       */
      TH1F getHistogram(std::string name, std::string title) const;

      /**
       * Returns photon times of selected photons in a given slot
       * @return times of TOPDigit::c_Good digits
       */
      const std::vector<double>& getPhotonTimes() const {return m_times;}

      /**
       * Returns number of photons
       * @return number of photons
       */
      int getNumOfPhotons() const {return m_times.size();}

      /**
       * Returns minimal time of signal PDF
       * @return minimal time
       */
      double getTminPDF() const {return m_tminPDF;}

      /**
       * Returns maximal time of signal PDF
       * @return maximal time
       */
      double getTmaxPDF() const {return m_tmaxPDF;}

      /**
       * Returns minimal time of selected photons
       * @return minimal time
       */
      double getTminFot() const {return m_tminFot;}

      /**
       * Returns maximal time of selected photons
       * @return maximal time
       */
      double getTmaxFot() const {return m_tmaxFot;}

      /**
       * Returns log likelihood
       * @param timeShift time shift of PDF
       * @return log likelihood
       */
      double getLogL(double timeShift);


    private:

      int m_moduleID = 0;   /**< slot number */
      int m_numBins = 0;    /**< number of bins for signal PDF */
      double m_minTime = 0; /**< lower edge of the first bin */
      double m_maxTime = 0; /**< upper edge of the last bin */

      std::vector<double> m_times;  /**< photon times, from a given slot */
      double m_tminPDF = 0; /**< minimal time of signal PDF */
      double m_tmaxPDF = 0; /**< maximal time of signal PDF */
      double m_tminFot = 0; /**< minimal time of photons */
      double m_tmaxFot = 0; /**< maximal time of photons */
      double m_bkg = 0;     /**< background [photons/bin] */

      double m_binSize = 0;        /**< bin size */
      std::vector<double> m_logF;  /**< log(PDF) values */
      double m_logBkg = 0;         /**< log(m_bkg) */

    };

  } // namespace TOP
} // namespace Belle2

