/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Kuhr                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>

#include "TFile.h"
#include "TTree.h"

#include "masterclass/dataobjects/BEvent.h"


namespace Belle2 {

  /**
  * Module to write out data in a format for Belle II masterclasses
  */
  class MasterClassModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    MasterClassModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** Write out particles */
    virtual void event() override;

    /** Close ntuple */
    virtual void terminate() override;

  private:

    StoreArray<Track> m_tracks; /**< Track objects */
    StoreArray<ECLCluster> m_clusters; /**< Cluster objects */
    std::string m_filename;  /**< output file name */
    TFile* m_file = 0;  /**< root ntuple file */
    TTree* m_tree = 0;  /**< output tree */
    int m_index = 0;  /**< event number */
    BEvent* m_event = 0;  /**< output event object */
  };

}

