/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/InverseRaytracer.h>
#include <top/reconstruction_cpp/FastRaytracer.h>
#include <top/reconstruction_cpp/YScanner.h>
#include <top/reconstruction_cpp/SignalPDF.h>
#include <top/geometry/TOPGeometryPar.h>
#include <vector>
#include <map>
#include <limits>

namespace Belle2 {
  namespace TOP {

    /**
     * PDF construction and log likelihood determination for a given track and particle hypothesis.
     */
    class PDFConstructor {

    public:

      /**
       * Signal PDF construction options
       */
      enum EPDFOption {
        c_Rough = 0,   /**< no dependence on y */
        c_Fine = 1,    /**< y dependent everywhere */
        c_Optimal = 2  /**< y dependent only where necessary */
      };

      /**
       * Options for storing signal PDF parameters
       */
      enum EStoreOption {
        c_Reduced = 0, /**< only PDF peak data */
        c_Full = 1     /**< also extra information */
      };


      /**
       * Class constructor
       * @param track track at TOP
       * @param hypothesis particle hypothesis
       * @param PDFOption signal PDF construction option
       * @param storeOption signal PDF store option
       */
      PDFConstructor(const TOPTrack& track, const Const::ChargedStable& hypothesis,
                     EPDFOption PDFOption = c_Optimal, EStoreOption storeOption = c_Reduced);

      /**
       * Sets time window for likelihood determination.
       * Window edges must not exceed those used during data taking or in simulation.
       * @param minTime lower edge
       * @param maxTime upper edge
       */
      static void setTimeWindow(double minTime, double maxTime)
      {
        s_minTime = minTime;
        s_maxTime = maxTime;
      }

      /**
       * Returns cosine of total reflection angle
       * @return cosine of total reflection angle at mean photon energy for beta = 1
       */
      double getCosTotal() const {return m_cosTotal;}

      /**
       * Returns cosine of Cerenkov angle at given photon energy
       * @param E photon energy [eV]
       * @return cosine of Cerenkov angle
       */
      double getCosCerenkovAngle(double E) const;

      /**
       * Returns signal PDF for all pixels
       * @return signal PDF
       */
      const std::vector<SignalPDF>& getSignalPDF() const {return m_signalPDFs;}

      /**
       * Returns the expected number of signal photons
       * @return expected number of signal photons
       */
      double getExpectedSignalPhotons() const {return m_signalPhotons;}

      /**
       * Returns number of calls of template function setSignalPDF<T> for a given peak type
       * @param type PDF peak type
       * @return number of calls
       */
      int getNCalls_setPDF(SignalPDF::EPeakType type) const {return m_ncallsSetPDF[type];}

      /**
       * Returns number of calls of function expandSignalPDF for a given peak type
       * @param type PDF peak type
       * @return number of calls
       */
      int getNCalls_expandPDF(SignalPDF::EPeakType type) const {return m_ncallsExpandPDF[type];}


    private:

      /**
       * Structure that enables defining a template function: direct photons
       */
      struct InverseRaytracerDirect {
        const InverseRaytracer* inverseRaytracer = 0; /**< inverse ray-tracer */
        const SignalPDF::EPeakType type = SignalPDF::c_Direct; /**< PDF peak type */

        /**
         * Solve inverse ray-tracing for direct photon.
         * @param xD unfolded position in x of photon at detection plane (unfolding w.r.t mirror)
         * @param zD position of detection plane
         * @param assumedEmission photon emission position and track angles
         * @param cer sine and cosine of Cerenkov angle
         * @param step step for numerical derivative calculation
         * @return index of solution on success or InverseRaytracer::ErrorCodes on fail
         */
        int solve(double xD, double zD, int, double, double,
                  const TOPTrack::AssumedEmission& assumedEmission,
                  const InverseRaytracer::CerenkovAngle& cer, double step = 0) const
        {
          return inverseRaytracer->solveDirect(xD, zD, assumedEmission, cer, step);
        }
      };

      /**
       * Structure that enables defining a template function: reflected photons
       */
      struct InverseRaytracerReflected {
        const InverseRaytracer* inverseRaytracer = 0; /**< inverse ray-tracer */
        const SignalPDF::EPeakType type = SignalPDF::c_Reflected; /**< PDF peak type */

        /**
         * Solve inverse ray-tracing for reflected photon.
         * @param xD unfolded position in x of photon at detection plane (unfolding w.r.t mirror)
         * @param zD position of detection plane
         * @param Nxm signed number of reflections in x before mirror
         * @param xmMin lower limit of the reflection positions on the mirror
         * @param xmMax upper limit of the reflection positions on the mirror
         * @param assumedEmission photon emission position and track angles
         * @param cer sine and cosine of Cerenkov angle
         * @param step step for numerical derivative calculation
         * @return index of solution on success or InverseRaytracer::ErrorCodes on fail
         */
        int solve(double xD, double zD, int Nxm, double xmMin, double xmMax,
                  const TOPTrack::AssumedEmission& assumedEmission,
                  const InverseRaytracer::CerenkovAngle& cer, double step = 0) const
        {
          return inverseRaytracer->solveReflected(xD, zD, Nxm, xmMin, xmMax, assumedEmission, cer, step);
        }
      };

      /**
       * Template function: sets signal PDF in a pixel column and for specific reflection in x
       * @param t template parameter of type InverseRaytracerDirect or InverseRaytracerReflected
       * @param col pixel column (0-based)
       * @param xD unfolded detection position in x
       * @param zD detection position in z
       * @param Nxm signed number of reflections in x before mirror (dummy for direct PDF)
       * @param xmMin lower limit of the reflection positions on the mirror (dummy for direct PDF)
       * @param xmMax upper limit of the reflection positions on the mirror (dummy for direct PDF)
       */
      template<class T>
      void setSignalPDF(T& t, unsigned col, double xD, double zD, int Nxm = 0, double xmMin = 0, double xmMax = 0);

      /**
       * Returns cosine and sine of cerenkov angle
       * @param dE energy difference to mean photon energy
       * @return cosine and sine of cerenkov angle
       */
      const InverseRaytracer::CerenkovAngle& cerenkovAngle(double dE = 0);

      /**
       * Sets signal PDF
       */
      void setSignalPDF();

      /**
       * Sets signal PDF for direct photons
       */
      void setSignalPDF_direct();

      /**
       * Sets signal PDF for reflected photons
       */
      void setSignalPDF_reflected();

      /**
       * Sets signal PDF for reflected photons at given reflection number
       * @param Nxm reflection number in x before mirror
       * @param xmMin lower limit of the reflection positions on the mirror
       * @param xmMax upper limit of the reflection positions on the mirror
       */
      void setSignalPDF_reflected(int Nxm, double xmMin, double xmMax);

      /**
       * Calculates unfolded detection position from known reflection position on the mirror and emission point
       * @param xM reflection position x on the mirror
       * @param Nxm reflection number before the mirror
       * @param xDs unfolded detection positions (appended at each call) [in/out]
       * @param minLen minimum of propagation lengths (updated at each call) [in/out]
       * @return true on success
       */
      bool detectionPositionX(double xM, int Nxm, std::vector<double>& xDs, double& minLen);

      /**
       * Corrects the solution of inverse ray-tracing with fast ray-tracing.
       * Corrected solution is available in m_fastRaytracer.
       * @param sol solution of inverse raytracing
       * @param dFic_dx derivative of Cerenkov azimuthal angle over photon detection coordinate x
       * @param xD unfolded detection position in x
       * @return true on success
       */
      bool doRaytracingCorrections(const InverseRaytracer::Solution& sol, double dFic_dx, double xD);

      /**
       * Returns the difference in xD between ray-traced solution rotated by dFic and input argument
       * @param dFic rotation angle around track direction (delta Cerenkov azimuthal angle)
       * @param sol solution of inverse raytracing
       * @param xD unfolded detection position in x
       * @return difference in xD
       */
      double deltaXD(double dFic, const InverseRaytracer::Solution& sol, double xD);

      /**
       * Checks if scan method of YScanner is really required to be called when option is c_Optimal.
       * With c_Optimal the scan method is called only when one of the detected photons is within the PDF peak.
       * @param col pixel column (0-based)
       * @param time PDF peak time
       * @param wid PDF peak width squared
       * @return true if required
       */
      bool isScanRequired(unsigned col, double time, double wid);

      /**
       * Returns photon propagation losses (bulk absorption, surface reflectivity, mirror reflectivity)
       * @param E photon energy
       * @param propLen propagation length
       * @param nx total number of reflections in x
       * @param ny total number of reflections in y
       * @param type peak type, e.g. direct or reflected
       * @return survival probability due to propagation losses
       */
      double propagationLosses(double E, double propLen, int nx, int ny, SignalPDF::EPeakType type);

      /**
       * Expands signal PDF in y (y-scan)
       * @param col pixel column (0-based)
       * @param D derivatives
       * @param type peak type, e.g. direct or reflected
       */
      void expandSignalPDF(unsigned col, const YScanner::Derivatives& D, SignalPDF::EPeakType type);

      /**
       * Estimates range of unfolded x coordinate of the hits on given plane perpendicular to z-axis
       * @param z position of the plane
       * @param xmi lower limit [out]
       * @param xma upper limit [out]
       * @return true if at least some photons can reach the plane within the time window
       */
      bool rangeOfX(double z, double& xmi, double& xma);

      /**
       * Performs a clip on x w.r.t xmi and xma
       * @param x true coordinate
       * @param Nx signed number of reflections
       * @param A bar width
       * @param xmi lower limit on unfolded coordinate x
       * @param xma upper limit on unfolded coordinate x
       * @return clipped x
       */
      double clip(double x, int Nx, double A, double xmi, double xma);

      /**
       * Finds the position on the mirror of the extreme reflection.
       * @param xE true emission position in x
       * @param zE emission position in z
       * @param zD detection position in z
       * @param Nxm signed number of reflections before mirror
       * @param A width of the mirror segment
       * @param mirror spherical mirror data
       * @return position of the extreme if exists or -A/2
       */
      double findReflectionExtreme(double xE, double zE, double zD, int Nxm, double A,
                                   const RaytracerBase::Mirror& mirror) const;

      /**
       * Returns the derivative of reflected position at given x.
       * This function is used to find the position of reflection extreme.
       * All arguments must be given in the mirror frame and in units of mirror radius.
       * @param x  position in x on the mirror
       * @param xe unfolded emission position in x
       * @param ze emission position in z
       * @param zd unfolded detection position in z
       * @return the derivative
       */
      double derivativeOfReflectedX(double x, double xe, double ze, double zd) const;

      int m_moduleID = 0; /**< slot ID */
      const TOPTrack& m_track;   /**< track at TOP */
      const Const::ChargedStable& m_hypothesis; /**< particle hypothesis */
      const InverseRaytracer* m_inverseRaytracer = 0; /**< inverse ray-tracer */
      const FastRaytracer* m_fastRaytracer = 0; /**< fast ray-tracer */
      const YScanner* m_yScanner = 0; /**< PDF expander in y */

      double m_tof = 0; /**< time-of-flight from IP to average photon emission position */
      double m_groupIndex = 0; /**< group refractive index at mean photon energy */
      double m_groupIndexDerivative = 0; /**< derivative (dn_g/dE) of group refractive index at mean photon energy */
      double m_cosTotal = 0; /**< cosine of total reflection angle */

      EPDFOption m_PDFOption = c_Optimal; /**< signal PDF construction option */
      EStoreOption m_storeOption = c_Reduced; /**< signal PDF storing option */
      std::vector<SignalPDF> m_signalPDFs; /**< parameterized signal PDF in pixels (index = pixelID - 1) */
      double m_signalPhotons = 0; /**< expected number of signal photons */

      std::map<double, InverseRaytracer::CerenkovAngle> m_cerenkovAngles; /**< sine and cosine of  Cerenkov angles */

      double m_dFic = 0; /**< temporary storage for dFic used in last call to deltaXD */
      double m_Fic = 0;  /**< temporary storage for Cerenkov azimuthal angle */
      mutable std::map <SignalPDF::EPeakType, int> m_ncallsSetPDF; /**< counter for number of calls to setSignalPDF<T> */
      mutable std::map <SignalPDF::EPeakType, int> m_ncallsExpandPDF; /**< counter for number of calls to expandSignalPDF */

      static double s_minTime; /**< time window lower edge */
      static double s_maxTime; /**< time window upper edge */

    };


    inline double PDFConstructor::clip(double x, int Nx, double A, double xmi, double xma)
    {
      x = func::unfold(x, Nx, A);
      x = std::max(std::min(x, xma), xmi) - Nx * A;
      if (Nx % 2 != 0) x = -x;
      return x;
    }

    inline double PDFConstructor::deltaXD(double dFic, const InverseRaytracer::Solution& sol, double xD)
    {
      m_dFic = dFic;
      m_fastRaytracer->propagate(m_inverseRaytracer->getReconstructedPhoton(sol, dFic));
      if (not m_fastRaytracer->getPropagationStatus()) return std::numeric_limits<double>::quiet_NaN();
      return m_fastRaytracer->getXD() - xD;
    }

    inline double PDFConstructor::getCosCerenkovAngle(double E) const
    {
      double beta = m_yScanner->getBeta();
      double refind = TOPGeometryPar::Instance()->getPhaseIndex(E);
      return std::min(1 / beta / refind, 1.0);
    }

    inline const InverseRaytracer::CerenkovAngle& PDFConstructor::cerenkovAngle(double dE)
    {
      auto& cer = m_cerenkovAngles[dE];
      if (cer.cosThc == 0 and cer.sinThc == 0) {
        double meanE = m_yScanner->getMeanEnergy();
        double cosThc = getCosCerenkovAngle(meanE + dE);
        cer = InverseRaytracer::CerenkovAngle(cosThc);
      }
      return cer;
    }

    template<class T>
    void PDFConstructor::setSignalPDF(T& t, unsigned col, double xD, double zD, int Nxm, double xmMin, double xmMax)
    {
      m_ncallsSetPDF[t.type]++;

      m_inverseRaytracer->clear();
      t.inverseRaytracer = m_inverseRaytracer;

      // central solutions

      int i0 = t.solve(xD, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle());
      if (i0 < 0 or not m_inverseRaytracer->getStatus()) return;

      // solutions with xD displaced by dx

      double dx = 0.1; // cm
      int i_dx = t.solve(xD + dx, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle(), dx);
      if (i_dx < 0) return;
      int k = 0;
      while (m_inverseRaytracer->isNymDifferent()) { // get rid of discontinuities
        if (k > 4) {
          B2WARNING("PDFConstructor::setSignalPDF: failed to find the same Nym (dx)");
          return;
        }
        dx = - dx / 2;
        i_dx = t.solve(xD + dx, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle(), dx);
        if (i_dx < 0) return;
        k++;
      }

      // solutions with emission point dispaced by dL

      double dL = 0.1; // cm
      int i_dL = t.solve(xD, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(dL), cerenkovAngle(), dL);
      if (i_dL < 0) return;
      k = 0;
      while (m_inverseRaytracer->isNymDifferent()) { // get rid of discontinuities
        if (k > 4) {
          B2WARNING("PDFConstructor::setSignalPDF: failed to find the same Nym (dL)");
          return;
        }
        dL = - dL / 2;
        i_dL = t.solve(xD, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(dL), cerenkovAngle(), dL);
        if (i_dL < 0) return;
        k++;
      }

      // solutions with photon energy changed by de

      double de = 0.1; // eV
      int i_de = t.solve(xD, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle(de), de);
      if (i_de < 0) return;
      k = 0;
      while (m_inverseRaytracer->isNymDifferent()) { // get rid of discontinuities
        if (k > 4) {
          B2WARNING("PDFConstructor::setSignalPDF: failed to find the same Nym (de)");
          return;
        }
        de = - de / 2;
        i_de = t.solve(xD, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle(de), de);
        if (i_de < 0) return;
        k++;
      }

      // loop over the two solutions, compute the derivatives, do ray-tracing corrections and expand PDF in y

      for (unsigned i = 0; i < 2; i++) {
        if (not m_inverseRaytracer->getStatus(i)) continue;
        const auto& solutions = m_inverseRaytracer->getSolutions(i);
        const auto& sol = solutions[i0];
        double time = m_tof + sol.len * m_groupIndex / Const::speedOfLight;
        if (time > s_maxTime) continue;

        const auto& sol_dx = solutions[i_dx];
        const auto& sol_de = solutions[i_de];
        const auto& sol_dL = solutions[i_dL];
        YScanner::Derivatives D(sol, sol_dx, sol_de, sol_dL);
        m_dFic = 0;
        bool ok = doRaytracingCorrections(sol, D.dFic_dx, xD);
        if (not ok) continue;

        time = m_tof + m_fastRaytracer->getPropagationLen() * m_groupIndex / Const::speedOfLight;
        if (time > s_maxTime) continue;

        m_Fic = sol.getFic() + m_dFic;

        expandSignalPDF(col, D, t.type);
      }
    }

  } // namespace TOP
} // namespace Belle2
