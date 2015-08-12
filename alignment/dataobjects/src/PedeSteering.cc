#include <alignment/dataobjects/PedeSteering.h>
#include <fstream>

using namespace Belle2;

ClassImp(PedeSteering)
// ------------
// PedeSteering
// ------------
string PedeSteering::make(string filename)
{
  command("end");
  if (filename == "")
    filename = name;
  ofstream file(filename.c_str());
  file << "! Steerig file for Pede generated by PedeSteering class !" << endl;
  file << "Cfiles" << endl;
  for (string& cfile : files)
    file << cfile << endl;

  file << endl;

  for (string& command : commands)
    file << command << endl;

  file.close();
  return filename;
}
void PedeSteering::command(std::string line)
{
  commands.push_back(line);
}
void PedeSteering::import(string filename)
{
  commands.clear();
  ifstream file(filename);
  if (!file.is_open())
    return;

  string commandLine;
  while (getline(file, commandLine))
    command(commandLine);
}

void PedeSteering::fixParameters(vector< int > labels, vector< double > values, vector< double > presigmas)
{
  command("Parameters");
  bool useValues = (labels.size() == values.size());
  bool usePresigmas = (labels.size() == presigmas.size());

  for (unsigned int i = 0; i < labels.size(); i++) {
    double value = 0.;
    double presigma = -1.;
    if (useValues)
      value = values[i];
    if (usePresigmas)
      presigma = presigmas[i];

    command(to_string(labels[i]) + " " + to_string(value) + " " + to_string(presigma));
  }
}
/*
void PedeSteering::addConstraint(double constraint, vector< int > labels, vector< double > coefficients) {
  command("Constraint " + to_string(constraint));
  if (labels.size() != coefficients.size())
    return;

  for (unsigned int i = 0; i < labels.size(); i++) {
    command(to_string(labels[i]) + " " + to_string(coefficients[i]));
  }
}
*/


