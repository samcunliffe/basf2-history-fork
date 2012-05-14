//+
// File : file2rb.cc
// Description : Get an event from a SeqRoot file and place it in Rbuf
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Apr - 2012
//-
#include <string>
#include <vector>

#include "framework/pcore/SeqFile.h"
#include "framework/pcore/RingBuffer.h"

#define RBUFSIZE 100000000
#define MAXEVTSIZE 40000000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("file2rb : rbufname filename neof\n");
    exit(-1);
  }

  SeqFile* file = new SeqFile(argv[2], "r");
  if (file->status() <= 0) {
    perror("file open");
    exit(-1);
  }
  RingBuffer* rbuf = new RingBuffer(argv[1], RBUFSIZE);
  char* evbuf = new char[MAXEVTSIZE];

  for (;;) {
    int is = file->read(evbuf, MAXEVTSIZE);
    if (is <= 0) break;

    // Put the message in ring buffer
    int irb = 0;
    for (;;) {
      irb = rbuf->insq((int*)evbuf, (is - 1) / 4 + 1);
      if (irb >= 0) break;
      usleep(200);
    }
  }
}



