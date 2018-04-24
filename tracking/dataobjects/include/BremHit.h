

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <TVector3.h>

namespace Belle2 {

  class BremHit : public RelationsObject {
  public:
    /**
     * Empty constructor for root.
     */
    BremHit() {};

    /**
     * Create a BremHit, with information about the energy and position of the radiation
     * Automatically adds relation to the RecoTrack and the bremsstrahlung ECLCluster
     * @param recoTrack The RecoTrack to which the bremsstrahlung was found
     * @param bremCluster The found bremsstrahlung cluster
     * @param position The Position where the radiation took place
     * @param bremEnergy The energy of the bremsstrahlung photon, which was radiated
     */
    BremHit(const RecoTrack* recoTrack, const ECLCluster* bremCluster, const TVector3 position, const double bremEnergy,
            const double clusterDistance) :
      m_position(position),
      m_bremEnergy(bremEnergy),
      m_clusterDistance(clusterDistance)
    {
      //addRelationTo(recoTrack);
      //addRelationTo(bremCluster);
    }

    /** Set the radiation position. */
    void setPosition(TVector3 position) {m_position = position;}

    /** Set the radiated energy. */
    void setBremEnergy(double bremEnergy) {m_bremEnergy = bremEnergy;}

    /** Set the distance between the extrapolation and the bremCluster position. */
    void setClusterDistance(double distance) {m_clusterDistance = distance;}

    /** Return the radiation position. */
    TVector3 getPosition() const
    {
      return m_position;
    }
    /** Return the radiated energy. */
    double getBremEnergy() const
    {
      return m_bremEnergy;
    }

    /** Return the distance between the extrapolation and the bremCluster position. */
    double getClusterDistance() const
    {
      return m_clusterDistance;
    }

  private:
    TVector3 m_position;

    double m_bremEnergy;

    double m_clusterDistance;

    ClassDef(BremHit, 1);
  };
}
