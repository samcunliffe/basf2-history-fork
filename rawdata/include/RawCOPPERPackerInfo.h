#ifndef RAWCOPPERPACKERINFO_H
#define RAWCOPPERPACKERINFO_H

namespace Belle2 {
  /**
   * struct to contain header information used by RawCOPPERFormat::Packer()
   */
  struct RawCOPPERPackerInfo {
    unsigned int exp_num; // 10bit
    unsigned int run_subrun_num; // 22bit
    unsigned int eve_num; // 32bit
    unsigned int node_id; // 32bit
    unsigned int tt_ctime; // 27bit clock ticks at trigger timing distributed by FTSW. For details, see Nakao-san's belle2link user guide
    unsigned int tt_utime; // 32bit unitx time at trigger timing distributed by FTSW. For details, see Nakao-san's belle2link user guide
    unsigned int b2l_ctime; // 27bit clock ticks at trigger timing measured by HSLB on COPPER. For details, see Nakao-san's belle2link user guide
    unsigned int hslb_crc16_error_bit; // 4bit errorflag for CRC errors in data transfer via b2link. ( bit0,1,2,3 -> finesse slot a,b,c,d)
    unsigned int truncation_mask; // Not defined yet
    unsigned int type_of_data; // Not defined yet
  };
}
#endif
