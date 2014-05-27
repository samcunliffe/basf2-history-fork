/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMHIT2D_H
#define BKLMHIT2D_H

#include <TVector3.h>
#include <framework/datastore/RelationsObject.h>
#include <bklm/dataobjects/BKLMStatus.h>

namespace Belle2 {

  class BKLMHit1d;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMHit2d : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMHit2d();

    //! Constructor with initial values
    //! @param hit1 reconstructed BKLMHit1d
    //! @param hit2 reconstructed BKLMHit1d in the same module's orthogonal readout direction
    BKLMHit2d(const BKLMHit1d*, const BKLMHit1d*);

    //! Copy constructor
    BKLMHit2d(const BKLMHit2d&);

    //! Destructor
    virtual ~BKLMHit2d() {}

    //! @return whether this 2D hit is in RPC (true) or scintillator (false)
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! @return detector end (TRUE=forward or FALSE=backward) of this 2D hit
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! @return sector number (1..8) of this 2D hit
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! @return layer number (1..15) of this 2D hit
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! @return lowest phi-measuring strip number of this 2D hit
    int getPhiStripMin() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! @return highest phi-measuring strip number of this 2D hit
    int getPhiStripMax() const { return (((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1); }

    //! @return average phi-measuring strip number of this 2D hit
    float getPhiStripAve() const;

    //! @return lowest z-measuring strip number of this 2D hit
    int getZStripMin() const { return (((m_ZStrips & BKLM_ZSTRIP_MASK) >> BKLM_ZSTRIP_BIT) + 1); }

    //! @return highest z-measuring strip number of this 2D hit
    int getZStripMax() const { return (((m_ZStrips & BKLM_ZMAXSTRIP_MASK) >> BKLM_ZMAXSTRIP_BIT) + 1); }

    //! @return average z-measuring strip number of this 2D hit
    float getZStripAve() const;

    //! @return detector-module identifier
    //! @sa BKLMStatus.h
    int getModuleID() const { return m_ModuleID; }

    //! @return whether this 2D hit is outside the trigger-coincidence window (true) or not (false)
    bool isOutOfTime() { return (m_ModuleID & BKLM_OUTOFTIME_MASK) != 0; }

    //! @return whether this 2D hit is associated with a muid-extrapolated track (true) or not (false)
    bool isOnTrack() { return (m_ModuleID & BKLM_ONTRACK_MASK) != 0; }

    //! @return whether this 2D hit is deemed inefficient (true) or not (false)
    bool isInefficient() { return (m_ModuleID & BKLM_INEFFICIENT_MASK) != 0; }

    //! @return 3D hit position in global coordinates (cm)
    TVector3 getGlobalPosition(void) const { return TVector3(m_GlobalPosition[0], m_GlobalPosition[1], m_GlobalPosition[2]); }

    //! @return 2D reconstructed hit time (ns), the average of the 2 projections
    float getTime() const { return m_Time; }

    //! @return 2D reconstructed energy deposition (MeV), the sum of the 2 projections
    float getEDep() const { return m_EDep; }

    //! @return whether the two BKLMHit2ds are in the same module (true) or not (false)
    bool match(const BKLMHit2d* h) const { return (((h->getModuleID() ^ m_ModuleID) & BKLM_MODULEID_MASK) == 0); }

    //! set or clear the OutOfTime flag
    //! @param flag whether this hit is outside the trigger-coincidence window (true) or not (false)
    void isOutOfTime(bool flag) { if (flag) { m_ModuleID |= BKLM_OUTOFTIME_MASK; } else { m_ModuleID &= ~BKLM_OUTOFTIME_MASK; } }

    //! set or clear the OnTrack flag
    //! @param flag whether this hit is associated with a muid-extrapolated track (true) or not (false)
    void isOnTrack(bool flag) { if (flag) { m_ModuleID |= BKLM_ONTRACK_MASK; } else { m_ModuleID &= ~BKLM_ONTRACK_MASK; } }

    //! set or clear the Inefficient flag
    //! @param flag whether this his is deemed inefficient (true) or not (false)
    void isInefficient(bool flag) { if (flag) { m_ModuleID |= BKLM_INEFFICIENT_MASK; } else { m_ModuleID &= ~BKLM_INEFFICIENT_MASK; } }

    //! set the global position of this 2D hit
    //! @param x Cartesian x coordinate (cm)
    //! @param y Cartesian y coordinate (cm)
    //! @param z Cartesian z coordinate (cm)
    void setGlobalPosition(double x, double y, double z) { m_GlobalPosition[0] = x; m_GlobalPosition[1] = y; m_GlobalPosition[2] = z; }

  private:

    //! detector-module identifier
    //! @sa BKLMStatus.h
    int m_ModuleID;

    //! z-measuring strip numbers of the 2D hit
    //! @sa BKLMStatus.h
    int m_ZStrips;

    //! position of the 2D hit in global coordinates (cm)
    float m_GlobalPosition[3];

    //! reconstructed hit time relative to trigger (ns)
    float m_Time;

    //! reconstructed pulse height (MeV)
    float m_EDep;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit2d, 4)

  };

} // end of namespace Belle2

#endif //BKLMHIT2D_H
