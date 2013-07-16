//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDCFrontEndImplementation.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A program to generate VHDL code for the CDC trigger front-end.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "trg/trg/Utilities.h"
#include "trg/cdc/FrontEnd.h"

using namespace std;
using namespace Belle2;

#define DEBUG_LEVEL   0
#define NAME          "TRGCDCFrontEndImplementation"
#define VERSION       "version 0.00"
#define NOT_CONNECTED 99999

int
main(int argc, char * argv[]) {

    cout << NAME << " ... " << VERSION << endl;
    const string tab = "    ";

    //...Check arguments...
    if (argc < 2) {
        cout << NAME << " !!! one argument necessary" << endl
             << tab << " 1 : Version for new config file" << endl;
        return -1;
    }

    //...Date...
    string ts0 = TRGUtil::dateStringF();
    string ts1 = TRGUtil::dateString();

    //...2nd argument...
    const string version = argv[1];

    //...1st argument...
    const string outname = "TRGCDCFrontEndImplementation_" + version + "_"
        + ts0 + ".vhdl";

    //...Open configuration data...
    ofstream outfile(outname.c_str(), ios::out);
    if (outfile.fail()) {
        cout << NAME << " !!! can not open file" << endl
             << "    " << outname << endl;
        return -2;
    }

    //...Inner...
    outfile << "-- Generated by " << NAME << " " << VERSION << endl;
    outfile << "-- " << TRGCDCFrontEnd::version() << endl;
    outfile << "-- " << ts1 << endl;
    outfile << "--" << endl;
    TRGCDCFrontEnd::implementation(TRGCDCFrontEnd::innerInside, outfile);
    outfile.close();

    //...Termination...
    cout << NAME << " ... terminated" << endl;
}
