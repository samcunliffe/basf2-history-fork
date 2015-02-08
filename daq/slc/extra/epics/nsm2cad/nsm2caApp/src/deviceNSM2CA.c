#include "devNSM2CA.h"

#include <epicsExport.h>
#include <dbAccess.h>
#include <dbScan.h>
#include <devSup.h>
#include <recSup.h>
#include <recGbl.h>

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_stringin;
} dev_nsm2_request_in = {
  5, NULL, NULL, init_nsm2_request_stringin,
  get_ioint_info_nsm2_stringin, read_nsm2_request_stringin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN write_stringin;
} dev_nsm2_state_in = {
  5, NULL, NULL, init_nsm2_state_stringin,
  get_ioint_info_nsm2_stringin, read_nsm2_state_stringin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN write_stringout;
} dev_nsm2_request_out = {
  5, NULL, NULL, init_nsm2_request_stringout,
  NULL, write_nsm2_request_stringout
};

struct
{
  long num;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_ai;
  DEVSUPFUN special_linconv;
} dev_nsm2_data_ai = {
  6, NULL, NULL, init_nsm2_data_ai,
  NULL, read_nsm2_data_ai, NULL
};

struct
{
  long num;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_ai;
  DEVSUPFUN special_linconv;
} dev_nsm2_data_longin = {
  6, NULL, NULL, init_nsm2_data_longin,
  NULL, read_nsm2_data_longin, NULL
};

epicsExportAddress(dset, dev_nsm2_request_in);
epicsExportAddress(dset, dev_nsm2_state_in);
epicsExportAddress(dset, dev_nsm2_request_out);
epicsExportAddress(dset, dev_nsm2_data_ai);
epicsExportAddress(dset, dev_nsm2_data_longin);
