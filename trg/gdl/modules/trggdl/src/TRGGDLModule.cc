//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDLModule.cc
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for GDL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#define TRGGDL_SHORT_NAMES

#include <stdlib.h>
#include <iostream>
#include "framework/core/ModuleManager.h"
#include "trg/trg/Debug.h"
#include "trg/gdl/modules/trggdl/TRGGDLModule.h"

using namespace std;

namespace Belle2 {

REG_MODULE(TRGGDL);

string
TRGGDLModule::version() const {
    return string("TRGGDLModule 0.00");
}

TRGGDLModule::TRGGDLModule()
    : Module::Module(),
      _debugLevel(0),
      _configFilename("TRGGDLConfig.dat"),
      _simulationMode(1),
      _fastSimulationMode(0),
      _firmwareSimulationMode(0) {

    string desc = "TRGGDLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    addParam("DebugLevel", _debugLevel, "TRGGDL debug level", _debugLevel);
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             _configFilename);
    addParam("SimulationMode",
	     _simulationMode,
	     "TRGGDL simulation switch",
	     _simulationMode);
    addParam("FastSimulationMode",
	     _fastSimulationMode,
	     "TRGGDL fast simulation mode",
	     _fastSimulationMode);
    addParam("FirmwareSimulationMode",
	     _firmwareSimulationMode,
	     "TRGGDL firmware simulation mode",
	     _firmwareSimulationMode);

    if (TRGDebug::level())
	cout << "TRGGDLModule ... created" << endl;
}

TRGGDLModule::~TRGGDLModule() {

    if (_gdl)
        TRGGDL::getTRGGDL("good-bye");

    if (TRGDebug::level())
        cout << "TRGGDLModule ... destructed " << endl;
}

void
TRGGDLModule::initialize() {

    TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
	cout << "TRGGDLModule::initialize ... options" << endl;
	cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
	     << endl;
    }
}

void
TRGGDLModule::beginRun() {

    //...CDC trigger config. name...
    static string cfn = _configFilename;

    //...CDC trigger...
    if ((cfn != _configFilename) || (_gdl == 0))
	_gdl = TRGGDL::getTRGGDL(_configFilename,
				 _simulationMode,
				 _fastSimulationMode,
				 _firmwareSimulationMode);

    if (TRGDebug::level())
	cout << "TRGGDLModule ... beginRun called " << endl;
}

void
TRGGDLModule::event() {

    if (TRGDebug::level()) {
//      _gdl->dump("geometry superLayers layers wires detail");
//      _gdl->dump("geometry superLayers layers detail");
    }

    //...CDC trigger simulation...
    _gdl->update(true);
    _gdl->simulate();
}

void
TRGGDLModule::endRun() {
    if (TRGDebug::level())
        cout << "TRGGDLModule ... endRun called " << endl;
}

void
TRGGDLModule::terminate() {

    _gdl->terminate();

    if (TRGDebug::level())
	cout << "TRGGDLModule ... terminate called " << endl;
}

} // namespace Belle2
