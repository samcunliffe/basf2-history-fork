/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

namespace Belle2 {

  /// Forward declaration
  class CDCSimHit;

  namespace TrackFindingCDC {

    /** Class representing an oriented hit wire including a hypotheses
     *  whether the causing track passes left or right
     */
    class CDCRLWireHit {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRLWireHit();

      /// Constructs an oriented wire hit
      /** Constructs an oriented wire hit
       *  @param wireHit the wire hit the oriented hit is assoziated with.
       *  @param rlInfo the right left passage information the _wire_ relativ to the track */
      CDCRLWireHit(const CDCWireHit* wireHit, RightLeftInfo rlInfo = 0);

      /// Constructs an oriented wire hit from a sim hit and the assoziated wirehit.
      /** This translates the sim hit to an oriented wire hit mainly to be able to compare the \n
       *  reconstructed values from the algorithm with the Monte Carlo information. \n
       *  It merely evalutates, if the true trajectory passes right or left of the wire. */
      static CDCRLWireHit fromSimHit(const CDCWireHit* wirehit, const CDCSimHit& simhit);

    public:
      /// Returns the oriented wire hit with the opposite right left information.
      const CDCRLWireHit* reversed() const;

      /// Make the wire hit automatically castable to its underlying cdcHit
      operator const Belle2::CDCHit* () const
      { return getWireHit().getHit(); }

      /// Equality comparision based on wire hit, left right passage information.
      bool operator==(const CDCRLWireHit& other) const
      { return getWireHit() == other.getWireHit() and getRLInfo() == other.getRLInfo(); }

      /** Total ordering relation based on
       *  wire hit and left right passage information in this order of importance.
       */
      bool operator<(const CDCRLWireHit& other) const
      {
        return getWireHit() <  other.getWireHit() or (
                 getWireHit() == other.getWireHit() and (
                   getRLInfo() < other.getRLInfo()));
      }

      /// Defines wires and oriented wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCRLWireHit& rlWireHit, const CDCWire& wire)
      { return rlWireHit.getWire() < wire; }

      /// Defines wires and oriented wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWire& wire, const CDCRLWireHit& rlWireHit)
      { return wire < rlWireHit.getWire(); }

      /** Defines wire hits and oriented wire hits to be coaligned on the wire hit
       *  on which they are based.
       */
      friend bool operator<(const CDCRLWireHit& rlWireHit, const CDCWireHit& wireHit)
      { return rlWireHit.getWireHit() < wireHit; }

      /** Defines wire hits and oriented wire hits to be coaligned on the wire hit
       * on which they are based.
       */
      friend bool operator<(const CDCWireHit& wireHit, const CDCRLWireHit& rlWireHit)
      { return wireHit < rlWireHit.getWireHit(); }



      /// The two dimensional reference position of the underlying wire
      const Vector2D& getRefPos2D() const
      { return getWireHit().getRefPos2D(); }

      /// The distance from the beam line at reference position of the underlying wire
      FloatType getRefCylindricalR() const
      { return getWire().getRefCylindricalR(); }

      /// Getter for the wire the oriented hit assoziated to.
      const CDCWire& getWire() const
      { return getWireHit().getWire(); }

      /// Getter for the  drift length at the reference position of the wire
      FloatType getRefDriftLength() const
      { return getWireHit().getRefDriftLength(); }

      /// Getter for the  drift length at the reference position of the wire
      FloatType getSignedRefDriftLength() const
      { return ((SignType)(getRLInfo())) * getRefDriftLength(); }

      /// Getter for the variance of the drift length at the reference position of the wire.
      FloatType getRefDriftLengthVariance() const
      { return getWireHit().getRefDriftLengthVariance(); }

      /// Getter for the wire hit assoziated with the oriented hit.
      const CDCWireHit& getWireHit() const
      { return *m_wirehit; }

      /// Getter for the right left passage information.
      const RightLeftInfo& getRLInfo() const
      { return m_rlInfo; }

      /** Reconstructs a position of primary ionisation on the drift circle.
       *
       *  The result is the position of closest approach on the drift circle to the trajectory.
       *
       *  All positions and the trajectory are interpreted to lie at z=0.
       *  Also the right left passage hypotheses does not play a role in
       *  the reconstruction in any way.
       */
      Vector2D reconstruct2D(const CDCTrajectory2D& trajectory2D) const
      { return getWireHit().reconstruct2D(trajectory2D); }

      /** Attempts to reconstruct a three dimensional position (especially of stereo hits)
       *
       *  This method makes a distinct difference between axial and stereo hits:
       *  * Stereo hits are moved out of the reference plane such that the
       *    oriented drift circle meets the trajectory in one point. Therefore the
       *    left right passage hypothese has to be taken into account
       *  * For axial hits the reconstructed position is ambiguous in the z coordinate.
       *    Also the drift circle cannot moved such that it would meet the
       *    trajectory. Hence we default to the result of reconstruct2D, which
       *    yield the closest approach of the drift circle to the trajectory
       *    in the reference plane.
       */
      Vector3D reconstruct3D(const CDCTrajectory2D& trajectory2D) const
      { return getWireHit().reconstruct3D(trajectory2D, getRLInfo()); }

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference
       *  there is no way to tell if one should use the dot '.' or operator '->' for method look up.
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'
       */
      const CDCRLWireHit* operator->() const
      { return this; }

      /// Checks if the oriented hit is assoziated with the give wire
      bool hasWire(const CDCWire& wire) const
      { return getWireHit().getWire() == wire; }

      /// Checks if the oriented hit is assoziated with the give wire hit
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getWireHit() == wirehit; }

      /// Getter for the stereo type of the underlying wire.
      StereoType getStereoType() const
      { return getWireHit().getStereoType(); }

      /// Getter for the superlayer id
      ILayerType getISuperLayer() const
      { return getWireHit().getISuperLayer(); }

      /// Output operator. Help debugging.
      friend std::ostream& operator<<(std::ostream& output, const CDCRLWireHit& rlWireHit)
      {
        output << "CDCRLWireHit(" << rlWireHit.getWireHit() << ","
               << rlWireHit.getRLInfo() << ")" ;
        return output;
      }

    private:
      /// Memory for the reference to the assiziated wire hit
      const CDCWireHit* m_wirehit;

      /// Memory for the right left passage information of the oriented wire hit.
      RightLeftInfo m_rlInfo;

    }; //class CDCRLWireHit

  } // namespace TrackFindingCDC
} // namespace Belle2
