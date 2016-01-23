/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef QCSMONITORSTUDYMODULE_H
#define QCSMONITORSTUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <string>
#include <vector>


#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


namespace Belle2 {
  namespace qcsmonitor {

    /**
     * Study module for Qcsmonitors (BEAST)
     *
     * Produces histograms from BEAST data for the Qcsmonitors.   *
     */
    class QcsmonitorStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      QcsmonitorStudyModule();

      /**  */
      virtual ~QcsmonitorStudyModule();

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

      /** Defines the histograms*/
      virtual void defineHisto();


    private:

      /** reads data from QCSMONITOR.xml */
      virtual void getXMLData();
      /** Energy threshold */
      double m_Ethres;

      /** Energy deposited vs TOF */
      TH2F* h_qcsms_Evtof1[20];
      /** Energy deposited vs TOF */
      TH2F* h_qcsms_Evtof2[20];
      /** Energy deposited vs TOF */
      TH2F* h_qcsms_Evtof3[20];
      /** Energy deposited */
      TH1F* h_qcsms_edep[20];

    };

  }
}

#endif /* QCSMONITORSTUDYMODULE_H */
