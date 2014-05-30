/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/FileHandler.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;
namespace io = boost::iostreams;
namespace fs = boost::filesystem;

namespace Belle2 {
  namespace gearbox {

    FileContext::FileContext(const string& filename, bool compressed)
    {
      if (compressed) m_stream.push(io::gzip_decompressor());
      m_stream.push(io::file_source(filename));
    }

    FileHandler::FileHandler(const std::string& uri): InputHandler(uri), m_path(uri)
    {

      if (m_path.empty()) {
        m_path = "/data/";
      } else {
        //Check if we have placeholder for experiment or run information
        boost::regex exp("\\{EXP(?::(\\d+))?\\}");
        boost::regex run("\\{RUN(?::(\\d+))?\\}");
        m_rundependence = boost::regex_search(m_path, exp) || boost::regex_search(m_path, run);
        if (m_rundependence) {
          //Apparently we do have placeholders, replace them by something
          //boost::format will understand. The placeholder is something like
          //{EXP} or {EXP:<n>} where <n> is an integer to make the number
          //zero-filled with <n> digits, so we replace it with %1$0d or
          //%1$0<n>d to denote to the first argument in a boost format string.
          //Same for {RUN} and {RUN:<n>} but as second argument
          StoreObjPtr<EventMetaData> eventMetaDataPtr;
          eventMetaDataPtr.isRequired();
          std::string tmp = boost::regex_replace(m_path, exp, std::string("%1$$0$1d"));
          tmp = boost::regex_replace(tmp, run, std::string("%2$$0$1d"));
          B2DEBUG(300, "Found run-dependence in file path, resulting in " << tmp);
          m_pathformat = boost::format(tmp);
          m_pathformat.exceptions(boost::io::all_error_bits ^
                                  (boost::io::too_many_args_bit | boost::io::too_few_args_bit));
        }
      }
      B2DEBUG(300, "Created FileHandler for directory " << m_path);
    }

    InputContext* FileHandler::open(const std::string& path)
    {
      if (m_rundependence) {
        StoreObjPtr<EventMetaData> eventMetaDataPtr;
        if (!eventMetaDataPtr.isValid()) {
          B2DEBUG(300, "No run info, probably initialize? Skipping backend");
          return 0;
        }
        int exp = eventMetaDataPtr->getExperiment();
        int run = eventMetaDataPtr->getRun();
        m_path = (m_pathformat % exp % run).str();
      }
      fs::path basedir(m_path);
      std::string filename = (basedir / path).string();
      bool repeat(false);
      do {
        //Ok, let's try to find the file
        B2DEBUG(350, "Trying to find " << filename);

        std::string fullpath = FileSystem::findFile(filename);
        if (!fullpath.empty()) return new FileContext(fullpath, false);
        fullpath = FileSystem::findFile(filename + ".gz");
        if (!fullpath.empty()) return new FileContext(fullpath, true);

        //did not work, replace last slash by a - and try again if such a
        //replacement was successful. This allows flattening the directory
        //structure from e.g. data/pxd/PXD-Alignment.xml to
        //data-pxd-PXD-Alignment.xml to more easily override single files
        size_t last_slash = filename.find_last_of('/');
        repeat = last_slash != 0 && last_slash != std::string::npos;
        if (repeat) {
          filename[last_slash] = '-';
        }
      } while (repeat);
      return 0;
    }

    B2_GEARBOX_REGISTER_INPUTHANDLER(FileHandler, "file");
  }
}
