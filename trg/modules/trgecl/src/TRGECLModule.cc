//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLModule.cc
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A trigger module for ECL
//---------------------------------------------------------------
// 1.00 : 2010/11/18 : First version
//---------------------------------------------------------------

#define TRGECL_SHORT_NAMES

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/ModuleManager.h>
#include <framework/logging/Logger.h>

#include <G4RunManager.hh>

//trg package headers
#include "trg/trg/Debug.h"
#include "trg/modules/trgecl/TRGECLModule.h"
//#include "trg/ecl/TRGECLHit.h"
//#include "trg/ecl/TRGECLTCHit.h"
//#include <ecl/hitecl/ECLSimHit.h>
//#include <ecl/geoecl/ECLGeometryPar.h>

#include <stdlib.h>
#include <iostream>

using namespace std;

namespace Belle2 {
//
//
//
REG_MODULE(TRGECL);
//
//
//
string
TRGECLModule::version() const {
  return string("TRGECLModule 1.00");
}
//
//
//
TRGECLModule::TRGECLModule()
  : Module::Module(),
    _debugLevel(0),_ecl(0)
{

  string desc = "TRGECLModule(" + version() + ")";
  setDescription(desc);

  addParam("DebugLevel", _debugLevel, "TRGECL debug level", _debugLevel);

  if (TRGDebug::level()){
    std::cout << "TRGECLModule ... created" << std::endl;
  }
}
//
//
//
TRGECLModule::~TRGECLModule() {

  if (TRGDebug::level()){
    std::cout << "TRGECLModule ... destructed " << std::endl;
  }
}
//
//
//
void
TRGECLModule::initialize() {

  TRGDebug::level(_debugLevel);

  if (TRGDebug::level()) {
    std::cout << "TRGECLModule::initialize ... options" << std::endl;
    std::cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
	      << std::endl;
  }
  //
  //
  //
  m_nRun   = 0 ;
  m_nEvent = 0 ;
  m_hitNum = 0;
  m_hitTCNum = 0;
}
//
//
//
void
TRGECLModule::beginRun() {

  if (TRGDebug::level()){
    std::cout << "TRGECLModule ... beginRun called " << std::endl;
  }
  _ecl = TRGECL::getTRGECL();

}
//
//
//
void
TRGECLModule::event() {

  if (TRGDebug::level()) {
    std::cout << "TRGECLMoudle ... event called" << std::endl;
  }
  //
  // simulation
  //
  _ecl->initialize(m_nEvent);
  _ecl->simulate(m_nEvent);
  printf("TRGECLModule> bitECLtoGDL = %i \n", _ecl->getECLtoGDL());
  //
  //
  //

  m_nEvent++;
  //
  //
  //
}
//
//
//
void
TRGECLModule::endRun() {
  if (TRGDebug::level()){
    std::cout << "TRGECLModule ... endRun called " << std::endl;
  }
}
//
//
//
void
TRGECLModule::terminate() {
  if (TRGDebug::level()){
    std::cout << "TRGECLModule ... terminate called " << std::endl;
  }
}
//
//
//
} // namespace Belle2
