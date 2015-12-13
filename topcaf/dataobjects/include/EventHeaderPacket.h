#ifndef EventHeaderPacket_H
#define EventHeaderPacket_H

////////////////////////////////////////////////////////////////////////////////////
// Data class for the Event Packet: Header information as defined in:             //
// http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format //
// Author: Malachi Schram (malachi.schram@pnnl.gov)                               //
////////////////////////////////////////////////////////////////////////////////////

#include <topcaf/dataobjects/Packet.h>

namespace Belle2 {

#define MASK_TRIGGER (0x000000FF)
#define MASK_FLAG (0x000000FF)

  class EventHeaderPacket: public Packet {

  public:
    // empty default constructor
    // do not remove. This is necessary to put this class in the event store
    EventHeaderPacket();
    EventHeaderPacket(const unsigned int* temp_buffer, int nwords);

    //--- Getters ---//
    unsigned int GetFreezeDate() const {return m_freeze_date;}
    int GetEventNumber() const {return m_evt_num;}
    bool GetEventTrigger() const {return (MASK_TRIGGER & m_trigger);} //use only 1st byte
    bool GetEventFlag() const {return (MASK_FLAG & m_flag);} //use only 1st byte
    int GetNumWaveformPackets() const {return m_nwaves;}
    int GetNumAuxiliaryPackets() const {return m_aux;}
    double GetFTSW() {return m_ftsw;}

    //--- Setters ---//
    void SetFTSW(double ftsw) {m_ftsw = ftsw;}

  private:
    packet_word_t m_freeze_date;
    packet_word_t m_evt_num;
    packet_word_t m_trigger;
    packet_word_t m_flag;
    packet_word_t m_nwaves;
    packet_word_t m_aux;
    double m_ftsw;

    ClassDef(EventHeaderPacket, 1);
  };
}
#endif
