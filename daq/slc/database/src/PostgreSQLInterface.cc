#include "daq/slc/database/PostgreSQLInterface.h"

#ifndef NOT_USE_PSQL
#include <libpq-fe.h>
#endif

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

PostgreSQLInterface::PostgreSQLInterface(const std::string& host,
                                         const std::string& database,
                                         const std::string& user,
                                         const std::string& password,
                                         int port) throw()
  : DBInterface(host, database, user, password, port)
{
  m_sq_conn = NULL;
  m_sq_result = NULL;
}

void PostgreSQLInterface::connect() throw(DBHandlerException)
{
#ifndef NOT_USE_PSQL
  m_sq_conn = PQconnectdb(StringUtil::form("host=%s dbname=%s user=%s password=%s",
                                           m_host.c_str(), m_database.c_str(),
                                           m_user.c_str(), m_password.c_str()).c_str());
  if (PQstatus(m_sq_conn) == CONNECTION_BAD) {
    close();
    throw (DBHandlerException("Failed to connect to the database : (%s)",
                              PQerrorMessage(m_sq_conn)));
  }
#else
  throw (DBHandlerException("PGLIB is not available"));
#endif
}

bool PostgreSQLInterface::isConnected() throw()
{
#ifndef NOT_USE_PSQL
  return m_sq_conn != NULL &&
         PQstatus(m_sq_conn) == CONNECTION_OK;
#else
  return false;
#endif
}

void PostgreSQLInterface::execute_imp(const char* command)
throw(DBHandlerException)
{
  clear();
#ifndef NOT_USE_PSQL
  m_sq_result = PQexec(m_sq_conn, command);
  ExecStatusType status = PQresultStatus(m_sq_result);
  if (status == PGRES_FATAL_ERROR) {
    throw (DBHandlerException("Failed to execute command : %s (%s)",
                              command, PQerrorMessage(m_sq_conn)));
  }
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

DBRecordList PostgreSQLInterface::loadRecords() throw(DBHandlerException)
{
#ifndef NOT_USE_PSQL
  if (PQresultStatus(m_sq_result) != PGRES_TUPLES_OK) {
    throw (DBHandlerException("DB records are not ready for reading"));
  }
  const size_t nrecords = PQntuples(m_sq_result);
  const size_t nfields = PQnfields(m_sq_result);
  m_record_v = DBRecordList();
  std::vector<std::string> name_v;
  for (size_t ifield = 0; ifield < nfields; ifield++) {
    const char* name = PQfname(m_sq_result, ifield);
    if (name != NULL) name_v.push_back(name);
  }
  for (size_t irecord = 0; irecord < nrecords; irecord++) {
    DBRecord record;
    for (size_t ifield = 0; ifield < nfields; ifield++) {
      if (!PQgetisnull(m_sq_result, irecord, ifield)) {
        const char* value = PQgetvalue(m_sq_result, irecord, ifield);
        if (value != NULL) {
          record.add(name_v[ifield], value);
        }
      }
    }
    m_record_v.push_back(record);
  }
  return m_record_v;
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

void PostgreSQLInterface::clear() throw()
{
#ifndef NOT_USE_PSQL
  if (m_sq_result != NULL) {
    PQclear(m_sq_result);
  }
  m_sq_result = NULL;
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

void PostgreSQLInterface::close() throw(DBHandlerException)
{
  clear();
#ifndef NOT_USE_PSQL
  if (m_sq_conn != NULL) {
    PQfinish(m_sq_conn);
    m_sq_conn = NULL;
  }
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

bool PostgreSQLInterface::checkTable(const std::string& tablename) throw(DBHandlerException)
{
#ifndef NOT_USE_PSQL
  execute("select relname from pg_stat_user_tables where relname='%s';",
          tablename.c_str());
  DBRecordList ret(loadRecords());
  return ret.size() > 0;
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

DBFieldTypeList PostgreSQLInterface::getTableContents(const std::string& tablename)
throw(DBHandlerException)
{
#ifndef NOT_USE_PSQL
  DBFieldTypeList name_m;
  execute("select attname, typname from pg_class, pg_attribute, pg_type "
          "where relkind ='r'and relname = '%s' and attrelid = relfilenode "
          "and attnum > 0 and pg_type.oid = atttypid;", tablename.c_str());
  DBRecordList ret(loadRecords());
  for (size_t i = 0; i < ret.size(); i++) {
    name_m.insert(DBFieldTypeList::value_type(ret[i].get("attname"),
                                              ret[i].get("typname")));
  }
  return name_m;
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}
