/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "svd/online/SVDOnlineToOfflineMap.h"
#include <boost/property_tree/xml_parser.hpp>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;
using boost::property_tree::ptree;


SVDOnlineToOfflineMap::SVDOnlineToOfflineMap(const string& xml_filename)
{

  // Create an empty property tree object

  ptree propertyTree;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(xml_filename, propertyTree);

  // traverse pt: let us navigate through the daughters of <SVD>
  for (ptree::value_type const & v : propertyTree.get_child("SVD")) {
    // if the daughter is a <layer> then read it!
    if (v.first == "layer")
      ReadLayer(v.second.get<int>("<xmlattr>.n"), v.second);
  }
}


SVDDigit* SVDOnlineToOfflineMap::NewDigit(unsigned char FADC,
                                          unsigned char APV25, unsigned char channel, float charge = 0.0,
                                          float time = 0.0)
{

  ChipID id(FADC, APV25);
  auto chipIter = m_chips.find(id);
  if (chipIter == m_chips.end()) {
    B2WARNING(" FADC #" <<  int(FADC) << " and " << "APV # " << int(APV25) <<
              " : combination not found in the SVD On-line to Off-line map ");
    return NULL;
  }

  if (channel > 127) {
    B2WARNING(" channel #" <<  int(channel) << " out of range (0-127).");
    return NULL;
  }

  ChipInfo info = chipIter->second;
  short strip = info.m_channel0 + ((unsigned short)channel) * (info.m_parallel ? 1 : -1);
  return new SVDDigit(info.m_sensorID, info.m_uSide, strip, 0., charge, time);
}


void
SVDOnlineToOfflineMap::ReadLayer(int nlayer, ptree const& xml_layer)
{
  // traverse xml_layer: let us navigate through the daughters of <layer>
  for (ptree::value_type const & v : xml_layer) {
    // if the daughter is a <ladder> then read it!
    if (v.first == "ladder")
      ReadLadder(nlayer, v.second.get<int>("<xmlattr>.n") , v.second);
  }
}

void
SVDOnlineToOfflineMap::ReadLadder(int nlayer, int nladder, ptree const& xml_ladder)
{
  // traverse xml_ladder: let us navigate through the daughters of <ladder>
  for (ptree::value_type const & v : xml_ladder) {
    // if the daughter is a <sensor> then read it!
    if (v.first == "sensor")
      ReadSensor(nlayer, nladder, v.second.get<int>("<xmlattr>.n") , v.second);
  }
}

void
SVDOnlineToOfflineMap::ReadSensor(int nlayer, int nladder, int nsensor, ptree const& xml_sensor)
{
  // traverse xml_sensor: let us navigate through the daughters of <sensor>
  for (ptree::value_type const & v : xml_sensor) {
    // if the daughter is one side <> then read it!
    if (v.first == "side") {
      std::string tagSide = v.second.get<std::string>("<xmlattr>.side");

      bool isOnSideU = (tagSide == "U" || tagSide == "u");
      bool isOnSideV = (tagSide == "V" || tagSide == "v");

      if ((! isOnSideU) && (! isOnSideV)) {
        B2ERROR("Side '" << tagSide << "' on layer " << nlayer
                << " ladder " << nladder << " sensor " << nsensor
                << " is neither 'U' nor 'V'");
      }

      ReadSensorSide(nlayer, nladder, nsensor, isOnSideU, v.second);
    }
  }

}

void
SVDOnlineToOfflineMap::ReadSensorSide(int nlayer, int nladder, int nsensor, bool isU,
                                      ptree const& xml_side)
{

  // traverse xml_sensor: let us navigate through the daughters of <side>

  for (ptree::value_type const & v : xml_side) {
    // if the daughter is a <chip>
    if (v.first == "chip") {
      auto tags = v.second;
      unsigned char  chipN = tags.get<unsigned char>("<xmlattr>.n");
      unsigned char  FADCn = tags.get<unsigned char>("<xmlattr>.FADCn");
      ChipID id(FADCn, chipN);
      auto chipIter = m_chips.find(id);
      if (chipIter != m_chips.end()) {
        B2WARNING("Repeated insertion for FADC " << FADCn << " and APV "
                  << chipN << ", layer/ladder/sensor " << nlayer << "/" << nladder
                  << "/" << nsensor << ", side " << (isU ? "u" : "v"));
      }
      unsigned short stripNumberCh0  = tags.get<unsigned short>("<xmlattr>.strip_number_of_ch0");
      unsigned short stripNumberCh127 = tags.get<unsigned short>("<xmlattr>.strip_number_of_ch127");

      ChipInfo info;
      info.m_sensorID = VxdID(nlayer, nladder, nsensor);
      info.m_uSide = isU;
      info.m_parallel = (stripNumberCh127 > stripNumberCh0);
      info.m_channel0 = stripNumberCh0;
      info.m_channel127 = stripNumberCh127;

      m_chips[id] = info;
    } //chip

  } // for daughters

}
