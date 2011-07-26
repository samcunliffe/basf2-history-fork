/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZER_H
#define EKLMDIGITIZER_H

#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/eklmhit/EKLMSimHit.h>
#include <eklm/eklmhit/EKLMStripHit.h>
#include <map>
#include <vector>
#include  "CLHEP/Vector/ThreeVector.h"



namespace Belle2 {


  //! Digitize EKLMSimHits  to get EKLM StripHits
  class EKLMDigitizer {

  public:


    //! Constructor
    EKLMDigitizer() {};


    //! Destructor
    ~EKLMDigitizer() {};

    //! Read hits from the store
    void readSimHits();

    //! Sort Sim Hits and fill m_HitStripMap
    void sortSimHits();

    //! merges hits from the same strip. Creates EKLMStripHits
    void mergeSimHitsToStripHits();

    //! Saves EKLMStripHits to the DataStore
    void saveStripHits();


    //! Operator new
    inline void *operator new(size_t);

    //! Operator delete
    inline void operator delete(void *aEKLMDigitizer);


  private:

    //! std::map for hits sorting according strip name
    std::map<std::string, std::vector<EKLMSimHit*> > m_HitStripMap;

    //! vector of EKLMStripHits
    std::vector<EKLMStripHit*> m_HitVector;

    //! sim hits vector
    std::vector<EKLMSimHit*> m_simHitsVector;


  };

  //! Extern allocator
  extern G4Allocator<EKLMDigitizer>   EKLMDigitizerAllocator;

  //! Operator new
  inline void* EKLMDigitizer::operator new(size_t)
  {
    void *aEKLMDigitizer;
    aEKLMDigitizer = (void *) EKLMDigitizerAllocator.MallocSingle();
    return aEKLMDigitizer;
  }

  //! Operator delete
  inline void EKLMDigitizer::operator delete(void *aEKLMDigitizer)
  {
    EKLMDigitizerAllocator.FreeSingle((EKLMDigitizer*) aEKLMDigitizer);
  }

} // end of namespace Belle2

#endif
