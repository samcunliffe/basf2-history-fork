//+
// File : StorageDeserializer.cc
// Description : Module to receive data from eb2rx and store online disk
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#include <daq/storage/modules/StorageDeserializer.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/datastore/StoreArray.h>

#include <rawdata/dataobjects/RawPXD.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/Time.h>

#include <TSystem.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StorageDeserializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

StorageDeserializerModule* StorageDeserializerModule::g_module = NULL;

StorageDeserializerModule::StorageDeserializerModule() : Module()
{
  setDescription("Encode DataStore into RingBuffer");

  addParam("CompressionLevel", m_compressionLevel, "Compression level", 0);
  addParam("InputBufferName", m_ibuf_name, "Input buffer name", std::string(""));
  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);

  m_count = 0;
  B2INFO("StorageDeserializer: Constructor done.");
  g_module = this;
}


StorageDeserializerModule::~StorageDeserializerModule()
{
}

void StorageDeserializerModule::initialize()
{
  B2INFO("StorageDeserializer: initialize() started.");
  m_ibuf.open(m_ibuf_name, 25000000);
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      m_info.open(m_nodename, sizeof(storage_info) / sizeof(int));
      m_sinfo = (storage_info*)m_info.getReserved();
      memset(m_sinfo, 0, sizeof(storage_info));
    }
  }
  StoreArray<RawPXD>::registerPersistent();
  if (m_shmflag > 0) {
    m_info.reportRunning();
  }
  m_count = 0;
  while (true) {
    m_package.setSerial(m_ibuf.read((int*)m_package.getData().getBuffer()));
    MsgHandler handler(m_compressionLevel);
    if (m_package.decode(handler)) {
      m_package.restore(false);
      if (m_info.isAvailable()) {
        m_sinfo->count_in = 1;
        m_sinfo->nword_in = m_package.getData().getWordSize();
      }
      break;
    }
  }
  B2INFO("StorageDeserializer: initialize() done.");
}

void StorageDeserializerModule::event()
{
  if (m_count == 0) {
    m_count++;
    return;
  }
  m_count++;
  while (true) {
    while (true) {
      m_package.setSerial(m_ibuf.read((int*)m_package.getData().getBuffer()));
      MsgHandler handler(m_compressionLevel);
      if (m_package.decode(handler)) {
        m_package.restore();
        if (m_info.isAvailable()) {
          m_sinfo->count_in++;
          m_sinfo->nword_in += m_package.getData().getWordSize();
        }
        break;
      }
    }
    StoreObjPtr<EventMetaData> evtmetadata;
    if (evtmetadata.isValid()) {
      m_expno = evtmetadata->getExperiment();
      m_runno = evtmetadata->getRun();
      m_evtno = evtmetadata->getEvent();
      break;
    } else {
      BinData& data();
      B2WARNING("NO event meta data " << m_package.getData().getExpNumber() << "." <<
                m_package.getData().getRunNumber() << "." <<
                m_package.getData().getEventNumber() << " nword = " <<
                m_package.getData().getWordSize());
      B2WARNING("Last event meta data " << m_expno << "." << m_runno << "." << m_evtno);
      DataStore::Instance().reset();
    }
  }
}

void StorageDeserializerModule::beginRun()
{
  B2INFO("StorageDeserializer: beginRun called.");
}

void StorageDeserializerModule::endRun()
{
  B2INFO("StorageDeserializer: endRun done.");
}


void StorageDeserializerModule::terminate()
{
  B2INFO("StorageDeserializer: terminate called")
}


