//+
// File : Ds2RbufModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rfarm/event/modules/Ds2RbufModule.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Ds2Rbuf)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Ds2RbufModule::Ds2RbufModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
           string("OutputRbuf"));
  addParam("CompressionLevel", m_compressionLevel, "Compression level",
           0);

  m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Ds2Rbuf: Constructor done.");
}


Ds2RbufModule::~Ds2RbufModule()
{
}

void Ds2RbufModule::initialize()
{

  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Ds2Rbuf initialized.");
}


void Ds2RbufModule::beginRun()
{
  B2INFO("Ds2Rbuf: beginRun called.");
}


void Ds2RbufModule::event()
{
  // Clear msghandler
  m_msghandler->clear();

  // Set durability
  DataStore::EDurability durability = DataStore::c_Event;

  // Stream objects in msg_handler
  const DataStore::StoreObjMap& objmap = DataStore::Instance().getObjectMap(durability);
  int nobjs = 0;
  for (DataStore::StoreObjConstIter it = objmap.begin(); it != objmap.end(); ++it) {
    if (m_msghandler->add(it->second, it->first)) {
      B2INFO("Tx: adding obj " << it->first);
      nobjs++;
    }
  }
  // Stream arrays in msg_handler
  const DataStore::StoreArrayMap& arymap = DataStore::Instance().getArrayMap(durability);
  int narrays = 0;
  for (DataStore::StoreObjConstIter it = arymap.begin(); it != arymap.end(); ++it) {
    if (m_msghandler->add(it->second, it->first)) {
      B2INFO("Ds2Rbuf: adding array " << it->first);
      narrays++;
    }
  }
  B2INFO("Ds2Rbuf: nobjs = " << nobjs << ", narrays = " << narrays <<
         " (pid=" << (int)getpid() << ")");

  // Encode event message
  EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);

  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobjs;       // No. of objects
  (msg->header())->reserved[2] = narrays;    // No. of arrays

  // Put the message in ring buffer
  for (;;) {
    int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    if (stat >= 0) break;
    usleep(200);
  }

  B2INFO("Ds2Rbuf: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;

}

void Ds2RbufModule::endRun()
{
  //fill Run data

  B2INFO("Ds2Rbuf: endRun done.");
}


void Ds2RbufModule::terminate()
{
  B2INFO("Ds2Rbuf: terminate called")
}

