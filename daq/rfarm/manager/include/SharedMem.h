#ifndef SHAREDMEM_H
#define SHAREDMEM_H
//+
// File : sharedmem.h
// Description : shared memory library for communication among processes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 11 - Dec - 2002
//-

#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

namespace Belle2 {
  class SharedMem {
  public:
    SharedMem(char* name, int size);
    SharedMem(int shm_id);
    ~SharedMem(void);

    void* ptr(void);
    int shmid(void);

    bool IsCreated(void);

  private:
    bool m_new; /**< True if we created the ring buffer ourselves (and need to clean it). */
    bool m_file;
    std::string m_pathname;
    int  m_pathfd; /** Associated file descriptor. */
    key_t m_shmkey; /**< SHM key, see shmget(2). */

    int m_shmid;
    void* m_shmadr;
    int m_shmsize;
  };
}

#endif

