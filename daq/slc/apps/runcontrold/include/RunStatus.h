#ifndef _Belle2_RunStatus_hh
#define _Belle2_RunStatus_hh

#include <daq/slc/base/DataObject.h>
#include <daq/slc/base/NSMNode.h>

namespace Belle2 {

  class RunConfig;

  class RunStatus : public DataObject {

  public:
    RunStatus(const std::string& data_name = "",
              int revision = -1) throw();
    virtual ~RunStatus() throw() {}

  public:
    int getExpNumber() const throw() { return getInt("exp_number"); }
    int getColdNumber() const throw() { return getInt("cold_number"); }
    int getHotNumber() const throw() { return getInt("hot_number"); }
    int getStartTime() const throw() { return getInt("start_time"); }
    int getEndTime() const throw() { return getInt("end_time"); }
    int getRunConfig() const throw() { return getInt("run_config"); }
    const std::string getOperators() const throw() {return getText("operators"); }
    void setExpNumber(int exp_number) throw() { setInt("exp_number", exp_number); }
    void setColdNumber(int run_number) throw() { setInt("cold_number", run_number); }
    void setHotNumber(int run_number) throw() { setInt("hot_number", run_number); }
    void setStartTime(int start_time) throw() { setInt("start_time", start_time); }
    void setEndTime(int end_time) throw() { setInt("end_time", end_time); }
    void setRunConfig(int run_config) throw() { setInt("run_config", run_config); }
    void setOperators(const std::string& operators) throw() { setText("operators", operators); }
    int incrementExpNumber() throw();
    int incrementColdNumber() throw();
    int incrementHotNumber() throw();

  public:
    void add(NSMNode* node);
    void update();
    void setConfig(RunConfig* config) { _config = config; }

  private:
    std::vector<NSMNode*> _node_v;
    RunConfig* _config;

  };

}

#endif
