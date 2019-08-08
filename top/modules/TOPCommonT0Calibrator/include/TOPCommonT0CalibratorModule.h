/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/utilities/TrackSelector.h>
#include <top/utilities/Chi2MinimumFinder1D.h>
#include <top/reconstruction/TOPreco.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalCommonT0.h>

#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   * A module for common T0 calibration with collision data (dimuons or bhabhas)
   */
  class TOPCommonT0CalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCommonT0CalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPCommonT0CalibratorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * Checks if running offset is subtracted in TOPDigits
     * @return true if subtracted at least in one digit
     */
    bool isRunningOffsetSubtracted();

    /**
     * Sizes
     */
    enum {c_numModules = 16, /**< number of modules */
          c_numSets = 32,  /**< number of statistically independent subsamples */
         };

    // steering parameters
    int m_numBins;      /**< number of bins to which search region is divided */
    double m_timeRange; /**< time range in which to search for the minimum [ns] */
    double m_minBkgPerBar; /**< minimal assumed background photons per module */
    double m_scaleN0; /**< scale factor for figure-of-merit N0 */
    double m_sigmaSmear;  /**< additional smearing of PDF in [ns] */
    std::string m_sample; /**< sample type */
    double m_minMomentum; /**< minimal track momentum if sample is "cosmics" */
    double m_deltaEcms; /**< c.m.s energy window if sample is "dimuon" or "bhabha" */
    double m_dr; /**< cut on POCA in r */
    double m_dz; /**< cut on POCA in z */
    double m_minZ; /**< minimal local z of extrapolated hit */
    double m_maxZ; /**< maximal local z of extrapolated hit */
    std::string m_outFileName; /**< Root output file name containing results */
    std::string m_pdfOption;   /**< PDF option name */

    // procedure
    TOP::TrackSelector m_selector; /**< track selection utility */
    TOP::Chi2MinimumFinder1D m_finders[c_numSets]; /**< finders */
    TOP::TOPreco::PDFoption m_PDFOption = TOP::TOPreco::c_Rough; /**< PDF option */
    double m_bunchTimeSep = 0; /**< time between two bunches */

    // datastore objects
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // database
    DBObjPtr<TOPCalCommonT0> m_commonT0;   /**< common T0 calibration constants */

    // output root file
    TFile* m_file = 0;                 /**< TFile */

    // control histograms
    TH1F m_hits1D;  /**< number of photon hits in a slot */
    TH2F m_hits2D;  /**< hit times vs. slot */

    // tree and its variables
    TTree* m_tree = 0;  /**< TTree containing selected track parameters etc */
    int m_moduleID = 0; /**< slot to which the track is extrapolated to */
    int m_numPhotons = 0; /**< number of photons in this slot */
    float m_x = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_y = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_z = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_p = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_theta = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_phi = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_pocaR = 0; /**< r of POCA */
    float m_pocaZ = 0; /**< z of POCA */
    float m_pocaX = 0; /**< x of POCA */
    float m_pocaY = 0; /**< y of POCA */
    float m_cmsE = 0; /**< c.m.s. energy if dimuon or bhabha */
    int m_charge = 0; /**< track charge */
    int m_PDG = 0; /**< track MC truth (simulated data only) */

  };

} // Belle2 namespace

