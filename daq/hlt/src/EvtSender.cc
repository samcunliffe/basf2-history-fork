/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cstdlib>

#include <daq/hlt/EvtSender.h>
#include <daq/hlt/B2SocketException.h>
#include "framework/logging/Logger.h"

using namespace Belle2;

/// @brief EvtSender constructor
///        Creates and connects to the socket
/// @param host Host IP address to send data
/// @param port Port number for the communication
EvtSender::EvtSender(std::string host, int port)
    : m_host(host), m_port(port)
{
}

void EvtSender::init()
{
  B2INFO("Initializing EvtSender...");
  if (!B2Socket::create())
    throw B2SocketException("Could not create sending socket.");
  //B2ERROR ("Could not create sending socket.");
}

void EvtSender::init(RingBuffer* buffer)
{
  init();
  m_buffer = buffer;
}

void EvtSender::setDestination(std::string dest)
{
  m_host = dest;
}

int EvtSender::connect()
{
  if (!B2Socket::connect(m_host, m_port)) {
    //throw B2SocketException ("Could not bind to port.");
    B2ERROR("Could not bind to port.");
    return -1;
  } else
    return 1;
}

int EvtSender::broadCasting()
{
  if (m_buffer->numq() > 0) {
    if (connect()) {
      char* tmp = new char[MAXPACKETSIZE];
      m_buffer->remq((int*)tmp);
      std::string input(tmp);

      //B2INFO ("Sending message: " << input);
      if (B2Socket::send(input)) {
        if (input == "EOF") {
          return 2;
        } else {
          return 1;
        }
      } else
        return -1;
    } else {
      B2ERROR("Connection lost");
      return -1;
    }
  }
}

/// @brief EvtSender destructor
EvtSender::~EvtSender()
{
}

/// @brief EvtSender << operator
///        Sends string to socket
/// @param s String to send
const EvtSender& EvtSender::operator << (const std::string& s) const
{
  if (!B2Socket::send(s))
    throw B2SocketException("Could not write to socket.");

  return *this;
}

/// @brief EvtSender >> operator
///        Retrieves string from socket
/// @param s Buffer to take string from socket
const EvtSender& EvtSender::operator >> (std::string& s) const
{
  if (!B2Socket::recv(s))
    throw B2SocketException("Could not read from socket.");

  return *this;
}
