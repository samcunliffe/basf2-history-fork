//+
// File : DeSerializer.cc
// Description : Base class for Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/DeSerializer.h>

//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DeSerializerModule::DeSerializerModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");

  addParam("DumpFileName", dump_fname, "filename to record data", string(""));
  addParam("EventDataBufferWords", BUF_SIZE_WORD, "DataBuffer words per event", 4800);
  addParam("MaxEventNum", max_nevt, "Maximum event number in one run", -1);


  n_basf2evt = -1;
  m_totbytes = 0;
  m_compressionLevel = 0;


  prev_event = -1;

  //Parameter definition
  B2INFO("DeSerializer: Constructor done.");
}


DeSerializerModule::~DeSerializerModule()
{
  delete[] m_buffer;
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    delete[] m_bufary[i];
  }

  if (dump_fname.size() > 0) {
    fclose(fp_dump);
  }

}


void DeSerializerModule::initialize()
{

  // allocate buffer
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];

}

void DeSerializerModule::event()
{
}


void DeSerializerModule::beginRun()
{
  B2INFO("beginRun called.");
}


#define POS_HEADER_MAGIC 7
#define POS_FOOTER_MAGIC 3
#define POS_EVENT_NUM 16
int DeSerializerModule::check_data(char* buf, int prev_eve, int* cur_eve)
{

  int pos_header_magic, pos_footer_magic, pos_event_num;

#ifdef WITH_HEADER_TRAILER
  pos_header_magic = POS_HEADER_MAGIC + WORD_SENDHEADER + WORD_RAWCPRHEADER;
  pos_footer_magic = POS_FOOTER_MAGIC + WORD_SENDTRAILER;
  pos_event_num = POS_EVENT_NUM + WORD_SENDHEADER + WORD_RAWCPRHEADER;
#else
  pos_header_magic = POS_HEADER_MAGIC;
  pos_footer_magic = POS_FOOTER_MAGIC;
  pos_event_num = POS_EVENT_NUM;
#endif

  unsigned int* m_buf = (unsigned int*)buf;

  // check footer magic number
  int word_num = m_buf[ 0 ];
//   printf("posheader %d\n", pos_header_magic);
//     for( int i = 0; i < 100; i++){
//       printf("%.8x ", m_buf[i]);
//       if( i + 1 % 10 == 0 ) printf("\n");
//     }
//     exit(1);

  // check header magic number
  if (m_buf[ pos_header_magic ] != 0xfffffafa) {

    printf("invalid header magic word : %d 0x%x\n", pos_header_magic, m_buf[ pos_footer_magic ]);

    exit(1);
  }


  // check event number
  *cur_eve = m_buf[ pos_event_num ];
  if (prev_eve >= 0 && m_buf[ pos_event_num ] != prev_eve + 1) {
    perror("invalid event number");
    exit(1);
  }

  //  printf("%d 0x%x 0x%x\n", m_buf[ POS_EVENT_NUM ], m_buf[ POS_HEADER_MAGIC ], m_buf[ word_num - POS_FOOTER_MAGIC ] );
  if (m_buf[ word_num - pos_footer_magic ] != 0xfffff5f5) {
    printf("invalid footer magic word : %d 0x%x\n", word_num - pos_footer_magic, m_buf[ word_num - pos_footer_magic ]);
    exit(1);
  }

  return 0;


}




void DeSerializerModule::endRun()
{
  //fill Run data
  B2INFO("endRun done.");

}


void DeSerializerModule::terminate()
{
  B2INFO("terminate called");

  //  exit(1);
}

double DeSerializerModule::GetTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}


void DeSerializerModule::RecordTime(int event, double* array)
{
  if (event >= 10000 && event < 10500) {
    array[ event - 10000 ] = GetTimeSec() - m_start_time;
  }
  return;
}


void DeSerializerModule::OpenOutputFile()
{
  if ((fp_dump = fopen(dump_fname.c_str(), "wb")) == NULL) {
    perror("Failed to open file.");
    printf("Failed to open file %s. Exting...\n", dump_fname.c_str());
    exit(-1);
  }
}

void DeSerializerModule::DumpData(char* buf, int size)
{
  if (fwrite(buf, size, 1, fp_dump) <= 0) {
    perror("Failed to write buffer to a file. Exiting...");
    exit(-1);
  }
}
