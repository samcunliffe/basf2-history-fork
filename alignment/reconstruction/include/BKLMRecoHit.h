/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan, Tadeas Bilka                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <bklm/dataobjects/BKLMHit2d.h>


// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <genfit/PlanarMeasurement.h>
#include <genfit/HMatrixUV.h>
#include <genfit/TrackCandHit.h>

namespace Belle2 {
  /**
   * BKLMRecoHit
   */
  class BKLMRecoHit: public genfit::PlanarMeasurement {
  public:

    /** Default constructor for ROOT IO. */
    BKLMRecoHit();

    /** Construct BKLMRecoHit from a PXD cluster
     *
     * @param hit    PXDCluster to use as base
     */
    BKLMRecoHit(const BKLMHit2d* hit, const genfit::TrackCandHit* trackCandHit = NULL);

    /** Destructor. */
    virtual ~BKLMRecoHit() {}

    /** Creating a deep copy of this hit. */
    genfit::AbsMeasurement* clone() const;

    /** Methods that actually interface to Genfit.  */
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const;

    /** Get pointer to the TrueHit used when creating this RecoHit, can be NULL if created from something else */
    const BKLMHit2d* getBKLMHit2d() const { return m_bklmHit2d; }

    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const { return new genfit::HMatrixUV(); };

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };

    unsigned short m_moduleID; /**< Unique module identifier.*/
    const BKLMHit2d* m_bklmHit2d; /**< Pointer to the BKLMHit2d used when creating this object */

    //!the number of fired phi-measuring strips
    //int m_numPhiStrips;
    //!the number of fired z-measuring strips
    //int m_numZStrips;

    ClassDef(BKLMRecoHit, 1)
  };

} // namespace Belle2
