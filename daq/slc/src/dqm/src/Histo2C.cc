#include "Histo2C.hh"

#include "CharArray.hh"

using namespace B2DQM;

Histo2C::Histo2C() : Histo2()
{
  _data = new CharArray();
}

Histo2C::Histo2C(const Histo2C& h) : Histo2(h)
{
  if (h._data != NULL) _data = new CharArray(*h._data);
  else _data = new CharArray();
}

Histo2C::Histo2C(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  _data = new CharArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2C::getDataType() const throw()
{
  return std::string("H2C");
}
