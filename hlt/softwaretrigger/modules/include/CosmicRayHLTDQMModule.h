#ifndef COSMICRAYHLTDQMMODULE_H
#define COSMICRAYHLTDQMMODULE_H
//+
// File : CosmicRayHLTDQMModule.h
// Description : Module to monitor the reconstruction of cosmic ray on hlt
//
// Author : Chunhua LI
// Date :  19 - May - 2017
//-

#include <string>
#include <vector>
#include <map>

#include <TH1F.h>

#include <framework/core/HistoModule.h>

namespace Belle2 {
  namespace SoftwareTrigger {

    /*! A class definition of an input module for Sequential ROOT I/O */

    class CosmicRayHLTDQMModule : public HistoModule {

      // Public functions
    public:

      //! Constructor / Destructor
      CosmicRayHLTDQMModule();
      virtual ~CosmicRayHLTDQMModule();

      //! Module functions to be called from main process
      virtual void initialize();

      //! Module functions to be called from event process
      virtual void event();

      //! Histogram definition
      virtual void defineHisto();


    private:
      /// Directory to put the generated histograms
      std::string m_param_histogramDirectoryName;

      //CDC
      /**Signed distance to the POCA in the r-phi plane*/
      TH1F* h_d0;
      /**z coordinate of the POCA*/
      TH1F* h_z0;
      /**number of traks*/
      TH1F* h_ntrk;
      /**momentum information of track*/
      TH1F* h_p[5];
      /**chi2 probalility of the track fit*/
      TH1F* h_pValue;
      /**Number of CDC hits associated to CDC track*/
      TH1F* h_ncdchits;
      /**Angle of the transverse momentum in the r-phi plane*/
      TH1F* h_phi0;
      /**charge of track*/
      TH1F* h_charge;

      //ECL
      /**number of ecl clusters*/
      TH1F* h_ncluster;
      /**phi angle of ECLCluster position*/
      TH1F* h_phi_eclcluster;
      /**theta angle of ECLCluster position*/
      TH1F* h_theta_eclcluster;
      /**the ecl cluster time*/
      TH1F* h_Time_eclcluster;
      /**the E1/E9 energy ratio*/
      TH1F* h_E1oE9_eclcluster;
      /**energy of ECL cluster*/
      TH1F* h_e_eclcluster;

      //KLM
      /**number of KLM clusters*/
      TH1F* h_nklmcluster;
      /**KLMCluster's x, y, z positions*/
      TH1F* h_pos_klmcluster[3];
      /**KLMCluster's number of the innermost layer with hits*/
      TH1F* h_innermost_klmcluster;
      /**KLMCluster's number of layers with hits*/
      TH1F* h_nlayer_klmcluster;
      /**KLMCluster's Time*/
      TH1F* h_Time_klmcluster;


    };

  } // end namespace SoftwareTrigger
} // end namespace Belle2

#endif // SOFTWARETRIGGERCOSMICRAYDQMMODULE_H
