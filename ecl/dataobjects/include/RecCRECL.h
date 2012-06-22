/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECCRECL_H
#define RECCRECL_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class RecCRECL : public TObject {
  public:

    /** The members of RecCRECL. */
    int m_showerId;/**shower id of this shower. */
    double m_Energy;/**Energy of this shower. */
    double m_Theta;/**Theta of this shower. */
    double m_Phi;/**Phi of this shower. */
    double m_Distance;/**Distance of this shower. */
    double m_Mass;/**Mass of this shower. */
    double m_Width;/**Width of this shower. */
    double m_E9oE25;/**E9oE25 of this shower. */
    double m_TotEnergy;/**TotEnergy of this shower. */
    double m_E9oE25unf;/**E9oE25unf of this shower. */
    double m_UncEnergy;/**EUncEnergy of this shower. */
    double m_NHits;/**NHits of this shower. */
    double m_WNHits;/**WNHits of this shower. */
    double m_NHitsUsed;/**NHitsUsed of this shower. */
    int m_Status;/**Status of this shower. */
    int m_Grade;/**Grade of this shower. */

    //! The method to set showerId
    void setShowerId(int showerId) { m_showerId = showerId; }
    //! The method to set Energy
    void setEnergy(double Energy) { m_Energy = Energy; }
    //! The method to set Theta
    void setTheta(double Theta) { m_Theta = Theta; }
    //! The method to set Phi
    void setPhi(double Phi) { m_Phi = Phi; }
    //! The method to set Distance
    void setDistance(double Distance) { m_Distance = Distance; }
    //! The method to set Mass
    void setMass(double Mass) { m_Mass = Mass; }
    //! The method to set Width
    void setWidth(double Width) { m_Width = Width; }
    //! The method to set E9oE25
    void setE9oE25(double E9oE25) { m_E9oE25 = E9oE25; }
    //! The method to set TotEnergy
    void setTotEnergy(double TotEnergy) { m_TotEnergy = TotEnergy; }
    //! The method to set E9oE25unf
    void setE9oE25unf(double E9oE25unf) { m_E9oE25unf = E9oE25unf; }
    //! The method to set m_UncEnergy
    void setUncEnergy(double UncEnergy) { m_UncEnergy = UncEnergy; }
    //! The method to set NHits
    void setNHits(double NHits) { m_NHits = NHits; }
    //! The method to set WNHits
    void setWNHits(double WNHits) { m_WNHits = WNHits; }
    //! The method to set NHitsUsed
    void setNHitsUsed(double NHitsUsed) { m_NHitsUsed = NHitsUsed; }
    //! The method to set Status
    void setStatus(int Status) { m_Status = Status; }
    //! The method to set Grade
    void setGrade(int Grade) { m_Grade = Grade ; }


    //! The method to get showerId
    int GetShowerId() const { return m_showerId ; }
    //! The method to get Energy
    double GetEnergy() const { return m_Energy ; }
    //! The method to get Theta
    double GetTheta() const { return m_Theta ; }
    //! The method to get Phi
    double GetPhi() const { return m_Phi ; }
    //! The method to get Distance
    double GetDistance() const { return m_Distance ; }
    //! The method to get Mass
    double GetMass() const { return m_Mass ; }
    //! The method to get Width
    double GetWidth() const { return m_Width ; }
    //! The method to get E9oE25
    double GetE9oE25() const { return m_E9oE25 ; }
    //! The method to get TotEnergy
    double GetTotEnergy() const { return m_TotEnergy ; }
    //! The method to get E9oE25unf
    double GetE9oE25unf() const { return m_E9oE25unf ; }
    //! The method to get return m_UncEnergy
    double GetUncEnergy() const { return m_UncEnergy ; }
    //! The method to get NHits
    double GetNHits() const { return m_NHits ; }
    //! The method to get WNHits
    double GetWNHits() const { return m_WNHits ; }
    //! The method to get NHitsUsed
    double GetNHitsUsed() const { return m_NHitsUsed ; }
    //! The method to get Status
    int GetStatus() const { return m_Status ; }
    //! The method to get Grade
    int GetGrade() const { return m_Grade  ; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    RecCRECL() {;}

    ClassDef(RecCRECL, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
