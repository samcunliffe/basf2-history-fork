/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: KLM group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* KLM headers. */
#include <klm/muid/MuidElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <map>


namespace Belle2 {

  /**
   * Database object used to store the parameters for KLM likelihood computation.
   */
  class KLMLikelihoodParameters: public TObject {

  public:

    /**
     * Constructor
     */
    KLMLikelihoodParameters()
    {
    }

    /**
     * Destructor.
     */
    ~KLMLikelihoodParameters()
    {
    }

    /**
     * Set the longitudinal probability density function for specific hypothesis, outcome and last layer.
     * @param[in] hypothesis Hypothesis number.
     * @param[in] outcome    Track extrapolation outcome.
     * @param[in] lastLayer  Last layer crossed during the extrapolation.
     * @param[in] params     Likelihood parameters.
     */
    void setLongitudinalPDF(int hypothesis, int outcome, int lastLayer, const std::vector<double>& params)
    {
      const int id =  getLongitudinalID(hypothesis, outcome, lastLayer);
      setLongitudinalPDF(id, params);
    }

    /**
     * Set the transverse probability density function for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] params           Likelihood parameters.
     */
    void setTransversePDF(int hypothesis, int detector, int degreesOfFreedom, const std::vector<double>& params)
    {
      int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransversePDF(id, params);
    }

    /**
     * Set the transverse probability density function (analytical): threshold for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] params           Likelihood parameters.
     */
    void setTransverseThreshold(int hypothesis, int detector, int degreesOfFreedom, const double threshold)
    {
      int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransverseThreshold(id, threshold);
    }

    /**
     * Set the transverse probability density function (analytical): horizontal scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] params           Likelihood parameters.
     */
    void setTransverseScaleX(int hypothesis, int detector, int degreesOfFreedom, const double scaleX)
    {
      int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransverseScaleX(id, scaleX);
    }

    /**
     * Set the transverse probability density function (analytical): vertical scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] params           Likelihood parameters.
     */
    void setTransverseScaleY(int hypothesis, int detector, int degreesOfFreedom, const double scaleY)
    {
      int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransverseScaleY(id, scaleY);
    }

    /**
     * Get the longitudinal probability density function for specific hypothesis, outcome and last layer.
     * @param[in] hypothesis Hypothesis number.
     * @param[in] outcome    Track extrapolation outcome.
     * @param[in] lastLayer  Last layer crossed during the extrapolation.
     */
    const std::vector<double>& getLongitudinalPDF(int hypothesis, int outcome, int lastLayer) const
    {
      const int id = getLongitudinalID(hypothesis, outcome, lastLayer);
      return getLongitudinalPDF(id);
    }

    /**
     * Get the transverse probability density function for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    const std::vector<double>& getTransversePDF(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransversePDF(id);
    }

    /**
     * Get the transverse probability density function (analytical): threshold for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    double getTransverseThreshold(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransverseThreshold(id);
    }

    /**
     * Get the transverse probability density function (analytical): horizontal scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    double getTransverseScaleX(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransverseScaleX(id);
    }

    /**
     * Get the transverse probability density function (analytical): vertical scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    double getTransverseScaleY(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransverseScaleY(id);
    }

  private:

    /**
     * Get the unique longitudinal identifier for given hypothesis, outcome and last layer crossed.
     * bits 0-3 for hypothesis;
     * bits 4-10 for outcome;
     * bits 11-14 for last layer crossed.
     * @param[in] hypothesis Hypothesis number.
     * @param[in] outcome    Track extrapolation outcome.
     * @param[in] lastLayer  Last layer crossed during the extrapolation..
     */
    int getLongitudinalID(int hypothesis, int outcome, int lastLayer) const
    {
      int id = lastLayer + (outcome << MuidElementNumbers::c_LastLayerBit) + (hypothesis << (MuidElementNumbers::c_LastLayerBit +
               MuidElementNumbers::c_OutcomeBit));
      return id;
    }

    /**
     * Get the unique transverse identifier for given hypothesis, detector and degrees of freedom.
     * bit 0-3 for hypothesis;
     * bit 4-5 for detector;
     * bit 6-11 for degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    int getTransverseID(int hypothesis, int detector, int degreesOfFreedom) const
    {
      int id = degreesOfFreedom + (detector << MuidElementNumbers::c_DegreesOfFreedomBit) + (hypothesis <<
               (MuidElementNumbers::c_DegreesOfFreedomBit +
                MuidElementNumbers::c_DetectorBit));
      return id;
    }

    /**
     * Set the longitudinal probability density function for a given identifier.
     * @param[in] id     Longitudinal identifier.
     * @param[in] params Likelihood parameters.
     */
    void setLongitudinalPDF(const int id, const std::vector<double>& params)
    {
      m_LongitudinalPDF.insert(std::pair<int, std::vector<double>>(id, params));
    }

    /**
     * Set the transverse probability density function for a given identifier.
     * @param[in] id     Transverse identifier.
     * @param[in] params Likelihood parameters.
     */
    void setTransversePDF(const int id, const std::vector<double>& params)
    {
      m_TransversePDF.insert(std::pair<int, std::vector<double>>(id, params));
    }

    /**
     * Set the transverse probability density function (analytical): threshold for a given identifier.
     * @param[in] id     Transverse identifier.
     * @param[in] params Likelihood parameters.
     */
    void setTransverseThreshold(const int id, const double threshold)
    {
      m_TransverseThreshold.insert(std::pair<int, double>(id, threshold));
    }

    /**
     * Set the transverse probability density function (analytical): horizontal scale for a given identifier.
     * @param[in] id     Transverse identifier.
     * @param[in] params Likelihood parameters.
     */
    void setTransverseScaleX(const int id, const double scaleX)
    {
      m_TransverseScaleX.insert(std::pair<int, double>(id, scaleX));
    }

    /**
     * Set the transverse probability density function (analytical): vertical scale for a given identifier.
     * @param[in] id     Transverse identifier.
     * @param[in] params Likelihood parameters.
     */
    void setTransverseScaleY(const int id, const double scaleY)
    {
      m_TransverseScaleY.insert(std::pair<int, double>(id, scaleY));
    }

    /**
     * Get the longitudinal probability density function for a given identifier.
     * @param[in] id Longitudinal identifier.
     */
    const std::vector<double>& getLongitudinalPDF(const int id) const
    {
      std::map<int, std::vector<double>>::const_iterator it = m_LongitudinalPDF.find(id);
      if (it != m_LongitudinalPDF.end()) {
        return it->second;
      } else {
        B2FATAL("Longitudinal for KLMLikelihoodParameters not found!");
      }
    }

    /**
     * Get the transverse probability density function for a given identifier.
     * @param[in] id Transverse identifier.
     */
    const std::vector<double>& getTransversePDF(const int id) const
    {
      std::map<int, std::vector<double>>::const_iterator it = m_TransversePDF.find(id);
      if (it != m_TransversePDF.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF for KLMLikelihoodParameters not found!");
      }
    }

    /**
     * Get the transverse probability density function (analytical): threshold for a given identifier.
     * @param[in] id Transverse identifier.
     */
    double getTransverseThreshold(const int id) const
    {
      std::map<int, double>::const_iterator it = m_TransverseThreshold.find(id);
      if (it != m_TransverseThreshold.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse threshold for KLMLikelihoodParameters not found!");
      }
    }

    /**
     * Get the transverse probability density function (analytical): horizontal scale for a given identifier.
     * @param[in] id Transverse identifier.
     */
    double getTransverseScaleX(const int id) const
    {
      std::map<int, double>::const_iterator it = m_TransverseScaleX.find(id);
      if (it != m_TransverseScaleX.end()) {
        return it->second;
      } else {
        B2FATAL("Horizontal transverse scale for KLMLikelihoodParameters not found!");
      }
    }

    /**
     * Get the transverse probability density function (analytical): vertical scale for a given identifier.
     * @param[in] id Transverse identifier.
     */
    double getTransverseScaleY(const int id) const
    {
      std::map<int, double>::const_iterator it = m_TransverseScaleY.find(id);
      if (it != m_TransverseScaleY.end()) {
        return it->second;
      } else {
        B2FATAL("Vertical transverse scale for KLMLikelihoodParameters not found!");
      }
    }

    /** Longitudinal probability density function. */
    std::map<int, std::vector<double>> m_LongitudinalPDF;

    /** Transverse probability density function. */
    std::map<int, std::vector<double>> m_TransversePDF;

    /** Transverse probability density function (analytical): threshold. */
    std::map<int, double> m_TransverseThreshold;

    /** Transverse probability density function (analytical): horizontal scale. */
    std::map<int, double> m_TransverseScaleX;

    /** Transverse probability density function (analytical): vertical scale. */
    std::map<int, double> m_TransverseScaleY;

    /** ClassDef */
    ClassDef(KLMLikelihoodParameters, 1);

  };

}
