#ifndef _B2DAQ_PackageManager_hh
#define _B2DAQ_PackageManager_hh

#include "PackageSerializer.hh"

#include <dqm/HistoPackage.hh>
#include <dqm/RootPanel.hh>

#include <system/RWLock.hh>

#include <string>

namespace B2DQM {

  class HistoFileReader;
  class HistoManager;

  class PackageManager {
    
  public:
    PackageManager(HistoManager* manager);
    virtual ~PackageManager() throw();
    
  public:
    void clear();
    bool init();
    void update();

  public:
    void setFilePath(const std::string& path) { _map_path = path; }
    void setAvailable(bool available);
    bool isAvailable();
    char* createConfig(size_t& buf_size);
    char* createContents(size_t& buf_size);
    char* createContentsAll(size_t& buf_size);
    size_t copyConfig(char* buf, size_t size);
    size_t copyContents(char* buf, size_t size, int& update_id);
    size_t copyContentsAll(char* buf, size_t size, int& update_id);
    const std::string createXML();
    int getUpdateId();
    std::string getName();

  private:
    HistoPackage* createPackage();
    RootPanel* createRootPanel();
    void analyze();

  protected:
    std::string _map_path;
    HistoManager* _manager;
    HistoFileReader* _reader;
    HistoPackage* _pack; 
    RootPanel* _panel;
    PackageSerializer _serializer;
    bool _available;
    B2DAQ::RWLock _lock;

  };

}

#endif
