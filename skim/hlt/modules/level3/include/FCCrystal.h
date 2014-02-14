/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FCCrystal_H
#define FCCrystal_H

namespace Belle2 {

  //...Defs...
#define FCCrystalEHit 0
#define FCCrystalCheckedOrNotHit 1
#define FCCrystalNeighbor 79
#define FCCrystalNeighbor1 2
#define FCCrystalNeighbor2 4
#define FCCrystalNeighbor3 8
#define FCCrystalNeighbor6 64
#define FCCrystalAppendedNeighbor1 5
#define FCCrystalAppendedNeighbor2 3
#define FCCrystalAppendedNeighbor3 65
#define FCCrystalAppendedNeighbor6 9

  // CsI crystal class for the Level-3 Fast Cluster Finder
  class FCCrystal {
  public:
    //! constructor
    FCCrystal(const int thetaId, const int phiId, FCCrystal* const, FCCrystal* const);

    //! constructor
    FCCrystal();

    //! destructor
    ~FCCrystal() {}

  public: //Selectors
    //! returns energy
    double energy(void) const;

    //! returns theta id
    int thetaId(void) const;

    //! returns phi id
    int phiId(void) const;

    //! returns neighbor of + phiId side
    FCCrystal* neighborPlusPhi(void) const;

    //! returns neighbor of - phiId side
    FCCrystal* neighborMinusPhi(void) const;

    //! returns neighbor of + thetaId side
    FCCrystal* neighborPlusTheta(void) const;

    //! returns neighbor of - thetaId side
    FCCrystal* neighborMinusTheta(void) const;

    //! returns cid_status
    //double status(void) const;

    //! returns state
    unsigned state(void) const;

    //! returns state bit
    unsigned stateAND(const unsigned mask) const;

  public: // Modifires

    //! clear information
    void clear(void);

    //! sets counter energy
    double energy(double src);

    //! set status
    //double status(double status);

    //! set state
    unsigned state(const unsigned state);

    //! set state bit
    void stateOR(const unsigned mask);

    //! check state bit and append to the list of clusters
    void checkAndAppend(FTList<FCCrystal*>& hits, const unsigned& mask);

  private: //private member functions
    //! returns max phi id of this phi-ring
    int phiMax(void) const;

  private: //static data members
    static const int m_phiMaxFE[13]; // maximum phi ID for FE
    static const int m_phiMaxBE[10]; // maximum phi ID for BE
  private: //private data members
    const int m_thetaId; // theta ID
    const int m_phiId; // phi ID
    FCCrystal* const m_neighborPlus; // pointer of neighbor channel
    FCCrystal* const m_neighborMinus; // pointer of neighbor channel
    double m_energy; // energy deposit
    //double m_status;
    unsigned int m_state; // status bits
  };

  //----------------------------------------------
#ifdef FCCrystal_NO_INLINE
#define inline
#else
#undef inline
#define FCCrystal_INLINE_DEFINE_HERE
#endif

#ifdef FCCrystal_INLINE_DEFINE_HERE

  inline
  FCCrystal::FCCrystal(const int thetaId, const int phiId,
                       FCCrystal* const plus, FCCrystal* const minus)
    : m_thetaId(thetaId),
      m_phiId(phiId),
      m_neighborPlus(plus),
      m_neighborMinus(minus),
      m_energy(0.),
      //m_status(1.),
      m_state(FCCrystalCheckedOrNotHit)
  {
  }

  inline
  FCCrystal::FCCrystal(void)
    : m_thetaId(-999),
      m_phiId(-999),
      m_neighborPlus(NULL),
      m_neighborMinus(NULL),
      m_energy(0.),
      //m_status(1.),
      m_state(FCCrystalCheckedOrNotHit)
  {
  }

  inline
  void
  FCCrystal::clear(void)
  {
    m_energy = 0.;
    m_state = 1;
  }

  inline
  double
  FCCrystal::energy(void) const
  {
    return m_energy;
  }

  inline
  double
  FCCrystal::energy(double src)
  {
    return m_energy = src;
  }

  inline
  int
  FCCrystal::thetaId(void) const
  {
    return m_thetaId;
  }

  inline
  int
  FCCrystal::phiId(void) const
  {
    return m_phiId;
  }

  inline
  int
  FCCrystal::phiMax(void) const
  {
    return (m_thetaId < 13) ? *(m_phiMaxFE + m_thetaId) :
           ((m_thetaId < 59) ? 143 : * (m_phiMaxBE + (m_thetaId - 59)));
  }

  inline
  FCCrystal*
  FCCrystal::neighborPlusPhi(void) const
  {
    const int max(phiMax());
    return (m_phiId == max)
           ? const_cast<FCCrystal*>(this) - max
           : const_cast<FCCrystal*>(this) + 1;
  }

  inline
  FCCrystal*
  FCCrystal::neighborMinusPhi(void) const
  {
    return (m_phiId)
           ? const_cast<FCCrystal*>(this) - 1
           : const_cast<FCCrystal*>(this) + phiMax();
  }

  inline
  FCCrystal*
  FCCrystal::neighborPlusTheta(void) const
  {
    return m_neighborPlus;
  }

  inline
  FCCrystal*
  FCCrystal::neighborMinusTheta(void) const
  {
    return m_neighborMinus;
  }
  /*
  inline
  double
  FCCrystal::status(void) const
  {
    return m_status;
  }

  inline
  double
  FCCrystal::status(double status)
  {
    return m_status = status;
  }
  */
  inline
  unsigned
  FCCrystal::state(void) const
  {
    return m_state;
  }

  inline
  unsigned
  FCCrystal::state(const unsigned state)
  {
    return m_state = state;
  }

  inline
  unsigned
  FCCrystal::stateAND(const unsigned mask) const
  {
    return m_state & mask;
  }

  inline
  void
  FCCrystal::stateOR(const unsigned mask)
  {
    m_state |= mask;
  }

  inline
  void
  FCCrystal::checkAndAppend(FTList<FCCrystal*>& hits, const unsigned& mask)
  {
    if (m_state ^ FCCrystalCheckedOrNotHit) {
      if (!m_state) hits.append(this);
      m_state |= mask;
    }
  }

#endif

#undef inline

}

#endif /* FCCrystal_FLAG_ */

