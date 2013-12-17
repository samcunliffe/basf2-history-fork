/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HISTMAKER_MODULE_H
#define HISTMAKER_MODULE_H

#include <framework/core/Module.h>

#include <map>
#include <string>

class TH1;
class TFile;

namespace Belle2 {

  class Particle;

  /**
   * This module fills the ParticleInfo with the value calculated by a TMVA method for every Particle in the given ParticleLists
   * The module depends on the ParticleInfoModule
   */

  class HistMakerModule : public Module {
  public:

    /**
     * Constructor
     */
    HistMakerModule();

    /**
     * Destructor
     */
    virtual ~HistMakerModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     */
    virtual void beginRun();

    /**
     * Called for each event.
     */
    virtual void event();

    /**
     * Called when run ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    typedef std::tuple<std::string, int, double, double> Range;
    typedef std::tuple<TH1*, TH1*> SBHists;

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::string m_identifierName; /**< identifier name for the TFile where the hists are stored */
    std::string m_truth_variable; /**< the name of the truth variable which determines if given Particle is signal or background */
    std::vector< Range> m_hist_variables; /**< the name and range of the variables, for which the signal and background distrbution is determined */
    bool m_make2dHists; /**< If true, 2D Histograms are generated for all pairs of input variables */

    std::map< std::string, std::vector< SBHists > > m_hists; /**< Map of listName to vector of Signal/BackgroundHistogramms for the input variables */
    std::map< std::string, std::vector< SBHists > > m_hists2d; /**<  Map of listName to vector of Signal/BackgroundHistogramms  for all pairs of input variables */
    std::vector< const VariableManager::Var* > m_variables; /**< Pointers to the input variables */
    std::vector< float > m_values; /**< Vector for saving the current values of the input variables */
    const VariableManager::Var* m_truth; /**< Pointer to the truth variables: >0.5 is signal, <0.5 is background */
    TFile* m_outputFile; /**< The output file stores all generated histograms */

  };

} // Belle2 namespace

#endif


