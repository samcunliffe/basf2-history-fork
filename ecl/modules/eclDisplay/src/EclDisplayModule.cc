/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//This module
#include <ecl/modules/eclDisplay/EclDisplayModule.h>

//Root
#include <TApplication.h>
#include <TSystem.h>
#include <TFile.h>

//Framework
#include <framework/utilities/FileSystem.h>

//ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/modules/eclDisplay/EclFrame.h>
#include <ecl/modules/eclDisplay/EclData.h>
#include <ecl/modules/eclDisplay/geometry.h>

using namespace Belle2;
using namespace ECLDisplayUtility;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EclDisplay)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EclDisplayModule::EclDisplayModule() : Module(),
  m_frame_closed(false),
  m_frame(0),
  m_app(0),
  m_data(0)
{
  // Set module properties
  setDescription("Event display module for ECL.");

  // Parameter definitions
  addParam("keepOpen", m_keepOpen,
           "Keep window open after all events have been processed", false);
  addParam("displayEnergy", m_displayEnergy,
           "If true, energy distribution per channel (shaper, crate) is displayed. Otherwise, number of counts is displayed", false);
  addParam("displayMode", m_displayMode,
           "Default display mode. Can be later changed in GUI.", 9);
  addParam("autoDisplay", m_autoDisplay,
           "If true, events are displayed as soon as they are loaded.", true);
  addParam("InitFileName", m_eclMapperInitFileName,
           "Initialization file for eclMapper", std::string("ecl/data/ecl_channels_map.txt"));

  m_evtNum = 0;
}

EclDisplayModule::~EclDisplayModule()
{
}

void EclDisplayModule::initialize()
{
  m_eclarray.isRequired();

  initFrame();
}

void EclDisplayModule::initFrame()
{
  //== Init temporary file so TTree is not kept in memory.
  m_tempname = "ecldisplay_tmp";

  if (gSystem->TempFileName(m_tempname) == 0) {
    throw std::runtime_error("ECLDisplay: failed to create temp file.");
  } else {
    m_tempfile = new TFile(m_tempname, "recreate");
    gSystem->Unlink(m_tempname);
  }

  SetMode(m_displayEnergy);
  m_app   = new TApplication("ECLDisplay App", 0, 0);
  m_data  = new EclData();
  m_frame = new EclFrame(m_displayMode, m_data, m_autoDisplay, &m_mapper);

  m_frame->Connect("CloseWindow()", "Belle2::EclDisplayModule", this, "handleClosedFrame()");

  B2DEBUG(100, "EclDisplayModule::create ECLFrame");
}

void EclDisplayModule::handleClosedFrame()
{
  m_frame_closed = true;
}

void EclDisplayModule::beginRun()
{
  // Initialize channel mapper at run start to account for possible
  // changes in ECL mapping between runs.
  if (!m_mapper.initFromDB()) {
    B2FATAL("ECL Display:: Can't initialize eclChannelMapper");
  }
}

void EclDisplayModule::event()
{
  // EclFrame is closed, skipping data reading.
  if (m_frame_closed) return;

  int added_entries = 0;

  for (int i = 0; i < m_eclarray.getEntries(); i++) {
    ECLCalDigit* record = m_eclarray[i];
    if (record->getEnergy() >= 1e-4) { //TODO: Move to constant ENERGY_THRESHOLD.
      if (m_data->addEvent(record, m_evtNum) == 0) {
        added_entries++;
      }
    }
  }

  if (m_autoDisplay) {
    m_data->update(true);
    gSystem->ProcessEvents();
    if (!m_frame_closed)
      m_frame->loadNewData();
  }
  if (added_entries > 0)
    m_evtNum++;
}

void EclDisplayModule::endRun()
{
}

void EclDisplayModule::terminate()
{
  if (m_keepOpen) {
    if (!m_frame_closed) {
      m_data->update(false);
      m_frame->loadNewData();
    }

    while (!m_frame_closed) {
      gSystem->ProcessEvents();
      gSystem->Sleep(0);
    }
  }

  delete m_frame;
  delete m_data;
}

