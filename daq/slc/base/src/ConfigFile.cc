#include "daq/slc/base/ConfigFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <cstdlib>
#include <fstream>

using namespace Belle2;

std::map<std::string, std::string> ConfigFile::__value_m;

void ConfigFile::read(const std::string& filename, bool overload)
{
  if (filename.size() == 0) return;
  std::string file_path;
  if (filename.at(0) != '/') {
    const char* path = getenv("BELLE2_LOCAL_DIR");
    if (path == NULL) {
      Belle2::debug("[ERROR] Enveriment varialble : BELLE_LOCAL_DIR");
      exit(1);
    }
    file_path = path;
    if (filename.find("/") == std::string::npos) {
      file_path += "/daq/slc/data/config/" + filename + ".conf";
    } else {
      file_path += "/daq/slc/data/" + filename + ".conf";
    }
  } else {
    file_path = filename;
  }
  std::ifstream fin(file_path.c_str());
  std::string s;
  while (fin && getline(fin, s)) {
    if (s.size() == 0 || s.at(0) == '#') continue;
    std::vector<std::string> str_v = Belle2::split(s, ':');
    if (str_v.size() >= 2) {
      std::string label = Belle2::replace(Belle2::replace(str_v[0], " ", ""), "\t", "");
      std::string value = "";
      size_t i = 0;
      for (; i < str_v[1].size(); i++) {
        if (str_v[1].at(i) == '#' || str_v[1].at(i) == '\n') break;
        if (str_v[1].at(i) == ' ' || str_v[1].at(i) == '\t') continue;
        if (str_v[1].at(i) == '"') {
          for (i++ ; i < str_v[1].size(); i++) {
            if (str_v[1].at(i) == '"') break;
            value += str_v[1].at(i);
          }
          break;
        }
        if (str_v[1].at(i) == '$') {
          i++;
          if (str_v[1].at(i) == '{') {
            for (i++ ; i < str_v[1].size(); i++) {
              if (str_v[1].at(i) == '}') break;
              value += str_v[1].at(i);
            }
          }
          const char* env = getenv(value.c_str());
          if (env != NULL) {
            value = env;
          } else if (__value_m.find(value) != __value_m.end()) {
            value = __value_m[value];
          }
          continue;
        }
        value += str_v[1].at(i);
      }
      if (__value_m.find(label) == __value_m.end()) {
        __value_m.insert(std::map<std::string, std::string>::value_type(label, value));
      } else if (overload) {
        __value_m[label] = value;
      }
    }
  }
  fin.close();
}

void ConfigFile::clear()
{
  __value_m.clear();
}

const std::string ConfigFile::get(const std::string& label)
{
  if (__value_m.find(label) != __value_m.end()) {
    return __value_m[label];
  } else {
    const char* env = getenv(label.c_str());
    if (env != NULL) {
      std::string value = env;
      Belle2::debug("%s %s", label.c_str(), value.c_str());
      __value_m.insert(std::map<std::string, std::string>::value_type(label, value));
      return value;
    }
  }
  return "";
}

int ConfigFile::getInt(const std::string& label)
{
  std::string value = get(label);
  if (value.size() > 0) return atoi(value.c_str());
  else return 0;
}

double ConfigFile::getDouble(const std::string& label)
{
  std::string value = get(label);
  if (value.size() > 0) return atof(value.c_str());
  else return 0;
}
