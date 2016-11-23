//+
// File : PXDReadRawONSEN.cc
// Description : Module to Load Raw PXD Data from ONSEN network-dump file and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
//
// Author : Bjoern Spruck
// Date : 15 - Apr - 2014
//-

#include <pxd/modules/pxdUnpacking/PXDReadRawONSEN.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDReadRawONSEN)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDReadRawONSENModule::PXDReadRawONSENModule() : Module()
{
  fh = 0;
  m_msghandler = 0;
  //Set module properties
  setDescription("Read a Raw PXD-Data Dump from ONSEN (or a simulator) and stores it as RawPXD in Data Store");
  //setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  //setPropertyFlags(c_Input);

  addParam("FileName", m_filename, "file name");
  m_nread = 0;
  m_compressionLevel = 0;
  m_buffer = new int[MAXEVTSIZE];

  B2DEBUG(0, "PXDReadRawONSENModule: Constructor done.");
}


PXDReadRawONSENModule::~PXDReadRawONSENModule()
{
  delete[] m_buffer;
}

void PXDReadRawONSENModule::initialize()
{
  // Open receiver sockets
//  m_recv = new EvtSocketSend(m_host, m_port);
  fh = fopen(m_filename.c_str(), "rb");
  if (fh) {
    B2INFO("Read Raw ONSEN Data from " << m_filename);
  } else {
    B2ERROR("Could not open Raw ONSEN Data: " << m_filename);
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  // Initialize Array of RawCOPPER
  StoreArray<RawPXD> storeRawPIDs;
  storeRawPIDs.registerInDataStore();

  B2DEBUG(0, "PXDReadRawONSENModule: initialized.");
}

void PXDReadRawONSENModule::beginRun()
{
  B2DEBUG(0, "beginRun called.");
}

int PXDReadRawONSENModule::read_data(char* data, size_t len)
{
  size_t l = 0;
  if (fh) l = fread(data, 1, len, fh);
  if (l != len) return 0;
  return l;
}

int PXDReadRawONSENModule::readOneEvent()
{
  char* data = (char*)m_buffer;
  int len = MAXEVTSIZE * sizeof(int);

#define MAX_PXD_FRAMES  256
  const int headerlen = 8;
  ubig32_t* pxdheader = (ubig32_t*) data;
  ubig32_t* pxdheadertable = (ubig32_t*) &data[headerlen];
  int framenr = 0, tablelen = 0, datalen = 0;
  int br = read_data(data, headerlen);
  if (br <= 0) return br;
  if (pxdheader[0] != 0xCAFEBABEu) {
    printf("pxdheader wrong : Magic %X , Frames %X \n", (unsigned int) pxdheader[0], (unsigned int) pxdheader[1]);
    exit(0);
  }
  framenr = pxdheader[1];
  if (framenr > MAX_PXD_FRAMES) {
    printf("MAX_PXD_FRAMES too small : %d(%d) \n", framenr, MAX_PXD_FRAMES);
    exit(0);
  }
  tablelen = 4 * framenr;
  br = read_data((char*)&data[headerlen], tablelen);
  if (br <= 0) return br;
  for (int i = 0; i < framenr; i++) {
    datalen += (pxdheadertable[i] + 3) & 0xFFFFFFFC;
  }

  if (datalen + headerlen + tablelen > len) {
    printf("buffer too small : %d %d %d(%d) \n", headerlen, tablelen, datalen, len);
    exit(0);
  }
  int bcount = read_data(data + headerlen + tablelen, datalen);
  if (br <= 0) return br;
  return (headerlen + tablelen + bcount);
}

void PXDReadRawONSENModule::event()
{
  if (fh == 0) {
    B2ERROR("Unexpected close of dump file.");
    PXDReadRawONSENModule::endRun();
    PXDReadRawONSENModule::terminate();
    return;
  }
  // DataStore interface
  StoreArray<RawPXD> rawpxdary;

  // Get a record from socket
  int stat;
  do {
    stat = readOneEvent();
    if (stat <= 0) {
      /// End of File
      PXDReadRawONSENModule::endRun();
      PXDReadRawONSENModule::terminate();
      return;
    };
  } while (stat == 0);

  // Fill RawPXD in DataStore, stat=lenght_in_Bytes
  rawpxdary.appendNew(m_buffer, stat);


  // Update EventMetaData
  m_eventMetaDataPtr.create();
  for (auto& it : rawpxdary) {
    if (getTrigNr(it)) break; // only first (valid) one
  }

  m_nread++;

  return;
}

void PXDReadRawONSENModule::endRun()
{
  //fill Run data

  B2DEBUG(0, "endRun done.");
}


void PXDReadRawONSENModule::terminate()
{
  B2INFO("terminate called");
  if (fh) fclose(fh);
  fh = 0;
}


bool PXDReadRawONSENModule::getTrigNr(RawPXD& px)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Trigger Shifter --> invalid packet size (32bit words) " << hex << px.size());
    return false;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());


  if (fullsize < 8) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    return false;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    B2ERROR("Magic invalid: Will not unpack anything. Header corrupted! " << hex << data[0]);
    return false;
  }

  Frames_in_event = ((ubig32_t*)data.data())[1];
  if (Frames_in_event < 1 || Frames_in_event > 250) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    return false;
  }

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_in_event];
  datafullsize = fullsize - 2 * 4 - Frames_in_event * 4; // minus header, minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_in_event; j++) {
    int lo;/// len of frame in bytes
    lo = ((ubig32_t*)tableptr)[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      return false;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
      return false;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
    } else {
      if (unpack_dhc_frame(ll + (char*)dataptr)) return true;
      ll += lo; /// no rounding needed
    }
  }
  return false;
}

bool PXDReadRawONSENModule::unpack_dhc_frame(void* data)
{
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1 //CLUSTER FRAME
#define DHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_START   0x3
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_END     0x4
// DHC envelope, new
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_START  0xB
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_END    0xC
// Onsen processed data, new
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP     0xD
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE     0x9
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI     0xF
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG     0xE

  switch (((*(ubig16_t*)data) & 0x7800) >> 11) {
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG: {
      unsigned int trignr = ((ubig32_t*)data)[2];
      unsigned int tag = ((ubig32_t*)data)[3];

      B2INFO("Set event and time from PXD: $" << hex << trignr << ", $" << hex << tag);
//       evtPtr.create();
      m_eventMetaDataPtr->setEvent(trignr);
      m_eventMetaDataPtr->setRun((tag & 0x003FFF00) >> 8);
      m_eventMetaDataPtr->setSubrun(tag & 0xFF);
      m_eventMetaDataPtr->setExperiment((tag & 0xFFC00000) >> 22);
      m_eventMetaDataPtr->setTime(0);
      break;
    }
    default:
      break;

  }
  return false;
}
