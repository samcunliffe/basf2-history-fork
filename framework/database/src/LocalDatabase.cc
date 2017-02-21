/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/def.hpp>

#include <framework/database/LocalDatabase.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <fstream>
#include <sys/file.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

void LocalDatabase::createInstance(const std::string& fileName, const std::string& payloadDir, bool readOnly,
                                   LogConfig::ELogLevel logLevel, bool invertLogging)
{
  LocalDatabase* database = new LocalDatabase(fileName, payloadDir, readOnly);
  database->setLogLevel(logLevel, invertLogging);
  Database::setInstance(database);
}

LocalDatabase::LocalDatabase(const std::string& fileName, const std::string& payloadDir, bool readOnly):
  m_fileName(fileName), m_absFileName(fs::absolute(m_fileName).string()), m_payloadDir(payloadDir), m_readOnly(readOnly)
{
  if (m_payloadDir.empty()) {
    m_payloadDir = fs::path(m_absFileName).parent_path().string();
  } else {
    m_payloadDir = fs::absolute(m_payloadDir).string();
  }
  readDatabase();
}


bool LocalDatabase::readDatabase()
{
  m_database.clear();

  if (!fs::exists(m_absFileName)) return true;

  // get lock for read access to database file
  FileSystem::Lock lock(m_absFileName, true);
  if (!m_readOnly && !lock.lock()) {
    B2ERROR("Locking of database file " << m_fileName << " failed.");
    return false;
  }

  // read and parse the database content
  std::ifstream file(m_absFileName.c_str());
  if (!file.is_open()) {
    B2ERROR("Opening of database file " << m_fileName << " failed.");
    return false;
  }
  int lineno{0};
  try {
    while (!file.eof()) {
      // read the file line by line
      string line;
      std::getline(file, line);
      ++lineno;
      // and remove comments from the line
      size_t commentChar = line.find('#');
      if (commentChar != std::string::npos) {
        line = line.substr(0, commentChar);
      }
      // trim whitespace on each side
      boost::algorithm::trim(line);
      // if nothing is left skip the line
      if (line.empty()) continue;
      // otherwise read name, revision and iov from the line
      string name;
      string revisionStr;
      IntervalOfValidity iov;
      try {
        std::stringstream(line) >> name >> revisionStr >> iov;
      } catch (std::runtime_error& e) {
        throw std::runtime_error("line must be of the form 'dbstore/<payloadname> <revision> <firstExp>,<firstRun>,<finalExp>,<finalRun>'");
      }
      int revision{ -1};
      try {
        revision = stoi(revisionStr);
      } catch (std::invalid_argument& e) {
        throw std::runtime_error("revision must be an integer");
      }
      // parse name
      size_t pos = name.find("/");
      if (pos == std::string::npos) {
        throw std::runtime_error("payload name must be of the form dbstore/<payloadname>");
      }
      string package = name.substr(0, pos);
      string module = name.substr(pos + 1, name.length());
      // and add to map of payloads
      B2DEBUG(100, m_fileName << ":" << lineno << ": found payload " << boost::io::quoted(module)
              << ", revision " << revision << ", iov " << iov);
      m_database[package][module].push_back(make_pair(revision, iov));
    }
  } catch (std::exception& e) {
    B2ERROR(m_fileName << ":" << lineno << " error: " << e.what());
    return false;
  }

  return true;
}

pair<TObject*, IntervalOfValidity> LocalDatabase::tryDefault(const std::string& package, const std::string& module)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  std::string defaultName = payloadFileName(m_payloadDir, package, module, 0);
  if (FileSystem::fileExists(defaultName)) {
    result.first = readPayload(defaultName, module);
    if (!result.first) return result;
    result.second = IntervalOfValidity(0, -1, -1, -1);
    if (m_invertLogging)
      B2LOG(m_logLevel, 0, "Obtained " << module << " from " << defaultName << ". IoV="
            << result.second);
    return result;
  }

  if (!m_invertLogging)
    B2LOG(m_logLevel, 0, "Failed to get " << module << " from local database " << m_fileName << ".");
  return result;
}

pair<TObject*, IntervalOfValidity> LocalDatabase::getData(const EventMetaData& event, const string& package,
                                                          const std::string& module)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  // find the entry for package and module in the maps
  const auto& packageEntry = m_database.find(package);
  if (packageEntry == m_database.end()) return tryDefault(package, module);
  const auto& moduleEntry = packageEntry->second.find(module);
  if (moduleEntry == packageEntry->second.end()) return tryDefault(package, module);

  // find the payload whose IoV contains the current event and load it
  for (auto& entry : boost::adaptors::reverse(moduleEntry->second)) {
    if (entry.second.contains(event)) {
      int revision = entry.first;
      result.first = readPayload(payloadFileName(m_payloadDir, package, module, revision), module);
      if (!result.first) return result;
      result.second = entry.second;
      if (m_invertLogging)
        B2LOG(m_logLevel, 0, "Obtained " << module << " from local database " << m_fileName <<
              ". IoV=" << result.second);
      return result;
    }
  }

  if (!m_invertLogging)
    B2LOG(m_logLevel, 0, "Failed to get " << module << " from local database " << m_fileName <<
          ". No matching entry for experiment/run " << event.getExperiment() << "/" << event.getRun() << " found.");
  return result;
}


bool LocalDatabase::storeData(const std::string& package, const std::string& module, TObject* object,
                              const IntervalOfValidity& iov)
{
  if (m_readOnly) {
    B2ERROR("Database file " << m_fileName << " is opened in read-only mode.");
    return false;
  }

  if (!fs::exists(m_payloadDir)) {
    fs::create_directories(m_payloadDir);
  }
  // get lock for write access to database file
  FileSystem::Lock lock(m_absFileName);
  if (!lock.lock()) {
    B2ERROR("Locking of database file " << m_fileName << " failed.");
    return false;
  }

  // get revision number
  int revision = 1;
  while (FileSystem::fileExists(payloadFileName(m_payloadDir, package, module, revision))) revision++;

  // write payload file
  if (!writePayload(payloadFileName(m_payloadDir, package, module, revision), module, object, &iov)) return false;

  // add to database and update database file
  m_database[package][module].push_back(make_pair(revision, iov));
  std::ofstream file(m_absFileName.c_str(), std::ios::app);
  if (!file.is_open()) return false;
  file << package << "/" << module << " " << revision << " " << iov << endl;

  return true;
}

bool LocalDatabase::addPayload(const std::string& package, const std::string& module, const std::string& fileName,
                               const IntervalOfValidity& iov)
{
  if (m_readOnly) {
    B2ERROR("Database file " << m_fileName << " is opened in read-only mode.");
    return false;
  }

  // get lock for write access to database file
  FileSystem::Lock lock(m_absFileName);
  if (!lock.lock()) {
    B2ERROR("Locking of database file " << m_fileName << " failed.");
    return false;
  }

  if (!fs::exists(m_payloadDir)) {
    fs::create_directories(m_payloadDir);
  }

  // get revision number
  int revision = 1;
  while (FileSystem::fileExists(payloadFileName(m_payloadDir, package, module, revision))) revision++;

  // copy payload file to payload directory and rename it to follow the file name convention
  boost::filesystem::copy(fileName, payloadFileName(m_payloadDir, package, module, revision));

  // add to database and update database file
  m_database[package][module].push_back(make_pair(revision, iov));
  std::ofstream file(m_absFileName.c_str(), std::ios::app);
  if (!file.is_open()) return false;
  file << package << "/" << module << " " << revision << " " << iov << endl;

  return true;
}
