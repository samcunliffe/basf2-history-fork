/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <string>
#include "BaseTFInfo.h"

namespace Belle2 {
  /** Sector TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   *  Members:
   *    m_points (TVector3[4]) = 4 Points of the Sector
   *    m_sectorID (int) = real Sector ID
   *    m_isOnlyFriend (bool) = true if = sector is only loaded as friend-sector
   *    m_friends (< int >) =  all IDs of the Sector Friends
   *    m_useCounter (int) = Countes the Hits using this cell (alive and connected)
   *    m_maxCounter (int) = max. m_useCounter
   *
   *  Important Methodes:
   *    isOverlapped(): returns true if = used more then one time by Hits = Sector is overlapped = m_useCounter > 1
   *
   */



  class SectorTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    SectorTFInfo() {
      m_points[0].SetXYZ(0., 0., 0.);
      m_points[1].SetXYZ(0., 0., 0.);
      m_points[2].SetXYZ(0., 0., 0.);
      m_points[3].SetXYZ(0., 0., 0.);
      m_sectorID = -1;
      m_friends.clear();
      m_useCounter = 0;
      m_maxCounter = 0;
      m_isOnlyFriend = false;
    }


    /** Standard constructor */
    SectorTFInfo(int parPassIndex, int parSectorid): BaseTFInfo(parPassIndex) {
      m_sectorID = parSectorid;
      m_points[0].SetXYZ(0., 0., 0.);
      m_points[1].SetXYZ(0., 0., 0.);
      m_points[2].SetXYZ(0., 0., 0.);
      m_points[3].SetXYZ(0., 0., 0.);
      m_sectorID = -1;
      m_useCounter = 0;
      m_maxCounter = 0;
      m_isOnlyFriend = false;
      m_friends.clear();
    }

    /** getter - getPoint Point int (from POINT 0 to POINT 3 */
    TVector3 getPoint(int valuePoint)  { return m_points[valuePoint]; }

    /** setter - Point */
    void setPoint(int valuePoint, TVector3 value) { m_points[valuePoint] = value; }

    /** getter - Sector ID */
    int getSectorID()  { return m_sectorID; }

    /** setter - Sector ID */
    void setSectorID(int value) { m_sectorID = value; }

    /** getter - getFriends*/
    std::vector<int>& getFriends()  { return m_friends; }

    /** add new int to Friends */
    void push_back_Friends(int newMember) {
      m_friends.push_back(newMember);
    }

    /** set all Friends */
    void setAllFriends(std::vector<int> parFriends) {
      m_friends = parFriends;
    }

    /** returns size of Friends */
    int sizeFriends() { return m_friends.size(); }

    /** getter - IsOnlyFriend */
    bool getIsOnlyFriend()  { return m_isOnlyFriend; }

    /** setter - IsOnlyFriend */
    void setIsOnlyFriend(bool value) { m_isOnlyFriend = value; }

    /** getter - UseCounter */
    int getUseCounter()  { return m_useCounter; }

    /** getMaxCounter */
    int getMaxCounter()  { return m_maxCounter; }

    /** setter - UseCounter */
    void setUseCounter(int value) {
      m_useCounter = value;
      if (m_useCounter > m_maxCounter) {
        m_maxCounter = m_useCounter;
      }
    }

    /** UseCounte add / minus */
    void changeUseCounter(int value) {
      m_useCounter = m_useCounter + value;
      if (m_useCounter > m_maxCounter) {
        m_maxCounter = m_useCounter;
      }
    }


    // Sector is overlapped if it is used more then one time
    /** if the sector is overlaped */
    bool isOverlapped()  {
      if (m_useCounter > 1) {
        return true;
      } else {
        return false;
      }
    }

    /** returns the String for the display - Information */
    TString getDisplayInformation() {

      // NOT FINAL !!!

      return TString::Format("Point 1: (%.3f, %.3f, %.3f)\n Point 2: (%.3f, %.3f, %.3f)\n Point 3: (%.3f, %.3f, %.3f)\n Point 4: (%.3f, %.3f, %.3f)\n Friend Only: %s\n Died_ID: %d ", m_points[0].X(), m_points[0].Y(), m_points[0].Z(), m_points[1].X(), m_points[1].Y(), m_points[1].Z(), m_points[2].X(), m_points[2].Y(), m_points[2].Z(), m_points[3].X(), m_points[3].Y(), m_points[3].Z(), m_isOnlyFriend ? "true" : "false", getDiedID());
    }

    /** returns the String for the display - AlternativeBox */
    TString getDisplayAlternativeBox() {

      std::string diedAt = getDiedAt();

      return TString::Format("SectorID: %d, PassIndex: %d\n Died_ID: %s, Count Friend Sectors: %d", m_sectorID, getPassIndex(), diedAt.c_str(), sizeFriends());
    }


  protected:

    TVector3 m_points[4]; /**< m_points (TVector3[4]) = 4 Points of the Sector **/

    int m_sectorID;   /**< real Sector ID **/
    bool m_isOnlyFriend;  /**< true if = sector is only loaded as friend-sector **/

    std::vector<int> m_friends;  /**< all IDs of the Sector Friends **/

    int m_useCounter;  /**< Countes the Hits (alive and connected) */
    int m_maxCounter;  /**<  Max. Counts of Hits (max. m_useCounter)  */

    ClassDef(SectorTFInfo, 1)
  };

}
