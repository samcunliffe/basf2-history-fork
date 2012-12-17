/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSECTORHIT_H
#define EKLMSECTORHIT_H

/* C++ headers. */
#include <vector>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitBase.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>

namespace Belle2 {

  /**
   * Class for handling hits in the whole sector. Needed to form 2d hits
   * in a comfortable way.
   */
  class EKLMSectorHit : public EKLMHitBase  {

  public:

    /**
     * Constructor.
     */
    EKLMSectorHit() {};

    /**
     * Constructor.
     */
    EKLMSectorHit(int nEndcap, int nLayer, int nSector);

    /**
     * Destructor.
     */
    ~EKLMSectorHit() {};

    /**
     * Print information about the sector hit.
     */
    void Print();

    /**
     * Get pointer to the vector of pointers to the EKLMDigits included to
     * the EKLMSectorHit.
     * @return Pointer.
     */
    std::vector <EKLMDigit*> * getStripHitVector() ;

    /**
     * Get pointer to the vector of pointers to 2d hits included to the
     * EKLMSectorHit.
     * @return Pointer.
     */
    std::vector <EKLMHit2d*>* get2dHitVector() ;

  private:

    /**
     * Vector of pointers to the Belle2::EKLMDigits included to the
     * EKLMSectorHit.
     */
    std::vector<EKLMDigit*> m_stripHitVector;

    /** Vector of pointers to 2d hits included to the EKLMSectorHit. */
    std::vector<EKLMHit2d*> m_hit2dVector;

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMSectorHit, 1);

  };

} // end of namespace Belle2

#endif //EKLMSECTORHIT_H
