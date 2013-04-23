/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Path.h>
#include <display/async/AsyncWrapper.h>

#include <framework/core/InputController.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/PathManager.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>

#include <cstdlib>
#include <iostream>


using namespace Belle2;

AsyncWrapper::AsyncWrapper(Module* wrapMe): Module(), m_wrappedModule(wrapMe), m_procHandler(0), m_ringBuffer(0), m_rx(0), m_tx(0)
{
  setParamList(wrapMe->getParamList()); //inherit parameters from wrapped module
}

AsyncWrapper::~AsyncWrapper()
{
  if (m_procHandler) {
    if (!m_procHandler->isEvtProc()) {
      //can't use basf2 logging in destructor
      std::cout << "\nWaiting for asynchronous process...\n";
      EvtMessage term(NULL, 0, MSG_TERMINATE);
      while (m_ringBuffer->insq((int*)term.buffer(), (term.size() - 1) / sizeof(int) + 1) < 0) {
        usleep(200);
      }
      m_procHandler->wait_event_processes();
      std::cout << "Done, cleaning up...\n";
      delete m_ringBuffer;
    }
  }
  delete m_wrappedModule;
}

void AsyncWrapper::initialize()
{
  B2INFO("Initializing AsyncWrapper...");

  m_procHandler = new ProcHandler();
  m_ringBuffer = new RingBuffer(10000000); //TODO: replace with some constant
  m_rx = new RxModule(m_ringBuffer);
  m_tx = new TxModule(m_ringBuffer);
  m_tx->setBlockingInsert(false); //actually decouple this process

  //fork out one extra process
  m_procHandler->init_EvtProc(1);
  if (m_procHandler->isEvtProc()) {
    //forked thread:
    PathManager pathMgr;
    PathPtr path = pathMgr.createPath();
    path->addModule(ModulePtr(m_rx));
    path->addModule(ModulePtr(m_wrappedModule));

    EventProcessor eventProc(pathMgr);
    eventProc.process(path);

    /*
    InputController::setNumEntries(m_ringBuffer->numq());
    InputController::setCanControlInput(true); //TODO this is a lie
    */
    B2INFO("Asynchronous process done!");
    exit(0);
  } else {
    //main thread: chain tx and return
    m_tx->initialize();
  }
}

void AsyncWrapper::event()
{
  if (!m_procHandler->isEvtProc()) {
    m_tx->event();
  }
}

void AsyncWrapper::terminate()
{
  if (!m_procHandler->isEvtProc()) {
    m_tx->terminate();
  }
}
