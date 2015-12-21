/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <algorithm>
#include <set>

#include <TVector3.h>

#include <cdc/dataobjects/WireID.h>


#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {


    /// Class representating the sense wire arrangement in the whole of the central drift chamber.
    /** This class represents the the whole CDC layer geometry and the neighborhood relations in it. \n
     *  Since their is only one CDC at a time it is implemented as a singletone object, with getInstance(). \n
     *  Their should rarely be the need to construct  additional instances. \n
     *  It provides getters for all wires, layers and superlayers and can check the validity of wire id combinations. \n
     *  It also presents an iterable range of all wires in the CDC. \n
     *  The range is sorted with increasing wire ids for maximal fast look up. \n
     *  Also there are methods for getting to the closest and secondary of each wire. \n
     *  The class does provide the memory for all instances of wires, layers and superlayers by itself.
     */
    class CDCWireTopology  {

    public:
      /// The underlying container type storing range of wires represented by this class
      typedef std::vector<Belle2::TrackFindingCDC::CDCWire> Container;
      /// The value type of the iterator range
      typedef Belle2::TrackFindingCDC::CDCWire value_type;

      /// The type of the iterator for the wire range
      typedef Container::iterator iterator;

      /// The type of the const_iterator for the wire range
      typedef Container::const_iterator const_iterator;

      /// A wire pointer pair as returned from getNeighborsOutward(), getNeighborsInward()
      typedef CDCWireSuperLayer::NeighborPair NeighborPair;

    public:

      /// Default constructor for ROOT compatibility. Use rather getInstance to get the shared singleton instance.
      CDCWireTopology() { initialize(); }




    public:
      /// Total number of wires
      static const IWire N_WIRES = 14336;
      ///Total number of layers
      static const ILayer N_LAYERS = 56;
      ///Total number of superlayers
      static const ILayer N_SUPERLAYERS = 9;

    private:
      /// Initializes all wires, wire layers and wire superlayers their interdependences from the CDCGeometryPar.
      void initialize();

    public:
      /** @name Wire range
       */
      /**@{*/
      /// Getter for the begin iterator of the whole wire range
      const_iterator begin() const { return m_wires.begin(); }

      /// Getter for the end iterator of the whole wire range
      const_iterator end() const { return m_wires.end(); }

      /// Getter for the total number of wires
      IWire size() const { return m_wires.size(); }

      /// Getter for the first wire in the whole cdc
      const CDCWire& first() const { return *(begin()); }
      /// Getter for the last wire in the whole cdc
      const CDCWire& last() const { return *(--end()); }
      /**@}*/

      /** @name Wire getters
       *  Gets the wires from a WireID. */
      ///@{

      /// Checks the validity of a wireID convinience object.
      inline bool isValidIWire(const WireID& wireID) const
      { return isValidIWire(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Checks the validity of the continuous layer id, wire id combination.
      inline bool isValidIWire(ILayer iCLayer, IWire iWire) const
      { return isValidICLayer(iCLayer) and getWireLayer(iCLayer).isValidIWire(iWire); }

      /// Checks the validity of the superlayer id, layer id, wire id combination.
      inline
      bool isValidIWire(ISuperLayer iSuperLayer,
                        ILayer iLayer,
                        IWire iWire) const
      { return isValidISuperLayer(iSuperLayer) and getWireSuperLayer(iSuperLayer).isValidIWire(iLayer, iWire); }

      /// Getter for wire getter by wireID object.
      const CDCWire& getWire(const WireID& wireId) const
      { return getWireLayer(wireId.getICLayer()).getWire(wireId.getIWire()) ; }

      /// Getter for wire getter by continuous layer id and wire id.
      const CDCWire& getWire(ILayer iCLayer , IWire iWire) const
      { return getWireLayer(iCLayer).getWire(iWire) ; }

      /// Getter for wire getter by superlayer id, layer id and wire id.
      const CDCWire& getWire(ISuperLayer iSuperLayer, ILayer iLayer , IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getWireLayer(iLayer).getWire(iWire) ; }

      /// Getter for the underlying storing wire vector.
      const std::vector<Belle2::TrackFindingCDC::CDCWire>& getWires() const
      { return m_wires; }
      ///@}

      /** @name Wire layer getters
       *  Getters for the wire layer*/
      ///@{
      /// Getter for the total number of wire layers
      ILayer getNLayers() const
      { return getWireLayers().size(); }

      /// Checks the validity of the continuous layer id.
      bool isValidICLayer(ILayer iCLayer) const
      { return 0 <= iCLayer and iCLayer < int(getWireLayers().size()); }

      /// Checks the validity of the superlayer id, layer id combination.
      bool isValidILayer(ISuperLayer iSuperLayer, ILayer iLayer) const
      { return isValidISuperLayer(iSuperLayer) and getWireSuperLayer(iSuperLayer).isValidILayer(iLayer); }

      /// Getter for wire layer getter by wireID object.
      const CDCWireLayer& getWireLayer(const WireID& wireId) const
      { return getWireLayer(wireId.getICLayer()); }

      /// Getter for wire layer getter by continuous layer id.
      const CDCWireLayer& getWireLayer(ILayer iCLayer) const
      { return m_wireLayers[ iCLayer ]; }

      /// Getter for wire layer getter by superlayer id and layer id.
      const CDCWireLayer& getWireLayer(ISuperLayer iSuperLayer, ILayer iLayer) const
      { return getWireSuperLayer(iSuperLayer).getWireLayer(iLayer); }

      /// Getter for the underlying storing layer vector
      const std::vector<Belle2::TrackFindingCDC::CDCWireLayer>& getWireLayers() const
      { return m_wireLayers; }
      ///@}

      /** @name Wire superlayer getters
       *  Getters for the wire superlayers */
      ///@{
      /// Getter for the total number of superlayers
      ISuperLayer getNSuperLayers() const
      { return getWireSuperLayers().size(); }

      /// Checks the validity of the superlayer id.
      bool isValidISuperLayer(ISuperLayer iSuperLayer) const
      { return 0 <= iSuperLayer and iSuperLayer < int(getWireSuperLayers().size()); }

      /// Returns the logical superlayer number at the given radius
      ISuperLayer getISuperLayerAtCylindricalR(const double cylindricalR);

      /// Getter for wire superlayer getter by wireID object.
      const CDCWireSuperLayer& getWireSuperLayer(const WireID& wireId) const
      { return getWireSuperLayer(wireId.getISuperLayer()); }

      /// Getter for wire superlayer getter by superlayer id.
      const CDCWireSuperLayer& getWireSuperLayer(ISuperLayer iSuperLayer) const
      { return m_wireSuperLayers[ iSuperLayer ]; }

      /// Getter for the underlying storing superlayer vector
      const std::vector<Belle2::TrackFindingCDC::CDCWireSuperLayer>& getWireSuperLayers() const
      { return m_wireSuperLayers; }
      ///@}


      /** @name Getters for the closest neighbors of a wire by wireID
       *  They do not cross superlayer boundaries. \n
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      /// Checks if two wires are closest neighbors. see details.
      /** Returns the relation of the first wire to the second wire give by their
       *  layer id within the superlayer and the wire id.
       *  If they are not neighbors zero is returned.
       *  If they are neighbors the return value indicates
       *  the direction to go from the first wire to the second. \n
       *  The return value is one of \n
       *  0 for the wires being no neighbors \n
       *  EWireNeighborKind::c_CWOut = 1  for clockwise outwards \n
       *  EWireNeighborKind::c_CW = 3 for clockwise \n
       *  EWireNeighborKind::c_CWIn = 5 for clockwise inwards \n
       *  EWireNeighborKind::c_CCWIn = 7 for counterclockwise inwards \n
       *  EWireNeighborKind::c_CCW = 9 for counterclockwise \n
       *  EWireNeighborKind::c_CCWOut = 11 for counterclockwise outwards \n
       *  The values are choosen to have an assoziation with the numbers on a regular clock.
       *  ( compare secondary neighborhood ) */
      EWireNeighborKind getNeighborKind(const WireID& wire, const WireID& other) const;

      /// Checks if two wires are primary neighbors.
      bool areNeighbors(const WireID& wire, const WireID& other) const
      { return getNeighborKind(wire, other) != EWireNeighborKind::c_None; }

      /// Getter for the two closest neighbors in the layer outwards.
      NeighborPair getNeighborsOutwards(const WireID& wireID) const
      { return getNeighborsOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the two closest neighbors in the layer inwards.
      NeighborPair getNeighborsInwards(const WireID& wireID) const
      { return getNeighborsInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire* getNeighborCCWOutwards(const WireID& wireID) const
      { return getNeighborCCWOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire* getNeighborCWOutwards(const WireID& wireID) const
      { return getNeighborCWOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest counterclockwise neighbor.
      const CDCWire* getNeighborCCW(const WireID& wireID) const
      { return getNeighborCCW(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest clockwise neighbor.
      const CDCWire* getNeighborCW(const WireID& wireID) const
      { return getNeighborCW(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire* getNeighborCCWInwards(const WireID& wireID) const
      { return getNeighborCCWInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire* getNeighborCWInwards(const WireID& wireID) const
      { return getNeighborCWInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }
      /**@}*/


      /** @name Getters for the closest neighbors of a wire by superlayer id, layer id and wire id
       *  They do not cross superlayer boundaries. \n
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead. \n
       */
      /**@{*/
      /// Getter for the two closest neighbors in the layer outwards.
      NeighborPair getNeighborsOutwards(ISuperLayer iSuperLayer,
                                        ILayer iLayer,
                                        IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborsOutwards(iLayer, iWire); }

      /// Getter for the two closest neighbors in the layer inwards.
      NeighborPair getNeighborsInwards(ISuperLayer iSuperLayer,
                                       ILayer iLayer,
                                       IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborsInwards(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire* getNeighborCCWOutwards(ISuperLayer iSuperLayer,
                                            ILayer iLayer,
                                            IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCWOutwards(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire* getNeighborCWOutwards(ISuperLayer iSuperLayer,
                                           ILayer iLayer,
                                           IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCWOutwards(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor.
      const CDCWire* getNeighborCCW(ISuperLayer iSuperLayer,
                                    ILayer iLayer,
                                    IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCW(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor.
      const CDCWire* getNeighborCW(ISuperLayer iSuperLayer,
                                   ILayer iLayer,
                                   IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCW(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCCWInwards(ISuperLayer iSuperLayer,
                            ILayer iLayer,
                            IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCWInwards(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire* getNeighborCWInwards(ISuperLayer iSuperLayer,
                                          ILayer iLayer,
                                          IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCWInwards(iLayer, iWire); }
      /**@}*/



      /** @name Secondary neighbor getters by wireID object
       *  The secondary neighbors are numbered like positions on the normal twelve hour clock.
       *
       *  The rational of this is that the secondary neighbors are positioned \n
       *  just like the numbers on the clock if you took the center of the clock to the wire position \n
       *  and the twelve o'clock points in the direction outwards. \n
       *
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      ///Getter for secondary neighbor at the one o'clock position
      const CDCWire* getSecondNeighborOneOClock(const WireID& wireID) const
      { return getSecondNeighborOneOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the two o'clock position
      const CDCWire* getSecondNeighborTwoOClock(const WireID& wireID) const
      { return getSecondNeighborTwoOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the three o'clock position
      const CDCWire* getSecondNeighborThreeOClock(const WireID& wireID) const
      { return getSecondNeighborThreeOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the four o'clock position
      const CDCWire* getSecondNeighborFourOClock(const WireID& wireID) const
      { return getSecondNeighborFourOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the five o'clock position
      const CDCWire* getSecondNeighborFiveOClock(const WireID& wireID) const
      { return getSecondNeighborFiveOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the six o'clock position
      const CDCWire* getSecondNeighborSixOClock(const WireID& wireID) const
      { return getSecondNeighborSixOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the seven o'clock position
      const CDCWire* getSecondNeighborSevenOClock(const WireID& wireID) const
      { return getSecondNeighborSevenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the eight o'clock position
      const CDCWire* getSecondNeighborEightOClock(const WireID& wireID) const
      { return getSecondNeighborEightOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the nine o'clock position
      const CDCWire* getSecondNeighborNineOClock(const WireID& wireID) const
      { return getSecondNeighborNineOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the ten o'clock position
      const CDCWire* getSecondNeighborTenOClock(const WireID& wireID) const
      { return getSecondNeighborTenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the eleven o'clock position
      const CDCWire* getSecondNeighborElevenOClock(const WireID& wireID) const
      { return getSecondNeighborElevenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the twelve o'clock position
      const CDCWire* getSecondNeighborTwelveOClock(const WireID& wireID) const
      { return getSecondNeighborTwelveOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }
      /**@}*/





      /** @name Secondary neighbor getters by superlayer id , layer id and wire id.
       *  The secondary neighbors are numbered like positions on the normal twelve hour clock.
       *
       *  The rational of this is that the secondary neighbors are positioned \n
       *  just like the numbers on the clock if you took the center of the clock to the wire position \n
       *  and the twelve o'clock points in the direction outwards.
       *
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      ///Getter for secondary neighbor at the one o'clock position
      const CDCWire* getSecondNeighborOneOClock(ISuperLayer iSuperLayer,
                                                ILayer iLayer,
                                                IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborOneOClock(iLayer, iWire);  }

      ///Getter for secondary neighbor at the two o'clock position
      const CDCWire* getSecondNeighborTwoOClock(ISuperLayer iSuperLayer,
                                                ILayer iLayer,
                                                IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborTwoOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the three o'clock position
      const CDCWire* getSecondNeighborThreeOClock(ISuperLayer iSuperLayer,
                                                  ILayer iLayer,
                                                  IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborThreeOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the four o'clock position
      const CDCWire* getSecondNeighborFourOClock(ISuperLayer iSuperLayer,
                                                 ILayer iLayer,
                                                 IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborFourOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the five o'clock position
      const CDCWire* getSecondNeighborFiveOClock(ISuperLayer iSuperLayer,
                                                 ILayer iLayer,
                                                 IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborFiveOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the six o'clock position
      const CDCWire* getSecondNeighborSixOClock(ISuperLayer iSuperLayer,
                                                ILayer iLayer,
                                                IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborSixOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the seven o'clock position
      const CDCWire* getSecondNeighborSevenOClock(ISuperLayer iSuperLayer,
                                                  ILayer iLayer,
                                                  IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborSevenOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the eight o'clock position
      const CDCWire* getSecondNeighborEightOClock(ISuperLayer iSuperLayer,
                                                  ILayer iLayer,
                                                  IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborEightOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the nine o'clock position
      const CDCWire* getSecondNeighborNineOClock(ISuperLayer iSuperLayer,
                                                 ILayer iLayer,
                                                 IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborNineOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the ten o'clock position
      const CDCWire* getSecondNeighborTenOClock(ISuperLayer iSuperLayer,
                                                ILayer iLayer,
                                                IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborTenOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the eleven o'clock position
      const CDCWire* getSecondNeighborElevenOClock(ISuperLayer iSuperLayer,
                                                   ILayer iLayer,
                                                   IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborElevenOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the twelve o'clock position
      const CDCWire* getSecondNeighborTwelveOClock(ISuperLayer iSuperLayer,
                                                   ILayer iLayer,
                                                   IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborTwelveOClock(iLayer, iWire); }
      /**@}*/

    public:
      /// Getter for the outer radius of the outer most wire layer.
      double getOuterCylindricalR() const
      { return m_wireLayers.back().getOuterCylindricalR(); }

      /// Getter for the inner radius of the inner most wire layer.
      double getInnerCylindricalR() const
      { return m_wireLayers.front().getInnerCylindricalR(); }

    private:
      /// Storage for all wires in the CDC
      std::vector<Belle2::TrackFindingCDC::CDCWire> m_wires;
      /// Storage for all wire layers in the CDC
      std::vector<Belle2::TrackFindingCDC::CDCWireLayer> m_wireLayers;
      /// Storage for all wire superlayers in the CDC
      std::vector<Belle2::TrackFindingCDC::CDCWireSuperLayer> m_wireSuperLayers;

      //The singleton implementation for the wire topology.
    private:
      static CDCWireTopology* s_instance;  ///< The Topology instance to be used by the CDCWires

    public:
      /// Getter for the singleton instance of the wire topology. Used by the CDCWires.
      static CDCWireTopology& getInstance()
      {
        if (not s_instance) s_instance = new CDCWireTopology;
        return *s_instance;
      }


    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2
