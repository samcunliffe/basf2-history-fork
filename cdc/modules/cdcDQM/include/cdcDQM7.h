/* Nanae Taniguchi 2017.07.12 */
/* Nanae Taniguchi 2018.02.06 */

#ifndef CDCDQM7MODULE_H
#define CDCDQM7MODULE_H

#include <framework/core/HistoModule.h>

#include <vector>
#include "TH1F.h"
#include "TH1D.h"
#include "TH2D.h"

namespace Belle2 {

  /**
   * The module for Data Quality Monitor.
   */
  class cdcDQM7Module : public HistoModule {

  public:

    //! Constructor
    cdcDQM7Module();

    //! Destructor
    virtual ~cdcDQM7Module();

    //! Module functions
    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    //! function to define histograms
    void defineHisto() override;

  private:
    TH1D* h_nhits_L[56];/**<histogram hit in each layer*/
    TH1D* h_tdc_L[56];/**<histogram tdc in each layer*/
    TH1D* h_adc_L[56];/**<histogram adc in each layer*/

    TH1D* h_tdc_sL[9];/**<tdc each super layer*/
    TH1D* h_adc_sL[9];/**<adc each super layer*/

    TH1D* h_fast_tdc;/**<fastest TDC in each event*/

    TH2D* bmap_2;/**<board status map 2D*/

  };
}
#endif
