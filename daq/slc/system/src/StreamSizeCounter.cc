#include "daq/slc/system/StreamSizeCounter.h"

using namespace Belle2;

size_t StreamSizeCounter::write(const void*, size_t count)
{
  m_count += count;
  return count;
}
