/* Nanae Taniguchi 2015.05.19 */

#ifndef CDCDQM7MODULE_H
#define CDCDQM7MODULW_H

#include <framework/core/HistoModule.h>        // <- Substitution of HistoModule.h
#include <vector>
#include "TH1F.h"

namespace Belle2 {

  class cdcDQM7Module : public HistoModule {  // <- derived from HistoModule class

  public:
    //! Constructor
    cdcDQM7Module();
    //! Destructor
    virtual ~cdcDQM7Module();

    //!Module functions
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! function to define histograms
    virtual void defineHisto();

  private:
    TH1F* h_cdc;         // histogram
    TH1F* h_tdc;
    TH1F* h_adc;
    TH1F* h_layer;
    TH1F* h_nhits;

  };
}
#endif
