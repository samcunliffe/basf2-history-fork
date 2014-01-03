#include "daq/storage/modules/DataStorePackage.h"

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"

#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <TClonesArray.h>
#include <TClass.h>

#include <cstring>

using namespace Belle2;

Mutex DataStorePackage::g_mutex;
bool DataStorePackage::g_init = false;

void DataStorePackage::decode(MsgHandler& msghandler, BinData& data)
{
  if (sizeof(m_buf) < data.getByteSize()) {
    B2ERROR(__FILE__ << ":" << __LINE__ << " Too large data size " << data.getByteSize());
    return ;
  }
  memcpy(m_buf, data.getBuffer(), data.getByteSize());
  m_data.setBuffer(m_buf);
  m_data_hlt.setBuffer(m_data.getBody());
  bool contains_sub = true;
  if (m_data_hlt.getTrailerMagic() != BinData::TRAILER_MAGIC) {
    m_data_hlt.setBuffer(m_data.getBuffer());
    contains_sub = false;
  }
  m_objlist = std::vector<TObject*>();
  m_namelist = std::vector<std::string>();
  EvtMessage* msg = new EvtMessage((char*)m_data_hlt.getBody());
  msghandler.decode_msg(msg, m_objlist, m_namelist);
  m_nobjs = (msg->header())->reserved[1];
  m_narrays = (msg->header())->reserved[2];
  m_durability = (DataStore::EDurability)(msg->header())->reserved[0];
  if (contains_sub && m_data.getBodyByteSize() > m_data_hlt.getByteSize()) {
    m_data_pxd.setBuffer(m_data.getBody() + m_data_hlt.getWordSize());
  } else {
    m_data_pxd.setBuffer(NULL);
  }
  delete msg;
}

void DataStorePackage::restore()
{
  g_mutex.lock();
  bool is_array = false;
  for (int i = 0; i < m_nobjs + m_narrays; i++) {
    if (m_objlist.at(i) != NULL) {
      is_array = (dynamic_cast<TClonesArray*>(m_objlist.at(i)) != 0);
      TObject* obj = m_objlist.at(i);
      const TClass* cl = obj->IsA();
      if (is_array)
        cl = static_cast<TClonesArray*>(obj)->GetClass();
      if (!g_init) {
        bool is_transient = obj->TestBit(c_IsTransient);
        DataStore::Instance().createEntry(m_namelist.at(i), m_durability, cl, is_array, is_transient, false);
      }
      bool is_null = !obj->TestBit(c_IsNull);
      if (is_null) {
        DataStore::Instance().createObject(obj, true,
                                           StoreAccessorBase(m_namelist.at(i), m_durability, cl, is_array));
        B2DEBUG(100, "restoreDS: " << (is_array ? "Array" : "Object") << ": " << m_namelist.at(i) << " stored");

        obj->SetBit(c_IsTransient, false);
        obj->SetBit(c_IsNull, false);
      } else {
        delete obj;
      }
    } else {
      B2ERROR("restoreDS: " << (is_array ? "Array" : "Object") << ": " << m_namelist.at(i) << " is NULL!");
    }
  }
  if (!g_init) {
    g_init = true;
  }
  if (m_data_pxd.getBuffer() != NULL) {
    StoreArray<RawPXD> rawpxdary;
    rawpxdary.appendNew(RawPXD((int*)m_data_pxd.getBody(), m_data_pxd.getByteSize()));
  }
  g_mutex.unlock();
}

void DataStorePackage::copy(DataStorePackage& package)
{
  m_serial = package.m_serial;
  memcpy(m_buf, package.m_buf, package.m_data.getByteSize());
  m_data.setBuffer(m_buf);
  m_data_hlt.setBuffer(m_data.getBody());
  bool contains_sub = true;
  if (m_data_hlt.getTrailerMagic() != BinData::TRAILER_MAGIC) {
    m_data_hlt.setBuffer(m_data.getBuffer());
    contains_sub = false;
  }
  m_objlist = package.m_objlist;
  m_namelist = package.m_namelist;
  m_nobjs = package.m_nobjs;
  m_narrays = package.m_narrays;
  m_durability = package.m_durability;
  if (contains_sub && package.m_data_pxd.getBuffer() != NULL) {
    int nword = package.m_data_hlt.getWordSize();
    m_data_pxd.setBuffer(m_data.getBody() + nword);
  } else {
    m_data_pxd.setBuffer(NULL);
  }
}
