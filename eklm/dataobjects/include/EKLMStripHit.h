/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSTRIPHIT_H
#define EKLMSTRIPHIT_H

#include <framework/datastore/DataStore.h>

#include  <eklm/dataobjects/EKLMHitBase.h>
#include  <eklm/dataobjects/EKLMSimHit.h>
#include  "globals.hh"
#include  "CLHEP/Vector/ThreeVector.h"

#include "G4VPhysicalVolume.hh"

#include <string>
namespace Belle2 {

  //! Main reconstruction hit class. Containes infromation about the
  //! hitted strips
  class EKLMStripHit : public EKLMHitBase {

  public:

    //! Constructor
    EKLMStripHit() {};

    //! Constructor from the EKLMHitBase
    EKLMStripHit(const EKLMSimHit * Hit);

    //! Destructor
    ~EKLMStripHit() {};

    //! Print stip name and some other useful info
    void Print();



    //! returns number of photo electrons
    double getNumberPhotoElectrons() const;

    //! sets the number of photo electorns
    void setNumberPhotoElectrons(double);



    //! returns physical volume
    const G4VPhysicalVolume *getVolume() const;

    //! set physical volume
    void setVolume(const G4VPhysicalVolume *pv);




    //! returns true if strips intersects (does not pay attention to the layer)
    bool doesIntersect(Belle2::EKLMStripHit *, CLHEP::Hep3Vector &);

    //! returns distance btw. the hit and SiPM
    double getLightPropagationLength(CLHEP::Hep3Vector &);

    /**
     * Get plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     */
    void setStrip(int Strip);



  private:


    /**
     * Number of plane.
     */
    int m_Plane;

    /**
     * Number of strip.
     */
    int m_Strip;



    //! number of photo electrons
    double m_NumberPhotoElectrons;


    //! distance btw. the hit and SiPM
    double m_LightPropagationLength;

    //! Physical volume (for simulation)
    const G4VPhysicalVolume *m_pv;   //! {ROOT streamer directive}


    //! Makes objects storable
    ClassDef(Belle2::EKLMStripHit, 1);

  };

} // end of namespace Belle2

#endif //EKLMSTRIPHIT_H
