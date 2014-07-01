//+
// File : RawCOPPER.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawCOPPER.h>



using namespace std;
using namespace Belle2;

//#define DESY
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK

ClassImp(RawCOPPER);

RawCOPPER::RawCOPPER()
{
  m_access = NULL;
  m_version = -1;
}

RawCOPPER::~RawCOPPER()
{
  if (m_access != NULL) delete m_access;
  m_access = NULL;
}

void RawCOPPER::SetVersion()
{

  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }

  if (m_access != NULL) {
    delete m_access;
  }

  m_version = ((m_buffer[ POS_FORMAT_VERSION ]) & FORMAT_MASK) >> 8;
  switch (m_version) {
    case 1 :
      m_access = new PostRawCOPPERFormat_latest;
      //      printf("########################## PostRawCOPPERFormat_latest\n");
      break;
    case 129 :
      m_access = new PreRawCOPPERFormat_latest;
      //      printf("########################## PreRawCOPPERFormat_latest\n");
      break;
    case 0 :
      m_access = new RawCOPPERFormat_v0;
      //      printf("########################## RawCOPPERFormat_v0\n");
      break;
    case 128 :
      m_access = new PreRawCOPPERFormat_v0;
      //      printf("########################## PreRawCOPPERFormat_v0\n");
      break;
    default : {
      char err_buf[500];
      sprintf(err_buf, "Invalid version of a data format(0x%.2x). Exiting...\n %s %s %d\n",
              m_buffer[ POS_FORMAT_VERSION ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
      perror(err_buf);
      string err_str = err_buf; throw (err_str);
    }
    exit(1);
  }

  m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);



}

void RawCOPPER::SetVersion(string class_name)
{

  if (m_access != NULL) {
    delete m_access;
  }

  if (class_name == "RawCOPPERFormat_latest") {
    m_access = new PostRawCOPPERFormat_latest;
    m_version = (0 << 7) | 1;
  } else if (class_name == "PreRawCOPPERFormat_latest") {
    m_access = new PreRawCOPPERFormat_latest;
    m_version = (1 << 7) | 1;
  } else if (class_name == "RawCOPPERFormat_v0") {
    m_access = new RawCOPPERFormat_v0;
    m_version = (0 << 7) | 0;
  } else if (class_name == "PreRawCOPPERFormat_v0") {
    m_access = new PreRawCOPPERFormat_v0;
    m_version = (1 << 7) | 0;
  } else {
    char err_buf[500];
    sprintf(err_buf, "Invalid name of a data format class (%s). Exiting...\n %s %s %d\n",
            class_name.c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__);
    perror(err_buf);
    string err_str = err_buf;
    throw (err_str);
  }

//   if( class_name == "RawCOPPERFormat_v0" ){
//     m_access = new RawCOPPERFormat_v0;
//     m_version = ( 0 << 7 ) | 0;
//   }else if( class_name == "PreRawCOPPERFormat_v0" ){
//     m_access = new PreRawCOPPERFormat_v0;
//     m_version = ( 1 << 7 ) | 0;
//   }else if( class_name == "RawCOPPERFormat_v2" ){
//     m_access = new RawCOPPERFormat_v2;
//     m_version = ( 0 << 7 ) | 2;
//   }else if( class_name == "PreRawCOPPERFormat_v2" ){
//     m_access = new PreRawCOPPERFormat_v2;
//     m_version = ( 1 << 7 ) | 2;
//   }
  m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);

}



void RawCOPPER::SetBuffer(int* bufin, int nwords, int malloc_flag, int num_events, int num_nodes)
{
  if (bufin == NULL) {
    printf("[DEBUG] bufin is NULL. Exting...\n");
    exit(1);
  }

  if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;

  if (malloc_flag == 0) {
    m_use_prealloc_buf = true;
  } else {
    m_use_prealloc_buf = false;
  }
  //  m_nwords = bufin[0];
  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  SetVersion();

}


void RawCOPPER::ShowBuffer()
{

  printf("\n");
  printf("POINTER %p\n", m_buffer);
  for (int k = 0; k < 100; k++) {
    printf("%.8x ", m_buffer[ k ]);
    if ((k + 1) % 10 == 0) printf("\n%.8x : ", k);
  }
  printf("\n");

}
