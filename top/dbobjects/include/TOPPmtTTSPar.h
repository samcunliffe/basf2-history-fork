/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Alessandro Gaz                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

#include <TRandom.h>

namespace Belle2 {

  /**
   * Parameterized TTS for each PMT channel
   */
  class TOPPmtTTSPar : public TObject {

  public:

    struct Gaussian {
      float fraction = 0;
      float mean = 0;
      float sigma = 0;
    };

    /**
     * number of PMT channels
     */
    enum {c_NumChannels = 16};

    /**
     * Default constructor
     */
    TOPPmtTTSPar()
    {}

    /**
     * Full constructor
     * @param serialNumber serial number
     */
    TOPPmtTTSPar(const std::string& serialNumber):
      m_serialNumber(serialNumber)
    {}


    /**
     * Append struct gauss
     * @param gaussian to be appended
     */
    void appendGaussian(const Gaussian& gaus) { m_gaussians.push_back(gaus);}


    /**
     * Append gaussian using its parameters (frac, mean, sigma)
     * @params fraction, mean, sigma
     */
    void appendGaussian(double fraction, double mean, double sigma)
    {
      Gaussian gaus;
      gaus.fraction = fraction;
      gaus.mean = mean;
      gaus.sigma = sigma;
      appendGaussian(gaus);
    }


    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}


    /**
     * Returns vector of gaussians
     * @return vector of gaussians
     */
    const std::vector<Gaussian>& getGaussians() const {return m_gaussian;}


    /**
     * Returns a random number, generated according to the distribution
     * @return random time
     */
    double getRandomTime() const
    {
      double prob = gRandom->Rndm();
      double s = 0;
      for (const auto& gaus : m_gaussians) {
        s = s + gaus.fraction;
        if (prob < s) {
          return gRandom->Gaus(gaus.mean, gaus.sigma);
        }
      }
      return 0;  // this should not happen, if fractions are properly normalized
    }


    /**
     * Normalizes the gaussian fractions to unity
     */
    void normalizeFractions()
    {
      double sum = 0;
      for (const auto& gaus : m_gaussians) {
        sum = sum + gaus.fraction;
      }
      if (sum == 0) return;
      for (const auto& gaus : m_gaussians) {
        gaus.fraction = gaus.fraction / sum;
      }
      return;
    }



  private:

    std::string m_serialNumber; /**< serial number, e.g. JTxxxx */
    std::vector<Gaussian> m_gaussians;

    ClassDef(TOPPmtTTSPar, 1); /**< ClassDef */

  };

} // end namespace Belle2


