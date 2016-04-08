/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BGODIGITIZERMODULE_H
#define BGODIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TMath.h>
#include <TF1.h>
#include <TVector3.h>


namespace Belle2 {
  namespace bgo {
    /**
     * Bgo tube digitizer
     *
     * Creates BgoHits from BgoSimHits
     *
     */
    class BgoDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      BgoDigitizerModule();

      /**  */
      virtual ~BgoDigitizerModule();

      /**  */
      virtual void initialize();

      /**  */
      virtual void beginRun();

      /**  */
      virtual void event();

      /**  */
      virtual void endRun();
      /**  */
      virtual void terminate();


    private:

      /** reads data from BGO.xml: threshold in MeV, range in MeV, and resolution in % */
      virtual void getXMLData();
      /*
      Double_t GetEnergyResolutionGeV(Double_t pEnergy) const
      {
      // Returns energy resolution in GeV when supplied Energy in GeV
      return (m_EnergyResolutionFactor * TMath::Sqrt(pEnergy) + m_EnergyResolutionConst * pEnergy);

      }
      */
      Double_t GetEnergyResolutionGeV(Double_t);

      /** number of detectors. Read from BGO.xml*/
      int nBGO = 8;
      /** Cremat gain */
      double m_Threshold[8];
      /** Work function */
      double m_Range[8];
      /** Fano Factor */
      double m_Resolution[8];
      /** Energy resolution factor */
      double m_EnergyResolutionFactor;
      /** Energy resolution constant */
      double m_EnergyResolutionConst;

    };

  }
}

#endif /* BGODIGITIZERMODULE_H */
