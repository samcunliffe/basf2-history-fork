#include "nsm/NSMData.h"

#include "nsm/NSMCommunicator.h"

#include "base/StringUtil.h"

#include "nsm/belle2nsm.h"
extern "C" {
#include "nsm/nsmlib2.h"
}

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

void* NSMData::open(NSMCommunicator* comm) throw(NSMHandlerException)
{
  b2nsm_context(comm->getContext());
  if ((_pdata = b2nsm_openmem(_data_name.c_str(), _format.c_str(), _revision))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to open data memory"));
  }
  initProperties();
  return _pdata;
}

void* NSMData::allocate(NSMCommunicator* comm, int interval) throw(NSMHandlerException)
{
  b2nsm_context(comm->getContext());
  if ((_pdata = b2nsm_allocmem(_data_name.c_str(), _format.c_str(), _revision, interval))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to allocate data memory"));
  }
  initProperties();
  return _pdata;
}

void* NSMData::parse(const char* incpath) throw(NSMHandlerException)
{
  if (!nsmlib_parse(_format.c_str(), _revision, incpath)) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to parse header file"));
  }
  int size = initProperties();
  return (_pdata = malloc(size));
}

int NSMData::initProperties() throw()
{
  int size = 0;
  nsm_data_att_t* nsm_data_att_p = nsm_data_att_list;
  while (nsm_data_att_p->length != 0) {
    NSMDataProperty pro;
    int length = nsm_data_att_p->length;
    std::string type = nsm_data_att_p->type;
    std::string label = nsm_data_att_p->label;
    pro.offset = nsm_data_att_p->offset;
    nsm_data_att_p++;
    if (type == "int64") pro.type = INT64;
    else if (type == "int32") pro.type = INT32;
    else if (type == "int16") pro.type = INT16;
    else if (type == "char" && length < 0) pro.type = CHAR;
    else if (type == "char" && length > 0) pro.type = TEXT;
    else if (type == "uint64") pro.type = UINT64;
    else if (type == "uint32") pro.type = UINT32;
    else if (type == "uint16") pro.type = UINT16;
    else if (type == "byte8" || type == "uchar") pro.type = BYTE8;
    else if (type == "double") pro.type = DOUBLE;
    else if (type == "float") pro.type = FLOAT;
    size += (length < 0) ? pro.type % 100 : (pro.type % 100) * length;
    pro.length = length;
    _pro_m.insert(NSMDataPropertyMap::value_type(label, pro));
    _label_v.push_back(label);
  }
  return size;
}

void* NSMData::get() throw(NSMHandlerException)
{
  if (_pdata == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "NSM data is not ready."));
  }
  return _pdata;
}

const void* NSMData::get() const throw(NSMHandlerException)
{
  if (_pdata == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "NSM data is not ready."));
  }
  return _pdata;
}

void* NSMData::getValue(const std::string& label) throw(NSMHandlerException)
{
  char* data = (char*)get();
  return (data + _pro_m[label].offset);
}

void* NSMData::getValueArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  char* data = (char*)get();
  length = _pro_m[label].length;
  return (data + _pro_m[label].offset);
}

void NSMData::setValue(const std::string& label,
                       const void* value, size_t length) throw(NSMHandlerException)
{
  char* data = (char*)get();
  memcpy(data + _pro_m[label].offset, value, length);
}

int64 NSMData::getInt64(const std::string& label) throw(NSMHandlerException)
{
  return *(int64*)getValue(label);
}

int32 NSMData::getInt32(const std::string& label) throw(NSMHandlerException)
{
  return *(int32*)getValue(label);
}

int16 NSMData::getInt16(const std::string& label) throw(NSMHandlerException)
{
  return *(int16*)getValue(label);
}

char NSMData::getChar(const std::string& label) throw(NSMHandlerException)
{
  return *(char*)getValue(label);
}

uint64 NSMData::getUInt64(const std::string& label) throw(NSMHandlerException)
{
  return *(uint64*)getValue(label);
}

uint32 NSMData::getUInt32(const std::string& label) throw(NSMHandlerException)
{
  return *(uint32*)getValue(label);
}

uint16 NSMData::getUInt16(const std::string& label) throw(NSMHandlerException)
{
  return *(uint16*)getValue(label);
}

byte8 NSMData::getByte(const std::string& label) throw(NSMHandlerException)
{
  return *(byte8*)getValue(label);
}

double NSMData::getDouble(const std::string& label) throw(NSMHandlerException)
{
  return *(double*)getValue(label);
}

float NSMData::getFloat(const std::string& label) throw(NSMHandlerException)
{
  return *(float*)getValue(label);
}

int64* NSMData::getInt64Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (int64*)getValueArray(label, length);
}

int32* NSMData::getInt32Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (int32*)getValueArray(label, length);
}

int16* NSMData::getInt16Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (int16*)getValueArray(label, length);
}

char* NSMData::getCharArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (char*)getValueArray(label, length);
}

uint64* NSMData::getUInt64Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (uint64*)getValueArray(label, length);
}

uint32* NSMData::getUInt32Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (uint32*)getValueArray(label, length);
}

uint16* NSMData::getUInt16Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (uint16*)getValueArray(label, length);
}

byte8* NSMData::getByteArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (byte8*)getValueArray(label, length);
}

double* NSMData::getDoubleArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (double*)getValueArray(label, length);
}

float* NSMData::getFloatArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (float*)getValueArray(label, length);
}

void NSMData::setInt64(const std::string& label, int64 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(int64));
}

void NSMData::setInt32(const std::string& label, int32 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(int32));
}

void NSMData::setInt16(const std::string& label, int16 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(int16));
}

void NSMData::setChar(const std::string& label, char value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(char));
}

void NSMData::setUInt64(const std::string& label, uint64 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(uint64));
}

void NSMData::setUInt32(const std::string& label, uint32 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(uint32));
}

void NSMData::setUInt16(const std::string& label, uint16 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(uint16));
}

void NSMData::setByte8(const std::string& label, byte8 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(byte8));
}

void NSMData::setDouble(const std::string& label, double value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(double));
}

void NSMData::setFloat(const std::string& label, float value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(float));
}

const std::string NSMData::toSQLConfig()
{
  std::stringstream ss;
  ss << "record_time timestamp";
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    std::string type_s;
    switch (pro.type) {
      case INT64:  type_s = "bigint"; break;
      case INT32:  type_s = "int"; break;
      case INT16:  type_s = "smallint"; break;
      case CHAR:   type_s = "tinyint"; break;
      case UINT64: type_s = "bigint unsigned"; break;
      case UINT32: type_s = "int unsigned"; break;
      case UINT16: type_s = "smallint unsigned"; break;
      case BYTE8:  type_s = "tinyint unsigned"; break;
      case DOUBLE: type_s = "double"; break;
      case FLOAT:  type_s = "float"; break;
      case TEXT:   type_s = "text"; break;
      default : break;
    }
    if (pro.length < 0 && pro.type == TEXT) {
      ss << ", " << label << " " << type_s;
    } else {
      for (size_t i = 0; i < pro.length; i++) {
        ss << ", `" << label << ":" << i << "` " << type_s;
      }
    }
  }
  return ss.str();
}

const std::string NSMData::toSQLNames()
{
  std::stringstream ss;
  ss << "record_time";
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    if (pro.length < 0 && pro.type == TEXT) {
      ss << ", " << label;
    } else {
      for (size_t i = 0; i < pro.length; i++) {
        ss << ", `" << label << ":" << i << "`";
      }
    }
  }
  return ss.str();
}

const std::string NSMData::toSQLValues()
{
  char* data = (char*)get();
  std::stringstream ss;
  ss << "current_timestamp";
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    int length = pro.length;
    if (length < 0 && pro.type == TEXT) length = 1;
    for (int i = 0; i < length; i++) {
      ss << ", ";
      switch (pro.type) {
        case INT64:  ss << ((int64*)(data + pro.offset))[i]; break;
        case INT32:  ss << ((int32*)(data + pro.offset))[i]; break;
        case INT16:  ss << ((int16*)(data + pro.offset))[i]; break;
        case CHAR:   ss << (int)((char*)(data + pro.offset))[i]; break;
        case UINT64: ss << ((uint64*)(data + pro.offset))[i]; break;
        case UINT32: ss << ((uint32*)(data + pro.offset))[i]; break;
        case UINT16: ss << ((uint16*)(data + pro.offset))[i]; break;
        case BYTE8:  ss << (uint32)((byte8*)(data + pro.offset))[i]; break;
        case DOUBLE: ss << ((double*)(data + pro.offset))[i]; break;
        case FLOAT:  ss << ((float*)(data + pro.offset))[i]; break;
        case TEXT:
          ss << "'" << (const char*)(data + pro.offset) << "'";
          break;
        default : break;
      }
    }
  }
  return ss.str();
}

void NSMData::setSQLValues(std::vector<std::string>& name_v,
                           std::vector<std::string>& value_v)
{
  char* data = (char*)get();
  for (size_t i = 0; i < name_v.size(); i++) {
    std::string name = name_v[i];
    std::vector<std::string> str_v = Belle2::split(name, ':');
    int length = 1;
    if (str_v.size() > 1) {
      name = str_v[0];
      length = atoi(str_v[1].c_str());
    }
    NSMDataProperty& pro(_pro_m[name]);
    for (int l = 0; l < length; l++) {
      switch (pro.type) {
        case INT64: ((int64*)(data + pro.offset))[l] = atoll(value_v[i].c_str()); break;
        case INT32: ((int32*)(data + pro.offset))[l] = atoi(value_v[i].c_str()); break;
        case INT16: ((int16*)(data + pro.offset))[l] = atoi(value_v[i].c_str()); break;
        case CHAR: ((char*)(data + pro.offset))[l] = (char)atoi(value_v[i].c_str()); break;
        case UINT64: ((uint64*)(data + pro.offset))[l] = strtoll(value_v[i].c_str(), NULL, 0); break;
        case UINT32: ((uint32*)(data + pro.offset))[l] = strtol(value_v[i].c_str(), NULL, 0); break;
        case UINT16: ((uint16*)(data + pro.offset))[l] = strtol(value_v[i].c_str(), NULL, 0); break;
        case BYTE8: ((byte8*)(data + pro.offset))[l] = (byte8)atoi(value_v[i].c_str()); break;
        case DOUBLE: ((double*)(data + pro.offset))[l] = atof(value_v[i].c_str()); break;
        case FLOAT: ((float*)(data + pro.offset))[l] = atof(value_v[i].c_str()); break;
        case TEXT:   strncpy((char*)(data + pro.offset), value_v[i].c_str(), pro.length); break;
        default : break;
      }
    }
  }
}

void NSMData::getSQLValues(std::vector<std::string>& name_v,
                           std::vector<std::string>& value_v)
{
  char* data = (char*)get();
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    int length = pro.length;
    if (length < 0 && pro.type == TEXT) length = 1;
    for (int i = 0; i < length; i++) {
      std::stringstream ss;
      switch (pro.type) {
        case INT64:  ss << ((int64*)(data + pro.offset))[i];  break;
        case INT32:  ss << ((int32*)(data + pro.offset))[i];  break;
        case INT16:  ss << ((int16*)(data + pro.offset))[i];  break;
        case CHAR:   ss << ((char*)(data + pro.offset))[i];   break;
        case UINT64: ss << ((uint64*)(data + pro.offset))[i]; break;
        case UINT32: ss << ((uint32*)(data + pro.offset))[i]; break;
        case UINT16: ss << ((uint16*)(data + pro.offset))[i]; break;
        case BYTE8:  ss << ((byte8*)(data + pro.offset))[i];  break;
        case DOUBLE: ss << ((double*)(data + pro.offset))[i]; break;
        case FLOAT:  ss << ((float*)(data + pro.offset))[i];  break;
        case TEXT:   ss << (char*)(data + pro.offset);        break;
        default : break;
      }
      value_v.push_back(ss.str());
    }
  }
}

DataObject* NSMData::createDataObject()
{
  DataObject* obj = new DataObject();
  char* data = (char*)get();
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    std::stringstream ss;
    DataObject::ParamInfo pinfo = { DataObject::INT, 0, NULL };
    pinfo.buf = data + pro.offset;
    pinfo.length = (pro.length < 0) ? 0 : pro.length;
    switch (pro.type) {
      case INT64:  pinfo.type = DataObject::LONG;   break;
      case INT32:  pinfo.type = DataObject::INT;    break;
      case INT16:  pinfo.type = DataObject::SHORT;  break;
      case CHAR:   pinfo.type = DataObject::CHAR;   break;
      case UINT64: pinfo.type = DataObject::ULONG;  break;
      case UINT32: pinfo.type = DataObject::UINT;   break;
      case UINT16: pinfo.type = DataObject::USHORT; break;
      case BYTE8:  pinfo.type = DataObject::UCHAR;  break;
      case DOUBLE: pinfo.type = DataObject::DOUBLE; break;
      case FLOAT:  pinfo.type = DataObject::FLOAT;  break;
      case TEXT:   pinfo.type = DataObject::TEXT;   break;
      default : break;
    }
    obj->add(label, pinfo.buf, pinfo.type, pinfo.length);
  }
  return obj;
}

const std::string NSMData::toXML()
{
  std::stringstream ss;
  ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
     << "<object class=\"" << _data_name
     << "\" revision=\"" << _revision << "\" >" << std::endl;
  char* data = (char*)get();
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    switch (pro.type) {
      case INT64:  ss << "<int name=\"" << label
                        << "\" value=\"" << *(int64*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case INT32:  ss << "<int name=\"" << label
                        << "\" value=\"" << *(int32*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case INT16:  ss << "<short name=\"" << label
                        << "\" value=\"" << *(int16*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case CHAR:   ss << "<int name=\"" << label
                        << "\" value=\"" << *(char*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case UINT64: ss << "<int name=\"" << label
                        << "\" value=\"" << *(int64*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case UINT32: ss << "<int name=\"" << label
                        << "\" value=\"" << *(int32*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case UINT16: ss << "<short name=\"" << label
                        << "\" value=\"" << *(int16*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case BYTE8:  ss << "<byte name=\"" << label
                        << "\" value=\"" << *(byte8*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case DOUBLE: ss << "<double name=\"" << label
                        << "\" value=\"" << *(double*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case FLOAT:  ss << "<float name=\"" << label
                        << "\" value=\"" << *(float*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case TEXT:   ss << "<text name=\"" << label
                        << "\" value=\"" << (char*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      default : break;
    }
  }
  return ss.str();
}

