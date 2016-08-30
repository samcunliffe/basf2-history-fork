/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

namespace Belle2 {
  class CDCSimHit;

  namespace TrackFindingCDC {

    /**
     *  Class representing an oriented hit wire including a hypotheses
     *  whether the causing track passes left or right.
     *  The right left information is freely setable.
     *  More than one CDCRLWireHit can point to one CDCWireHit.
     *  For more information see @sa CDCWireHit.
     */
    class CDCRLWireHit {

    public:
      /// Default constructor for ROOT
      CDCRLWireHit() = default;

      /**
       *  Constructs an oriented wire hit with unknown left right passage information.
       *  @param wireHit      The wire hit the oriented hit is associated with.
       */
      CDCRLWireHit(const CDCWireHit* wireHit);

      /**
       *  Constructs an oriented wire hit.
       *  @param wireHit      The wire hit the oriented hit is associated with.
       *  @param rlInfo       The right left passage information of the _wire_ relative to the track
       *  @param driftLength  The reestimated drift length
       */
      CDCRLWireHit(const CDCWireHit* wireHit,
                   ERightLeft rlInfo,
                   double driftLength);

      /**
       *  Constructs the average of two wire hits with right left passage informations.
       *  Takes the average of the estimated drift lengths.
       */
      static CDCRLWireHit average(const CDCRLWireHit& rlWireHit1,
                                  const CDCRLWireHit& rlWireHit2);

      /**
       *  Constructs the average of three wire hits with right left passage informations.
       *  Takes the average of the estimated drift lengths.
       */
      static CDCRLWireHit average(const CDCRLWireHit& rlWireHit1,
                                  const CDCRLWireHit& rlWireHit2 ,
                                  const CDCRLWireHit& rlWireHit3);


      /**
       *  Constructs an oriented wire hit from a CDCSimHit and the associated wirehit.
       *  This translates the sim hit to an oriented wire hit mainly to be able to compare the
       *  reconstructed values from the algorithm with the Monte Carlo information.
       *  It merely evaluates, if the true trajectory passes right or left of the wire.
       */
      static CDCRLWireHit fromSimHit(const CDCWireHit* wirehit, const CDCSimHit& simhit);

      /// Returns the oriented wire hit with the opposite right left information.
      CDCRLWireHit reversed() const
      { return CDCRLWireHit(m_wireHit, NRightLeft::reversed(m_rlInfo), m_refDriftLength); }

      /// Swiches the right left passage to its opposite inplace.
      void reverse()
      { m_rlInfo = NRightLeft::reversed(m_rlInfo); }

      /// Equality comparison based on wire hit, left right passage information.
      bool operator==(const CDCRLWireHit& rhs) const
      { return getWireHit() == rhs.getWireHit() and getRLInfo() == rhs.getRLInfo(); }


      /**
       *  Total ordering relation based on wire hit and left right passage information
       *  in this order of importance.
       */
      bool operator<(const CDCRLWireHit& rhs) const
      {
        return getWireHit() <  rhs.getWireHit() or (
                 getWireHit() == rhs.getWireHit() and (
                   getRLInfo() < rhs.getRLInfo()));
      }

      /// Defines wires and oriented wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCRLWireHit& rlWireHit, const CDCWire& wire)
      { return rlWireHit.getWire() < wire; }

      /// Defines oriented wire hits and wires to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWire& wire, const CDCRLWireHit& rlWireHit)
      { return wire < rlWireHit.getWire(); }

      /**
       *  Defines wire hits and oriented wire hits to be coaligned on the wire hit
       *  on which they are based.
       */
      friend bool operator<(const CDCRLWireHit& rlWireHit, const CDCWireHit& wireHit)
      { return rlWireHit.getWireHit() < wireHit; }

      /**
       *  Defines oriented wire hits and wire hits to be coaligned on the wire hit
       *  on which they are based.
       */
      friend bool operator<(const CDCWireHit& wireHit, const CDCRLWireHit& rlWireHit)
      { return wireHit < rlWireHit.getWireHit(); }

      /**
       *  Access the object methods and methods from a pointer in the same way.
       *  In situations where the type is not known to be a pointer or a reference
       *  there is no way to tell if one should use the dot '.' or operator '->' for method look up.
       *  So this function defines the -> operator for the object.
       *  No matter you have a pointer or an object access is given with '->'
       *  The object is effectively equal to a pointer to itself.
       */
      const CDCRLWireHit* operator->() const
      { return this; }

      /// Make the wire hit automatically castable to its underlying cdcHit.
      operator const Belle2::CDCHit* () const
      { return getWireHit().getHit(); }

      /// Getter for the CDCHit pointer into the StoreArray.
      const CDCHit* getHit() const
      { return getWireHit().getHit(); }

      /// Getter for the wire the oriented hit associated to.
      const CDCWire& getWire() const
      { return getWireHit().getWire(); }

      /// Checks if the oriented hit is associated with the give wire.
      bool isOnWire(const CDCWire& wire) const
      { return getWire() == wire; }

      /// Getter for the WireID of the wire the hit is located on.
      const WireID& getWireID() const
      { return getWire().getWireID(); }

      /// Getter for the superlayer id.
      ISuperLayer getISuperLayer() const
      { return getWire().getISuperLayer(); }

      /// Getter for the stereo type of the underlying wire.
      EStereoKind getStereoKind() const
      { return getWire().getStereoKind(); }

      /// Indicator if the underlying wire is axial.
      bool isAxial() const
      { return getWire().isAxial(); }

      /// The two dimensional reference position of the underlying wire.
      const Vector2D& getRefPos2D() const
      { return getWire().getRefPos2D(); }

      /// The distance from the beam line at reference position of the underlying wire.
      double getRefCylindricalR() const
      { return getWire().getRefCylindricalR(); }

      /// Getter for the wire hit associated with the oriented hit.
      const CDCWireHit& getWireHit() const
      { return *m_wireHit; }

      /// Checks if the oriented hit is associated with the give wire hit.
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getWireHit() == wirehit; }

      /// Getter for the  drift length at the reference position of the wire.
      double getRefDriftLength() const
      { return m_refDriftLength; }

      /// Setter for the  drift length at the reference position of the wire.
      void setRefDriftLength(double driftLength)
      { m_refDriftLength = driftLength; }

      /// Getter for the  drift length at the reference position of the wire.
      double getSignedRefDriftLength() const
      { return static_cast<ESign>(getRLInfo()) * getRefDriftLength(); }

      /// Getter for the variance of the drift length at the reference position of the wire.
      double getRefDriftLengthVariance() const
      { return getWireHit().getRefDriftLengthVariance(); }

      /// Getter for the right left passage information.
      ERightLeft getRLInfo() const
      { return m_rlInfo; }

      /// Setter for the right left passage information.
      void setRLInfo(const ERightLeft rlInfo)
      { m_rlInfo = rlInfo; }

      /**
       *  Reconstructs a position of primary ionisation on the drift circle.
       *
       *  The result is the position of closest approach on the drift circle to the trajectory.
       *
       *  All positions and the trajectory are interpreted to lie at z=0.
       *  Also the right left passage hypotheses does not play a role in
       *  the reconstruction in any way.
       */
      Vector2D reconstruct2D(const CDCTrajectory2D& trajectory2D) const;

      /**
       *  Attempts to reconstruct a three dimensional position (especially of stereo hits).
       *
       *  This method makes a distinct difference between axial and stereo hits:
       *  * Stereo hits are moved out of the reference plane such that the
       *    oriented drift circle meets the trajectory in one point. Therefore the
       *    left right passage hypothesis has to be taken into account
       *  * For axial hits the reconstructed position is ambiguous in the z coordinate.
       *    Also the drift circle cannot moved such that it would meet the
       *    trajectory. Hence we default to the result of reconstruct2D, which
       *    yield the closest approach of the drift circle to the trajectory
       *    in the reference plane.
       */
      Vector3D reconstruct3D(const CDCTrajectory2D& trajectory2D) const;

      /// Output operator. Help debugging.
      friend std::ostream& operator<<(std::ostream& output, const CDCRLWireHit& rlWireHit)
      {
        output << "CDCRLWireHit(" << rlWireHit.getWireHit() << ","
               << static_cast<typename std::underlying_type<ERightLeft>::type>(rlWireHit.getRLInfo()) << ")" ;
        return output;
      }

    private:
      /// Memory for the reference to the assiziated wire hit.
      const CDCWireHit* m_wireHit = nullptr;

      /// Memory for the right left passage information of the oriented wire hit.
      ERightLeft m_rlInfo = ERightLeft::c_Unknown;

      /// Memory for the reestimated drift length
      double m_refDriftLength = 0.0;

    }; // class CDCRLWireHit
  } // namespace TrackFindingCDC
} // namespace Belle2
