/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <string>
#include <boost/array.hpp>


namespace Belle2 {

  /** Class to identify a filter type used by sectorFriends
   *
   * is a simple converter to convert the enum used to access the filter-values (cutoffs) into readable values
   * it contains enum values for speed optimized access of filter-data and strings for human-readable output
   *
   */
  class FilterID {
  public:
    /** filterTyoes contains enums assigning unique values for each filter type allowing fast filter type recognition */
    enum filterTypes {
      angles3D, /**< string name of filter a3D */
      anglesRZ, /**< string name of filter aRZ */
      anglesXY, /**< string name of filter aXY */
      distance3D, /**< string name of filter d3D */
      distanceXY, /**< string name of filter dXY */
      distanceZ, /**< string name of filter dZ */
      helixFit, /**< string name of filter hFit */
      slopeRZ, /**< string name of filter slopeRZ */
      deltaSlopeRZ, /**< string name of filter dslopeRZ */
      pT, /**< string name of filter pT */
      deltapT, /**< string name of filter dPt */
      normedDistance3D, /**< string name of filter nd3D */
      distance2IP, /**< string name of filter d2IP */
      deltaDistance2IP, /**< string name of filter dd2IP */

      anglesHighOccupancy3D, /**< string name of filter a3D high occupancy */
      anglesHighOccupancyXY,  /**< string name of filter aXY high occupancy */
      anglesHighOccupancyRZ, /**< string name of filter aRZ high occupancy */
      distanceHighOccupancy2IP, /**< string name of filter d2IP high occupancy */
      deltaSlopeHighOccupancyRZ, /**< string name of filter dslopeRZ high occupancy */
      pTHighOccupancy, /**< string name of filter pT high occupancy */
      helixHighOccupancyFit, /**< string name of filter hFit high occupancy */
      deltapTHighOccupancy, /**< string name of filter dPt high occupancy */
      deltaDistanceHighOccupancy2IP, /**< string name of filter dd2IP high occupancy */

      //Collector FilterIDs
      nbFinderLost, /**< Nb Finder filter */
      cellularAutomaton, /**< CA filter */


      hopfield,  /**< Hopfield filter */
      greedy,  /**< Greedy filter */
      tcDuel,  /**< TC Duel filter */
      tcFinderCurr,  /**< TC Finder Curr filter */
      ziggZaggXY,  /**< ziggZaggXY filter */
      ziggZaggRZ,  /**< ziggZaggRZ filter */
      calcQIbyKalman,  /**< calcQIbyKalman filter */
      overlapping,  /**< overlapping filter */
      circlefit,  /**< circlefit filter */

      silentTcc,  /**< Silent Kill TCC filter */
      silentHitFinder,  /**< Silent Kill HitFinder filter */
      silentSegFinder,  /**< Silent Kill SefFinder filter */

      // tests which are only for debugging and validation processes
      alwaysTrue, /**< a filter with this ID will always say yes */
      alwaysFalse, /**< a filter with this ID will always say no */
      randomTrue,  /**< a filter with this ID will say yes or no by random choice */

      numFilters /**< knows number of filters existing. If a member returns this value, the input-value was wrong */
    };

    const static std::string nameAngles3D;/**< string name of filter a3D */
    const static std::string nameAnglesRZ; /**< string name of filter aRZ */
    const static std::string nameAnglesXY; /**< string name of filter aXY */
    const static std::string nameDistance3D; /**< string name of filter d3D */
    const static std::string nameDistanceXY; /**< string name of filter dXY */
    const static std::string nameDistanceZ; /**< string name of filter dZ */
    const static std::string nameHelixFit; /**< string name of filter hFit */
    const static std::string nameSlopeRZ; /**< string name of filter slopeRZ */
    const static std::string nameDeltaSlopeRZ; /**< string name of filter dslopeRZ */
    const static std::string namePT; /**< string name of filter pT */
    const static std::string nameDeltapT; /**< string name of filter dPt */
    const static std::string nameNormedDistance3D; /**< string name of filter nd3D */
    const static std::string nameDistance2IP; /**< string name of filter d2IP */
    const static std::string nameDeltaDistance2IP; /**< string name of filter dd2IP */

    const static std::string nameAnglesHighOccupancy3D; /**< string name of filter a3D high occupancy */
    const static std::string nameAnglesHighOccupancyXY;  /**< string name of filter aXY high occupancy */
    const static std::string nameAnglesHighOccupancyRZ; /**< string name of filter aRZ high occupancy */
    const static std::string nameDistanceHighOccupancy2IP; /**< string name of filter d2IP high occupancy */
    const static std::string nameDeltaSlopeHighOccupancyRZ; /**< string name of filter dslopeRZ high occupancy */
    const static std::string namePTHighOccupancy; /**< string name of filter pT high occupancy */
    const static std::string nameHelixHighOccupancyFit; /**< string name of filter hFit high occupancy */
    const static std::string nameDeltapTHighOccupancy; /**< string name of filter dPt high occupancy */
    const static std::string nameDeltaDistanceHighOccupancy2IP; /**< string name of filter dd2IP high occupancy */

    const static std::string nameNbFinderLost; /**< string name Nb Finder filter */
    const static std::string nameCellularAutomaton; /**< string name CA filter */


    const static std::string nameHopfield;  /**< string name Hopfield filter */
    const static std::string nameGreedy;   /**< string name Greedy filter */
    const static std::string nameTcDuel;   /**< string name TC Duel filter */
    const static std::string nameTcFinderCurr;   /**< string name TC Finder Curr filter */
    const static std::string nameZiggZaggXY;   /**< string name ZiggZaggXY filter */
    const static std::string nameZiggZaggRZ;    /**< string name ZiggZaggRZ filter */
    const static std::string nameCalcQIbyKalman;   /**< string name CalcQIbyKalman filter */
    const static std::string nameOverlapping;   /**< string name Overlapping filter */
    const static std::string nameCirclefit;     /**< string name Circlefit filter */

    const static std::string nameSilentTcc;  /**< string name Silent Kill TCC filter */
    const static std::string nameSilentHitFinder;   /**< string name Silent Kill Hit Finder filter */
    const static std::string nameSilentSegFinder;    /**< string name Silent Kill Seg Finder filter */

    const static std::string nameAlwaysTrue; /**< string name of a filter which will always say yes */
    const static std::string nameAlwaysFalse; /**< string name of a filter which will always say no */
    const static std::string nameRandomTrue;  /**< string name of a filter which will say yes or no by random choice */

    const static boost::array<std::string, numFilters> nameVector; /**< array storing all the strings where the enums can be used to access their position */

    const static boost::array<FilterID::filterTypes, FilterID::numFilters> enumVector; /**< array storing all the enums where ints can be used to access their position (this is needed since enums can be converted to int but not otherwise)*/

    /** Constructor. */
    FilterID() {}

    /** Destructor. */
    ~FilterID() {}

    /** converts existing string with name of filter type into suitable enum value. */
    static filterTypes getFilterType(std::string filterString);

    /** converts existing int with filter type-compativble value into suitable enum value. */
    static filterTypes getFilterType(int filterInt);

    /** converts existing enum value into suitable string with name of filter type. */
    static std::string getFilterString(filterTypes filterType) { return nameVector[filterType]; }

    /** converts int value into suitable string with name of filter type (if int value is equal to values of enum). */
    static std::string getFilterString(int filterType);

  protected:
  }; //end class FilterID
} //end namespace Belle2
