//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Debug.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Debug utility functions
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "trg/trg/Debug.h"

using namespace std;

namespace Belle2 {

vector<string> TRGDebug::_stages;
int TRGDebug::_level = 0;

void
TRGDebug::enterStage(const string & name) {
    if (_level)
        cout << tab() << "--> ";
    _stages.push_back(name);
    if (_level)
        cout << name << endl;
}

void
TRGDebug::leaveStage(const string & name) {
    if (name != _stages.back()) {
        cout << "TRGDebug !!! given stage name(" << name << ") doesn't match"
             << " to expected stage name(" << _stages.back() << endl;
        return;
    }
    _stages.pop_back();
    if (_level)
        cout << tab() << "<-- " << name << endl;
}

string
TRGDebug::tab(void) {
    string t;
    const unsigned n = _stages.size();
    for (unsigned i = 0; i < n; i++)
        t += "    ";
    return t;
}

string
TRGDebug::tab(int extra) {
    string t = tab();
    if (extra > 0)
        for (unsigned i = 0; i < unsigned(extra); i++)
            t += " ";
    return t;
}

int
TRGDebug::level(void) {
    return _level;
}

int
TRGDebug::level(int a) {
    return _level = a;
}

} // namespace Belle2

