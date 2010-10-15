/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITBASE_H
#define EKLMHITBASE_H

#include <TObject.h>
#include <simulation/simkernel/B4VHit.h>

#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>



namespace Belle2 {

  // inherit HitBase class from TObject to make all hits storable and from B4VHit for HitCollections
  class EKLMHitBase: public TObject  {

  public:

    //! Constructor
    EKLMHitBase() {};

    //! Destructor
    virtual ~EKLMHitBase() {};

    virtual void Print();
    virtual void Draw();



  private:
    ClassDef(EKLMHitBase, 1);   // needed to be storable


  };


} // end of namespace Belle2

#endif //EKLMHITBASE_H
