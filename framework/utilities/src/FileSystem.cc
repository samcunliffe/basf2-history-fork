/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Thomas Kuhr                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/FileSystem.h>

#include <framework/logging/Logger.h>

#include <boost/filesystem.hpp>

//dlopen etc.
#include <dlfcn.h>
#include <chrono>
#include <random>
#include <sys/file.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

bool FileSystem::fileExists(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  return fs::exists(fullPath);
}

bool FileSystem::fileDirExists(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  fullPath.remove_filename();
  return fs::exists(fullPath);
}

bool FileSystem::isFile(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  return (fs::exists(fullPath)) && (fs::is_regular_file(fullPath));
}

bool FileSystem::isDir(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  return (fs::exists(fullPath)) && (fs::is_directory(fullPath));
}

bool FileSystem::loadLibrary(std::string library, bool fullname)
{
  if (!fullname) library = "lib" + library + ".so";

  B2DEBUG(1, "Loading shared library " << library);
  void* libPointer = dlopen(library.c_str() , RTLD_LAZY | RTLD_GLOBAL);

  if (libPointer == NULL) {
    B2ERROR("Could not open shared library file (error in dlopen) : " + string(dlerror()));
    return false;
  }

  return true;
}

std::string FileSystem::findFile(const string& path)
{
  //environment doesn't change, so only done once
  static const char* localdir = getenv("BELLE2_LOCAL_DIR");
  static const char* reldir = getenv("BELLE2_RELEASE_DIR");

  //check in local directory
  string fullpath;
  if (localdir and fileExists(fullpath = (fs::path(localdir) / path).string()))
    return fullpath;
  //check in central release directory
  else if (reldir and fileExists(fullpath = (fs::path(reldir) / path).string()))
    return fullpath;
  //check if this thing exists as normal path (absolute / relative to PWD)
  else if (fileExists(fullpath = (fs::absolute(path).string())))
    return fullpath;
  //nothing found
  else
    return string("");
}


FileSystem::Lock::Lock(std::string fileName)
{
  m_file = open(fileName.c_str(), O_RDWR | O_CREAT, 0640);
}

FileSystem::Lock::~Lock()
{
  if (m_file >= 0) close(m_file);
}

bool FileSystem::Lock::lock(int timeout)
{
  if (m_file < 0) return false;

  auto const maxtime = std::chrono::steady_clock::now() + std::chrono::seconds(timeout);
  std::default_random_engine random;
  std::uniform_int_distribution<int> uniform(1, 100);

  while (std::chrono::steady_clock::now() < maxtime) {
    int lock = flock(m_file, LOCK_EX | LOCK_NB);
    if (lock == 0) return true;
    auto next = std::chrono::steady_clock::now() + std::chrono::milliseconds(uniform(random));
    while (std::chrono::steady_clock::now() < next);
  }

  return false;
}

FileSystem::TemporaryFile::TemporaryFile(std::ios_base::openmode mode): std::fstream()
{
  fs::path filename = fs::temp_directory_path() / fs::unique_path();
  m_filename = filename.native();
  open(m_filename.c_str(), mode);
  if (!is_open()) {
    B2ERROR("Cannot create temporary file:" << strerror(errno));
  }
}

FileSystem::TemporaryFile::~TemporaryFile()
{
  close();
  fs::remove(m_filename);
}
