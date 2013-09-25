#include "FloatArray.hh"

#include <sstream>

using namespace B2DQM;

FloatArray::FloatArray() throw() : NumberArray(0), _value_v(NULL) {}

FloatArray::FloatArray(size_t size) throw() : NumberArray(), _value_v(NULL)
{
  resize(size);
}

FloatArray::FloatArray(const NumberArray& v) throw() : NumberArray(), _value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, _value_v[n]);
}

FloatArray::~FloatArray() throw()
{
  if (_value_v != NULL) delete [] _value_v;
}

double FloatArray::get(size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  else return -1;
}
void FloatArray::set(size_t n, double v) throw()
{
  if (n < size()) _value_v[n] = (float)v;
}
void FloatArray::resize(int s) throw()
{
  if (_value_v != NULL) delete [] _value_v;
  _value_v = new float [s];
  _size = s;
  for (size_t n = 0; n < size(); n++) set(n, 0);
}

std::string FloatArray::toScript() const throw()
{
  std::stringstream ss("");
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}

float& FloatArray::operator[](size_t n) throw()
{
  if (n < size()) return _value_v[n];
  return _value_v[0];
}

float FloatArray::operator[](size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  return 0;
}

void FloatArray::readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    _value_v[n] = reader.readFloat();
  }
}

double FloatArray::readElement(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  return reader.readFloat();
}

void FloatArray::writeObject(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeFloat(_value_v[n]);
  }
}

void FloatArray::writeElement(double value, B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  writer.writeFloat(value);
}


