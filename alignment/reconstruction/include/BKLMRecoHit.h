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
#include <bklm/geometry/GeometryPar.h>


// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <genfit/PlanarMeasurement.h>
#include <genfit/HMatrixUV.h>
#include <genfit/TrackCandHit.h>
// Calibration/Alignment interface
#include <genfit/ICalibrationParametersDerivatives.h>

namespace Belle2 {
  /**
   * BKLMRecoHit
   */
  class BKLMRecoHit: public genfit::PlanarMeasurement, public genfit::ICalibrationParametersDerivatives {
  public:

    /** Default constructor for ROOT IO. */
    BKLMRecoHit() {};

    /** Construct BKLMRecoHit from a BKLMHit2d */
    BKLMRecoHit(const BKLMHit2d* hit, const genfit::TrackCandHit* trackCandHit = NULL);

    /** Destructor. */
    virtual ~BKLMRecoHit() {}

    /** Creating a deep copy of this hit. */
    genfit::AbsMeasurement* clone() const;

    /** Methods that actually interface to Genfit.  */
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const;

    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const { return new genfit::HMatrixUV(); };

    virtual TMatrixD derivatives(const genfit::StateOnPlane* sop);

    virtual std::vector< int > labels();

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };

    unsigned short m_moduleID; /**< Unique module identifier.*/

    /** Forward or not. */
    bool m_IsForward;

    /** Sector number. */
    int m_Sector;

    /** Layer number. */
    int m_Layer;

    /** module used to get geometry information */
    const bklm::Module* module; //! not streamed

    /** global coordiante of the hit */
    CLHEP::Hep3Vector global; //! not streamed

    /** half height of scintillator module*/
    const double halfheight_sci = 0.5;

    //!the number of fired phi-measuring strips
    //int m_numPhiStrips;
    //!the number of fired z-measuring strips
    //int m_numZStrips;

    /** Needed to make object storable. */
    ClassDef(BKLMRecoHit, 2);

  };

} // namespace Belle2
