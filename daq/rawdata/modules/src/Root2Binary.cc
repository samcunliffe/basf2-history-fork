//+
// File : PrintData.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/Root2Binary.h>

using namespace std;
using namespace Belle2;

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Root2Binary)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Root2BinaryModule::Root2BinaryModule() : PrintDataTemplateModule()
{
  addParam("outputFileName", m_fname_out, "Output binary filename", string(""));
}

Root2BinaryModule::~Root2BinaryModule()
{
}

void Root2BinaryModule::initialize()
{
  B2INFO("Root2Binary: initialize() started.");
  m_msghandler = new MsgHandler(m_compressionLevel);

  m_fp_out = fopen(m_fname_out.c_str(), "w");
  if (!m_fp_out) {
    char    err_buf[500];
    sprintf(err_buf, "Cannot open an output file: %s : Exiting...\n",
            m_fname_out.c_str());
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }

  B2INFO("Root2Binary: initialize() done.");
}

void Root2BinaryModule::endRun()
{
  //fill Run data
  //  fclose( m_fp_out );
  B2INFO("endRun done.");
}


void Root2BinaryModule::terminate()
{
  fclose(m_fp_out);
  B2INFO("terminate called");
}


void Root2BinaryModule::write_event(RawDataBlock* raw_dblk, int* first_flag, int* break_flag, int* dblk_pos, unsigned int* dblk_eve)
{
  //
  // Data Block
  //
  *break_flag = 0;
  *first_flag = 0;


  for (int j = *dblk_pos; j < raw_dblk->GetNumEntries(); j++) {
    printf("numentries %d %d\n", raw_dblk->GetNumEntries(), j);
    int size = 4 * raw_dblk->GetBlockNwords(j);

    unsigned int prev_eve = *dblk_eve;
    int num_nodes = 1;
    int num_events = 1;
    int malloc_flag = 0;
    int nwords = raw_dblk->GetBlockNwords(j);
    int* temp_buf = raw_dblk->GetBuffer(j);


    if (raw_dblk->CheckFTSWID(j)) {
      RawFTSW temp_raw_ftsw;
      temp_raw_ftsw.SetBuffer(temp_buf, nwords, malloc_flag, num_nodes, num_events);
      *dblk_eve = temp_raw_ftsw.GetEveNo(0);
    } else {
      RawCOPPER temp_raw_copper;
      temp_raw_copper.SetBuffer(temp_buf, nwords, malloc_flag, num_nodes, num_events);
      *dblk_eve = temp_raw_copper.GetEveNo(0);
    }

    if (*dblk_eve != prev_eve && *first_flag == 1) {
      *dblk_pos = j;
      return;
    } else {
      fwrite((char*)temp_buf, 1, nwords * 4,  m_fp_out);
      printf("eve %d size %d j %d\n", *dblk_eve, nwords * 4, j);
    }

    *first_flag = 1;
  }
  *dblk_pos = 0;

  return;
}




void Root2BinaryModule::event()
{


  B2INFO("Root2Binary: event() started.");
  int array_entries;
  int num_eve;
  int num_nodes;

  //
  // Data Block
  //
  StoreArray<RawDataBlock> raw_dblkarray;
  StoreArray<RawFTSW> raw_ftswarray;
  StoreArray<RawCOPPER> raw_copperarray;
  StoreArray<RawSVD> raw_svdarray;
  StoreArray<RawCDC> raw_cdcarray;
  StoreArray<RawBPID> raw_bpidarray;
  StoreArray<RawEPID> raw_epidarray;
  StoreArray<RawKLM> raw_klmarray;
  StoreArray<RawECL> raw_eclarray;

  int dblk_array = 0;
  int ftsw_array = 0;
  int copper_array = 0;
  int svd_array = 0;
  int cdc_array = 0;
  int ecl_array = 0;
  int bpid_array = 0;
  int epid_array = 0;
  int klm_array = 0;

  int dblk_pos = 0;
  int ftsw_pos = 0;
  int copper_pos = 0;
  int svd_pos = 0;
  int cdc_pos = 0;
  int ecl_pos = 0;
  int bpid_pos = 0;
  int epid_pos = 0;
  int klm_pos = 0;

  unsigned int dblk_eve = 0;
  unsigned int ftsw_eve = 0;
  unsigned int copper_eve = 0;
  unsigned int svd_eve = 0;
  unsigned int cdc_eve = 0;
  unsigned int ecl_eve = 0;
  unsigned int bpid_eve = 0;
  unsigned int epid_eve = 0;
  unsigned int klm_eve = 0;


  int cnt = 0;
  while (true) {
    int write_flag = 0;

    //
    // DataBlock
    //
    int break_flag;
    int first_flag;
    unsigned int eve;

    break_flag = 0;
    first_flag = 0;
    dblk_eve = 0;
    array_entries = raw_dblkarray.getEntries();
    for (int i = dblk_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_dblkarray[ i ], &first_flag, &break_flag, &dblk_pos, &dblk_eve);
      if (break_flag == 1) {
        dblk_array = i;
        break;
      }
      if (i == array_entries - 1) {
        dblk_array = array_entries;
      }
    }

    break_flag = 0;
    first_flag = 0;
    ftsw_eve = 0;
    array_entries = raw_ftswarray.getEntries();
    for (int i = ftsw_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_ftswarray[ i ], &first_flag, &break_flag, &ftsw_pos, &ftsw_eve);
      if (break_flag == 1) {
        ftsw_array = i;
        break;
      }
      if (i == array_entries - 1) {
        ftsw_array = array_entries;
      }
    }


    break_flag = 0;
    first_flag = 0;
    copper_eve = 0;
    array_entries = raw_copperarray.getEntries();
    for (int i = copper_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_copperarray[ i ], &first_flag, &break_flag, &copper_pos, &copper_eve);
      if (break_flag == 1) {
        copper_array = i;
        break;
      }
      if (i == array_entries - 1) {
        copper_array = array_entries;
      }
    }


    break_flag = 0;
    first_flag = 0;
    svd_eve = 0;
    array_entries = raw_svdarray.getEntries();
    for (int i = svd_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_svdarray[ i ], &first_flag, &break_flag, &svd_pos, &svd_eve);
      if (break_flag == 1) {
        svd_array = i;
        break;
      }
      if (i == array_entries - 1) {
        svd_array = array_entries;
      }
    }


    break_flag = 0;
    first_flag = 0;
    cdc_eve = 0;
    array_entries = raw_cdcarray.getEntries();
    for (int i = cdc_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_cdcarray[ i ], &first_flag, &break_flag, &cdc_pos, &cdc_eve);
      if (break_flag == 1) {
        cdc_array = i;
        break;
      }
      if (i == array_entries - 1) {
        cdc_array = array_entries;
      }
    }


    break_flag = 0;
    first_flag = 0;
    bpid_eve = 0;
    array_entries = raw_bpidarray.getEntries();
    for (int i = bpid_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_bpidarray[ i ], &first_flag, &break_flag, &bpid_pos, &bpid_eve);
      if (break_flag == 1) {
        bpid_array = i;
        break;
      }
      if (i == array_entries - 1) {
        bpid_array = array_entries;
      }
    }



    break_flag = 0;
    first_flag = 0;
    epid_eve = 0;
    array_entries = raw_epidarray.getEntries();
    for (int i = epid_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_epidarray[ i ], &first_flag, &break_flag, &epid_pos, &epid_eve);
      if (break_flag == 1) {
        epid_array = i;
        break;
      }
      if (i == array_entries - 1) {
        epid_array = array_entries;
      }
    }



    break_flag = 0;
    first_flag = 0;
    ecl_eve = 0;
    array_entries = raw_eclarray.getEntries();
    for (int i = ecl_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_eclarray[ i ], &first_flag, &break_flag, &ecl_pos, &ecl_eve);
      if (break_flag == 1) {
        ecl_array = i;
        break;
      }
      if (i == array_entries - 1) {
        ecl_array = array_entries;
      }
    }

    break_flag = 0;
    first_flag = 0;
    klm_eve = 0;
    array_entries = raw_klmarray.getEntries();
    for (int i = klm_array; i < array_entries; i++) {
      write_flag = 1;
      write_event(raw_klmarray[ i ], &first_flag, &break_flag, &klm_pos, &klm_eve);
      if (break_flag == 1) {
        klm_array = i;
        break;
      }
      if (i == array_entries - 1) {
        klm_array = array_entries;
      }
    }
    if (write_flag == 0)break;
  }

  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;
  return;

}
