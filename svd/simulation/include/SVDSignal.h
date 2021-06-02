/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSIGNAL_H
#define SVDSIGNAL_H

#include <framework/dataobjects/RelationElement.h>
#include <string>
#include <sstream>
#include <deque>
#include <map>
#include <svd/simulation/SVDSimulationTools.h>

namespace Belle2 {
  namespace SVD {

    /**
     * The SVD signal class.
     *
     * The class implements the waveform appearing on an SVD strip as a result of
     * ionization by particles passing through the detector.
     * The class accumulates elementary waveforms formed by chargelets contributing to the
     * charge accumulated on the strip.
     * The elementary waveforms are currently of the form
     * w(t|delta,tau) = charge * (t-delta)/tau * exp(1.0 - (t-delta)/tau)) for t >= delta,
     *                  otherwise 0.
     * with delta being the initial time, and tau the decay time.
     * Maximum is attained at delta + tau and is equal to charge.
     * The class is a functor returning values of summary waveform at a given time.
     * Poisson and gaussian noises will be added externally.
     */

    class SVDSignal {

    public:

      /** Type to store elementary waveforms.
       *  Waveforms are parameterized (in this order) by location parameter (initTime),
       *  norm (charge) and scale factor (tau) - decay time.
       *  It is assumed that each chargelet is generated by a single MCParticle and
       *  it contributes to a single TrueHit.
       */
      struct Wave {
        /** Constructor.*/
        Wave(double initTime, double charge, double tau,
             RelationElement::index_type particle, RelationElement::index_type truehit, waveFunction wfun):
          m_initTime(initTime), m_charge(charge), m_tau(tau), m_particle(particle),
          m_truehit(truehit), m_wfun(wfun)
        {}
        /** Create a string containing data of this Wave object */
        std::string toString() const
        {
          std::ostringstream os;
          os << m_initTime << '\t' << m_charge << '\t' << m_tau << std::endl;
          return os.str();
        }
        /** Start time of the waveform. */
        double m_initTime;
        /** Charge of the wave. */
        double m_charge;
        /** Decay time of the waveform.*/
        double m_tau;
        /** DataStore index of the generating MC Particle. */
        RelationElement::index_type m_particle;
        /** DataStore index of the associated SVDTrueHit. */
        RelationElement::index_type m_truehit;
        /** Waveform function. */
        waveFunction m_wfun;
      };

      /** List of elementary waveforms. */
      typedef std::deque< Wave > function_list;

      /** Type to store contributions to strip signal by different particles on output of SVDSignal */
      typedef std::map<RelationElement::index_type, RelationElement::weight_type> relations_map;
      /** Type of relation_map elements. */
      typedef std::map<RelationElement::index_type, RelationElement::weight_type>::value_type relation_value_type;

      /** Default constructor */
      SVDSignal(): m_charge(0) {}

      /** Copy ctor. */
      SVDSignal(const SVDSignal& other)
      {
        m_charge = other.getCharge();
        for (Wave wave : other.getFunctions())
          m_functions.push_back(wave);
      }

      /** Check whether this is a noise signal.
       * If charge is negative, there is no useful information in the signal.
       * @return True if noise signal.
       */
      bool isNoise() const { return m_charge < 0; }

      /** Add a chargelet to the strip signal.
       * Negative sign of charge has a special meaning, it designates noise signal
       * carrying otherwise no useful information.
       * @param charge Charge in electrons to be added
       * @param initTime Time of arrival of the chargelet to the sensitive surface of the sensor.
       * @param tau Characteristic time of waveform decay.
       * @param particle Index of the particle contributing the charge, -1 for no particle/noise
       * @param truehit Index of the truehit corresponding to the particle that contributed
       * the charge.
       */
      void add(double initTime, double charge, double tau, int particle = -1, int truehit = -1, waveFunction wfun = w_betaprime)
      {
        if (charge > 0) {
          m_charge += charge;
          m_functions.push_back(Wave(initTime, charge, tau, particle, truehit, wfun));
          if (particle > -1) m_particles[particle] += static_cast<float>(charge);
          if (truehit > -1) m_truehits[truehit] += static_cast<float>(charge);
        } else if (m_charge == 0)
          m_charge += charge;
      }

      /** Make the SVDSignal assignable.
       * @param other The original signal.
       * @return This signal with all quantities set to those of the other.
       */
      SVDSignal& operator=(const SVDSignal& other)
      {
        m_charge = other.getCharge();
        m_functions.clear();
        std::copy(other.getFunctions().begin(), other.getFunctions().end(), m_functions.begin());
        return *this;
      }

      /** Waveform shape.
       * @param t The time at which the function is to be calculated.
       * @param initTime The initial time of the waveform.
       * @param charge The charge (peak value) of the waveform.
       * @param tau The scale parameter (decay time) of the waveform.
       * @param wfun The functional form of the waveform. Default is betaprime.
       * The function is normalized to peak value of 1.
       * @return Value of the waveform at t.
       */
      double waveform(double t, double initTime, double charge, double tau,
                      waveFunction wfun) const
      {
        double z = (t - initTime) / tau;
        return charge * wfun(z);
      }

      /** Waveform taking parameters from a Wave struct.
       * @param t The time at which the function is to be calculated.
       * @param wave The SVDSignal::Wave struct with parameters of a waveform.
       * @return THe value of the waveform at time time.
       */
      double waveform(double t, const Wave& wave) const
      { return waveform(t, wave.m_initTime, wave.m_charge, wave.m_tau, wave.m_wfun); }

      /** Make SVDSignal a functor.
       * @param t The time at which output is to be calculated.
       * @return value of the waveform
       */
      double operator()(double t) const
      {
        double wave_sum = 0;
        for (SVDSignal::Wave wave : m_functions) {
          wave_sum += waveform(t, wave);
        }
        return wave_sum;
      }

      /** Return the charge collected in the strip.
       * @return Total charge of the waveform.
       */
      double getCharge() const { return m_charge; }

      /** Return the list of waveform parameters.
       * @return The deque containing SVDSignal::Wave structs with waveform parameters.
       */
      const function_list& getFunctions() const { return m_functions; }

      /** Return the list of MCParticle relations.
       * @return SVDSignal::relations_map with MCParticle relations of the signal.
       */
      const relations_map& getMCParticleRelations() const { return m_particles; }

      /** Return the list of TrueHit relations.
       * @return SVDSignal::relations_map with TrueHit relations of the signal.
       */
      const relations_map& getTrueHitRelations() const { return m_truehits; }
      /** Produce a string representation of the object */
      std::string toString() const
      {
        std::ostringstream os;
        size_t i = 0;
        for (auto w : m_functions)
          os << ++i << '\t' << w.toString();
        return os.str();
      }

    protected:

      /** charge of the pixel */
      double m_charge;
      /** list of elementary waveform parameters.*/
      function_list m_functions;
      /** Map of MCParticle associations.*/
      relations_map m_particles;
      /** Map of TrueHit associations. */
      relations_map m_truehits;

    }; // class SVDSignal

  } // end namespace SVD
} // end namespace Belle2

#endif
