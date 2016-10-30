/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>
#include <map>
#include <vector>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD half shell payload
  */

  class VXDHalfShellPar: public TObject {

  public:
    /** Constructor */
    VXDHalfShellPar(double shellAngle = 0) :  m_shellAngle(shellAngle) {}
    /** get shellAngle */
    double getShellAngle() const { return m_shellAngle; }
    /** add ladder */
    void addLadder(int layerID, int ladderID, double phi) { m_layers[layerID].push_back(std::pair<int, double>(ladderID, phi)); }
    /** get ladders */
    std::vector<std::pair<int, double>>& getLadders(int layerID) { return m_layers[layerID]; }

  private:
    double m_shellAngle;
    std::map< int, std::vector<std::pair<int, double>> > m_layers;

    ClassDef(VXDHalfShellPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

