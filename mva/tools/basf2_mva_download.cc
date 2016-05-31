/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <mva/utility/Utility.h>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{

  std::string filename;
  std::string identifier;

  int event = 0;
  int run = 0;
  int experiment = 0;

  po::options_description description("Options");
  description.add_options()
  ("help", "print this message")
  ("filename", po::value<std::string>(&filename)->required(), "Name of weightfile produced by basf2_mva_teacher")
  ("identifier", po::value<std::string>(&identifier)->required(), "Database identifier")
  ("experiment", po::value<int>(&experiment), "Experiment for which the weightfile should be valid")
  ("run", po::value<int>(&run), "Run for which the weightfile should be valid")
  ("event", po::value<int>(&event), "Experiment for which the weightfile should be valid");
  po::variables_map vm;

  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(description).run();
    po::store(parsed, vm);

    if (vm.count("help")) {
      std::cout << description << std::endl;
      return 1;
    }
    po::notify(vm);
  } catch (po::error& err) {
    std::cerr << "Error: " << err.what() << "\n";
    return 1;
  }

  Belle2::MVA::download(identifier, filename, experiment, run, event);

}
