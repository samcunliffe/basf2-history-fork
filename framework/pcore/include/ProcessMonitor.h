/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Baur, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/ProcHandler.h>

#include <zmq.hpp>
#include <memory>
#include <string>
#include <map>

namespace Belle2 {
  class ProcessMonitor {
  public:
    /// Start listening for process management information on the given address.
    void subscribe(ProcHandler& procHandler, const std::string& pubSocketAddress, const std::string& subSocketAddress,
                   const std::string& controlSocketAddress);
    /// Block until either the input process is running or the timeout (in seconds) is raised.
    void waitForRunningInput(const ProcHandler& procHandler, int timeout);
    /// Block until either the output process is running or the timeout (in seconds) is raised.
    void waitForRunningOutput(const ProcHandler& procHandler, int timeout);
    /// Block until either the worker process is running or the timeout (in seconds) is raised.
    void waitForRunningWorker(const ProcHandler& procHandler, int timeout);
    /// Ask all processes to terminate. If not, kill them after timeout seconds.
    void killProcesses(unsigned int timeout);

    void initialize(unsigned int requestedNumberOfWorkers);
    void terminate();

    /// check multicast for messages and kill workers if requested
    void checkMulticast(const ProcHandler& procHandler, int timeout = 0);
    /// check the child processes, if one has died
    void checkChildProcesses(const ProcHandler& procHandler);
    /// check if we have received any signal from the user or OS. Kill the processes if not SIGINT.
    void checkSignals(int g_signalReceived);

    /// If we have received a SIGINT signal or the last process is gone, we can end smoothly
    bool hasEnded();

    /// Compare our current list of workers of how many we want to have
    unsigned int needMoreWorkers();

  private:
    /// The context of the multicast
    std::unique_ptr<zmq::context_t> m_context;
    /// The publication socket of the multicast
    std::unique_ptr<zmq::socket_t> m_pubSocket;
    /// The subscribe socket of the multicast
    std::unique_ptr<zmq::socket_t> m_subSocket;
    /// The control socket of the multicast
    std::unique_ptr<zmq::socket_t> m_controlSocket;

    unsigned int m_requestedNumberOfWorkers = 0;

    std::map<ProcType, int> m_numberOfChildren;
    std::map<int, ProcType> m_processList;
    bool m_hasEnded = false;

    void checkForEndedProcesses(const std::vector<int>& currentProcessList);
    void checkForStartedProcesses(const std::vector<int>& currentProcessList, const ProcHandler& procHandler);
  };
} // namespace Belle2