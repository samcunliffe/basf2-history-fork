#include "daq/slc/base/Exception.h"

#include <sstream>

using namespace Belle2;

Exception::Exception() throw()
  : _comment("")
{
}

Exception::Exception(const std::string& comment) throw()
  : _comment(comment)
{
}

Exception::~Exception() throw()
{

}


const char* Exception::what() const throw()
{
  return _comment.c_str();
  /*
  static std::string str =
  std::stringstream ss;
  ss.clear();
  ss.str("");
  ss << "@File('" << _file_name
     << "',line=" << _line_no
     << "), comment='" << _comment
     << "'";
  str = ss.str();
  return str.c_str();
  */
}
