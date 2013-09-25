#include "HistoManagerGenerator.hh"

#include <util/StringUtil.hh>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <algorithm>
#include <string>

using namespace B2DQM;

void HistoManagerGenerator::create()
{
  std::cout << "making directory: " << _output_dir << std::endl;
  system(std::string(("mkdir -p ") + _output_dir).c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/include").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/src").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/tmp").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/lib").c_str());
  std::cout << "copying makefiles: " << _output_dir << std::endl;
  std::cout << "creating header file for manager class : "
            << createHeader() << std::endl;
  std::cout << "creating source file for manager class : "
            << createSource() << std::endl;
  std::cout << "creating source file for dynamic loader of histo manager : "
            << createLibSource() << std::endl;
  system(B2DAQ::form("echo \"USER_CLASS=%s\" > %s/Makefile ",
                     _name.c_str(), _output_dir.c_str()).c_str());
  system(B2DAQ::form("cat Makefile.template >> %s/Makefile",
                     _output_dir.c_str()).c_str());
  system(B2DAQ::form("make -C %s/ ", _output_dir.c_str()).c_str());
}

std::string HistoManagerGenerator::createHeader()
{
  const std::string class_name = _name + "HistoManager";
  std::string file_path = _output_dir + "/include/" + class_name + ".hh";
  std::ofstream fout(file_path.c_str());
  fout << "#ifndef _B2DQM_" << class_name << "_hh" << std::endl
       << "#define _B2DQM_" << class_name << "_hh" << std::endl
       << std::endl
       << "#include <dqm/HistoManager.hh>" << std::endl
       << std::endl
       << "namespace B2DQM {" << std::endl
       << std::endl
       << "  class " << class_name << " : public HistoManager {" << std::endl
       << std::endl
       << "  public:" << std::endl
       << "    " << class_name << "();" << std::endl
       << "    virtual ~" << class_name << "() throw() {};" << std::endl
       << std::endl
       << "  public:" << std::endl
       << "    virtual HistoPackage* createPackage();" << std::endl
       << "    virtual RootPanel* createRootPanel();" << std::endl
       << "    virtual void analyze();" << std::endl
       << std::endl
       << "  private:" << std::endl
       << "    HistoPackage* _pack;" << std::endl
       << std::endl
       << "  };" << std::endl
       << std::endl
       << "}" << std::endl
       << std::endl
       << "#endif" << std::endl
       << std::endl;
  fout.close();
  return file_path;
}

std::string HistoManagerGenerator::createSource()
{
  const std::string class_name = _name + "HistoManager";
  std::string file_path = _output_dir + "/src/" + class_name + ".cc";
  std::ofstream fout(file_path.c_str());
  fout << "#include \"" << class_name << ".hh\"" << std::endl
       << std::endl
       << "using namespace B2DQM;" << std::endl
       << std::endl
       << class_name << "::" << class_name << "() {" << std::endl
       << std::endl
       << "}" << std::endl
       << std::endl
       << "HistoPackage* " << class_name << "::createPackage() {" << std::endl
       << "  _pack = new HistoPackage(\"" << _name << "\");" << std::endl
       << "  return _pack;" << std::endl
       << "}" << std::endl
       << std::endl
       << "RootPanel* " << class_name << "::createRootPanel() {" << std::endl
       << "  RootPanel* root_panel = new RootPanel(\"" << _name << "\");" << std::endl
       << "  return root_panel;" << std::endl
       << "}" << std::endl
       << std::endl
       << "void " << class_name << "::analyze() {" << std::endl
       << "}" << std::endl;
  fout.close();
  return file_path;
}

std::string HistoManagerGenerator::createLibSource()
{
  const std::string class_name = _name + "HistoManager";
  std::string file_path = _output_dir + "/src/lib" + class_name + ".cc";
  std::ofstream fout(file_path.c_str());
  fout << "#include \"" << class_name << ".hh\"" << std::endl
       << std::endl
       << "extern \"C\" void* create" << class_name << "() {" << std::endl
       << "  return new B2DQM::" << class_name << "();" << std::endl
       << "}" << std::endl
       << std::endl;
  fout.close();
  return file_path;
}
