#ifndef _Belle2_Histo1F_h
#define _Belle2_Histo1F_h

#include "daq/slc/dqm/Histo1.h"
#include "daq/slc/dqm/FloatArray.h"

namespace Belle2 {

  class Histo1F : public Histo1 {

  public:
    Histo1F() throw();
    Histo1F(const Histo1F& h) throw();
    Histo1F(const std::string& name, const std::string& title,
            int nbinx, double min, double max) throw();
    virtual ~Histo1F() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
