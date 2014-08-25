#define MAX_NODES 10

const int rfunitinfo_revision = 1;

struct rfunitinfo {
  struct rfnodeinfo {
    int32 sysstate;
    int32 nevent_in;
    int32 nqueue_in;
    int32 nevent_out;
    int32 nqueue_out;
    int32 error;
    int32 i_reserved[9];
    float flowrate_in;
    float flowrate_out;
    float avesize_in;
    float avesize_out;
    float evtrate_in;
    float evtrate_out;
    float loadave;
    float r_reserved[9];
  } nodeinfo[10];
  uint32 nnodes;
  uint32 updatetime;
  int32 reserved[2];
};

