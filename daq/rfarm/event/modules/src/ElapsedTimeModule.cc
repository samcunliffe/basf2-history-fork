//+
// File : ElapsedTimeModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rfarm/event/modules/ElapsedTimeModule.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ElapsedTime)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ElapsedTimeModule::ElapsedTimeModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_InitializeInProcess);

  addParam("EventInterval", m_interval, "Event interval to measure time",
           100);

  //Parameter definition
  B2INFO("ElapsedTime: Constructor done.");
}


ElapsedTimeModule::~ElapsedTimeModule()
{
}

void ElapsedTimeModule::initialize()
{
}


void ElapsedTimeModule::beginRun()
{
  B2INFO("ElapsedTime: started to measure elapsed time.");
  gettimeofday(&m_t0, 0); \
  m_tnow = m_t0;
  m_tprev = m_tnow;
  m_nevent = 0;
  m_nevprev = 0;
}


void ElapsedTimeModule::event()
{
  if (m_nevent % m_interval == 0) {
    gettimeofday(&m_tnow, 0);
    double etime = (double)((m_tnow.tv_sec - m_t0.tv_sec) * 1000000 +
                            (m_tnow.tv_usec - m_t0.tv_usec));
    double delta = (double)((m_tnow.tv_sec - m_tprev.tv_sec) * 1000000 +
                            (m_tnow.tv_usec - m_tprev.tv_usec));
    double evtime = etime / ((double)m_nevent);
    //    double devtime = delta/(double)(m_nevent-m_nevprev);
    double devtime = delta / (double)m_interval;
    printf("Elapsed( %d ) : time = %7.2f (msec), time/evt = %5.2f [ %5.3f ](msec)\n",
           m_nevent, etime / 1000.0, evtime / 1000.0, devtime / 1000.0);
    m_nevprev = m_nevent;
    m_tprev = m_tnow;
  }
  m_nevent++;

}

void ElapsedTimeModule::endRun()
{
  //fill Run data
  gettimeofday(&m_tend, 0);
  double etime = (double)((m_tnow.tv_sec - m_t0.tv_sec) * 1000000 +
                          (m_tnow.tv_usec - m_t0.tv_usec));
  double evtime = etime / ((double)m_nevent);
  printf("Total Elapsed : time = %f (msec), time/evt = %f (msec)\n",
         etime / 1000.0, evtime / 1000.0);
}


void ElapsedTimeModule::terminate()
{
  B2INFO("ElapsedTime: terminate called")
}

