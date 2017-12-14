#ifndef _Belle2_Graph1F_h
#define _Belle2_Graph1F_h

#include "daq/slc/dqm/Graph1.h"
#include "daq/slc/dqm/FloatArray.h"

namespace Belle2 {

  class Graph1F : public Graph1 {

  private:

  public:
    Graph1F();
    Graph1F(const Graph1F& h);
    Graph1F(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax, double ymin, double ymax);
    virtual ~Graph1F() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
