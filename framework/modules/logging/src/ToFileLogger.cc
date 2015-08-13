/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 **************************************************************************/

#include <framework/modules/logging/ToFileLogger.h>
#include <stdio.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ToFileLogger)

ToFileLoggerModule::ToFileLoggerModule():
  m_fileDeviceOfStdout(0),
  m_fileDeviceOfStderr(0)
{
  setDescription("For better handling non BASF2-Logging lines, you can insert this module."
                 "It redirect all output to the console into the filename given as parameter."
                 "With the usage of the basf2-own logging framework you can even write the basf2-related logging"
                 "into another file as the non-basf2-related if you like.");

  addParam("fileName", m_param_fileName, "Filename to log into.");
}

void ToFileLoggerModule::initialize()
{
  fflush(stdout);
  fflush(stderr);

  m_fileDeviceOfStdout = dup(fileno(stdout));
  m_fileDeviceOfStderr = dup(fileno(stderr));

  if (m_fileDeviceOfStdout == -1) {
    B2FATAL("Something went wrong during dup of stdout! Aborting.");
  }

  if (m_fileDeviceOfStderr == -1) {
    B2FATAL("Something went wrong during dup of stderr! Aborting.");
  }

  if (freopen(m_param_fileName.c_str(), "a", stdout) == NULL) {
    B2FATAL("freopen() failed. Can not log stdout into the given file. Aborting.");
  }

  if (freopen(m_param_fileName.c_str(), "a", stderr) == NULL) {
    B2FATAL("freopen() failed. Can not log stderr into the given file. Aborting.");
  }
}

void ToFileLoggerModule::terminate()
{
  fflush(stdout);
  fflush(stderr);

  dup2(m_fileDeviceOfStdout, fileno(stdout));
  dup2(m_fileDeviceOfStderr, fileno(stderr));

  close(m_fileDeviceOfStdout);
  close(m_fileDeviceOfStderr);

  clearerr(stdout);
  clearerr(stderr);
}
