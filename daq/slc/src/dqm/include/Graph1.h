#ifndef _Belle2_Graph1_hh
#define _Belle2_Graph1_hh

#include "dqm/Histo.h"

namespace Belle2 {

  class Graph1 : public Histo {

  public:

    Graph1() throw();
    Graph1(const Graph1& h) throw();
    Graph1(std::string name, std::string title,
           int nbinx, double xmin, double xmax,
           double ymin, double ymax) throw();
    virtual ~Graph1() throw();

  public:

    virtual double getPointX(int n) const throw();
    virtual double getPointY(int n) const throw();
    virtual void setPointX(int n, double data) throw();
    virtual void setPointY(int n, double data) throw();
    virtual double getMaximum() const throw();
    virtual double getMinimum() const throw();
    virtual void setMaximum(double data) throw();
    virtual void setMinimum(double data) throw();
    virtual void fixMaximum(double data) throw();
    virtual void fixMinimum(double data) throw();
    virtual void reset() throw();
    virtual int getDim() const throw() { return 1; }

  public:
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);

  };

};

#endif
