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

      /** Time distribution */
      TH2F* h_time;
      /** Time distribution weighted per the energy deposited */
      TH2F* h_timeWeighted;
      /** Time distribution with energy threshold applied */
      TH2F* h_timeThres;
      /** Time distribution weighted per the energy deposited with energy threshold applied */
      TH2F* h_timeWeightedThres;
      /** Energy deposited per time bin */
      TH2F* h_edep;
      /** Energy deposited above threshold per time bin */
      TH2F* h_edepThres;
      /** Energy deposited vs z */
      TH1F* h_zvedep[2];

    };

  }
}

#endif /* QCSMONITORSTUDYMODULE_H */
