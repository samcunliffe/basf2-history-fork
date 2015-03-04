#include "daq/slc/apps/storagerd/StoragerCallback.h"
#include "daq/slc/apps/storagerd/storage_status.h"

#include "daq/slc/database/DBObjectLoader.h"

#include "daq/slc/system/LogFile.h"
#include "daq/slc/system/Time.h"

#include "daq/slc/base/StringUtil.h"

#include <sys/statvfs.h>
#include <stdlib.h>

using namespace Belle2;

StoragerCallback::StoragerCallback()
{
  setTimeout(1);
  system("killall storagein");
  system("killall storagerecord");
  system("killall storageout");
  system("killall basf2");
}

StoragerCallback::~StoragerCallback() throw()
{
  term();
}

bool StoragerCallback::initialize(const DBObject& obj) throw()
{
  allocData(getNode().getName() + "_STATUS", "storage_status",
            storage_status_revision);
  return configure(obj);
}

bool StoragerCallback::configure(const DBObject& obj) throw()
{
  abort();
  term();
  try {
    setUseSet("input.buf.name", false);
    setUseSet("input.buf.size", false);
    setUseSet("output.buf.name", false);
    setUseSet("output.buf.size", false);
    setUseSet("record.buf.name", false);
    setUseSet("record.buf.size", false);
    setUseSet("record.dir", false);
    setUseSet("record.ndisks", false);
    setUseSet("record.file.diskid", false);
    setUseSet("record.file.nfiles", false);
    setUseSet("record.file.dbtmp", false);
    const DBObject& record(obj.getObject("record"));
    const size_t nproc = record.getInt("nproc");
    m_con = std::vector<ProcessController>();
    for (size_t i = 0; i < 3 + nproc; i++) {
      m_con.push_back(ProcessController(this));
    }
    m_eb2rx = ProcessController(this);
    m_eb2rx.init("eb2rx", 1);
    m_con[0].init("storagein", 2);
    m_con[1].init("storagerecord", 3);
    m_con[2].init("storageout", 4);
    for (size_t i = 3; i < m_con.size(); i++) {
      m_con[i].init(StringUtil::form("storagebasf2_%d", i - 3), i + 2);
    }
  } catch (const std::out_of_range& e) {
    LogFile::error("Bad configuration : %s", e.what());
    return false;
  }
  return true;
}

void StoragerCallback::term() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
    m_con[i].getInfo().unlink();
  }
  m_eb2rx.abort();
  m_eb2rx.getInfo().unlink();
}

void StoragerCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  bool is_up_all = true;
  for (size_t i = 0; i < m_con.size(); i++) {
    if (!m_con[i].isAlive()) {
      is_up_all = false;
      break;
    }
  }
  if (is_up_all) return;
  const DBObject& eb2rx(obj.getObject("eb2rx"));
  const DBObject& input(obj.getObject("input"));
  const DBObject& output(obj.getObject("output"));
  const DBObject& record(obj.getObject("record"));
  const DBObject& ibuf(input.getObject("buf"));
  const DBObject& obuf(output.getObject("buf"));
  const DBObject& rbuf(record.getObject("buf"));
  const DBObject& isocket(input.getObject("socket"));
  const DBObject& osocket(output.getObject("socket"));
  const DBObject& file(record.getObject("file"));
  if (eb2rx.getBool("used")) {
    m_eb2rx.clearArguments();
    m_eb2rx.setExecutable(eb2rx.getText("exe"));
    m_eb2rx.addArgument("-l");
    m_eb2rx.addArgument("%d", eb2rx.getInt("port"));
    const DBObjectList& sender(eb2rx.getObjects("sender"));
    for (DBObjectList::const_iterator it = sender.begin();
         it != sender.end(); it++) {
      m_eb2rx.addArgument("%s:%d", it->getText("host").c_str(), it->getInt("port"));
    }
    m_eb2rx.load(0);
    LogFile::debug("Booted eb2rx");
  }

  m_con[0].clearArguments();
  m_con[0].setExecutable("storagein");
  m_con[0].addArgument(ibuf.getText("name"));
  m_con[0].addArgument("%d", ibuf.getInt("size"));
  m_con[0].addArgument(isocket.getText("host"));
  m_con[0].addArgument("%d", isocket.getInt("port"));
  m_con[0].addArgument("storagein");
  m_con[0].addArgument("2");
  if (!m_con[0].load(10)) {
    std::string emsg = "storagein: Failed to connect to eb2rx";
    LogFile::error(emsg);
    throw (RCHandlerException(emsg));
  }
  LogFile::debug("Booted storagein");

  m_con[1].clearArguments();
  m_con[1].setExecutable("storagerecord");
  m_con[1].addArgument(rbuf.getText("name"));
  m_con[1].addArgument("%d", rbuf.getInt("size"));
  m_con[1].addArgument(record.getText("dir"));
  m_con[1].addArgument("%d", record.getInt("ndisks"));
  m_con[1].addArgument(file.getText("diskid"));
  m_con[1].addArgument(file.getText("dbtmp"));
  m_con[1].addArgument(obuf.getText("name"));
  m_con[1].addArgument("%d", obuf.getInt("size"));
  m_con[1].addArgument("storagerecord");
  m_con[1].addArgument("3");
  if (!m_con[1].load(30)) {
    std::string emsg = "storagerecord: Failed to start";
    LogFile::error(emsg);
    throw (RCHandlerException(emsg));
  }
  LogFile::debug("Booted storagerecord");

  if (output.getBool("used")) {
    m_con[2].clearArguments();
    m_con[2].setExecutable("storageout");
    m_con[2].addArgument(obuf.getInt("oname"));
    m_con[2].addArgument("%d", obuf.getInt("size"));
    m_con[2].addArgument("%d", osocket.getInt("port"));
    m_con[2].addArgument("storageout");
    m_con[2].addArgument("4");
    if (!m_con[2].load(10)) {
      LogFile::warning("storageout: Not accepted connection from EXPRECO");
    }
    LogFile::debug("Booted storageout");
  } else {
    LogFile::notice("storageout gets OFF");
  }

  for (size_t i = 3; i < m_con.size(); i++) {
    m_con[i].clearArguments();
    m_con[i].setExecutable("basf2");
    m_con[i].addArgument("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                         record.getText("script").c_str());
    m_con[i].addArgument(ibuf.getText("name"));
    m_con[i].addArgument("%d", ibuf.getInt("size"));
    m_con[i].addArgument(rbuf.getText("name"));
    m_con[i].addArgument("%d", rbuf.getInt("size"));
    m_con[i].addArgument("storagebasf2_%d", i - 3);
    m_con[i].addArgument("%d", i + 2);
    m_con[i].addArgument("1");
    if (!m_con[i].load(10)) {
      std::string emsg = StringUtil::form("Failed to start %d-th basf2", i - 3);
      LogFile::error(emsg);
      throw (RCHandlerException(emsg));
    }
    LogFile::debug("Booted %d-th basf2", i - 3);
  }
  m_flow = std::vector<FlowMonitor>();
  for (size_t i = 0; i < m_con.size(); i++) {
    FlowMonitor flow;
    flow.open(&(m_con[i].getInfo()));
    m_flow.push_back(flow);
  }
  m_ibuf.open(ibuf.getText("name"), ibuf.getInt("size") * 1000000);
  m_rbuf.open(rbuf.getText("name"), rbuf.getInt("size") * 1000000);
}

void StoragerCallback::start(int expno, int runno) throw(RCHandlerException)
{
  storage_status* status = (storage_status*)m_data.get();
  status->stime = Time().getSecond();
  for (size_t i = 0; i < m_con.size(); i++) {
    std::string name = m_con[i].getName();
    if (!m_con[i].start(expno, runno)) {
      if (i != 2) {
        throw (RCHandlerException(name[i] + " is not started"));
      } else {
        std::string emsg = "storageout: Not accepted connection from EXPRECO yet";
        LogFile::warning(emsg);
      }
    }
    LogFile::debug(name[i] + " started");
  }
}

void StoragerCallback::stop() throw(RCHandlerException)
{
}

void StoragerCallback::recover() throw(RCHandlerException)
{
  abort();
}

void StoragerCallback::abort() throw(RCHandlerException)
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
  }
  m_eb2rx.abort();
}

void StoragerCallback::timeout(NSMCommunicator&) throw()
{
  NSMData& data(getData());
  if (!data.isAvailable()) return;
  storage_status* info = (storage_status*)data.get();
  info->ctime = Time().getSecond();
  info->nnodes = m_con.size();
  bool connected = false;
  bool writing = false;
  for (size_t i = 0; i < m_flow.size() && i < 8; i++) {
    ronode_status& rostatus(m_flow[i].monitor());
    info->node[i].connection_in = rostatus.connection_in;
    info->node[i].nevent_in = rostatus.nevent_in;
    info->node[i].evtrate_in = rostatus.evtrate_in;
    info->node[i].evtsize_in = rostatus.evtsize_in;
    info->node[i].flowrate_in = rostatus.flowrate_in;
    info->node[i].connection_out = rostatus.connection_out;
    info->node[i].nevent_out = rostatus.nevent_out;
    info->node[i].evtrate_out = rostatus.evtrate_out;
    info->node[i].evtsize_out = rostatus.evtsize_out;
    info->node[i].flowrate_out = rostatus.flowrate_out;
    if (i == 0) { //IN
      info->eflag = rostatus.eflag;
      info->ctime = rostatus.ctime;
      info->expno = rostatus.expno;
      info->runno = rostatus.runno;
      info->subno = rostatus.subno;
      info->node[0].nqueue_in = rostatus.nqueue_in;
      SharedEventBuffer::Header* hd = m_ibuf.getHeader();
      info->node[0].nqueue_out = hd->nword_in - hd->nword_out;
      connected = (info->node[0].connection_in > 0);
    } else if (i == 1) {
      SharedEventBuffer::Header* hd = m_rbuf.getHeader();
      info->node[1].nqueue_out = hd->nword_in - hd->nword_out;
      info->nfiles = rostatus.reserved_i[0];
      info->diskid = rostatus.reserved_i[1];
      info->nbytes = rostatus.reserved_f[0];
      writing = (rostatus.flowrate_out > 0);
    }
  }
  if (connected) {
    if (writing) info->state = 2;
    else info->state = 1;
  } else {
    info->state = 0;
  }
  struct statvfs statfs;
  if (m_file.hasKey("record.ndisks")) {
    std::string dir = m_file.get("record.dir");
    info->ndisks = m_file.getInt("record.ndisks");
    for (unsigned int i = 0; i < info->ndisks; i++) {
      std::string path = StringUtil::form("%s%02d", dir.c_str(), i + 1);
      statvfs(path.c_str(), &statfs);
      info->disk[i].size = (float)statfs.f_frsize * statfs.f_blocks / 1024 / 1024 / 1024;
      info->disk[i].available = 100 - ((float)statfs.f_bfree / statfs.f_blocks * 100);
    }
  }
  double loads[3];
  if (getloadavg(loads, 3) > 0) {
    info->loadavg = (float)loads[0];
  } else {
    info->loadavg = -1;
  }
}
