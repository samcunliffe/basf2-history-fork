/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMFIBERANDELECTRONICS_H
#define EKLMFIBERANDELECTRONICS_H

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/geometry/TransformData.h>
#include <eklm/simulation/Digitizer.h>
#include <eklm/simulation/FPGAFitter.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Digitize EKLMSimHits to get EKLM StripHits.
     */
    class FiberAndElectronics {

    public:

      /**
       * Constructor.
       */
      FiberAndElectronics(std::pair <int, std::vector<EKLMSimHit*> >,
                          struct EKLM::TransformData* transf,
                          struct EKLM::DigitizationParams* digPar);

      /**
       * Destructor.
       */
      ~FiberAndElectronics();

      /**
       * Process.
       */
      void processEntry();

      /**
       * Get fit parameter.
       * @return i'th parameter of the fit.
       */
      struct FPGAFitParams* getFitResults();

      /**
       * Get fit status.
       * @return Status of the fit.
       */
      enum FPGAFitStatus getFitStatus() const;

    private:

      /** Transformation data. */
      struct EKLM::TransformData* m_transf;

      /** Parameters. */
      struct EKLM::DigitizationParams* m_digPar;

      /** Stands for nDigitizations*ADCSamplingTime. */
      double m_histRange;

      /** Analog amplitude (direct). */
      float* m_amplitudeDirect;

      /** Analog amplitude (reflected). */
      float* m_amplitudeReflected;

      /** Analog amplitude. */
      float* m_amplitude;

      /** Digital amplitude. */
      int* m_ADCAmplitude;

      /** Digital fit result. */
      float* m_ADCFit;

      /** FPGA fit status. */
      enum FPGAFitStatus m_FPGAStat;

      /** FPGA fit results. */
      struct FPGAFitParams m_FPGAParams;

      /** Pointer to vector if the SimHits. */
      std::vector<EKLMSimHit*> m_vectorHits;

      /**
       * Distance from the hitpoint to SiPM for the (forward, backward) photons
       * (no account for the angle).
       */
      std::pair<double, double> m_hitDist;

      /** Name of the strip. */
      std::string m_stripName;

      /**
       * Convert time to the histogram.
       * @param times Vector of hits.
       * @param shape Histogram.
       */
      void timesToShape(const std::vector <double> & times, float* shape);

      /**
       * Calculate StripHit times (at the end of the strip),
       * @param[in] Number of photoelectrons.
       * @param[in] Time of the SimHit.
       * @param[in] If the hit is direct or reflected.
       * @return Vector of hit times.
       */
      std::vector<double> hitTimes(int nPE, double timeShift,
                                   bool isReflected = true);

      /**
       * Get delay depending on the distance to the hit.
       * @param[in] L Distance in cm.
       * @return Delay.
       */
      double lightPropagationTime(double L);

      /**
       * Calculate 'distances' to the direct and mirrored hits.
       * @param[in] sh EKLMSimHit.
       */
      void lightPropagationDistance(EKLMSimHit*);

      /**
       * Reflect time-shape of 1p.e. signal.
       * Amplitude should be 1, exp tail defined by 1 parameter
       * @param[in] t Time.
       * @return Signal shape.
       */
      double signalShape(double t);

      /**
       * Add random noise to the signal (amplitude-dependend).
       */
      void addRandomSiPMNoise();

      /**
       * Amplitude attenuation with a distance f(l)=distanceAttenuation(l)*f(0).
       * @param[in] dist Distance.
       * @return Amplitude attenuation.
       */
      double distanceAttenuation(double dist);

      /**
       * Simulate ADC (create digital signal from analog),
       */
      void simulateADC();

      /** Minimal hit time. */
      double m_min_time;

    };

  }

}

#endif

