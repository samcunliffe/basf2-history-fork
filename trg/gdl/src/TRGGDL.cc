//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDL.cc
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent GDL.
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGGDL_SHORT_NAMES

#include <fstream>
#include "trg/trg/Debug.h"
#include "trg/trg/Time.h"
#include "trg/trg/State.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/gdl/TRGGDL.h"

using namespace std;

namespace Belle2 {

string
TRGGDL::name(void) const {
    return "TRGGDL";
}

string
TRGGDL::version(void) const {
    return string("TRGGDL 0.00");
}

TRGGDL *
TRGGDL::_gdl = 0;

TRGGDL*
TRGGDL::getTRGGDL(const string & configFile,
                  unsigned simulationMode,
                  unsigned fastSimulationMode,
                  unsigned firmwareSimulationMode) {
    if (_gdl) {
        //delete _gdl;
        _gdl = 0;
    }

    if (configFile != "good-bye") {
        _gdl = new TRGGDL(configFile,
                          simulationMode,
                          fastSimulationMode,
                          firmwareSimulationMode);
    }
    else {
        cout << "TRGGDL::getTRGGDL ... good-bye" << endl;
        //        delete _gdl;
        _gdl = 0;
    }

    return _gdl;
}

TRGGDL*
TRGGDL::getTRGGDL(void) {
    if (! _gdl)
        cout << "TRGGDL::getTRGGDL !!! TRGGDL is not created yet" << endl;
    return _gdl;
}

TRGGDL::TRGGDL(const string & configFile,
               unsigned simulationMode,
               unsigned fastSimulationMode,
               unsigned firmwareSimulationMode)
    : _debugLevel(0),
      _configFilename(configFile),
      _simulationMode(simulationMode),
      _fastSimulationMode(fastSimulationMode),
      _firmwareSimulationMode(firmwareSimulationMode),
      _clock(Belle2_GDL::GDLSystemClock),
      _offset(15.3) {

    if (TRGDebug::level()) {
        cout << "TRGGDL ... TRGGDL initializing with " << _configFilename
             << endl
             << "           mode=0x" << hex << _simulationMode << dec << endl;
    }

    initialize();

    if (TRGDebug::level()) {
        cout << "TRGGDL ... TRGGDL created with " << _configFilename << endl;
        _clock.dump();
    }
}

void
TRGGDL::initialize(void) {
    configure();
}

void
TRGGDL::terminate(void) {
}

void
TRGGDL::dump(const string & msg) const {
    if (msg.find("name")    != string::npos ||
        msg.find("version") != string::npos ||
        msg.find("detail")  != string::npos ||
        msg == "") {
    }
    if (msg.find("detail") != string::npos ||
        msg.find("state") != string::npos) {
        cout << "Debug Level=" << _debugLevel;
        cout << endl;
    }
}

void
TRGGDL::clear(void) {
}

void
TRGGDL::fastClear(void) {
}

void
TRGGDL::update(bool) {
}

TRGGDL::~TRGGDL() {
    clear();
}

void
TRGGDL::simulate(void) {
    const bool fast = (_simulationMode & 1);
    const bool firm = (_simulationMode & 2);
    if (fast)
        fastSimulation();
    if (firm)
        firmwareSimulation();
}

void
TRGGDL::fastSimulation(void) {
}

void
TRGGDL::firmwareSimulation(void) {
}

void
TRGGDL::configure(void) {

    //...Open input data definition...
    const string fni = _configFilename + ".inp";
    ifstream infile(fni.c_str(), ios::in);
    if (infile.fail()) {
        cout << "TRGGDL !!! can not open file : " << fni << endl;
        return;
    }
    getInput(infile);
    infile.close();

    //...Open output data definition...
    const string fno = _configFilename + ".oup";
    ifstream outfile(fno.c_str(), ios::in);
    if (outfile.fail()) {
        cout << "TRGGDL !!! can not open file : " << fno << endl;
        return;
    }
    getOutput(outfile);
    outfile.close();

    //...Open algorithm data definition...
    const string fna = _configFilename + ".alg";
    ifstream algfile(fna.c_str(), ios::in);
    if (algfile.fail()) {
        cout << "TRGGDL !!! can not open file : " << fna << endl;
        return;
    }
    getAlgorithm(algfile);
    algfile.close();

    //...Summary...
    if (TRGDebug::level()) {
        cout << "TRGGDL Input Bits" << endl;
        for (unsigned i = 0; i < _input.size(); i++)
            cout << TRGDebug::tab(4) << i << " : " << _input[i] << endl;
        cout << "TRGGDL Output Bits" << endl;
        for (unsigned i = 0; i < _output.size(); i++)
            cout << TRGDebug::tab(4) << i << " : " << _output[i]
                 << " : " << _algorithm[i] << endl;
    }
}

void
TRGGDL::getInput(ifstream & ifs) {

    if (TRGDebug::level()) {
         cout << "TRGGDL::getInput ... reading input data" << endl;
    }

    char b[800];
    unsigned lines = 0;
    while (! ifs.eof()) {
        ifs.getline(b, 800);

        //...The first word should be input bit number...
        const string w0 = TRGUtilities::carstring(b);

        //...Bit name...
        string cdr = TRGUtilities::cdrstring(b);
        const string w1 = TRGUtilities::carstring(cdr);

        if (w1.size())
            _input.push_back(w1);

        
        // if (TRGDebug::level()) {
        //     cout << w0 << "," << w1 << endl;
        // }

        ++lines;
    }

}

void
TRGGDL::getOutput(ifstream & ifs) {

    if (TRGDebug::level()) {
         cout << "TRGGDL::getOutput ... reading output data" << endl;
    }

    char b[800];
    unsigned lines = 0;
    while (! ifs.eof()) {
        ifs.getline(b, 800);

        //...The first word should be input bit number...
        const string w0 = TRGUtilities::carstring(b);

        //...Bit name...
        string cdr = TRGUtilities::cdrstring(b);
        const string w1 = TRGUtilities::carstring(cdr);

        if (w1.size())
            _output.push_back(w1);

        // if (TRGDebug::level()) {
        //     cout << w0 << "," << w1 << endl;
        // }

        ++lines;
    }

}

void
TRGGDL::getAlgorithm(ifstream & ifs) {

    if (TRGDebug::level()) {
         cout << "TRGGDL::getAlgorithm ... reading algorithm data" << endl;
    }

    char b[800];
    unsigned lines = 0;
    while (! ifs.eof()) {
        ifs.getline(b, 800);

        //...The first word should be input bit number...
        const string w0 = TRGUtilities::carstring(b);

        //...':'...
        string cdr = TRGUtilities::cdrstring(b);
        const string w1 = TRGUtilities::carstring(cdr);

        //...Algorithm...
        cdr = TRGUtilities::cdrstring(cdr);
        const string w2 = cdr;

        if (w2.size())
            _algorithm.push_back(w2);

         if (TRGDebug::level()) {
             cout << w0 << "," << w1 << "," << w2 << endl;
         }

        ++lines;
    }

}

} // namespace Belle2
