#include "daq/slc/database/DBObject.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Date.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

DBObject::DBObject() : m_index(0)
{
  m_id = 0;
  m_isconfig = true;
}

DBObject::DBObject(const DBObject& obj)
  : m_index(obj.m_index), m_id(obj.m_id),
    m_revision(obj.m_revision), m_name(obj.m_name),
    m_node(obj.m_node), m_table(obj.m_table),
    m_isconfig(obj.m_isconfig) {}

DBObject::~DBObject() throw()
{
  reset();
}

void DBObject::print() const throw()
{
  const FieldNameList& name_v(getFieldNames());
  std::cout << "name : '" << getName() << "'" << std::endl;
  for (size_t ii = 0; ii < m_name_v.size(); ii++) {
    std::string name = name_v.at(ii);
    std::cout << name << " : ";
    FieldInfo::Type type = getProperty(name).getType();
    switch (type) {
      case FieldInfo::BOOL:   std::cout << getBool(name); break;
      case FieldInfo::CHAR:   std::cout << getChar(name); break;
      case FieldInfo::SHORT:  std::cout << getShort(name); break;
      case FieldInfo::INT:    std::cout << getInt(name); break;
      case FieldInfo::LONG:   std::cout << getLong(name); break;
      case FieldInfo::FLOAT:  std::cout << getFloat(name); break;
      case FieldInfo::DOUBLE: std::cout << getDouble(name); break;
      case FieldInfo::TEXT:   std::cout << getText(name); break;
      case FieldInfo::OBJECT: {
        size_t nobj = getNObjects(name);
        if (nobj > 0) {
          const DBObject& obj(getObject(name));
          std::cout << obj.getTable() << " (" << obj.getRevision()
                    << ")" << std::endl;
          std::cout << "-------------------------" << std::endl;
          for (size_t i = 0; i < nobj; i++) {
            std::cout << "index : " << i << std::endl;
            getObject(name, i).print();
            std::cout << "-------------------------" << std::endl;
          }
        }
        break;
      }
      case FieldInfo::NSM_CHAR:   std::cout << getChar(name); break;
      case FieldInfo::NSM_INT16:  std::cout << getShort(name); break;
      case FieldInfo::NSM_INT32:  std::cout << getInt(name); break;
      case FieldInfo::NSM_INT64:  std::cout << getLong(name); break;
      case FieldInfo::NSM_BYTE8:  std::cout << getUChar(name); break;
      case FieldInfo::NSM_UINT16: std::cout << getUShort(name); break;
      case FieldInfo::NSM_UINT32: std::cout << getUInt(name); break;
      case FieldInfo::NSM_UINT64: std::cout << getULong(name); break;
      case FieldInfo::NSM_FLOAT:  std::cout << getFloat(name); break;
      case FieldInfo::NSM_DOUBLE: std::cout << getDouble(name); break;
      case FieldInfo::NSM_OBJECT: {
        size_t nobj = getNObjects(name);
        std::cout << std::endl;
        std::cout << "-------------------------" << std::endl;
        for (size_t i = 0; i < nobj; i++) {
          getObject(name, i).print();
          std::cout << "-------------------------" << std::endl;
        }
        break;
      }
    }
    if (!hasObject(name)) std::cout << std::endl;
  }
}

void DBObject::reset() throw()
{
  for (size_t ii = 0; ii < m_name_v.size(); ii++) {
    const std::string& name(m_name_v[ii]);
    if (hasObject(name)) {
      size_t nobj = getNObjects(name);
      for (size_t i = 0; i < nobj; i++) getObject(name, i).reset();
    }
  }
  m_index = 0;
  m_name_v = FieldNameList();
  m_pro_m = FieldPropertyList();
}

FieldInfo::Property DBObject::getProperty(const std::string& name) const throw()
{
  return m_pro_m[name];
}

size_t DBObject::getLength(const std::string& name) const throw()
{
  return getProperty(name).getLength();
}

bool DBObject::hasField(const std::string& name) const throw()
{
  return m_pro_m.find(name) != m_pro_m.end();
}

bool DBObject::hasArray(const std::string& name) const throw()
{
  return hasField(name) && !hasObject(name) && !hasText(name);
}

bool DBObject::hasValue(const std::string& name) const throw()
{
  return hasField(name) &&
         m_pro_m[name].getType() != FieldInfo::TEXT &&
         m_pro_m[name].getType() != FieldInfo::OBJECT &&
         m_pro_m[name].getType() != FieldInfo::NSM_OBJECT;
}

bool DBObject::hasText(const std::string& name) const throw()
{
  return hasField(name) && m_pro_m[name].getType() == FieldInfo::TEXT;
}

bool DBObject::hasObject(const std::string& name, int index) const throw()
{
  return hasField(name) && (m_pro_m[name].getType() == FieldInfo::OBJECT ||
                            m_pro_m[name].getType() == FieldInfo::NSM_OBJECT) &&
         (index < 0 || index < getNObjects(name));
}

bool DBObject::getBool(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return false;
  return ((bool*)value)[index];
}

char DBObject::getChar(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((char*)value)[index];
}

short DBObject::getShort(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((short*)value)[index];
}

int DBObject::getInt(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((int*)value)[index];
}

long long DBObject::getLong(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((long long*)value)[index];
}

unsigned char DBObject::getUChar(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((unsigned char*)value)[index];
}

unsigned short DBObject::getUShort(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((unsigned short*)value)[index];
}

unsigned int DBObject::getUInt(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((unsigned int*)value)[index];
}

unsigned long long DBObject::getULong(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((unsigned long long*)value)[index];
}

float DBObject::getFloat(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((float*)value)[index];
}

double DBObject::getDouble(const std::string& name, int index) const throw()
{
  const void* value = getValue(name);
  if (value == NULL && getLength(name)) return 0;
  return ((double*)value)[index];
}

void DBObject::setBool(const std::string& name, bool value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setChar(const std::string& name, int value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setShort(const std::string& name, int value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setInt(const std::string& name, int value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setLong(const std::string& name, long long value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setUChar(const std::string& name, unsigned int value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setUShort(const std::string& name, unsigned int value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setUInt(const std::string& name, unsigned int value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setULong(const std::string& name, unsigned long long value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setFloat(const std::string& name, float value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::setDouble(const std::string& name, double value, int index) throw()
{
  setValue(name, &value, index);
}

void DBObject::add(const std::string& name, FieldInfo::Property pro) throw()
{
  if (!hasField(name)) {
    m_name_v.push_back(name);
    m_pro_m.insert(FieldPropertyList::value_type(name, pro));
  }
}

const std::string DBObject::getValueText(const std::string& name)
const throw()
{
  if (hasField(name)) {
    switch (getProperty(name).getType()) {
      case FieldInfo::BOOL:   return getBool(name) ? "true" : "false";
      case FieldInfo::CHAR:   return StringUtil::form("%d", (int)getChar(name));
      case FieldInfo::SHORT:  return StringUtil::form("%d", (int)getShort(name));
      case FieldInfo::INT:    return StringUtil::form("%d", (int)getInt(name));
      case FieldInfo::LONG:   return StringUtil::form("%ld", getLong(name));
      case FieldInfo::FLOAT:  return StringUtil::form("%f", getFloat(name));
      case FieldInfo::DOUBLE: return StringUtil::form("%f", getDouble(name));
      case FieldInfo::NSM_CHAR:   return StringUtil::form("%d", (int)getChar(name));
      case FieldInfo::NSM_INT16:  return StringUtil::form("%d", (int)getShort(name));
      case FieldInfo::NSM_INT32:  return StringUtil::form("%d", (int)getInt(name));
      case FieldInfo::NSM_INT64:  return StringUtil::form("%ld", getLong(name));
      case FieldInfo::NSM_BYTE8:  return StringUtil::form("%d", (int)getUChar(name));
      case FieldInfo::NSM_UINT16: return StringUtil::form("%d", (int)getUShort(name));
      case FieldInfo::NSM_UINT32: return StringUtil::form("%u", getUInt(name));
      case FieldInfo::NSM_UINT64: return StringUtil::form("%lu", getULong(name));
      case FieldInfo::NSM_FLOAT:  return StringUtil::form("%f", getFloat(name));
      case FieldInfo::NSM_DOUBLE: return StringUtil::form("%f", getDouble(name));
      default: break;
    }
  }
  return "";
}

void DBObject::setValueText(const std::string& name,
                            const std::string& value) throw()
{
  if (hasField(name)) {
    switch (getProperty(name).getType()) {
      case FieldInfo::BOOL:       setBool(name, value == "true" || value == "t"); break;
      case FieldInfo::NSM_CHAR:
      case FieldInfo::CHAR:       setChar(name, (char)atoi(value.c_str())); break;
      case FieldInfo::NSM_INT16:
      case FieldInfo::SHORT:      setShort(name, (short)atoi(value.c_str())); break;
      case FieldInfo::NSM_INT32:
      case FieldInfo::INT:        setInt(name, (int)atoi(value.c_str())); break;
      case FieldInfo::NSM_INT64:
      case FieldInfo::LONG:       setLong(name, (long long)atoi(value.c_str())); break;
      case FieldInfo::NSM_FLOAT:
      case FieldInfo::FLOAT:      setFloat(name, atof(value.c_str())); break;
      case FieldInfo::NSM_DOUBLE:
      case FieldInfo::DOUBLE:     setDouble(name, atof(value.c_str())); break;
      case FieldInfo::NSM_BYTE8:  setUChar(name, strtoul(value.c_str(), NULL, 0)); break;
      case FieldInfo::NSM_UINT16: setUShort(name, strtoul(value.c_str(), NULL, 0)); break;
      case FieldInfo::NSM_UINT32: setUInt(name, strtoul(value.c_str(), NULL, 0)); break;
      case FieldInfo::NSM_UINT64: setULong(name, strtoul(value.c_str(), NULL, 0)); break;
      case FieldInfo::TEXT:       setText(name, value); break;
      default: break;
    }
  }
}

