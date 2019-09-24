/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/zmq/utils/StreamHelper.h>
#include <framework/core/Environment.h>
#include <framework/core/RandomNumbers.h>
#include <framework/logging/Logger.h>

#include <TSystem.h>

using namespace Belle2;

void StreamHelper::initialize(int compressionLevel, bool handleMergeable)
{
  gSystem->Load("libdataobjects");
  m_streamer = std::make_unique<DataStoreStreamer>(compressionLevel, handleMergeable);

  if ((Environment::Instance().getStreamingObjects()).size() > 0) {
    m_streamer->setStreamingObjects(Environment::Instance().getStreamingObjects());
    B2INFO("Tx: Streaming objects limited : " << (Environment::Instance().getStreamingObjects()).size() << " objects");
  }
}

std::unique_ptr<EvtMessage> StreamHelper::stream(bool addPersistentDurability, bool streamTransientObjects)
{
  if (m_randomGenerator.isOptional()) {
    if (not m_randomGenerator.isValid()) {
      m_randomGenerator.construct(RandomNumbers::getEventRandomGenerator());
    } else {
      *m_randomGenerator = RandomNumbers::getEventRandomGenerator();
    }
  }
  return std::unique_ptr<EvtMessage>(m_streamer->streamDataStore(addPersistentDurability, streamTransientObjects));
}

void StreamHelper::read(std::unique_ptr<ZMQNoIdMessage> message)
{
  EvtMessage eventMessage(message->getMessagePartAsCharArray<ZMQNoIdMessage::c_data>());
  m_streamer->restoreDataStore(&eventMessage);

  if (m_randomGenerator.isValid()) {
    RandomNumbers::getEventRandomGenerator() = *m_randomGenerator;
  }
}
