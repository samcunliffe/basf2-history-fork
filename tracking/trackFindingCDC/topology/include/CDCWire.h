/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/topology/WireNeighborPair.h>
#include <tracking/trackFindingCDC/topology/WireLine.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>
#include <tracking/trackFindingCDC/topology/IWire.h>

#include <tracking/trackFindingCDC/topology/EWireNeighborKind.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <cdc/dataobjects/WireID.h>

#include <iostream>

namespace Belle2 {
  class CDCHit;
}

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Class representing a sense wire in the central drift chamber.
     *  It combines the wire id and a line representation of the wire from the CDC geometry.
     *  It also provides an interface for fetching the closest neighbors for local tracking purposes.
     *  Note : All possible wire objects are stored in the CDCWireTopology
     *  which you can get with the static getInstance() functions.
     *  There is rarely a need for constructing a wire object and it should be avoided.
     */
    class CDCWire {

    public:
      /// Constructor taking the combined wire id convenience object.
      CDCWire(const WireID& wireID);

      /// Constructor taking the superlayer id, the layer id and the wire id. Use rather getInstance() to avoid instance constructions.
      CDCWire(ISuperLayer iSuperLayer, ILayer iLayer, IWire iWire);

      /**
       *  @name Static instance getters
       *  Getter for the already constructed instances form the CDCWireTopology::getInstance() object.
       */
      /**@{*/

      /// Getter from the wireID convinience object. Does not construct a new object.
      static MayBePtr<const CDCWire> getInstance(const WireID& wireID);

      /// Getter from the superlayer id, the layer id and the wire id. Does not construct a new object.
      static MayBePtr<const CDCWire> getInstance(ISuperLayer iSuperLayer, ILayer iLayer, IWire iWire);

      /// Convenience getter for the wire from a hit object.
      static MayBePtr<const CDCWire> getInstance(const CDCHit& hit);
      /**@}*/


    public:
      /// Equality comparision based on wireID.
      bool operator==(const CDCWire& other) const
      { return getWireID() == other.getWireID(); }

      /**
      *  Total ordering relation based on the wire id
      *  Defines a total ordering scheme for wire objects based on the encoded wireID only.
      *  Therefore the wires can get sorted for the super layer,
      *  than for the layers and finally for the in layer wire id.
      *  Hence the wires increase in counterclockwise spiral like manner from the inside out.
      *  It is required for the wires to work with the stl algorithms and containers.
      */
      bool operator<(const CDCWire& other) const
      { return getWireID() < other.getWireID(); }

    public:
      /// Updates the line definition of this wire from the CDCGeometry
      void initialize();

      /**
      *  @name Wire index
      */
      /**@{*/

      /// Implicit downcast to WireID forgetting the line information as needed
      operator const Belle2::WireID& () const
      { return getWireID(); }

      /// Getter for the wire id
      const WireID& getWireID() const
      { return m_wireID; }

      /**
      *  Getter for the encoded wire number.
      *  It is unique to each wire and increases from the inside out.
      *  It increases counterclockwise in the same layer from the wire with wire id zero.
      *  Ranging from 0 to 35711 but discontinuously. ( See WireID class for details. )
      */
      unsigned short getEWire() const
      { return getWireID().getEWire(); }

      /**
      *  Getter for the wire id within its layer.
      *  ranging from 0 - 159 for superlayer 0, \n
      *  ranging from 0 - 159 for superlayer 1, \n
      *  ranging from 0 - 191 for superlayer 2, \n
      *  ranging from 0 - 223 for superlayer 3, \n
      *  ranging from 0 - 255 for superlayer 4, \n
      *  ranging from 0 - 287 for superlayer 5, \n
      *  ranging from 0 - 319 for superlayer 6, \n
      *  ranging from 0 - 351 for superlayer 7, \n
      *  ranging from 0 - 383 for superlayer 8.
      */
      IWire getIWire() const
      { return getWireID().getIWire(); }

      /// Getter for the continious layer id ranging from 0 - 55.
      ILayer getICLayer() const
      { return getWireID().getICLayer(); }

      /**
      *  Getter for the layer id within its superlayer
      *  Gives the layer id within its superlayer \n
      *  ranging from 0 - 7 for superlayer 0, \n
      *  ranging from 0 - 5 for superlayer 1 - 8.
      */
      ILayer getILayer() const
      { return getWireID().getILayer(); }

      ///  Gives the superlayer id ranging from 0 - 8.
      ISuperLayer getISuperLayer() const
      { return getWireID().getISuperLayer(); }

      /// Setter for the wireID
      void setWireID(const WireID& wireID)
      { m_wireID.setWireID(wireID); initialize(); }
      /**@}*/

      /**
      *  @name Geometry properties
      *  Read only. They get implicitly initialized from the CDCGeometryPar.
      */
      /**@{*/

      /// Indicates if the wire is axial or stereo
      bool isAxial() const
      { return getStereoKind() == EStereoKind::c_Axial; }

      /**
      *  Getter for the stereo type of the wire.
      *  Gives the stereo type of the wire.
      *  Result is one of EStereoKind::c_Axial, EStereoKind::c_StereoU and EStereoKind::c_StereoV
      *  The stereo type is shared by all wires in the same superlayer
      *  The superlayer pattern for Belle II is AUAVAUAVA according the TDR
      */
      EStereoKind getStereoKind() const
      { return ISuperLayerUtil::getStereoKind(getISuperLayer()); }

      /// Getter for the wire line represenation of the wire.
      const WireLine& getWireLine() const
      { return m_wireLine; }

      /// Gives the xy projected position of the wire at the given z coordinate
      Vector2D getWirePos2DAtZ(const double z) const
      { return getWireLine().pos2DAtZ(z); }

      /// Gives position of the wire at the given z coordinate
      Vector3D getWirePos3DAtZ(const double z) const
      { return getWireLine().pos3DAtZ(z); }

      /// Calculates the distance from the position to the wire
      double getDistance(const Vector3D& pos3D) const
      { return getWireLine().distance(pos3D); }

      /// Calculates the closest approach in the wire to the position
      Vector3D getClosest(const Vector3D& pos3D) const
      { return getWireLine().closest3D(pos3D); }

      /**
      *  Calculates the straight drift length from the position to the wire
      *  This is essentially the same as the distance to the wire
      *  but returns NAN if either
      *  * the position is outside of the CDC
      *  * the position is outside of the drift cell
      */
      double getDriftLength(const Vector3D& pos3D) const
      { return isInCell(pos3D) ? getDistance(pos3D) : NAN; }

      /**
      *  Getter for the wire reference position.
      *  Gives the wire's reference position
      *  which is the point of closest approach to the beam axes.
      */
      const Vector3D& getRefPos3D() const
      { return getWireLine().refPos3D(); }

      /**
      *  Getter for the wire reference position for 2D tracking
      * Gives the wire's reference position projected to the xy plane.
      */
      const Vector2D& getRefPos2D() const
      { return getWireLine().refPos2D(); }

      /**
      *  Getter for the wire reference z coordinate
      *  Gives the wire's reference z coordinate
      */
      double getRefZ() const
      { return getWireLine().refZ(); }

      /// Getter for the tangents of the stereo angle of the wire.
      double getTanStereoAngle() const
      { return getWireLine().tanTheta(); }

      /// Getter for the stereo angle of the wire.
      double getStereoAngle() const
      { return getWireLine().theta(); }

      /// Getter for the vector pointing from the back end ofthe wire to the front end of the wire
      Vector3D getWireVector() const
      { return getWireLine().tangential3D(); }

      /// Getter for the vector describing the positional change in the xy plane per unit z.
      Vector2D getMovePerZ() const
      { return getWireLine().movePerZ(); }

      /// Getter for the cylindrical radius at the wire reference position
      double getRefCylindricalR() const
      { return m_refCylindricalR; }

      /// Getter for the closest distance to the beamline ( z-axes )
      double getMinCylindricalR() const
      { return getWireLine().perigee2D().norm(); }

      /// Getter for the distance to the beamline ( z-axes ) at the forward joint point
      double getForwardCylindricalR() const
      { return getWireLine().forwardCylindricalR(); };

      /// Getter for the distance to the beamline ( z-axes ) at the backward joint point
      double getBackwardCylindricalR() const
      { return getWireLine().forwardCylindricalR(); };

      /// Getter for the z coordinate at the forward joint points of the wires
      double getForwardZ() const
      { return getWireLine().forwardZ(); }

      /// Getter for the z coordinate at the backward joint points of the wires
      double getBackwardZ() const
      { return getWireLine().backwardZ(); }

      /// Checks whether the position is in the drift cell surrounding the wire
      bool isInCell(const Vector3D& pos3D) const;

      /// Checks whether the position is in the z bounds of the drift cell (scaled by the factor) surrounding the wire
      bool isInCellZBounds(const Vector3D& pos3D, const double factor = 1) const
      { return getBackwardZ() * factor < pos3D.z() and pos3D.z() < getForwardZ() * factor; }
      /**@}*/


      /**
       *  @name Closest neighborhood
       *  Methods for getting the closest neighbors of the wire.
       *  They use the CDCWireTopology::getInstance() object to find the neighbors of this wire.
       */
      /**@{*/

      /**
       *  Returns gives the kind of neighborhood relation from this wire to the given wire.
       *  Gives the information if the given wire is a neighbor of this wire.
       *  A return value unequal zero states that the two are  indeed neighbors.
       *  Moreover the return value also gives the direction in which you have to go to get to the neighbor.
       *  Possible return values are: \n
       *  EWireNeighborKind::c_None = 0 for wires that are not neighbors \n
       *  EWireNeighborKind::c_CWOut = 1 for clockwise outwards \n
       *  EWireNeighborKind::c_CW = 3 for clockwise \n
       *  EWireNeighborKind::c_CWIn = 5 for clockwise inwards \n
       *  EWireNeighborKind::c_CCWIn = 7 for counterclockwise inwards \n
       *  EWireNeighborKind::c_CCW = 9 for counterclockwise \n
       *  EWireNeighborKind::c_CCWOut = 11 for counterclockwise outwards \n
       *  The values are choosen to have an assoziation with the numbers on a regular clock.
       */
      EWireNeighborKind getNeighborKind(const CDCWire& wire) const;

      /// Returns whether the give wire is a neighbor of this wire
      bool isNeighborWith(const CDCWire& wire) const;

      /**
       *  Gives the two wires in the next layer inward.
       *  Gives the two wire in the next layer closer to the interaction point from this wire.
       *  The pair is sorted such that the more counterclockwise wire is the .first.
       *  This does not cross superlayer boundaries. Trying to get the inwards neighbors from
       *  a wire in the innermost layer of a superlayer will return a pair of nullptr.
       *  So the result has to be checked before referencing.
       */
      WireNeighborPair getNeighborsInwards() const;

      /**
       *  Gives the two wires in the next layer outward.
       *  Gives the two wire in the next layer further away from the interaction point from this wire.
       *  The pair is sorted such that the more counterclockwise wire is the .first.
       *  This does not cross superlayer boundaries. Trying to get the outwards neighbors from
       *  a wire in the outermost layer of a superlayer will return a pair of nullptr.
       *  So the result has to be checked before referencing.
       */
      WireNeighborPair getNeighborsOutwards() const;

      /**
       *  Gives the closest neighbor in the counterclockwise direction - always exists.
       *  Always gives the counterclockwise neighbor of this wire, since it always exists.
       *  It never returns nullptr (if the wire itself is valid),
       *  but we give it as pointer for homogenity of the interface.
       */
      MayBePtr<const CDCWire> getNeighborCCW() const;

      /**
       *  Gives the closest neighbor in the clockwise direction - always exists.
       *  Always gives the clockwise neighbor of this wire, since it always exists.
       *  It never returns nullptr (if the wire itself is valid),
       *  but we give it as pointer for homogenity of the interface.
       */
      MayBePtr<const CDCWire> getNeighborCW() const;

      /**
       *  Gives the closest neighbor in the countclockwise inwards direction
       *  This does not cross superlayer boundaries. Trying to get the countclockwise inwards neighbor
       *  from a wire in the innermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      MayBePtr<const CDCWire> getNeighborCCWInwards() const;

      /**
       *  Gives the closest neighbor in the clockwise inwards direction
       *  This does not cross superlayer boundaries. Trying to get the clockwise inwards neighbor
       *  from a wire in the innermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      MayBePtr<const CDCWire> getNeighborCWInwards() const;

      /**
       *  Gives the closest neighbor in the countclockwise outwards direction
       *  This does not cross superlayer boundaries. Trying to get the countclockwise outwards neighbor
       *  from a wire in the outermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      MayBePtr<const CDCWire> getNeighborCCWOutwards() const;

      /**
       *  Gives the closest neighbor in the clockwise outwards direction
       *  This does not cross superlayer boundaries. Trying to get the clockwise outwards neighbor
       *  from a wire in the outermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      MayBePtr<const CDCWire> getNeighborCWOutwards() const;
      /**@}*/

      /// Sting output operator for wire objects to help debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCWire& wire)
      { return (output << "CDCWire(" << wire.getISuperLayer() << "," << wire.getILayer() << "," << wire.getIWire() << ")") ; }

    private:
      /// The wireID of the wire
      WireID m_wireID;

      /// The line representation of the wire
      WireLine m_wireLine;

      /// Precomputed distance to the beam line at the reference position.
      double m_refCylindricalR;

    }; // class
  } // namespace TrackFindingCDC
} // namespace Belle2
