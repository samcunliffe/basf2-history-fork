#include "daq/slc/nsm/NSMData.h"

#include "daq/slc/nsm/NSMCommunicator.h"
#include "daq/slc/nsm/NSMCallback.h"

#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Reader.h>

#include <nsm2/belle2nsm.h>
extern "C" {
#include <nsm2/nsmlib2.h>
}

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

NSMData::NSMData(const std::string& dataname,
                 const std::string& format, int revision)
throw() : DBObject(), m_allocated(false),
  m_pdata(NULL), m_size(0), m_offset(0)
{
  setName(dataname);
  setFormat(format);
  setRevision(revision);
}

NSMData::NSMData()
throw() : DBObject(), m_allocated(false),
  m_pdata(NULL), m_size(0), m_offset(0)
{
  setRevision(-1);
}

NSMData::NSMData(const NSMData& data) throw()
  : DBObject(data)
{
  m_allocated = data.m_allocated;
  m_size = data.m_size;
  setFormat(data.getFormat());
  setRevision(data.getRevision());
  setIndex(data.getIndex());
  setName(data.getName());
  m_pdata = NULL;
  if (m_allocated) {
    m_pdata = malloc(m_size);
    memcpy(m_pdata, data.m_pdata, m_size);
  } else {
    if (data.m_pdata != NULL) m_pdata = data.m_pdata;
  }
  const FieldNameList& name_v(data.getFieldNames());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    const DBField::Property& pro(data.getProperty(name));
    add(name, pro);
    if (pro.getType() == DBField::OBJECT) {
      NSMDataList data_v(data.getObjects(name));
      for (size_t i = 0; i < data_v.size(); i++) {
        NSMData& cdata(data_v[i]);
        cdata.m_pdata = (void*)((char*)m_pdata +
                                pro.getOffset() + cdata.m_size * i);
      }
      m_data_v_m.insert(NSMDataListMap::value_type(name, data_v));
    }
  }
}

NSMData::~NSMData() throw()
{
  reset();
  if (m_allocated && m_pdata != NULL) {
    free(m_pdata);
    m_pdata = NULL;
  }
}

void NSMData::reset() throw()
{
  DBObject::reset();
  if (m_allocated && m_pdata != NULL) {
    free(m_pdata);
    m_pdata = NULL;
  }
}

void NSMData::addValue(const std::string& name, const void* data,
                       DBField::Type type, int length) throw()
{
  DBField::Property pro(type, length, m_offset);
  if (length == 0) length = 1;
  int size = pro.getTypeSize() * length;
  if (size <= 0) return;
  if (!hasField(name)) {
    add(name, pro);
    m_offset += size;
  }
  if (data != NULL) {
    memcpy((char*)m_pdata + pro.getOffset(), data, size);
  }
}

void NSMData::setValue(const std::string& name, const void* data,
                       int length) throw()
{
  const DBField::Property& pro(getProperty(name));
  if (length == 0) length = 1;
  int size = pro.getTypeSize() * length;
  if (data != NULL && hasField(name) && size > 0) {
    memcpy((char*)m_pdata + pro.getOffset(), data, size);
  }
}

void* NSMData::open(NSMCommunicator& com) throw(NSMHandlerException)
{
  if (m_pdata == NULL) {
    b2nsm_context(com.getContext());
    if (getenv("NSM2_INCDIR") == NULL) {
      ConfigFile file("slowcontrol");
      setenv("NSM2_INCDIR", file.get("nsm.data.incpath").c_str(), 0);
    }
    if ((m_pdata = b2nsm_openmem(getName().c_str(), getFormat().c_str(),
                                 getRevision())) == NULL) {
      throw (NSMHandlerException("Failed to open data memory %s",
                                 nsmlib_strerror(com.getContext())));
    }
    parse();
  }
  return m_pdata;
}

void* NSMData::allocate(NSMCommunicator& com, int interval) throw(NSMHandlerException)
{
  if (m_pdata == NULL) {
    b2nsm_context(com.getContext());
    if (getenv("NSM2_INCDIR") == NULL) {
      ConfigFile file("slowcontrol");
      setenv("NSM2_INCDIR", file.get("nsm.data.incpath").c_str(), 0);
    }
    if ((m_pdata = b2nsm_allocmem(getName().c_str(), getFormat().c_str(),
                                  getRevision(), interval)) == NULL) {
      throw (NSMHandlerException("Failed to allocate data memory %s",
                                 nsmlib_strerror(com.getContext())));
    }
    parse();
    memset(m_pdata, 0, m_size);
    m_com = &com;
    m_tstamp = 0;
  }
  com.getCallback().add(new NSMVHandlerText("nsmdata.name", true, false, getName()));
  com.getCallback().add(new NSMVHandlerText("nsmdata.format", true, false, getFormat()));
  com.getCallback().add(new NSMVHandlerInt("nsmdata.revision", true, false, getRevision()));
  com.getCallback().add(new NSMVHandlerInt("nsmdata.tstamp", true, false, m_tstamp));
  return m_pdata;
}

void* NSMData::parse(const char* incpath, bool malloc_new)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  if (getenv("NSM2_INCDIR") == NULL) {
    ConfigFile file("slowcontrol");
    setenv("NSM2_INCDIR", file.get("nsm.data.incpath").c_str(), 0);
  }
  NSMparse* ptr = NULL;
  char fmtstr[256];
  int revision = 0;
  if ((ptr = (NSMparse*)nsmlib_parsefile(getFormat().c_str(), getRevision(),
                                         incpath, fmtstr, &revision)) == NULL) {
    throw (NSMHandlerException("Failed to parse header file (%s:%d) : %s",
                               getFormat().c_str(), getRevision(), nsmlib_parseerr(NULL)));
  }
  if (getRevision() != revision && revision > 0) {
    setRevision(revision);
  }
  int length;
  std::string name_in;
  parse(ptr, length, name_in);
  if (m_size > 0 && malloc_new) {
    m_allocated = true;
    return (m_pdata = malloc(m_size));
  }
#else
  throw (NSMHandlerException("too old nsmlib (nsmparse) : version = %d", NSM_PACKAGE_VERSION));
#endif
  return NULL;
}

void NSMData::flush() throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  m_tstamp = Time().getSecond();
  sleep(1);
  m_com->getCallback().set("nsmdata.tstamp", m_tstamp);
#endif
}

#if NSM_PACKAGE_VERSION >= 1914
NSMparse* NSMData::parse(NSMparse* ptr, int& length,
                         std::string& name_in) throw(NSMHandlerException)
{
  m_size = 0;
  while (ptr != NULL) {
    length = ptr->size;
    if (length < 0) length = 0;
    int type = ptr->type;
    std::string name = ptr->name;
    int offset = ptr->offset;
    m_offset = offset;
    if (type == 'l') type = DBField::LONG;//INT64;
    else if (type == 'i') type = DBField::INT;//INT32;
    else if (type == 's') type = DBField::SHORT;//INT16;
    else if (type == 'c') type = DBField::CHAR;//CHAR;
    else if (type == 'L') type = DBField::LONG;//UINT64;
    else if (type == 'I') type = DBField::INT;//UINT32;
    else if (type == 'S') type = DBField::SHORT;//UINT16;
    else if (type == 'C') type = DBField::CHAR;//BYTE8;
    else if (type == 'd') type = DBField::DOUBLE;
    else if (type == 'f') type = DBField::FLOAT;
    else if (type == '(') {
      NSMData data(getName(), getFormat() + "." + name, getRevision());
      data.m_pdata = (void*)((char*)m_pdata + offset);
      ptr = data.parse(ptr->next, length, name);
      data.setFormat(getFormat() + "." + name);
      int len = (length == 0) ? 1 : length;
      NSMDataList data_v;
      for (int i = 0; i < len; i++) {
        NSMData cdata(data);
        cdata.setIndex(i);
        cdata.m_pdata = (void*)((char*)data.get() + i * data.getSize());
        data_v.push_back(cdata);
      }
      m_data_v_m.insert(NSMDataListMap::value_type(name, data_v));
      type = DBField::OBJECT;
      m_size += data.m_size * length;
    } else if (type == ')') {
      name_in = name;
      return ptr;
    }
    DBField::Property pro((DBField::Type)type, length, offset);
    add(name, pro);
    int len = (length == 0) ? 1 : length;
    m_size += pro.getTypeSize() * len;
    if (ptr == NULL) break;
    ptr = ptr->next;
  }
  return 0;
}
#endif

void* NSMData::getValue(const std::string& name) throw(std::out_of_range)
{
  if (!hasValue(name)) return NULL;
  char* data = (char*)get();
  return (data + getProperty(name).getOffset());
}

const void* NSMData::getValue(const std::string& name)
const throw(std::out_of_range)
{
  if (!hasValue(name)) return NULL;
  char* data = (char*)get();
  return (data + getProperty(name).getOffset());
}

void NSMData::readObject(Reader& reader) throw(IOException)
{
  setName(reader.readString());
  setFormat(reader.readString());
  setRevision(reader.readInt());
  int size = reader.readInt();
  if (m_size > 0 && m_size < size) {
    m_allocated = true;
    free(m_pdata);
    m_pdata = malloc(size);
  }
  setSize(size);
  int npars = reader.readInt();
  for (int n = 0; n < npars; n++) {
    std::string name = reader.readString();
    DBField::Type type = (DBField::Type)reader.readInt();
    size_t length = reader.readInt();
    size_t offset = reader.readInt();
    if (!hasValue(name)) add(name, DBField::Property(type, length, offset));
    void* buf = getValue(name);
    if (length == 0) length = 1;
    if (type == DBField::OBJECT) {
      NSMDataList data_v;
      for (size_t i = 0; i < length; i++) {
        NSMData data;
        data.m_pdata = (char*)getValue(name) + length * i;
        reader.readObject(data);
        data_v.push_back(data);
      }
      m_data_v_m.insert(NSMDataListMap::value_type(name, data_v));
    } else {
      for (size_t i = 0; i < length; i++) {
        switch (type) {
          case DBField::CHAR: ((char*)buf)[i] = reader.readChar(); break;
          case DBField::SHORT: ((int16*)buf)[i] = reader.readShort(); break;
          case DBField::INT: ((int32*)buf)[i] = reader.readInt(); break;
          case DBField::LONG: ((int64*)buf)[i] = reader.readLong(); break;
          case DBField::FLOAT: ((float*)buf)[i] = reader.readFloat(); break;
          case DBField::DOUBLE: ((double*)buf)[i] = reader.readDouble(); break;
          default: break;
        }
      }
    }
  }
}

void NSMData::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeString(getName());
  writer.writeString(getFormat());
  writer.writeInt(getRevision());
  writer.writeInt(getSize());
  const FieldNameList& name_v(getFieldNames());
  writer.writeInt(name_v.size());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    writer.writeString(name);
    const DBField::Property& pro(getProperty(name));
    writer.writeInt(pro.getType());
    writer.writeInt(pro.getLength());
    writer.writeInt(pro.getOffset());
    size_t length = pro.getLength();
    const void* buf = getValue(name);
    if (length == 0) length = 1;
    if (pro.getType() == DBField::OBJECT) {
      const NSMDataList& data_v(getObjects(name));
      for (size_t i = 0; i < length; i++) {
        writer.writeObject(data_v[i]);
      }
    } else {
      for (size_t i = 0; i < length; i++) {
        switch (pro.getType()) {
          case DBField::CHAR:   writer.writeChar(((char*)buf)[i]); break;
          case DBField::SHORT:  writer.writeShort(((int16*)buf)[i]); break;
          case DBField::INT:    writer.writeInt(((int64*)buf)[i]); break;
          case DBField::LONG:   writer.writeLong(((int32*)buf)[i]); break;
          case DBField::FLOAT:  writer.writeFloat(((float*)buf)[i]); break;
          case DBField::DOUBLE: writer.writeDouble(((double*)buf)[i]); break;
          default : break;
        }
      }
    }
  }
}

void NSMData::print(const std::string& name_in) const throw()
{
  const FieldNameList& name_v(getFieldNames());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    const DBField::Property& pro(getProperty(name));
    size_t length = pro.getLength();
    std::string name_out = name_in;
    if (name_in.size() > 0) name_out += ".";
    name_out += name;
    std::string pvtype;
    if (length > 0) {
      if (pro.getType() == DBField::OBJECT) {
        const NSMDataList& data_v(getObjects(name));
        for (size_t i = 0; i < length; i++) {
          data_v[i].print(StringUtil::form("%s[%d]", name_out.c_str(), i));
        }
      } else {
        const void* buf = getValue(name);
        const char* name_c = name_out.c_str();
        for (int i = 0; i < (int)length; i++) {
          switch (pro.getType()) {
            case DBField::CHAR:
              printf("%s[%d] : char(%d)\n", name_c, i, ((char*)buf)[i]); break;
            case DBField::SHORT:
              printf("%s[%d] : short(%d)\n", name_c, i, ((int16*)buf)[i]); break;
            case DBField::INT:
              printf("%s[%d] : int(%d)\n", name_c, i, ((int32*)buf)[i]); break;
            case DBField::LONG:
              printf("%s[%d] : long(%ld)\n", name_c, i, ((int64*)buf)[i]); break;
            case DBField::FLOAT:
              printf("%s[%d] : float(%f)\n", name_c, i, ((float*)buf)[i]); break;
            case DBField::DOUBLE:
              printf("%s[%d] : double(%f)\n", name_c, i, ((double*)buf)[i]); break;
            default : break;
          }
        }
      }
    } else {
      if (pro.getType() == DBField::OBJECT) {
        const NSMDataList& data_v(getObjects(name));
        data_v[0].print(name_out);
      } else {
        const void* buf = getValue(name);
        const char* name_c = name_out.c_str();
        switch (pro.getType()) {
          case DBField::CHAR:
            printf("%s : char(%d)\n", name_c, *(char*)buf); break;
          case DBField::SHORT:
            printf("%s : short(%d)\n", name_c, *(int16*)buf); break;
          case DBField::INT:
            printf("%s : int(%d)\n", name_c, *(int32*)buf); break;
          case DBField::LONG:
            printf("%s : long(%ld)\n", name_c, *(int64*)buf); break;
          case DBField::FLOAT:
            printf("%s : float(%f)\n", name_c, *(float*)buf); break;
          case DBField::DOUBLE:
            printf("%s : double(%f)\n", name_c, *(double*)buf); break;
          default : break;
        }
      }
    }
  }
}

void NSMData::printPV(const std::string& name_in) const throw()
{
  const FieldNameList& name_v(getFieldNames());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    const DBField::Property& pro(getProperty(name));
    size_t length = pro.getLength();
    std::string name_out = name_in;
    if (name_in.size() > 0) name_out += ":";
    else {
      name_out += (getName() + ":") + (getFormat() + ":");
    }
    name_out += name;
    std::string pvtype;
    switch (pro.getType()) {
      case DBField::CHAR:
      case DBField::SHORT:
      case DBField::INT:
      case DBField::LONG:   pvtype = "longin"; break;
      case DBField::FLOAT:
      case DBField::DOUBLE: pvtype = "ai"; break;
      default : break;
    }
    if (length > 0) {
      if (pro.getType() == DBField::OBJECT) {
        const NSMDataList& data_v(getObjects(name));
        for (size_t i = 0; i < length; i++) {
          data_v[i].printPV(StringUtil::form("%s[%d]", name_out.c_str(), i));
        }
      } else {
        for (size_t i = 0; i < length; i++) {
          printf("record(%s, \"nsm2:%s[%d]\")\n"
                 "{\n"
                 "  field(SCAN, \".1 second\")\n"
                 "  field(DTYP, \"nsm2_data_%s\")\n"
                 "}\n\n", pvtype.c_str(), name_out.c_str(), (int)i, pvtype.c_str());
        }
      }
    } else {
      if (pro.getType() == DBField::OBJECT) {
        const NSMDataList& data_v(getObjects(name));
        data_v[0].print(name_out);
      } else {
        printf("record(%s, \"nsm2:%s\")\n"
               "{\n"
               "  field(SCAN, \".1 second\")\n"
               "  field(DTYP, \"nsm2_data_%s\")\n"
               "}\n\n", pvtype.c_str(), name_out.c_str(), pvtype.c_str());
      }
    }
  }
}

const void* NSMData::find(const std::string& name_in, DBField::Type& type)
const throw()
{
  size_t pos;
  std::string name_out = name_in;
  if ((pos = name_in.find(".")) != std::string::npos)  {
    StringList str = StringUtil::split(name_out, '.');
    StringList sstr = StringUtil::split(str[0], '[');
    const NSMDataList& data_v(getObjects(sstr[0]));
    int index = 0;
    if (sstr.size() > 1) {
      index = atoi(sstr[1].c_str());
    }
    name_out = name_in.substr(pos + 1);
    return data_v[index].find(name_out, type);
  }
  int index = 0;
  if ((pos = name_out.find("[")) != std::string::npos)  {
    StringList str = StringUtil::split(name_out, '[');
    name_out = str[0];
    index = atoi(str[1].c_str());
  }
  if (!hasValue(name_out)) return NULL;
  const DBField::Property& pro(getProperty(name_out));
  type = pro.getType();
  const void* buf = getValue(name_out);
  switch (type) {
    case DBField::CHAR: return (((const char*)buf) + index);
    case DBField::SHORT: return (((const int16*)buf) + index);
    case DBField::INT: return (((const int32*)buf) + index);
    case DBField::LONG: return (((const int64*)buf) + index);
    case DBField::FLOAT: return (((const float*)buf) + index);
    case DBField::DOUBLE: return (((const double*)buf) + index);
    default: return NULL;
  }
  return NULL;
}

int NSMData::getNObjects(const std::string& name) const throw()
{
  NSMDataListMap::const_iterator it = m_data_v_m.find(name);
  if (it != m_data_v_m.end()) return it->second.size();
  return 0;
}

const NSMData& NSMData::getObject(const std::string& name, int index) const throw(std::out_of_range)
{
  NSMDataListMap::const_iterator it = m_data_v_m.find(name);
  if (it != m_data_v_m.end()) return it->second[index];
  else throw (std::out_of_range(StringUtil::form("%s:%d", __FILE__, __LINE__)));
}

NSMData& NSMData::getObject(const std::string& name, int index) throw(std::out_of_range)
{
  NSMDataListMap::iterator it = m_data_v_m.find(name);
  if (it != m_data_v_m.end()) return it->second[index];
  else throw (std::out_of_range(StringUtil::form("%s:%d", __FILE__, __LINE__)));
}

const NSMData::NSMDataList& NSMData::getObjects(const std::string& name) const throw(std::out_of_range)
{
  NSMDataListMap::const_iterator it = m_data_v_m.find(name);
  if (it != m_data_v_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s:%d", __FILE__, __LINE__)));
}

NSMData::NSMDataList& NSMData::getObjects(const std::string& name) throw(std::out_of_range)
{
  NSMDataListMap::iterator it = m_data_v_m.find(name);
  if (it != m_data_v_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s:%d", __FILE__, __LINE__)));
}

