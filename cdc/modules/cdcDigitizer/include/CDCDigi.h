/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDIGI_H
#define CDCDIGI_H

//basf2 framework headers
#include <framework/core/Module.h>

//cdc package headers
#include <cdc/hitcdc/SimHitCDC.h>
#include <cdc/modules/cdcDigitizer/CDCSignal.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

//ROOT
#include <TRandom3.h>



namespace Belle2 {


  /** The Class for Detailed Digitization of CDC.
   *
   *  Currently a double Gaussian with steerable parameters is used for the digitization.
   *  If there are two or more hits in one cell, only the shortest drift length is selected.
   *  The signal amplitude is the sum of all hits deposited energy in this cell.
   *
   *  @todo More details will be considered soon, like X-T function, transfer time and so on.
   *        There is a bug in the Relation between the first CDCHit and the CDCSignal.
   *        This has to be corrected, but it is not critical for the current release.
   *        Therefore I stop working on it for the moment and focus on the parts, that have to be finished this week.
   *
   *  @author  Guofu Cao;<br>
   *           Issues regarding the Relations between SimHits and Hits: Martin Heck;
   *
   */

  class CDCDigi : public Module {

  public:

    typedef std::vector<SimHitCDC*>      SimHitCDCVec;   /**< For input from Geant4 simulation.*/
    typedef std::map< int, CDCSignal *>  CDCSignalMap;   /**< Map of Cell ID -> signal.*/
    typedef std::pair< int, CDCSignal *> vpair;          /**< Pair of CDCSignalMap. */

    /** Constructor.*/
    CDCDigi();

    /** Destructor.*/
    virtual ~CDCDigi();

    /** Initialize variables, print info, and start CPU clock. */
    virtual void initialize();

    /** Nothing so far.*/
    virtual void beginRun();

    /** Actual digitization of all hits in the CDC.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event();

    /** Nothing so far. */
    virtual void endRun();

    /** Stopping of CPU clock.*/
    virtual void terminate();

  protected:


    /** Method used to smear drift length.
     *
     *  @param driftLength The value of drift length.
     *  @param fraction Fraction of the first Gaussian used to smear drift length.
     *  @param mean1 Mean value of the first Gassian used to smear drift length.
     *  @param resolution1 Resolution of the first Gassian used to smear drift length.
     *  @param mean2 Mean value of the second Gassian used to smear drift length.
     *  @param resolution2 Resolution of the second Gassian used to smear drift length.
     *
     *  @return Drift length after smearing.
     */
    double smearDriftLength(double driftLength, double fraction, double mean1, double resolution1, double mean2, double resolution2);

    /** Method to add noise to pure mc signal.
     *
     *  Generating of random noise using Gaussian distribution and add this effect
     *  to the final results.
     *
     *  @param cdcSignalMap A map is used to store CDC signals and will be returned after adding noise.
     */
    void genNoise(CDCSignalMap & cdcSignalMap);

    /** The method to get drift time based on drift length
     *
     *  In this method, X-T function will be used to calculate drift time.
     *
     *  @param driftLength The value of drift length.
     *
     *  @return Drift time.
     *
     *  @todo implementation of non-cicular surfaces of constant drift time (in reverse).
     */
    double getDriftTime(double driftLength);

    /** Method to print SimHit information.
     *
     *  @param hit Information of which hit shall be printed?
     *
     *  @todo Revisit the question if this funtion is optimally implemented. Perhaps some print funtion should be part
     *        of the SimHit and other hits.
     */

    void printSimCDCHitInfo(const SimHitCDC & hit) const;

    /** Method to print SimHit information of many hits.
     *
     *  @param info Extra information.
     *  @param hitVec Vector to store SimHits.
     */
    void printSimCDCHitsInfo(std::string info, const SimHitCDCVec & hitVec) const;

    /** Method to print module parameters.
     *
     *  @todo This method seems to print only the steerable parameters.
     *        There is now the possibility to print steerable parameters in the framework.
     */
    void printModuleParams() const;

    /** Method to print CDC signal information.
     *
     *  @param info Extra information
     *  @param cdcSignalMap A map to store CDC signals.
     *
     *  @todo Again the question if not more of this work should be done in the CDCSignal.
     */
    void printCDCSignalInfo(std::string info, const CDCSignalMap & cdcSignalMap) const;


    std::string m_inColName;                /**< Input array name. */
    std::string m_outColName;               /**< Output array name. */
    std::string m_cdcHitOutColName;         /**< Output array name. */
    std::string m_relColNameSimHitToHit;    /**< Relation collection name - cdc signal (Digit)  <-> MCParticle */
    std::string m_relColNameMCToSim;        /**< Relation collection name - MCParticle        <-> SimTrkHit */

    double m_fraction;          /**< Fraction of the first Gaussian used to smear drift length */
    double m_mean1;             /**< Mean value of the first Gassian used to smear drift length */
    double m_resolution1;       /**< Resolution of the first Gassian used to smear drift length */
    double m_mean2;             /**< Mean value of the second Gassian used to smear drift length */
    double m_resolution2;       /**< Resolution of the second Gassian used to smear drift length */

    int   m_electronicEffects;       /*!< Add noise? */
    double m_elNoise;                 /*!< Noise added to the signal */

    int   m_randomSeed;   /*!< Random number seed */

    TRandom3 * m_random;   /*!< Random number generator - Gaussian distribution */

  private:

    double m_timeCPU;                /*!< CPU time     */
    int    m_nRun;                   /*!< Run number   */
    int    m_nEvent;                 /*!< Event number */
  };

} // end of Belle2 namespace

#endif // CDCDIGI_H
