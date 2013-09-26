#include "DBNodeSystemConfigurator.hh"

#include <util/StringUtil.hh>
#include <util/Debugger.hh>

#include <iostream>
#include <sstream>

using namespace B2DAQ;

void DBNodeSystemConfigurator::createTables() throw(DBHandlerException)
{
  createVersionControlTable();
  createHostTable();
  createCOPPERNodeTable();
  createHSLBTable();
  createTTDNodeTable();
  createFTSWTable();
  createDataReceiverNodeTable();
  for (std::map<std::string, std::vector<FEEModule*> >::iterator it = _system->getModuleLists().begin();
       it != _system->getModuleLists().end(); it++) {
    createFEEModuleTable(it->first, it->second);
  };
}

DBRecord DBNodeSystemConfigurator::readTables(int version) throw(DBHandlerException)
{
  _system->setVersion(version);
  DBRecord record = readVersionControlTable(version);
  _system->setRunType(record.getFieldValue("run_type"));
  _system->setDescription(record.getFieldValue("description"));
  readHostTable(record.getFieldValueInt("host_ver"));
  readCOPPERNodeTable(record.getFieldValueInt("copper_node_ver"));
  readHSLBTable(record.getFieldValueInt("hslb_ver"));
  readDataReceiverNodeTable(record.getFieldValueInt("reciever_node_ver"));
  for (std::map<std::string, std::vector<FEEModule*> >::iterator it = _system->getModuleLists().begin();
       it != _system->getModuleLists().end(); it++) {
    std::string label = it->first;
    if (label.size() > 0) {
      //std::cout << "'" << label << "' " << record.getFieldValueInt(label + "_ver") << std::endl;
      readFEEModuleTable(label, it->second, record.getFieldValueInt(label + "_ver"));
    }
  };
  try {
    readTTDNodeTable(record.getFieldValueInt("ttd_node_ver"));
    readFTSWTable(record.getFieldValueInt("ftsw_ver"));
  } catch (const DBHandlerException) {
    B2DAQ::debug("Error on ttd_node_conf or ftsw_conf");
  }
  return record;
}

void DBNodeSystemConfigurator::writeTables() throw(DBHandlerException)
{
  writeVersionControlTable();
  writeHostTable(0);
  writeCOPPERNodeTable(0);
  writeHSLBTable(0);
  writeTTDNodeTable(0);
  writeFTSWTable(0);
  writeDataReceiverNodeTable(0);
  for (std::map<std::string, std::vector<FEEModule*> >::iterator it = _system->getModuleLists().begin();
       it != _system->getModuleLists().end(); it++) {
    std::string label = it->first;
    if (label.size() > 0) {
      writeFEEModuleTable(label, it->second, 0);
    }
  };
}

void DBNodeSystemConfigurator::createVersionControlTable() throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("create table version_control (%s);", _system->getSQLFields().c_str()));
}

void DBNodeSystemConfigurator::createHostTable() throw(DBHandlerException)
{
  if (_system->getHosts().size() > 0)
    _db->execute(B2DAQ::form("create table host_conf (%s);",
                             _system->getHosts()[0]->getSQLFields().c_str()));
}

void DBNodeSystemConfigurator::createCOPPERNodeTable() throw(DBHandlerException)
{
  if (_system->getCOPPERNodes().size() > 0)
    _db->execute(B2DAQ::form("create table copper_node_conf (%s);",
                             _system->getCOPPERNodes()[0]->getSQLFields().c_str()));
}

void DBNodeSystemConfigurator::createHSLBTable() throw(DBHandlerException)
{
  if (_system->getHSLBs().size() > 0)
    _db->execute(B2DAQ::form("create table hslb_conf (%s);",
                             _system->getHSLBs()[0]->getSQLFields().c_str()));
}

void DBNodeSystemConfigurator::createTTDNodeTable() throw(DBHandlerException)
{
  if (_system->getTTDNodes().size() > 0)
    _db->execute(B2DAQ::form("create table ttd_node_conf (%s);",
                             _system->getTTDNodes()[0]->getSQLFields().c_str()));
}

void DBNodeSystemConfigurator::createFTSWTable() throw(DBHandlerException)
{
  if (_system->getFTSWs().size() > 0)
    _db->execute(B2DAQ::form("create table ftsw_conf (%s);",
                             _system->getFTSWs()[0]->getSQLFields().c_str()));
}

void DBNodeSystemConfigurator::createDataReceiverNodeTable() throw(DBHandlerException)
{
  if (_system->getReceiverNodes().size() > 0)
    _db->execute(B2DAQ::form("create table reciever_node_conf (%s);",
                             _system->getReceiverNodes()[0]->getSQLFields().c_str()));
}

void DBNodeSystemConfigurator::createFEEModuleTable(const std::string& module_class,
                                                    std::vector<FEEModule*>& module_v)
throw(DBHandlerException)
{
  if (module_v.size() > 0) {
    std::stringstream ss;
    ss << "create table " << module_class << "_conf ("
       << _system->getModules(module_class)[0]->getSQLFields() << "); ";
    _db->execute(ss.str());
  }
}

DBRecord DBNodeSystemConfigurator::readVersionControlTable(int version) throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from version_control where version = %d;", version));
  //B2DAQ::debug("select * from version_control where version = %d;", version);
  std::vector<DBRecord>& record_v(_db->loadRecords());
  if (record_v.size() == 0) {
    throw (DBHandlerException(__FILE__, __LINE__,
                              "No record was read from version_control."));
  }
  return record_v[record_v.size() - 1];
}

void DBNodeSystemConfigurator::readHostTable(int version) throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from host_conf where version = %d;", version));
  std::vector<DBRecord>& record_v(_db->loadRecords());
  std::vector<Host*>& host_v(_system->getHosts());
  for (size_t i = 0; i < record_v.size(); i++) {
    const int id = record_v[i].getFieldValueInt("id");
    if (id < 0 || id >= (int)host_v.size()) continue;
    Host* host = host_v[id];
    host->setName(record_v[i].getFieldValue("hostname"));
    host->setType(record_v[i].getFieldValue("type"));
    host->setProductID(record_v[i].getFieldValueInt("product_id"));
    host->setLocation(record_v[i].getFieldValue("location"));
  }
}

void DBNodeSystemConfigurator::readCOPPERNodeTable(int version) throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from copper_node_conf where version = %d;", version));
  std::vector<DBRecord>& record_v(_db->loadRecords());
  std::vector<Host*>& host_v(_system->getHosts());
  std::vector<COPPERNode*>& node_v(_system->getCOPPERNodes());
  std::vector<HSLB*>& hslb_v(_system->getHSLBs());
  for (size_t i = 0; i < record_v.size(); i++) {
    const int id = record_v[i].getFieldValueInt("id");
    if (id < 0 || id >= (int)node_v.size()) continue;
    COPPERNode* node = node_v[id];
    node->setName(record_v[i].getFieldValue("name"));
    node->setUsed(record_v[i].getFieldValueInt("used"));
    int host_id = record_v[i].getFieldValueInt("host_id");
    if (host_id >= 0 && host_id < (int)host_v.size()) node->setHost(host_v[host_id]);
    for (size_t slot = 0; slot < 4; slot++) {
      int hslb_id = record_v[i].getFieldValueInt(B2DAQ::form("hslb_id_%d", slot));
      if (hslb_id >= 0 && hslb_id < (int)hslb_v.size()) {
        node->setHSLB(slot, hslb_v[hslb_id]);
      } else {

      }
    }
    node->getSender()->setHost(host_v[host_id]->getName());
    node->getSender()->setScript(record_v[i].getFieldValue("script"));
    node->getSender()->setPort(record_v[i].getFieldValueInt("port"));
    node->getSender()->setEventSize(record_v[i].getFieldValueInt("event_size"));
  }
}

void DBNodeSystemConfigurator::readHSLBTable(int version) throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from hslb_conf where version = %d;", version));
  std::vector<DBRecord>& record_v(_db->loadRecords());
  std::vector<HSLB*>& hslb_v(_system->getHSLBs());
  for (size_t i = 0; i < record_v.size(); i++) {
    const int id = record_v[i].getFieldValueInt("id");
    if (id < 0 || id >= (int)hslb_v.size()) continue;
    HSLB* hslb = hslb_v[id];
    hslb->setUsed((bool)record_v[i].getFieldValueInt("used"));
    hslb->setProductID(record_v[i].getFieldValueInt("product_id"));
    hslb->setLocation(record_v[i].getFieldValue("location"));
    hslb->setFirmware(record_v[i].getFieldValue("firmware"));
    /*
    std::string module_type = record_v[i].getFieldValue("module_type");
    std::vector<FEEModule*>& module_v(_system->getModules(module_type));
    int module_id = record_v[i].getFieldValueInt("module_id");
    if ( module_id >= 0 && module_id < (int)module_v.size() )
      hslb->setFEEModule(module_v[module_id]);
    */
  }
}

void DBNodeSystemConfigurator::readTTDNodeTable(int version) throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from ttd_node_conf where version = %d;", version));
  std::vector<DBRecord>& record_v(_db->loadRecords());
  std::vector<Host*>& host_v(_system->getHosts());
  std::vector<TTDNode*>& node_v(_system->getTTDNodes());
  std::vector<FTSW*>& ftsw_v(_system->getFTSWs());
  for (size_t i = 0; i < record_v.size(); i++) {
    const int id = record_v[i].getFieldValueInt("id");
    if (id < 0 || id >= (int)node_v.size()) continue;
    TTDNode* node = node_v[id];
    node->setName(record_v[i].getFieldValue("name"));
    node->setUsed(record_v[i].getFieldValueInt("used"));
    int host_id = record_v[i].getFieldValueInt("host_id");
    if (host_id >= 0 && host_id < (int)host_v.size()) node->setHost(host_v[host_id]);
    node->clearFTSWs();
    for (size_t slot = 0; slot < 10; slot++) {
      int ftsw_id = record_v[i].getFieldValueInt(B2DAQ::form("ftsw_id_%d", slot));
      if (ftsw_id >= 0 && ftsw_id < (int)ftsw_v.size())
        node->addFTSW(ftsw_v[ftsw_id]);
    }
  }
}

void DBNodeSystemConfigurator::readFTSWTable(int version) throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from ftsw_conf where version = %d;", version));
  std::vector<DBRecord>& record_v(_db->loadRecords());
  std::vector<FTSW*>& ftsw_v(_system->getFTSWs());
  for (size_t i = 0; i < record_v.size(); i++) {
    const int id = record_v[i].getFieldValueInt("id");
    if (id < 0 || id >= (int)ftsw_v.size()) continue;
    FTSW* ftsw = ftsw_v[id];
    ftsw->setUsed(record_v[i].getFieldValueInt("used"));
    ftsw->setProductID(record_v[i].getFieldValueInt("product_id"));
    ftsw->setLocation(record_v[i].getFieldValue("location"));
    ftsw->clearModules();
    int mode = record_v[i].getFieldValueInt("trigger_mode");
    ftsw->setTriggerMode(mode);
    for (size_t slot = 0; slot < 20; slot++) {
      std::string module_type = record_v[i].getFieldValue(B2DAQ::form("module_type_%d", slot));
      int module_id = record_v[i].getFieldValueInt(B2DAQ::form("module_id_%d", slot));
      std::vector<FEEModule*>& module_v(_system->getModules(module_type));
      if (module_id >= 0 && module_id < (int)module_v.size())
        ftsw->addFEEModule(module_v[module_id]);
    }
  }
}

void DBNodeSystemConfigurator::readDataReceiverNodeTable(int version) throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from reciever_node_conf where version = %d;", version));
  std::vector<DBRecord>& record_v(_db->loadRecords());
  std::vector<DataReceiverNode*>& node_v(_system->getReceiverNodes());
  std::vector<COPPERNode*>& copper_v(_system->getCOPPERNodes());
  for (size_t i = 0; i < record_v.size(); i++) {
    const int id = record_v[i].getFieldValueInt("id");
    if (id < 0 || id >= (int)node_v.size()) continue;
    DataReceiverNode* node = node_v[id];
    node->setName(record_v[i].getFieldValue("name"));
    node->setUsed(record_v[i].getFieldValueInt("used"));
    node->setScript(record_v[i].getFieldValue("script"));
    node->clearSenders();
    for (size_t slot = 0; slot < 10; slot++) {
      int copper_id = record_v[i].getFieldValueInt(B2DAQ::form("sender_id_%d", slot));
      if (copper_id >= 0 && copper_id < (int)copper_v.size()) {
        COPPERNode* copper = copper_v[copper_id];
        //std::cout << "DEBUG:" << __FILE__ << ":" << __LINE__ << ":" << copper->getSender()->getHost() << std::endl;
        node->addSender(copper->getSender());
      }
    }
  }
}

void DBNodeSystemConfigurator::readFEEModuleTable(const std::string& module_type,
                                                  std::vector<FEEModule*>& module_v,
                                                  int version)
throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("select * from %s_conf where version = %d;",
                           module_type.c_str(), version));
  std::vector<DBRecord>& record_v(_db->loadRecords());
  for (size_t i = 0; i < record_v.size(); i++) {
    const int id = record_v[i].getFieldValueInt("id");
    if (id < 0 || id >= (int)module_v.size()) continue;
    FEEModule* module = module_v[id];
    for (size_t slot = 0; slot < module->getRegisters().size(); slot++) {
      FEEModule::Register& reg(module->getRegister(slot));
      for (size_t j = 0; j < reg.length(); j++) {
        int value = record_v[i].getFieldValueInt(B2DAQ::form("%s_%d", reg.getName().c_str(), j));
        reg.setValue(j, value);
      }
    }
  }
}

void DBNodeSystemConfigurator::writeVersionControlTable()
throw(DBHandlerException)
{
  _db->execute(B2DAQ::form("insert into version_control (%s) values (%s);",
                           _system->getSQLLabels().c_str(), _system->getSQLValues().c_str()));
}

void DBNodeSystemConfigurator::writeHostTable(int version) throw(DBHandlerException)
{
  std::vector<Host*>& host_v(_system->getHosts());
  std::stringstream ss;
  for (size_t i = 0; i < host_v.size(); i++) {
    host_v[i]->setVersion(version);
    ss << "insert into host_conf ("
       << host_v[i]->getSQLLabels() << ") values ("
       << host_v[i]->getSQLValues() << "); ";
    _db->execute(ss.str()); ss.str("");
  }
}

void DBNodeSystemConfigurator::writeCOPPERNodeTable(int version) throw(DBHandlerException)
{
  std::vector<COPPERNode*>& copper_v(_system->getCOPPERNodes());
  std::stringstream ss;
  for (size_t i = 0; i < copper_v.size(); i++) {
    copper_v[i]->setVersion(version);
    ss << "insert into copper_node_conf ("
       << copper_v[i]->getSQLLabels() << ") values ("
       << copper_v[i]->getSQLValues() << "); ";
    _db->execute(ss.str()); ss.str("");
  }
}

void DBNodeSystemConfigurator::writeHSLBTable(int version) throw(DBHandlerException)
{
  std::vector<HSLB*>& hslb_v(_system->getHSLBs());
  std::stringstream ss;
  for (size_t i = 0; i < hslb_v.size(); i++) {
    hslb_v[i]->setVersion(version);
    ss << "insert into hslb_conf ("
       << hslb_v[i]->getSQLLabels() << ") values ("
       << hslb_v[i]->getSQLValues() << "); ";
    _db->execute(ss.str()); ss.str("");
  }
}

void DBNodeSystemConfigurator::writeTTDNodeTable(int version) throw(DBHandlerException)
{
  std::vector<TTDNode*>& ttd_v(_system->getTTDNodes());
  std::stringstream ss;
  for (size_t i = 0; i < ttd_v.size(); i++) {
    ss << "insert into ttd_node_conf ("
       << ttd_v[i]->getSQLLabels() << ") values ("
       << ttd_v[i]->getSQLValues() << "); ";
    _db->execute(ss.str()); ss.str("");
  }
}

void DBNodeSystemConfigurator::writeFTSWTable(int version) throw(DBHandlerException)
{
  std::vector<FTSW*>& ftsw_v(_system->getFTSWs());
  std::stringstream ss;
  for (size_t i = 0; i < ftsw_v.size(); i++) {
    ftsw_v[i]->setVersion(version);
    ss << "insert into ftsw_conf ("
       << ftsw_v[i]->getSQLLabels() << ") values ("
       << ftsw_v[i]->getSQLValues() << "); ";
    _db->execute(ss.str()); ss.str("");
  }
}

void DBNodeSystemConfigurator::writeDataReceiverNodeTable(int version) throw(DBHandlerException)
{
  std::vector<DataReceiverNode*>& reciever_v(_system->getReceiverNodes());
  std::stringstream ss;
  for (size_t i = 0; i < reciever_v.size(); i++) {
    reciever_v[i]->setVersion(version);
    ss << "insert into reciever_node_conf ("
       << reciever_v[i]->getSQLLabels() << ") values ("
       << reciever_v[i]->getSQLValues() << "); ";
    _db->execute(ss.str()); ss.str("");
  }
}

void DBNodeSystemConfigurator::writeFEEModuleTable(const std::string& module_class,
                                                   std::vector<FEEModule*>& module_v,
                                                   int version)
throw(DBHandlerException)
{
  std::stringstream ss;
  for (size_t i = 0; i < module_v.size(); i++) {
    module_v[i]->setVersion(version);
    ss << "insert into " << module_v[i]->getType() << "_conf ("
       << module_v[i]->getSQLLabels() << ") values ("
       << module_v[i]->getSQLValues() << "); ";
    _db->execute(ss.str()); ss.str("");
  }
}

