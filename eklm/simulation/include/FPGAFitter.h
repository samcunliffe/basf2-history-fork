/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FPGAFITTER_H
#define FPGAFITTER_H

/**
 * @file
 * This file will contain an exact reimplementation of FPGA fitter.
 * Now it is just a fast fitter not using Minuit.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * FPGA fit status.
     */
    enum FPGAFitStatus {
      c_FPGASuccessfulFit, /**< Successful fit. */
      c_FPGANoSignal,      /**< Signal is too small to do any fitting. */
    };

    /**
     * Fit parameters/results.
     */
    struct FPGAFitParams {
      double startTime;       /**< Start of signal. */
      double peakTime;        /**< Peak time (from startTime). */
      double attenuationFreq; /**< Attenuation frequency. */
      double amplitude;       /**< Amplitude. */
      double bgAmplitude;     /**< Background amplitude. */
    };

    /**
     * FPGA fitter class.
     */
    class FPGAFitter {

    public:

      /**
       * Constructor.
       * @param[in]  nPoints Number of points in amplitude arrays.
       */
      FPGAFitter(int nPoints);

      /**
       * Destructor.
       */
      ~FPGAFitter();

      /**
       * FPGA fitter.
       * @param[in]  amp     Digital amplitude.
       * @param[in]  fit     Fit result histogram.
       * @param[out] par     Fit parameters.
       * @return Fit status.
       */
      enum FPGAFitStatus fit(int* amp, float* fit, struct FPGAFitParams* par);

    private:

      /** Number of points. */
      int m_nPoints;

      /** Signal shape. */
      float* m_sig;

    };

  }

}

#endif

