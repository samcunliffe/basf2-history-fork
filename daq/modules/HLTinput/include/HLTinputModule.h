/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTINPUTMODULE_H
#define HLTINPUTMODULE_H

#include <vector>

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreIter.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/core/Module.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/EvtMessage.h>

#include <daq/hlt/EvtReceiver.h>
#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/HLTBuffer.h>

#include <TTree.h>

#define MAXPACKET 10000000 * 4

namespace Belle2 {

  class HLTInputModule : public Module {

  public:
    HLTInputModule();
    virtual ~HLTInputModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    int readData(const DataStore::EDurability&);

  protected:


  private:
    HLTBuffer* m_inBuf;
    std::string m_inBufferName;

    std::vector<std::string> m_objectNames[DataStore::c_NDurabilityTypes];
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    MsgHandler* m_msgHandler;

    StoreObjPtr<EventMetaData> eventMetaDataPtr;
  };

} // end namespace Belle2

#endif // HLTINPUTMODULE_H
