#include "daq/slc/apps/storagerd/StoragerCallback.h"
#include "daq/slc/apps/storagerd/storage_info_all.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

#include <sys/statvfs.h>

using namespace Belle2;

StoragerCallback::StoragerCallback(const NSMNode& node)
  : RCCallback(node)
{
  setTimeout(1);
}

StoragerCallback::~StoragerCallback() throw()
{
}

void StoragerCallback::init() throw()
{
  LogFile::open("storage");
  m_data = NSMData("STORAGE_STATUS", "storage_info_all",
                   storage_info_all_revision);
  m_data.allocate(getCommunicator());
}

void StoragerCallback::term() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
    m_con[i].getInfo().unlink();
  }
}

bool StoragerCallback::load() throw()
{
  system("killall storagein");
  system("killall storagerecord");
  system("killall storageout");
  system("killall basf2");

  ConfigObject& obj(getConfig().getObject());
  const size_t nproc = obj.getInt("record_nproc");
  m_con = std::vector<ProcessController>();
  for (size_t i = 0; i < 3 + nproc; i++) {
    m_con.push_back(ProcessController(this));
  }
  m_con[0].init("storagein", 1);
  m_con[1].init("storagerecord", 2);
  m_con[2].init("storageout", 3);
  for (size_t i = 3; i < m_con.size(); i++) {
    m_con[i].init(StringUtil::form("basf2_%d", i - 3), i);
  }

  const std::string ibuf_name = obj.getText("ibuf_name");
  const std::string rbuf_name = obj.getText("rbuf_name");
  const std::string obuf_name = obj.getText("obuf_name");
  const std::string ibuf_size = obj.getValueText("ibuf_size");
  const std::string rbuf_size = obj.getValueText("rbuf_size");
  const std::string obuf_size = obj.getValueText("obuf_size");

  m_con[0].clearArguments();
  m_con[0].setExecutable("storagein");
  m_con[0].addArgument(ibuf_name);
  m_con[0].addArgument(ibuf_size);
  m_con[0].addArgument(obj.getText("in_host"));
  m_con[0].addArgument(obj.getValueText("in_port"));
  m_con[0].addArgument("storagein");
  m_con[0].addArgument("1");
  if (!m_con[0].load(20)) {
    std::string emsg = "storagein: Failed to connect to eb2rx";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagein");

  m_con[1].clearArguments();
  m_con[1].setExecutable("storagerecord");
  m_con[1].addArgument(rbuf_name);
  m_con[1].addArgument(rbuf_size);
  m_con[1].addArgument(obj.getText("record_dir"));
  m_con[1].addArgument(obj.getValueText("record_ndisks"));
  m_con[1].addArgument(obj.getText("record_file_diskid"));
  m_con[1].addArgument(obj.getText("record_file_nfiles"));
  m_con[1].addArgument(obuf_name);
  m_con[1].addArgument(obuf_size);
  m_con[1].addArgument("storagerecord");
  m_con[1].addArgument("2");
  if (!m_con[1].load(10)) {
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagerecord");

  /*
  m_con[2].clearArguments();
  m_con[2].setExecutable("storageout");
  m_con[2].addArgument(obuf_name);
  m_con[2].addArgument(obuf_size);
  m_con[2].addArgument(obj.getValueText("out_port"));
  m_con[2].addArgument("storageout");
  m_con[2].addArgument("3");
  if (!m_con[2].load(10)) {
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    LogFile::warning(emsg);
  }
  LogFile::debug("Booted storageout");
  */

  for (size_t i = 3; i < m_con.size(); i++) {
    m_con[i].clearArguments();
    m_con[i].setExecutable("basf2");
    m_con[i].addArgument(obj.getText("record_script"));
    m_con[i].addArgument(ibuf_name);
    m_con[i].addArgument(ibuf_size);
    m_con[i].addArgument(rbuf_name);
    m_con[i].addArgument(rbuf_size);
    m_con[i].addArgument(StringUtil::form("basf2_%d", i - 3));
    m_con[i].addArgument(StringUtil::form("%d", i + 1));
    m_con[i].addArgument("1");
    if (!m_con[i].load(10)) {
      std::string emsg = StringUtil::form("Failed to start %d-th basf2", i - 3);
      setReply(emsg);
      LogFile::error(emsg);
      return false;
    }
    LogFile::debug("Booted %d-th basf2", i - 3);
  }

  m_flow = std::vector<FlowMonitor>();
  for (size_t i = 0; i < m_con.size(); i++) {
    FlowMonitor flow;
    flow.open(&(m_con[i].getInfo()));
    m_flow.push_back(flow);
  }
  m_ibuf.open(ibuf_name, atoi(ibuf_size.c_str()) * 1000000);
  m_rbuf.open(rbuf_name, atoi(rbuf_size.c_str()) * 1000000);

  return true;
}

bool StoragerCallback::start() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    std::string name = m_con[i].getName();
    if (!m_con[i].start()) {
      if (i != 2) {
        std::string emsg = name[i] + " is not started";
        setReply(emsg);
        LogFile::error(emsg);
        return false;
      } else {
        std::string emsg = "storageout: Not accepted connection from EXPRECO yet";
        LogFile::warning(emsg);
      }
    }
    LogFile::debug(name[i] + " started");
  }
  return true;
}

bool StoragerCallback::stop() throw()
{
  return true;
}

bool StoragerCallback::resume() throw()
{
  return true;
}

bool StoragerCallback::pause() throw()
{
  return true;
}

bool StoragerCallback::recover() throw()
{
  abort();
  sleep(3);
  return load();
}

bool StoragerCallback::abort() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
  }
  return true;
}

void StoragerCallback::timeout() throw()
{
  storage_info_all* info = (storage_info_all*)m_data.get();
  for (size_t i = 0; i < m_flow.size() && i < 14; i++) {
    ronode_status& status(m_flow[i].monitor());
    for (int j = 0; j < 2; j++) {
      info->io[2 * i + j].state = status.io[j].state;
      info->io[2 * i + j].count = status.io[j].count;
      info->io[2 * i + j].freq = status.io[j].freq;
      info->io[2 * i + j].evtsize = status.io[j].evtsize;
      info->io[2 * i + j].rate = status.io[j].rate;
    }
    if (i == 0) { //IN
      info->ctime = status.ctime;
      info->expno = status.expno;
      info->runno = status.runno;
      info->subno = status.subno;
      info->io[0].nqueue = status.io[0].nqueue;
      SharedEventBuffer::Header* hd = m_ibuf.getHeader();
      info->io[1].nqueue = hd->nword_in - hd->nword_out;
    } else if (i == 1) {
      SharedEventBuffer::Header* hd = m_rbuf.getHeader();
      info->io[2].nqueue = hd->nword_in - hd->nword_out;
      info->nfiles = status.reserved[0];
      info->nbytes = (float)status.reserved[1];
    } else {
    }
  }
  struct statvfs statfs;
  ConfigObject& obj(getConfig().getObject());
  if (obj.hasField("record_ndisks")) {
    std::string dir = obj.getText("record_dir");
    int ndisks = obj.getInt("record_ndisks");
    for (int i = 0; i < ndisks; i++) {
      std::string path = StringUtil::form("%s%02d", dir.c_str(), i + 1);
      statvfs(path.c_str(), &statfs);
      info->disksize[i] = (float)statfs.f_frsize * statfs.f_blocks / 1024 / 1024 / 1024;
      info->diskusage[i] = 100 - ((float)statfs.f_bfree / statfs.f_blocks * 100);
    }
  }
}
