#pragma once

#include <zmq.hpp>

namespace Belle2 {

  class ZMQModuleMessage {
  public:
    bool isMessageType()
    {

    }

  private:
    unsigned  int m_zmqMessageFrames;
    std::vector<zmq::message_t> m_zmqMessages;


  };


}
