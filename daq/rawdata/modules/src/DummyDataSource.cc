//+
// File : DummyDataPacker.cc
// Description : Module to store dummy data in RawCOPPER object
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 14 - Jul - 2014
//-

#include <daq/rawdata/modules/DummyDataSource.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DummyDataSource)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DummyDataSourceModule::DummyDataSourceModule() : Module()
{
  ///Set module properties
  setDescription("an Example to pack data to a RawCOPPER object");

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("MaxEventNum", max_nevt, "Maximum event number to make", -1);

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("NodeID", m_nodeid, "Node ID", 0);

  B2INFO("DummyDataSource: Constructor done.");
}



DummyDataSourceModule::~DummyDataSourceModule()
{

}


void DummyDataSourceModule::initialize()
{
  B2INFO("DummyDataSource: initialize() started.");

  // Open message handler
  //  m_msghandler = new MsgHandler(m_compressionLevel);

  /// Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

#ifdef USE_RAWDATABLOCK
  raw_datablkarray.registerPersistent();
#endif

  rawcprarray.registerPersistent();

  n_basf2evt = 0;

  B2INFO("DummyDataSource: initialize() done.");
}




void DummyDataSourceModule::event()
{

  //    Make RawCOPPER array
  rawcprarray.create();


  //
  // Fill event info (These values will be stored in RawHeader )
  //
  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 1;
  rawcprpacker_info.run_subrun_num = 2; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = n_basf2evt;
  rawcprpacker_info.node_id = m_nodeid;
  rawcprpacker_info.tt_ctime = 0x7123456;
  rawcprpacker_info.tt_utime = 0xF1234567;
  rawcprpacker_info.b2l_ctime = 0x7654321;


  //
  // Prepare buffer to fill dummy data
  //
  RawCOPPER* raw_copper = rawcprarray.appendNew();

  int* buf1, *buf2, *buf3, *buf4;
  int nwords_1st = 0, nwords_2nd = 0, nwords_3rd = 0, nwords_4th = 0;

  nwords_1st = n_basf2evt % 10;
  buf1 = new int[ nwords_1st];

  for (int i = 0; i < nwords_1st; i++) {
    if (i == 0) {
      buf1[ i ] = 0x12345678;
    } else {
      buf1[ i ] = (i + 1) * buf1[ i - 1 ];
    }
  }

  nwords_2nd = (n_basf2evt + 1) % 10;
  buf2 = new int[ nwords_2nd];
  for (int i = 0; i < nwords_2nd; i++) {
    if (i == 0) {
      buf2[ i ] = 0x34567890;
    } else {
      buf2[ i ] = (i + 1) * buf2[ i - 1 ];
    }
  }

  nwords_3rd = 3 * (n_basf2evt + 2) % 10;
  buf3 = new int[ nwords_3rd];
  for (int i = 0; i < nwords_3rd; i++) {
    if (i == 0) {
      buf3[ i ] = 0x56789012;
    } else {
      buf3[ i ] = (i + 1) * buf3[ i - 1 ];
    }
  }

  nwords_4th = 4 * (n_basf2evt + 3)  % 10;
  buf4 = new int[ nwords_4th];
  for (int i = 0; i < nwords_4th; i++) {
    if (i == 0) {
      buf4[ i ] = 0x78901234;
    } else {
      buf4[ i ] = (i + 1) * buf4[ i - 1 ];
    }
  }

#ifdef USE_RAWDATABLOCK
  raw_copper->PackDetectorBuf4DummyData(buf1, nwords_1st,
                                        buf2, nwords_2nd,
                                        buf3, nwords_3rd,
                                        buf4, nwords_4th,
                                        rawcprpacker_info);
#else
  raw_copper->PackDetectorBuf(buf1, nwords_1st,
                              buf2, nwords_2nd,
                              buf3, nwords_3rd,
                              buf4, nwords_4th,
                              rawcprpacker_info);

#endif

#ifdef USE_RAWDATABLOCK
  raw_datablkarray.create();
  RawDataBlock* raw_datablk = raw_datablkarray.appendNew();
  raw_datablk->SetBuffer(raw_copper->GetWholeBuffer(), raw_copper->TotalBufNwords(),
                         false, 1, 1);
#endif

  delete [] buf1;
  delete [] buf2;
  delete [] buf3;
  delete [] buf4;

  //
  // Update EventMetaData : Not affect on the output
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(rawcprpacker_info.exp_num);
  m_eventMetaDataPtr->setRun(rawcprpacker_info.run_subrun_num);
  m_eventMetaDataPtr->setEvent(n_basf2evt);


  //
  // Monitor
  //
  if (max_nevt >= 0) {
    if (n_basf2evt >= max_nevt && max_nevt > 0) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d ) Processed Event %d \n",
             max_nevt , n_basf2evt);
      fflush(stdout);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  if (n_basf2evt % 1000 == 0) {
    printf("Dummy data : Event # %.8d\n", n_basf2evt);
    fflush(stdout);
  }

  n_basf2evt++;
  return;
}
