//+
// File : sock2rbr.cc
// Description : Get an event from RingBuffer and send it to socket
//               Reverse connection
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Jul - 2013
//-
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "daq/rfarm/event/RevSock2Rb.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], and port number[2]
  if (argc < 2) {
    printf("syntax : sock2rb rbufname src port\n");
    exit(-1);
  }

  string a1(argv[1]);
  string a2(argv[2]);
  int a3 = atoi(argv[3]);

  RevSock2Rb sr(a1, a2, a3);

  for (;;) {
    int stat = sr.ReceiveEvent();
    //    printf ( "sock2rb received : %d\n", stat );
    if (stat <= 0) break;
  }
  exit(0);
}



