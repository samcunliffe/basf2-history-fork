//+
// File : PrintData.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/PrintData.h>
#include <framework/core/InputController.h>


using namespace std;
using namespace Belle2;

#define NOT_USE_SOCKETLIB
//#define NOT_SEND
//#define DUMMY_DATA
#define TIME_MONITOR

//#define MULTIPLE_SEND
//#define MEMCPY_TO_ONE_BUFFER
#define SEND_BY_WRITEV

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintData)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintDataModule::PrintDataModule() : Module()
{
  //Set module properties

  setDescription("Encode DataStore into RingBuffer");
  n_basf2evt = -1;
  m_compressionLevel = 0;

}



PrintDataModule::~PrintDataModule()
{
}

void PrintDataModule::initialize()
{
  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerAsPersistent();

  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);

}


void PrintDataModule::beginRun()
{
  B2INFO("beginRun called.");
}




void PrintDataModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void PrintDataModule::terminate()
{
  B2INFO("terminate called");
}



//
// User defined functions
//





void PrintDataModule::VerifyCheckSum(int* buf)     // Should be modified
{

  int check_sum = 0;
  for (int i = 0 ; i < buf[0]; i++) {
    if (i != 2)   check_sum += buf[i];
  }

  if (buf[2] != check_sum) {
    cout << "Invalid checksum : " << check_sum << " " << buf[1] << endl;
    exit(1);
  }
}


void PrintDataModule::PrintEvent(RawCOPPER* raw_array)
{


  int tot_size_byte = raw_array->TotalBufNwords() * sizeof(int);
  for (int i = 0; i < raw_array->GetNumEntries(); i++) {

    RawHeader rawhdr;
    int* buf;
    int size_byte = 0;
    buf = raw_array->GetBuffer(i);

    rawhdr.SetBuffer(raw_array->GetRawHdrBufPtr(i));

    size_byte = raw_array->GetCprBlockNwords(i) * sizeof(int);

    //
    // Extract FEE buffer
    //
    int* finnesse_buf_1st;
    int* finnesse_buf_2nd;
    int* finnesse_buf_3rd;
    int* finnesse_buf_4th;

    int* detector_buf_1st;
    int* detector_buf_2nd;
    int* detector_buf_3rd;
    int* detector_buf_4th;


    printf("(size %d) \nexp %d run %d eve %d copperNode %d type %d\n",
           tot_size_byte, i, size_byte,
           rawhdr.GetExpNo(),
           rawhdr.GetRunNo(),
           rawhdr.GetEveNo(),
           rawhdr.GetSubsysId(),
           rawhdr.GetDataType());


    finnesse_buf_1st = raw_array->Get1stFINNESSEBuffer(i);
    finnesse_buf_2nd = raw_array->Get2ndFINNESSEBuffer(i);
    finnesse_buf_3rd = raw_array->Get3rdFINNESSEBuffer(i);
    finnesse_buf_4th = raw_array->Get4thFINNESSEBuffer(i);
    printf("FINNNESSE buf %p %p %p %p\n", finnesse_buf_1st, finnesse_buf_2nd, finnesse_buf_3rd, finnesse_buf_4th);

    detector_buf_1st = raw_array->Get1stDetectorBuffer(i);
    detector_buf_2nd = raw_array->Get2ndDetectorBuffer(i);
    detector_buf_3rd = raw_array->Get3rdDetectorBuffer(i);
    detector_buf_4th = raw_array->Get4thDetectorBuffer(i);
    printf("Detector  buf %p %p %p %p\n", detector_buf_1st, detector_buf_2nd, detector_buf_3rd, detector_buf_4th);

  }
}


void PrintDataModule::event()
{
  StoreArray<RawCOPPER> rawcprarray;
  StoreArray<RawCDC> raw_cdcarray;

  for (int j = 0; j < rawcprarray.getEntries(); j++) {
    printf("=== RawCOPPER event====\nBlk %d ", j);
    //  PrintEvent( &rawcprarray );
    PrintEvent(rawcprarray[ j ]);
  }

  for (int j = 0; j < raw_cdcarray.getEntries(); j++) {
    printf("=== RawCDC    event====\nBlk %d ", j);
    //  PrintEvent( &rawcprarray );
    PrintEvent(raw_cdcarray[ j ]);
  }


  n_basf2evt++;

}
