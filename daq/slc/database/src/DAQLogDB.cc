#include "daq/slc/database/DAQLogDB.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

using namespace Belle2;

bool DAQLogDB::createLog(DBInterface& db, const std::string& tablename,
                         const DAQLogMessage& log)
{
  std::stringstream ss;
  try {
    if (!db.isConnected()) db.connect();
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  try {
    std::string tablename_date = tablename + "_" + Date().toString("%Y");
    if (!db.checkTable("daqlog")) {
      db.execute("create table daqlog \n"
                 "(name text not null, \n"
                 "id bigserial, lastupdate timestamp, \n"
                 "UNIQUE(name));");
      db.execute("create index daqlog_id_index on daqlog(id);",
                 tablename.c_str(), tablename.c_str());
    }
    if (!db.checkTable(tablename_date)) {
      try {
        db.execute("insert into daqlog (name, lastupdate) values "
                   "('%s', current_timestamp);", tablename_date.c_str());
        db.execute("create table %s \n"
                   "(node  int not null, \n"
                   "priority int not null, \n"
                   "id bigserial, \n"
                   "date timestamp with time zone, \n"
                   "message text not null); ", tablename_date.c_str());
      } catch (const std::exception& e) {
        db.execute("update daqlog set lastupdate = current_timestamp where name = '%s';",
                   tablename_date.c_str());
      }
      db.execute("create index %s_id_index on %s(id);",
                 tablename_date.c_str(), tablename_date.c_str());
    }
    db.execute("select id,category from log_node where name = '" + log.getNodeName() + "';");
    DBRecordList record(db.loadRecords());
    int id = 1;
    int cid = 1;
    if (record.size() > 0) {
      id = record[0].getInt("id");
      cid = record[0].getInt("category");
    } else {
      db.execute("insert into log_node (name, category) values ('" + log.getNodeName() +
                 "', " + StringUtil::form("%d", log.getCategory()) + ") returning id,category;");
      DBRecordList record(db.loadRecords());
      id = record[0].getInt("id");
      cid = record[0].getInt("category");
    }
    db.execute("insert into %s (node, priority, date, message) values "
               "(%d, %d, to_timestamp(%d), '%s');",
               tablename_date.c_str(), id, log.getPriority(),
               log.getDateInt(), log.getMessage().c_str());
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

DAQLogMessageList DAQLogDB::getLogs(DBInterface& db, const std::string& tablename,
                                    const std::string& nodename, int max)
{
  std::string tablename_date = tablename + "_" + Date().toString("%Y");
  DAQLogMessageList logs;
  try {
    if (!db.isConnected()) db.connect();
    if (nodename.size() > 0) {
      if (max > 0) {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s where node = '%s' order by id desc limit %d;",
                   tablename_date.c_str(), nodename.c_str(), max);
      } else {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s where node = '%s' order by id desc;",
                   tablename_date.c_str(), nodename.c_str());
      }
    } else {
      if (max > 0) {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s order by id desc limit %d;", tablename_date.c_str(), max);
      } else {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s order by id desc;", tablename_date.c_str());
      }
    }
    DBRecordList record_v(db.loadRecords());
    std::map<int, std::string> nodemap;
    {
      db.execute("select name,id from log_node;");
      DBRecordList record_v(db.loadRecords());
      for (size_t i = 0; i < record_v.size(); i++) {
        DBRecord& record(record_v[i]);
        nodemap.insert(std::pair<int, std::string>(record.getInt("id"), record.get("name")));
      }
    }
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      logs.push_back(DAQLogMessage(nodemap[record.getInt("node")],
                                   (LogFile::Priority)(record.getInt("priority")),
                                   record.get("message"), Date(record.getInt("date"))));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  return logs;
}

DAQLogMessageList DAQLogDB::getLogs(DBInterface& db, const std::string& tablename,
                                    const std::string& nodename, const std::string& begin_date,
                                    const std::string& end_date, int max)
{
  std::string tablename_date = tablename + "_" + Date().toString("%Y");
  DAQLogMessageList logs;
  try {
    if (!db.isConnected()) db.connect();
    std::stringstream ss;
    ss << "select node, extract(epoch from date) date, priority, message "
       << "from " << tablename_date << " ";
    bool hasand = false;
    if (nodename.size() > 0) {
      hasand = true;
      ss << "where node = '" << nodename << "' ";
    }
    if (begin_date.size() > 0) {
      if (hasand) ss << "and ";
      else {
        ss << "where ";
        hasand = true;
      }
      ss << "date >= '" << begin_date << "' ";
    }
    if (end_date.size() > 0) {
      if (hasand) ss << "and ";
      else {
        ss << "where ";
      }
      ss << "date <= '" << end_date << "' ";
    }
    ss << "order by id desc ";
    if (max > 0) ss << "limit " << max;
    db.execute(ss.str().c_str());
    DBRecordList record_v(db.loadRecords());
    std::map<int, std::string> nodemap;
    {
      db.execute("select name,id from log_node;");
      DBRecordList record_v(db.loadRecords());
      for (size_t i = 0; i < record_v.size(); i++) {
        DBRecord& record(record_v[i]);
        nodemap.insert(std::pair<int, std::string>(record.getInt("id"), record.get("name")));
      }
    }
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      logs.push_back(DAQLogMessage(nodemap[record.getInt("node")],
                                   (LogFile::Priority)(record.getInt("priority")),
                                   record.get("message"), Date(record.getInt("date"))));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  return logs;
}
