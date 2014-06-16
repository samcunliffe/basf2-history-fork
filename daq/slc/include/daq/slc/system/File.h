#ifndef _Belle2_File_hh
#define _Belle2_File_hh

#include "daq/slc/system/FileDescriptor.h"

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class File : public FileDescriptor {

  public:
    File() throw() {}
    File(int fd) throw() : FileDescriptor(fd) {}
    virtual ~File() throw() {}

  public:
    void open(const std::string& path,
              const std::string& mode = "r") throw(IOException);
    void unlink(const std::string& path) throw(IOException);
    virtual size_t write(const void* v, size_t count) throw(IOException);
    virtual size_t read(void* v, size_t count) throw(IOException);

  };

}

#endif
