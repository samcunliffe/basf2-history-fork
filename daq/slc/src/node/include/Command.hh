#ifndef _B2DAQ_Command_hh
#define _B2DAQ_Command_hh

#include "Enum.hh"
#include "State.hh"

namespace B2DAQ {

  class State;
  
  class Command : public Enum {

  public:
    static const Command UNKNOWN;
    static const Command OK;
    static const Command ERROR;
    static const Command BOOT;
    static const Command LOAD;
    static const Command START;
    static const Command STOP;
    static const Command RESUME;
    static const Command PAUSE;
    static const Command ABORT;
    static const Command STATECHECK;

  public:
    static const int SUGGESTED = 2;
    static const int ENABLED = 1;
    static const int DISABLED = 0;

  public:
    Command() throw() {}
    Command(const Enum& e) throw() : Enum(e) {}
    Command(const Command& cmd) throw() : Enum(cmd) {}
    Command(const char* label) throw() { *this = label; }
    ~Command() throw() {}

  private:
    Command(int id, const char* label, const char* alias)
      throw() : Enum(id, label, alias) {}

  public:
    int isAvailable(const State& state) const throw();
    State nextState() const throw();

  public:
    const Command& operator=(const std::string& label) throw();
    const Command& operator=(const char* label) throw();

  };

}

#endif
