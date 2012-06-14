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

#include  <eklm/dataobjects/EKLMHitBase.h>
#include  <eklm/dataobjects/EKLMSimHit.h>
#include  "globals.hh"
#include  "CLHEP/Vector/ThreeVector.h"
#include  "TFitResultPtr.h"
#include  "TFitResult.h"

#include "G4VPhysicalVolume.hh"

#include <string>
namespace Belle2 {

  //! Main reconstruction hit class. Containes infromation about the
  //! hitted strips
  class EKLMStripHit : public EKLMHitBase {

  public:

    //! Constructor
    EKLMStripHit() {};

    //! Constructor from the EKLMSimHit
    EKLMStripHit(const EKLMSimHit* Hit);

    //! Destructor
    ~EKLMStripHit() {};

    //! Print stip name and some other useful info
    void Print() const;



    //! returns number of photo electrons
    double getNumberPhotoElectrons() const;

    //! sets the number of photo electorns
    void setNumberPhotoElectrons(double);



    //! returns physical volume
    const G4VPhysicalVolume* getVolume() const;

    //! set physical volume
    void setVolume(const G4VPhysicalVolume* pv);

    /**
     * whether hit could be used late (if it passed discriminator threshold)
     * (getter)
     */
    bool isGood() const;

    /**
     * whether hit could be used late (if it passed discriminator threshold)
     * (setter)
     */
    void isGood(bool) ;


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

    /**
     * Get fit results
     */
    const TFitResult* getFitResults() const;

    /**
     * Get fit status
     */
    int getFitStatus();

    /**
     * Set fit status
     */
    void setFitStatus(int s);

    /**
     * Set fit results
     */
    void setFitResults(TFitResult& res);


    /**
     * Set fit results
     */
    void setFitResults(TFitResultPtr resPtr);




  private:


    /**
     * Number of plane.
     */
    int m_Plane;

    /**
     * Number of strip.
     */
    int m_Strip;

    /**
     *  if hit passes threshold
     */
    bool m_good;



    //! number of photo electrons
    double m_NumberPhotoElectrons;

    //! number of photo electrons
    int m_fitStatus;




    //! Physical volume (for simulation)
    const G4VPhysicalVolume* m_pv;   //! {ROOT streamer directive}

    //! Fit results object
    TFitResult m_fitResults;


    //! Makes objects storable
    ClassDef(Belle2::EKLMStripHit, 1);

  };

} // end of namespace Belle2

#endif //EKLMSTRIPHIT_H
