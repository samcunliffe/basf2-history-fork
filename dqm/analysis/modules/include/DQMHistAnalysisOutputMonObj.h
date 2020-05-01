/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <dqm/dataobjects/DQMFileMetaData.h>
#include <string>
#include <TBranch.h>

namespace Belle2 {
  /*! Class definition for the module to store MonitoringObject to output root file*/

  class DQMHistAnalysisOutputMonObjModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputMonObjModule();
    virtual ~DQMHistAnalysisOutputMonObjModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    //! Makes entry with monitoring variables in the run summary TTree
    void addTreeEntry();

    //! if new variables is added to run summary TTree this files
    // missing (previous) entries in the TTree (with the value from the first run)
    void fillBranch(TBranch* br);

    // Data members
  private:

    std::string m_filename;/**< file name of root file */
    std::string m_procID;/**< processing id (online,proc10,etc)*/
    std::string m_treeFile;/**< if set, entry with monitoring variables is made in the run summary TTree*/
    DQMFileMetaData* m_metaData = NULL; /** output file meta data */
  };
} // end namespace Belle2

