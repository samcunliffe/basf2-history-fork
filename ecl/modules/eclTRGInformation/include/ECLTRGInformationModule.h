/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /** Forward declarations */
  class ECLCalDigit;
  class ECLCluster;
  class ECLTRGInformation;
  class ECLTC;
  class TRGECLUnpackerStore;
  class TrgEclMapping;

  /**
   * Module to get ECL TRG energy information
   */
  class ECLTRGInformationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLTRGInformationModule();

    /** initialize */
    virtual void initialize();

    /** event */
    virtual void event();

    /** terminate */
    virtual void terminate();

  private:

    /** threshold for eclcluster energy sum */
    double m_clusterEnergyThreshold;

    /** map TCId, energy*/
    typedef std::map <unsigned, float> tcmap;


    /** vector (8736+1 entries) with cell id to store array positions */
    std::vector< int > m_calDigitStoreArrPosition;

    /** vector (576+1 entries) with TC id to store array positions */
    std::vector< int > m_TCStoreArrPosition;

    /** TC mapping class */
    TrgEclMapping* m_trgmap;

    StoreArray<ECLCalDigit> m_eclCalDigits; /**< Required input array of ECLCalDigits  */
    StoreArray<ECLCluster> m_eclClusters; /**< Required input array of ECLClusters  */
    StoreArray<TRGECLUnpackerStore> m_trgUnpackerStore; /**< Required input array of TRGECLUnpackerStore  */

    StoreArray<ECLTC> m_eclTCs; /**< Output array of ECLTCs  */
    StoreObjPtr<ECLTRGInformation> m_eclTRGInformation; /**< Analysis level information per event holding TRG information*/
  };
}
