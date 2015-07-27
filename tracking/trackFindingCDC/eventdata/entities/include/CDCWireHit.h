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

#include <cdc/dataobjects/CDCHit.h>

#include <tracking/trackFindingCDC/algorithms/AutomatonCell.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    const FloatType SIMPLE_DRIFT_LENGTH_VARIANCE  = 0.000169;

    /** Class representing a hit wire in the central drift chamber
     *  This class combines the measurement informations from a CDCHit
     *  with the geometry informations.
     *  It forms the basis of all other higher level tracking objects.
     *  It contains an AutomataCell since we want to use it
     *  with the Clusterizer which makes use of the cell state.
     *  Additionally contains references to both the CDCWire instance from the CDCTopology and
     *  the CDCHit from the StoreArray of the event.
     */
    class CDCWireHit {

    public:
      static CDC::TDCCountTranslatorBase& getTDCCountTranslator();

    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHit();

    private:
      /// Constructor for interal use
      /** Construct for a dummy CDCWireHit. The wire hit is still assoziated with a wire
      but has no hit attached to it.
          The getHit() will yield nullptr. The getStoreIHit() yields -1 for this case. */
      explicit CDCWireHit(const CDCWire* ptrWire);

    public:
      /// Constructor for augmenting the CDCHit with the geometry information of the CDCWire.
      /** Binds to the CDCHit and the corresponding wire together and translates the TDC count to a driftlength.
       *  Also takes the index of the StoreArray
       *  from which the hit has been taken. Necessary for later translation to genfit.
       *
       *  @param  ptrHit          Reference to the CDCHit that should be
       *  @param  ptrTranslator   Optional xt relation used to translate the tdc count to a drift length.
       *                          If omitted use the default from the CDCWireHitTopology.
       */
      CDCWireHit(const CDCHit* ptrHit, CDC::TDCCountTranslatorBase* ptrTranslator = nullptr);

      /// Constructor to taking a wire ID and a driftlength at the reference. For testing only!
      CDCWireHit(const WireID& wireID,
                 const FloatType& driftLength,
                 const FloatType& driftlengthVariance = SIMPLE_DRIFT_LENGTH_VARIANCE);

    public:
      /// Make the wire hit automatically castable to its underlying cdcHit
      operator const Belle2::CDCHit* () const
      { return m_hit; }

      /// Equality comparision based the wire and the hit id.
      bool operator==(const CDCWireHit& other) const
      { return getWire() == other.getWire() and getRefDriftLength() == other.getRefDriftLength(); }


      /// Total ordering relation based on the wire and the hit id.
      bool operator<(const CDCWireHit& other) const
      {
        return
          std::make_pair(getWire(), getRefDriftLength()) <
          std::make_pair(other.getWire(), other.getRefDriftLength());
      }

      /// Defines wires and wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWireHit& wireHit, const CDCWire& wire)
      { return wireHit.getWire() < wire; }

      /// Defines wires and wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWire& wire, const CDCWireHit& wireHit)
      { return wire < wireHit.getWire(); }

      /// Defines wires and wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWireHit* wireHit, const CDCWire& wire)
      { assert(wireHit); return  wireHit->getWire() < wire; }

      /// Defines wires and wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWire& wire, const CDCWireHit* wireHit)
      { assert(wireHit); return wire < wireHit->getWire(); }

      /// Defines wire hits and raw hits to be coaligned.
      friend bool operator<(const CDCWireHit& wireHit, const CDCHit& hit)
      { return wireHit.getWire().getEWire() < hit.getID(); }

      /// Defines wire hits and raw hits to be coaligned.
      friend bool operator<(const CDCHit& hit, const CDCWireHit& wireHit)
      { return hit.getID() < wireHit.getWire().getEWire(); }

      /// Defines wires and wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWireHit& wireHit, const CDCWireSuperLayer& wireSuperLayer)
      { return wireHit.getISuperLayer() < wireSuperLayer.getISuperLayer(); }

      /// Defines wires and wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCWireHit& wireHit)
      { return wireSuperLayer.getISuperLayer() < wireHit.getISuperLayer(); }

      /// Getter for the CDCHit pointer into the StoreArray
      const CDCHit* getHit() const
      { return m_hit; }

      /// Getter for the CDCWire the hit is located on.
      const CDCWire& getWire() const
      { return *m_wire; }

      /// Getter for the WireID of the wire the hit is located on.
      const WireID& getWireID() const
      { return getWire().getWireID(); }

      /// The two dimensional reference position of the underlying wire
      const Vector2D& getRefPos2D() const
      { return getWire().getRefPos2D(); }

      /// The distance from the beam line at reference position of the underlying wire
      FloatType getRefCylindricalR() const
      { return getWire().getRefCylindricalR(); }

      /// Connection vector from the center of this wire hit to the other.
      Vector2D getRefVecTo2D(const CDCWireHit& to) const
      { return to.getRefPos2D() - getRefPos2D(); }

      /// Getter for the  drift length at the reference position of the wire.
      FloatType getRefDriftLength() const
      { return m_refDriftLength; }

      /// Getter for the variance of the drift length at the reference position of the wire.
      FloatType getRefDriftLengthVariance() const
      { return m_refDriftLengthVariance; }

      /// Getter for the skew of the wire.
      FloatType getSkew() const
      { return getWire().getSkew(); }

      /// Estimate the transvers travel distance on the given circle.
      /** Uses the point of closest approach on the circle and
       *  calculates the arc length from the reference on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getRefPos2D()); }

      /// Reconstuct the wire reference position onto the given trajectory
      Vector2D getRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      {
        StereoType stereoType = getStereoType();
        if (stereoType == AXIAL) {
          return trajectory2D.getClosest(getRefPos2D());
        } else {
          // TODO replace with something better if at all possible
          return trajectory2D.getCloseSameCylindricalR(getRefPos2D());
        }
      }

      /// Getter for the index  of the hit in the StoreArray holding this hit.
      Index getStoreIHit() const
      { return getHit() ? getHit()->getArrayIndex() : INVALID_INDEX; }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell()
      { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const
      { return m_automatonCell; }

      /** Cast operator to the reference of the automaton cell.
       *  Making AutomatonCell a constant pseudo base class of CDCWireHit
       */
      operator const Belle2::TrackFindingCDC::AutomatonCell& () const
      { return m_automatonCell; }


      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference
       *  there is no way to tell if one should use the dot '.' or operator '->' for method look up.
       *  So this function defines the -> operator for the object.
       *  No matter you have a pointer or an object access is given with '->'*/
      /**@{*/
      const CDCWireHit* operator->() const
      { return this; }

      /// Allow automatic taking of the address.
      /** Essentially pointers to (lvalue) objects is a subclass of the object itself.
       *  This method activally exposes this inheritance to be able to write algorithms
       *  that work for objects and poiinters alike without code duplication.
       *  \note Once reference qualifiers become available use an & after
       *  the trailing const to constrain the cast to lvalues.*/
      operator const Belle2::TrackFindingCDC::CDCWireHit* () const
      { return this; }
      /**@}*/


      /** @name Methods common to all tracking entities
       *  All entities ( track parts contained in a single superlayer ) share this interface to help
       *  the definition of collections of them. */
      /**@{*/
      /// Checks of the wire hit is base on the wire given
      bool hasWire(const CDCWire& wire) const
      { return getWire() == wire; }

      /// Checks if the wire hit is equal to the wire hit given.
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return operator==(wirehit); }

      /// Center of mass is just the refernce position for wire hits.
      const Vector2D& getCenterOfMass2D() const { return getRefPos2D(); }

      /// Getter for the stereo type of the underlying wire.
      StereoType getStereoType() const
      { return getWire().getStereoType(); }

      /// Getter for the super layer id
      ILayerType getISuperLayer() const
      { return getWire().getISuperLayer(); }

      /// Same as getPerpS().
      FloatType getFrontPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getBackPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getRecoPos2D()
      Vector2D getFrontRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return getRecoPos2D(trajectory2D); }

      /// Same as getRecoPos2D()
      Vector2D getBackRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return getRecoPos2D(trajectory2D); }

      /// Calculates the squared distance of the wire hit to a circle as see from the transvers plane
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const
      {
        FloatType distance = trajectory2D.getDist2D(getRefPos2D());
        distance = distance > 0 ? distance - getRefDriftLength() : distance + getRefDriftLength();
        return distance * distance;
      }
      /**@}*/

      /// Sting output operator for wire hit objects to help debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCWireHit& wirehit)
      {
        return output << "CDCWireHit(" << wirehit.getWire() << ",dl=" << wirehit.getRefDriftLength() << ")";
      }

      /// Sting output operator for wire hit pointers to help debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCWireHit* wirehit)
      { return wirehit == nullptr ? output << "nullptr" : output << *wirehit; }

    private:
      /** Memory for the automaton cell. Marked as mutable since its content should be changeable
       *  even if the wire and drift length information are required to be constant. */
      AutomatonCell m_automatonCell;

      /// Memory for the drift length at the wire reference point
      FloatType m_refDriftLength = 0;

      /// Memory for the variance of the drift length at the wire reference point
      FloatType m_refDriftLengthVariance = SIMPLE_DRIFT_LENGTH_VARIANCE;

      /// Memory for the CDCWire reference
      const CDCWire* m_wire = nullptr;

      /// Memory for the CDCWire reference
      const CDCHit* m_hit = nullptr;

    }; //class CDCWireHit

  } // namespace TrackFindingCDC
} // namespace Belle2
