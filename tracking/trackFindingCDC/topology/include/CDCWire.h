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

#include <vector>
#include <iostream>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/WireID.h>

#include <tracking/trackFindingCDC/topology/ISuperLayerType.h>
#include <tracking/trackFindingCDC/topology/ILayerType.h>
#include <tracking/trackFindingCDC/topology/IWireType.h>

#include <tracking/trackFindingCDC/topology/WireNeighborType.h>

#include <tracking/trackFindingCDC/topology/StereoType.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/WireLine.h>

namespace Belle2 {
  namespace TrackFindingCDC {


    /// Class representing a sense wire in the central drift chamber
    /**
     * CDCWire represents a sense wire of the central drift chamber. \n
     * It combines the wire id and a line representation of the wire from the CDC geometry. \n
     * It also provides an interface for fetching the closest neighbors for local tracking purposes. \n
     * Note : All possible wire object are stored in the CDCWireTopology
     * which you can get with the static getInstance() functions. \n
     * There is rarely a need for constructing a wire object it should be avoided for speed reasons.
     */
    class CDCWire  {

    public:
      /// A wire pointer pair as returned from getNeighborsOutward(), getNeighborsInward()
      typedef std::pair<const Belle2::TrackFindingCDC::CDCWire*, const Belle2::TrackFindingCDC::CDCWire*> NeighborPair;

    public:

      /// Default constructor for ROOT compatibility.
      CDCWire() {}

      /// Constructor taking the WireID convenience object. Use rather getInstance() to avoid instance constructions.
      explicit CDCWire(const WireID& wireID);

      /// Constructor taking the superlayer id, the layer id and the wire id. Use rather getInstance() to avoid instance constructions.
      CDCWire(const ISuperLayerType& iSuperLayer,
              const ILayerType& iLayer,
              const IWireType& iWire);

      /// Empty deconstructor
      ~CDCWire() {}

      /** @name Static instance getters
       *  Getter for the already constructed instances form the CDCWireTopology::getInstance() object.
       */
      /**@{*/
      /// Getter from the wireID convinience object. Does not construct a new object.
      static const CDCWire* getInstance(const WireID& wireID);

      /// Getter from the wireID convinience object. Does not construct a new object.
      static const CDCWire* getInstance(const CDCWire& wire);

      /// Getter from the superlayer id, the layer id and the wire id. Does not construct a new object.
      static const CDCWire* getInstance(const ISuperLayerType& iSuperLayer,
                                        const ILayerType& iLayer,
                                        const IWireType& iWire);

      /// Convinience getter for the wire from a hit object.
      static const CDCWire* getInstance(const CDCHit& hit);
      /**@}*/


    public:
      /// Equality comparision based on wireID.
      bool operator==(const CDCWire& other) const { return getWireID() == other.getWireID(); }

      /// Total ordering relation based on wire id
      /** Defines a total ordering sheme for wire objects based on the encoded wireID only.
       *  Therefor the wires can get sorted for the super layer, than for the layers and finally for the in layer wire id.
       *  Hence the wires increase in counterclockwise spiral like manner from the inside out.
       *  It needs be present for the wire to work with all kinds of stl algorithms and containers */
      bool operator<(const CDCWire& other) const { return getWireID() < other.getWireID(); }

      /// Getter for the wire with superlayer id 0, layer id 0 and wire id 0. Is always less in comparision to other wires.
      static const CDCWire& getLowest() { return  *(getInstance(0, 0, 0)); }

    public:
      /// Updates the line definition of this wire from the CDCGeometry
      void initialize();

      /** @name Wire index
       */
      /**@{*/
      /// Implicit downcast to WireID forgetting the line information as needed
      operator const Belle2::WireID& () const
      { return getWireID(); }

      /// Getter for the wireID
      /** Gives the wireId convenience object. */
      const WireID& getWireID() const { return m_wireID; }

      /// Getter for the encoded wire number.
      /** Gets the encoded wire number. \n
       *  Its is unique to each wire and increases from the inside out.
       *  It increases counterclockwise in the same layer from the wire with
       *  wire id zero.
       *  Ranging from 0 to 35711 but discontinuously. ( See WireID class for details. )*/
      unsigned short getEWire() const { return getWireID().getEWire(); }

      /// Getter for the wire id within its layer
      /**  Gives the layer id within its superlayer \n
       *   ranging from 0 - 159 for superlayer 0, \n
       *   ranging from 0 - 159 for superlayer 1, \n
       *   ranging from 0 - 191 for superlayer 2, \n
       *   ranging from 0 - 223 for superlayer 3, \n
       *   ranging from 0 - 255 for superlayer 4, \n
       *   ranging from 0 - 287 for superlayer 5, \n
       *   ranging from 0 - 319 for superlayer 6, \n
       *   ranging from 0 - 351 for superlayer 7, \n
       *   ranging from 0 - 383 for superlayer 8, */
      IWireType getIWire() const { return getWireID().getIWire(); }

      /// Getter for the continious layer id
      /**  Gives the continious layer id ranging from 0 - 55. */
      ILayerType getICLayer() const { return getWireID().getICLayer(); }

      /// Getter for the layer id within its superlayer
      /**  Gives the layer id within its superlayer \n
       *   ranging from 0 - 7 for superlayer 0, \n
       *   ranging from 0 - 5 for superlayer 1 - 8. */
      ILayerType getILayer() const { return getWireID().getILayer(); }


      /// Getter for superlayer id.
      /**  Gives the superlayer id ranging from 0 - 8. */
      ISuperLayerType getISuperLayer() const { return getWireID().getISuperLayer(); }

      /// Setter for the wireID
      void setWireID(const WireID& wireID)
      { m_wireID.setWireID(wireID); initialize(); }
      /**@}*/


      /** @name Geometry properties
       *  Read only. They get implicitly initialized from the CDCGeometryPar.
       */
      /**@{*/
      /// Getter for the skew line represenation of the wire.
      const WireLine& getSkewLine() const { return m_skewLine; }

      /// Indicates if the wire is axial or stereo
      inline bool isAxial() const { return getStereoType() == StereoType::c_Axial; }

      /// Getter for the stereo type of the wire
      /** Gives the stereo type of the wire.
       *  Result is one of StereoType::c_Axial, StereoType::c_StereoU and StereoType::c_StereoV
       *  The stereo type is shared by all wires in the same superlayer
       *  The superlayer pattern for Belle II is AUAVAUAVA according the TDR
       */
      inline StereoType getStereoType() const
      {
        if ((getISuperLayer() % 2) == 0)  return StereoType::c_Axial;
        else if ((getISuperLayer() % 4) == 1)  return StereoType::c_StereoU;
        else return StereoType::c_StereoV;
      }

      /// Gives the xy projected position of the wire at the given z coordinate
      Vector2D getWirePos2DAtZ(const double z) const
      { return getSkewLine().pos2DAtZ(z); }

      /// Gives position of the wire at the given z coordinate
      Vector3D getWirePos3DAtZ(const double z) const
      { return getSkewLine().pos3DAtZ(z); }

      /// Calculates the distance from the position to the wire
      double getDistance(const Vector3D& pos3D) const
      { return getSkewLine().distance(pos3D); }

      /// Calculates the closest approach in the wire to the position
      Vector3D getClosest(const Vector3D& pos3D) const
      { return getSkewLine().closest3D(pos3D); }

      /** Calculates the straight drift length from the position to the wire
       *  This is essentially the same as the distance to the wire
       *  but returns NAN if either
       *  * the position is outside of the CDC
       *  * the position is outside of the drift cell
       */
      double getDriftLength(const Vector3D& pos3D) const
      { return isInCell(pos3D) ? getDistance(pos3D) : NAN; }

      /// Getter for the wire reference position.
      /** Gives the wire's reference position
       *  which is the point of closest approach to the beam axes.
       */
      const Vector3D& getRefPos3D() const
      { return getSkewLine().refPos3D(); }

      /// Getter for the wire reference position for 2D tracking
      /** Gives the wire's reference position projected to the xy plane.
       */
      const Vector2D& getRefPos2D() const
      { return getSkewLine().refPos2D(); }

      /// Getter for the wire reference z coordinate
      /** Gives the wire's reference z coordinate
       */
      double getRefZ() const
      { return getSkewLine().refZ(); }

      /// Getter for the tangents of the stereo angle of the wire.
      double getTanStereoAngle() const { return getSkewLine().tanTheta(); }

      /// Getter for the stereo angle of the wire.
      double getStereoAngle() const { return getSkewLine().theta(); }

      /// Getter for the vector pointing from the back end ofthe wire to the front end of the wire
      Vector3D getWireVector() const { return getSkewLine().tangential3D(); }

      /// Getter for the vector describing the positional change in the xy plane per unit z.
      Vector2D getMovePerZ() const { return getSkewLine().movePerZ(); }

      /// Getter for the cylindrical radius at the wire reference position
      double getRefCylindricalR() const { return m_refCylindricalR; }

      /// Getter for the closest distance to the beamline ( z-axes )
      double getMinCylindricalR() const { return getSkewLine().perigee2D().norm(); }

      ///Getter for the distance to the beamline ( z-axes ) at the forward joint point
      double getForwardCylindricalR() const { return getSkewLine().forwardCylindricalR(); };

      ///Getter for the distance to the beamline ( z-axes ) at the backward joint point
      double getBackwardCylindricalR() const { return getSkewLine().forwardCylindricalR(); };

      /// Getter for the z coordinate at the forward joint points of the wires
      double getForwardZ() const { return getSkewLine().forwardZ(); }

      /// Getter for the z coordinate at the backward joint points of the wires
      double getBackwardZ() const { return getSkewLine().backwardZ(); }

      /// Checks whether the position is in the drift cell surrounding the wire
      bool isInCell(const Vector3D& pos3D) const;

      /// Checks whether the position is in the z bounds of the drift cell (scaled by the factor) surrounding the wire
      bool isInCellZBounds(const Vector3D& pos3D, const double factor = 1) const
      {
        return getBackwardZ() * factor < pos3D.z() and pos3D.z() < getForwardZ() * factor;
      }

      /// Getter for the azimuth angle of the forward joint point of the wire relativ to its reference
      double getForwardPhiToRef() const { return m_forwardPhiToRef; }

      /// Getter for the azimuth angle of the backward joint point of the wire relativ to its reference
      double getBackwardPhiToRef() const { return m_backwardPhiToRef; }

      /// Getter for azimuth angle range covered by the wire relativ to the reference point.
      /** Gives the range of azimuth angles a stereo wires covers relativ to its reference position. \n
       *  A stereo wire has a certain extension visible as a stretch in the xy projection. \n
       *  This translates to a range of azimuth angles, which can be used for a quick check \n
       *  if an xy projected track could have hit a wire or not. \n
       *  The range covers reaches from negativ to positiv values since the reference is somewhere \n
       *  in the middle of the wire. \n
       *  The smaller value is always negativ and gets stored as .first of the pair. \n
       *  The bigger value is positiv and gets stored as .second of the pair. \n
       *  For axial wires both values or zero since axial wires form a point in the xy projection.
       */
      const std::pair<double, double>& getPhiRangeToRef() const
      { return m_phiRangeToRef; }
      /**@}*/


      /** @name Closest neighborhood
       *  Methods for getting the closest neighbors of the wire. They use the CDCWireTopology::getInstance() object to find the neighbors of this wire.
       */
      /**@{*/

      /// Calculates whether the give wire is a neighbor of this wire
      /** Gives the information if the given wire is a neighbor of this wire.
       *  A return value unequal zero states that the two are  indeed neighbors.
       *  Moreover the return value also gives the direction in which you have to go to get to the neighbor.
       *  Possible return values are: \n
       *  CW_OUT_NEIGHBOR = 1  for clockwise outwards \n
       *  CW_NEIGHBOR = 3 for clockwise \n
       *  CW_IN_NEIGHBOR = 5 for clockwise inwards \n
       *  CCW_IN_NEIGHBOR = 7 for counterclockwise inwards \n
       *  CCW_NEIGHBOR = 9 for counterclockwise \n
       *  CCW_OUT_NEIGHBOR = 11 for counterclockwise outwards \n
       *  The values are choosen to have an assoziation with the numbers on a regular clock.
       */
      WireNeighborType isNeighborWith(const CDCWire& wire) const;

      /// Gives the two wires in the next layer inward.
      /** Gives the two wire in the next layer closer to the interaction point from this wire.
       *  The pair is sorted such that the more counterclockwise wire is the .first.
       *  This does not cross superlayer boundaries. Trying to get the inwards neighbors from
       *  a wire in the innermost layer of a superlayer will return a pair of nullptr.
       *  So the result has to be checked before referencing.
       */
      NeighborPair getNeighborsInwards() const;

      /// Gives the two wires in the next layer outward.
      /** Gives the two wire in the next layer further away from the interaction point from this wire.
       *  The pair is sorted such that the more counterclockwise wire is the .first.
       *  This does not cross superlayer boundaries. Trying to get the outwards neighbors from
       *  a wire in the outermost layer of a superlayer will return a pair of nullptr.
       *  So the result has to be checked before referencing.
       */
      NeighborPair getNeighborsOutwards() const;

      /// Gives the closest neighbor in the counterclockwise direction.
      /** Always gives the counterclockwise neighbor of this wire, since it always exists.
       *  It never returns nullptr (if the wire itself is valid),
       *  but we give it as pointer for homogenity of the interface.
       */
      const CDCWire* getNeighborCCW() const;

      /// Gives the closest neighbor in the clockwise direction
      /** Always gives the clockwise neighbor of this wire, since it always exists.
       *  It never returns nullptr (if the wire itself is valid),
       *  but we give it as pointer for homogenity of the interface.
       */
      const CDCWire* getNeighborCW() const;

      /// Gives the closest neighbor in the countclockwise inwards direction
      /**
       *  This does not cross superlayer boundaries. Trying to get the countclockwise inwards neighbor
       *  from a wire in the innermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      const CDCWire* getNeighborCCWInwards() const;

      /// Gives the closest neighbor in the clockwise inwards direction
      /**
       *  This does not cross superlayer boundaries. Trying to get the clockwise inwards neighbor
       *  from a wire in the innermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      const CDCWire* getNeighborCWInwards() const;

      /// Gives the closest neighbor in the countclockwise outwards direction
      /**
       *  This does not cross superlayer boundaries. Trying to get the countclockwise outwards neighbor
       *  from a wire in the outermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      const CDCWire* getNeighborCCWOutwards() const;

      /// Gives the closest neighbor in the clockwise outwards direction
      /**
       *  This does not cross superlayer boundaries. Trying to get the clockwise outwards neighbor
       *  from a wire in the outermost layer of a superlayer will return a nullptr.
       *  So the result has to be checked before referencing.
       */
      const CDCWire* getNeighborCWOutwards() const;
      /**@}*/

      /// Sting output operator for wire objects to help debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCWire& wire)
      {
        output << "CDCWire(" << wire.getISuperLayer() << "," << wire.getILayer() << "," << wire.getIWire() << ")" ;
        return output;
      }

      /// Sting output operator for wire pointers to help debugging
      //friend std::ostream& operator<<(std::ostream& output, const CDCWire* wire)
      //{ if (wire == nullptr) output << "nullptr"; else output << *wire;  return output; }

    private:

      WireID m_wireID; ///< The wireID of the wire

      WireLine m_skewLine; ///< The line representation of the wire
      double m_refCylindricalR;  ///< Precomputed distance to the beam line at the reference position.


      double m_forwardPhiToRef; ///< Storage of the angle between forward and ref position in the xy projection (see getPhiRangeToRef() for details)
      double m_backwardPhiToRef;///< Storage of the angle between backward and ref position in the xy projection (see getPhiRangeToRef() for details)
      std::pair<double, double>
      m_phiRangeToRef; ///< Storage of the angle range to ref position in the xy projection (see getPhiRangeToRef() for details)

    private:


    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
