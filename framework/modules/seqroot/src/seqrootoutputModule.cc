//+
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//         6 - Sep - 2012,  Use of DataStoreStreamer, clean up
//-

#include <framework/modules/seqroot/seqrootoutputModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/core/Environment.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <cmath>
#include <fcntl.h>

#include <TClonesArray.h>
#include <TClass.h>
#include <TList.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SeqRootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SeqRootOutputModule::SeqRootOutputModule() : Module(), m_nevt(0), m_streamer(nullptr), m_size(0), m_size2(0)
{
  //Set module properties
  setDescription("Save a sequential ROOT file (non-standard I/O format used in DAQ). See https://confluence.desy.de/display/BI/Software+PersistencyModules for further information and a comparison with the .root format.");
  m_file = 0;
  m_msghandler = 0;

  vector<string> emptyvector;
  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "SeqRoot file name.", string("SeqRootOutput.sroot"));
  addParam("compressionLevel", m_compressionLevel,
           "Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticable effect. NOTE: Because of a ROOT bug ( https://sft.its.cern.ch/jira/browse/ROOT-4550 ), this option currently causes memory leaks and is disabled.",
           0);
  addParam("saveObjs", m_saveObjs, "List of objects/arrays to be saved", emptyvector);

  B2DEBUG(1, "SeqRootOutput: Constructor done.");
}


SeqRootOutputModule::~SeqRootOutputModule() { }

void SeqRootOutputModule::initialize()
{
  const std::string& outputFileArgument = Environment::Instance().getOutputFileOverride();
  if (!outputFileArgument.empty())
    m_outputFileName = outputFileArgument;

  // Open output file
  m_file = new SeqFile(m_outputFileName.c_str(), "w");

  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  // DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);
  m_streamer->registerStreamObjs(m_saveObjs);

  //Write StreamerInfo at the beginning of a file
  writeStreamerInfos();

  B2INFO("SeqRootOutput: initialized.");
}


void SeqRootOutputModule::beginRun()
{

  // Statistics
  gettimeofday(&m_t0, 0);
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevt = 0;

  B2INFO("SeqRootOutput: beginRun called.");
}

void SeqRootOutputModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(false);

  // Store EvtMessage
  int stat = m_file->write(msg->buffer());
  //  printf("SeqRootOuput : write = %d\n", stat);

  // Clean up EvtMessage
  delete msg;

  // Statistics
  double dsize = (double)stat / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nevt++;
}

void SeqRootOutputModule::endRun()
{
  //fill Run data

  // End time
  gettimeofday(&m_tend, 0);
  double etime = (double)((m_tend.tv_sec - m_t0.tv_sec) * 1000000 +
                          (m_tend.tv_usec - m_t0.tv_usec));

  // Statistics
  // Sigma^2 = Sum(X^2)/n - (Sum(X)/n)^2

  double flowmb = m_size / etime * 1000.0;
  double avesize = m_size / (double)m_nevt;
  double avesize2 = m_size2 / (double)m_nevt;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  //  printf ( "m_size = %f, m_size2 = %f, m_nevt = %d\n", m_size, m_size2, m_nevt );
  //  printf ( "avesize2 = %f, avesize = %f, avesize*avesize = %f\n", avesize2, avesize, avesize*avesize );
  B2INFO("SeqRootOutput :  " << m_nevt << " events written with total bytes of " << m_size << " kB");
  B2INFO("SeqRootOutput : flow rate = " << flowmb << " (MB/s)");
  B2INFO("SeqRootOutput : event size = " << avesize << " +- " << sigma << " (kB)");

  B2INFO("SeqRootOutput: endRun done.");
}


void SeqRootOutputModule::terminate()
{
  delete m_msghandler;
  delete m_streamer;
  delete m_file;

  B2INFO("terminate called");
}


void SeqRootOutputModule::writeStreamerInfos()
{
  //
  // Write StreamerInfo to a file
  // Copy from RootOutputModule::initialize() and TSocket::SendStreamerInfos()
  //

  if (!m_msghandler) {
    B2FATAL("DataStoreStreamer : m_msghandler is NULL.");
    return;
  }

  TList* minilist = 0 ;
  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; durability++) {
    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));

    for (DataStore::StoreEntryIter iter = map.begin(); iter != map.end(); ++iter) {
      const TClass* entryClass = iter->second.objClass;
      TVirtualStreamerInfo* vinfo = entryClass->GetStreamerInfo();
      B2INFO("Recording StreamerInfo : durability " << durability << " : Class Name " << entryClass->GetName());
      if (!minilist) minilist  =  new TList();
      minilist->Add((TObject*)vinfo);
    }
  }

  if (minilist) {
    //       TMessage messinfo(kMESS_STREAMERINFO);
    //       messinfo.WriteObject(minilist);
    m_msghandler->add(minilist, "StreamerInfo");
    //      EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);
    EvtMessage* msg = m_msghandler->encode_msg(MSG_STREAMERINFO);
    (msg->header())->nObjects = 1;       // No. of objects
    (msg->header())->nArrays = 0;    // No. of arrays
    int size = m_file->write(msg->buffer());
    B2INFO("Wrote StreamerInfo to a file : " << size << "bytes");
    delete minilist;
  }

  return;
}
