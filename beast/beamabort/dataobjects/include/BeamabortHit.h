/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMABORTHIT_H
#define BEAMABORTHIT_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassBeamabortHit - digitization simulated hit for the Beamabort detector.
  *
  * This class holds particle hit data from digitization simulation. As the simulated
  * hit classes are used to generate detector response, they contain _local_
  * information.
  */
  class BeamabortHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    BeamabortHit(): m_edep(0), m_I(0), m_time(0), m_detNb(0), m_pdg(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    BeamabortHit(float edep, float I, float time, int detNb, int pdg):
      m_edep(edep), m_I(I), m_time(time), m_detNb(detNb), m_pdg(pdg)
    {
    }

    /** Return the energy deposited */
    float getedep() const { return m_edep; }
    /** Return the amplitude */
    float getI() const { return m_I; }
    /** Return the time */
    float gettime() const { return m_time; }
    /** Return the PIN diode number */
    int getdetNb()  const { return m_detNb; }
    /** Return the PDG of particles */
    int getPDG()  const { return m_pdg; }

  private:

    /** Energy deposited */
    float m_edep;
    /** V */
    float m_I;
    /** time */
    float m_time;
    /** Detector Number */
    int m_detNb;
    /** Particle PDG */
    int m_pdg;

    ClassDef(BeamabortHit, 1)
  };

} // end namespace Belle2

#endif
