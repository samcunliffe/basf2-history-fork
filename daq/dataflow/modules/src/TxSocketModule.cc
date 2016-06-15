//+
// File : TxSocketModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/dataflow/modules/TxSocketModule.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TxSocket)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TxSocketModule::TxSocketModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("DestHostName", m_dest, "Destination host", string("localhost"));
  addParam("DestPort", m_port, "Destination port", 1111);

  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Tx: Constructor done.");
}



TxSocketModule::~TxSocketModule()
{
}

void TxSocketModule::initialize()
{

  // Open Socket
  m_sock = new EvtSocketSend(m_dest, m_port);

  // Create Message Handler
  //  m_msghandler = new MsgHandler(m_compressionLevel);
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  B2INFO("Tx initialized.");
}


void TxSocketModule::beginRun()
{
  B2INFO("beginRun called.");
}


void TxSocketModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

  // Send the message to Socket
  int stat = m_sock->send(msg);
  B2INFO("Tx: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;

}

void TxSocketModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void TxSocketModule::terminate()
{
  B2INFO("terminate called");
}

