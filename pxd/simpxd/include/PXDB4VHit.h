/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDG4HIT_H_
#define PXDG4HIT_H_

#include <simulation/simkernel/B4VHit.h>

#include <list>

//Geant4 classes
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4Allocator.hh"


namespace Belle2 {

  /**
   * A struct to save detailed information about a single step.
   *
   * Not implemented yet.
   */
  struct PXDStep {

  };


  /**
   * PXD hit class for GEANT4 simulation.
   *
   * This class stores the data produced by GEANT4. For storage in DataStore,
   * it is converted in a TObject-ized class PXDSimHit by its own Save method.
   * This is an oversimplified implementation usable only for the very thin
   * PXD detectors - a single GEANT4 step is assumed per track.
   */
  class PXDB4VHit : public B4VHit {

  public:

    /**
     * Constructor.
     *
     * @param posIn Point of entry into the detector, in local coordinates.
     * @param posOut Point of exit from the detector, in local coordinates.
     * @param theta Angle theta (wrt. the global z axis).
     * @param momIn The momentum of particle on entry into the detector.
     * @param PDGcode The PDG code of particle that produced this track.
     * @param trackID ID of the track.
     * @param energyDep Energy deposition by the particle in the detector.
     * @param stepLength Length of G4 step.
     * @param globalTime Global time.
     * @param volumeName Name of the sensitive volume that the track crossed.
     */
    PXDB4VHit(G4ThreeVector posIn, G4ThreeVector posOut, G4double theta,
              G4ThreeVector momIn, G4int PDGcode, G4int trackID,
              G4double energyDep, G4double stepLength, G4double globalTime,
              G4String volumeName);

    /**
     * Destructor.
     */
    ~PXDB4VHit();

    /**
     * Update the PXD hit information.
     */
    void updateHit();

    /**
     * Convert the hit into PXDSimHit for storing to the DataStore.
     *
     * Called from G4 runManager event action after each event.
     * @param ihhit Position in the DataStore PXDSimHitArray collection
     * where the hit has to be saved.
     */
    void Save(G4int iHit);

    /**
     * Overloaded new operator.
     */
    inline void *operator new(size_t);

    /**
     * Overloaded delete operator.
     */
    inline void operator delete(void *aPXDB4VHit);

    /**
     * Setters
     */
    void setPosIn(double x, double y, double z) { m_posIn = G4ThreeVector(x, y, z); }
    void setPosOut(double x, double y, double z) { m_posOut.set(x, y, z); }
    void setTheta(double aTheta) { m_theta = aTheta; }
    void setMomIn(double px, double py, double pz) { m_momIn.set(px, py, pz); }
    void setTrackID(int trackID) { m_trackID = trackID; }
    void setPDGcode(int pdg) { m_PDGcode = pdg; }
    void setEnergyDep(double energyDep) { m_energyDep = energyDep; }
    void setStepLength(double stepLength) { m_stepLength = stepLength; }
    void setGlobalTime(double globalTime) { m_globalTime = globalTime; }
    void setVolumeName(const char* volumeName) { m_volumeName = volumeName; }

    /**
     * Getters.
     */
    const G4ThreeVector& getPosIn() const { return m_posIn; }
    const G4ThreeVector& getPosOut() const { return m_posOut; }
    double getTheta() const { return m_theta; }
    const G4ThreeVector& getMomIn() const { return m_momIn; }
    int getTrackID() const { return m_trackID; }
    int getPDGcode() const { return m_PDGcode; }
    double getEnergyDep() const { return m_energyDep; }
    double getStepLength() const { return m_stepLength; }
    double getGlobalTime() const { return m_globalTime; }
    const G4String & getVolumeName() const { return m_volumeName; }

  protected:

  private:

    G4ThreeVector m_posIn;  /**< LRF position at pre-step. */
    G4ThreeVector m_posOut; /**< LRF position at post-step. */
    G4double m_theta;       /**< Track angle wrt the global z. */
    G4ThreeVector m_momIn;  /**< Momentum at pre-step. */

    G4int m_PDGcode;       /**< The PDG value of particle which created the hit. */
    G4int m_trackID;       /**< The ID of the track which created the hit. */
    G4double m_energyDep;  /**< Deposited energy. */
    G4double m_stepLength; /**< Step length. */
    G4double m_globalTime; /**< Global time. */

    G4String m_volumeName; /**< Name of the sensitive volume. */

    std::list<PXDStep> m_stepList; /**< List of steps that contributed to the hit. */

  };


  //-------------------
  //    Typdefs
  //-------------------

  /** Geant4 collection for PXDB4VHits. */
  typedef G4THitsCollection<PXDB4VHit> PXDB4VHitsCollection;

  /** Geant4 Allocator for the PXDB4VHit class. */
  extern G4Allocator<PXDB4VHit> PXDB4VHitAllocator;

  //Operator new
  inline void* PXDB4VHit::operator new(size_t)
  {
    void *aPXDB4VHit;
    aPXDB4VHit = (void *) PXDB4VHitAllocator.MallocSingle();
    return aPXDB4VHit;
  }

  //Operator delete
  inline void PXDB4VHit::operator delete(void *aPXDB4VHit)
  {
    PXDB4VHitAllocator.FreeSingle((PXDB4VHit*) aPXDB4VHit);
  }

} // end of namespace Belle2

#endif /* PXDG4HIT_H_ */
