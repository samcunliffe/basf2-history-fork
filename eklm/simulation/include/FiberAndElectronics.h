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
#include <eklm/simulation/Digitizer.h>
#include <eklm/simulation/FPGAFitter.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Digitize EKLMSim2Hits to get EKLM StripHits.
     */
    class FiberAndElectronics : public EKLMHitMCTime {

    public:

      /** Photoelectron data. */
      struct Photoelectron {
        int bin;        /**< Hit time bin in ADC output histogram */
        double expTime; /**< exp(-m_DigPar->PEAttenuationFreq * (-time)) */
      };

      /**
       * Constructor (only to fill external ADC histograms).
       */
      FiberAndElectronics(struct EKLM::DigitizationParams* digPar);

      /**
       * Constructor.
       */
      FiberAndElectronics(struct EKLM::DigitizationParams* digPar,
                          FPGAFitter* fitter);

      /**
       * Destructor.
       */
      ~FiberAndElectronics();

      /**
       * Process.
       */
      void processEntry();

      /**
       * Get fit results.
       */
      struct FPGAFitParams* getFitResults();

      /**
       * Get fit status.
       * @return Status of the fit.
       */
      enum FPGAFitStatus getFitStatus() const;

      /**
       * Get number of photoelectrons (fit result).
       */
      double getNPE();

      /**
       * Get generated number of photoelectrons.
       */
      int getGeneratedNPE();

      /**
       * Set hit range.
       * @param[in] it  First hit in this strip.
       * @param[in] end End of hit range.
       */
      void setHitRange(std::multimap<int, EKLMSimHit*>::iterator& it,
                       std::multimap<int, EKLMSimHit*>::iterator& end);

      /**
       * Fill SiPM output.
       * @param[in]     stripLen    Strip length.
       * @param[in]     distSiPM    Distance from hit to SiPM.
       * @param[in]     nPE         Number of photons to be simulated.
       * @param[in]     timeShift   Time of hit.
       * @param[in]     isReflected Whether the hits are reflected or not.
       * @param[in,out] hist        Output histogram (signal is added to it).
       * @param[out]    gnpe        Number of generated photoelectrons.
       */
      void fillSiPMOutput(double stripLen, double distSiPM, int nPhotons,
                          double timeShift, bool isReflected, float* hist,
                          int* gnpe);

    private:

      /** Parameters. */
      struct EKLM::DigitizationParams* m_DigPar;

      /** Fitter. */
      FPGAFitter* m_fitter;

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

      /** Buffer for signal time dependence calculation. */
      double* m_SignalTimeDependence;

      /** Buffer for signal time dependence calculation. */
      double* m_SignalTimeDependenceDiff;

      /** Buffer for photoelectron data. */
      struct Photoelectron* m_Photoelectrons;

      /** Buffer for photoelectron indices. */
      int* m_PhotoelectronIndex;

      /** Buffer for photoelectron indices. */
      int* m_PhotoelectronIndex2;

      /** Size of photoelectron data buffer. */
      int m_PhotoelectronBufferSize;

      /** FPGA fit status. */
      enum FPGAFitStatus m_FPGAStat;

      /** FPGA fit results. */
      struct FPGAFitParams m_FPGAParams;

      /** Number of photoelectrons (generated). */
      int m_npe;

      /** First hit. */
      std::multimap<int, EKLMSimHit*>::iterator m_hit;

      /** End of hits. */
      std::multimap<int, EKLMSimHit*>::iterator m_hitEnd;

      /** Name of the strip. */
      std::string m_stripName;

      /**
       * Reallocate photoelectron buffers.
       * @param[in] size New size of buffers.
       */
      void reallocPhotoElectronBuffers(int size);

      /**
       * Sort photoelectrons.
       * @param[in] nPhotoelectrons Number of photoelectrons.
       * @return Pointer to index array.
       */
      int* sortPhotoelectrons(int nPhotoelectrons);

      /**
       * Add random noise to the signal (amplitude-dependend).
       */
      void addRandomSiPMNoise();

      /**
       * Simulate ADC (create digital signal from analog),
       */
      void simulateADC();

      /**
       * Debug output (signal and fit result histograms).
       */
      void debugOutput();

    };

  }

}

#endif

