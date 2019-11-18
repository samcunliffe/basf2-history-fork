/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQClasses.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQWorker.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQWorker worker;
  worker.initFromConsole("b2hlt_worker - mimick a worker process by sending a ready message back on every input (load-balanced connection) "
                         "and forwarding the message to the output, where it expects a confirmation (confirmed connection). "
                         "Behaves as a normal basf2-worker and can be used during testing."
                         , argc, argv);
  worker.main();

  B2RESULT("Program terminated.");
}