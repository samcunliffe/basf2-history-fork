/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Markus Prim                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <tracking/dataobjects/HitPatternCDC.h>
#include <tracking/dataobjects/HitPatternVXD.h>
#include <TVector3.h>
#include <TMatrixF.h>
#include <TMatrixDSym.h>
#include <cstdlib>

#include <vector>

namespace Belle2 {

  /** Values of the result of a track fit with a given particle hypothesis.
   */
  class TrackFitResult : public RelationsObject {
  public:
    /** Constructor initializing everything to zero. */
    TrackFitResult();

    /** Constructor initializing class with fit result. */
    TrackFitResult(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                   const short int charge, const Const::ParticleType& pType, const float pValue,
                   const float bField);

    /** Constructor initializing class with perigee parameters. Just for test cases*/
    TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                   const Const::ParticleType& pType, const float pValue);

    /** Getter for vector of position at closest approach of track in r/phi projection.*/
    TVector3 getPosition() const;

    /** Setter for position vector.
     * TODO delete, out of Business
     * */
    void setPosition(const TVector3& position) {
      B2WARNING("Deprecated, member variables are set with constructor call.");
    }

    /** Getter for vector of momentum at closest approach of track in r/phi projection.
     * This has a default value so that the basf2 compiles
     * */
    TVector3 getMomentum(const float bField = 1.5) const;

    /** Setter for momentum vector.
      * TODO delete, out of Business
      * */
    void setMomentum(const TVector3& momentum) {
      B2WARNING("Deprecated, member variables are set with constructor call.");
    }

    /** Position and Momentum Covariance Matrix.
     *
     *  This is a copy from the genfit::Track getPosMomCov matrix (implicating the order of the matrix),
     *  however, it uses just floating point precision rather than double.
     *  As well currently no TMatrixSym is used (which might change, but doesn't matter much due to the misconstruction of TMatrixSym).
     *  @TODO Study if double precision matrix is needed and if TMatrixSym helps somewhere.
     */
    TMatrixF getCovariance6(const float bField = 1.5) const;

    /** Setter for Covariance matrix of position and momentum.
     * TODO delete, out of Business
     * */
    void setCovariance6(const TMatrixF& covariance) {
      B2WARNING("Deprecated, member variables are set with constructor call.");
    }

    /** Get back a ParticleCode of the hypothesis of the track fit.*/
    Const::ParticleType getParticleType() const {
      return Const::ParticleType(m_pdg);
    }

    /** Setter for the PDGCode.
     * TODO delete, out of Business
     * */
    void setParticleType(const Const::ParticleType& pType) {
      B2WARNING("Deprecated, member variables are set with constructor call.");
    }

    /** Return track charge (1 or -1). */
    short getCharge() const {
      // getOmega needs an argument, so one is given
      return getOmega(1.5) >= 0 ? 1 : -1;
    }

    /** Setter for Charge
     * TODO delete, out of Business
     * */
    void setCharge(int charge) {
      B2WARNING("Deprecated, member variables are set with constructor call.");
    }

    /** Getter for Chi2 Probability of the track fit. */
    float getPValue() const {
      return m_pValue;
    }

    /** Setter for Chi2 Probability of the track fit.
     * TODO delete, out of Business
     * */
    void setPValue(float pValue) {
      B2WARNING("Deprecated, member variables are set with constructor call.");
    }

    //---------------------------------------------------------------------------------------------------------------------------
    // --- Getters for perigee helix parameters
    //---------------------------------------------------------------------------------------------------------------------------
    /**
     * Getter for d0. This is the signed distance to the POCA in the r-phi plane.
     * @return
     */
    float getD0() const { return m_tau.at(0); }

    /**
     * Getter for phi. This is the angle of the transverse momentum in the r-phi plane.
     * @return
     */
    float getPhi() const { return m_tau.at(1); }

    /**
     * Getter for omega. This is the curvature of the track. It's sign is defined by the charge of the particle.
     * @return
     */
    float getOmega(const float bField) const { return m_tau.at(2); }

    /**
     * Getter for z0. This is the z coordinate of the POCA.
     * @return
     */
    float getZ0() const { return m_tau.at(3); }

    /**
     * Getter for cotTheta. This is the slope of the track in the r-z plane.
     * @return
     */
    float getCotTheta() const { return m_tau.at(4); }

    /**
     * Getter for all perigee parameters
     * @return vector with 5 elements
     */
    std::vector<float> getTau(const float bField) const { return m_tau; }

    /**
     * Getter for all covariance matrix elements of perigee parameters
     * TODO: Implement bField
     * @return vector with 15 elements
     */
    std::vector<float> getCov(const float bField) const { return m_cov5; }

    /**
     * Getter for covariance matrix of perigee parameters in matrix form.
     * TODO: Implement bField
     * @return
     */
    TMatrixF getCovariance5(const float bField) const;

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Hit Pattern Arithmetics
    //---------------------------------------------------------------------------------------------------------------------------
    /** Number of PXD hits used in the TrackFitResult. */
    /*    unsigned short getNPXDHits() const {
          return (m_hitPattern[0] + m_hitPattern[1]);
        }
    */
    /** Number of SVD hits used in the TrackFitResult. */
    /*    unsigned short getNSVDHits() const {
          return (m_hitPattern[2] + m_hitPattern[3] + m_hitPattern[4] + m_hitPattern[5]);
        }
    */
    /** Number of CDC hits used in the TrackFitResult. */
    /*    unsigned short getNCDCHits() const {
          return (m_hitPattern.count() - getNPXDHits() - getNSVDHits());
        }
    */
    /** Was there a hit in the specified layer?
     *
     *  @param  iVXDLayer  layer for which the information is requested.
     */
    /*    bool hitInVXDLayer(unsigned short iVXDLayer) const {
          return m_hitPattern[iVXDLayer];
        }
    */
    /** Was there a hit in the specified layer?
     *
     *  @param  iCDCLayer  layer for which the information is requested.
     */
    /*    bool hitInCDCLayer(unsigned short iCDCLayer) const {
          return m_hitPattern[iCDCLayer + 6];
        }
    */
    /** Similar as above, but asking for any hit in the corresponding SuperLayer.*/
//    bool hitInSuperLayer(unsigned int iSuperLayer) const;

    /** Were any Stereo hits in the CDC. */
    /*    bool hitInStereoLayer() const {
          return (hitInSuperLayer(2) || hitInSuperLayer(4) || hitInSuperLayer(6) || hitInSuperLayer(8));
        }
    */
    /** Returns the count of the innermost Layer. */
//    unsigned short getIInnermostLayer() const;

    ///--------------------------------------------------------------------------------------------------------------------------
  private:
    /** Calculates the alpha value for a given magnetic field in Tesla */
    double getAlpha(const float bField) const;

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Functions for internal conversions between cartesian and perigee helix parameters
    //--- This can be placed in a seperate header which handles all the conversion stuff
    //---------------------------------------------------------------------------------------------------------------------------
    float calcD0FromCartesian(const TVector3& position, const TVector3& momentum) const ;
    float calcPhiFromCartesian(const TVector3& momentum) const;
    float calcOmegaFromCartesian(const TVector3& momentum, const short int charge, const float bField) const;
    float calcZ0FromCartesian(const TVector3& position) const;
    float calcCotThetaFromCartesian(const TVector3& momentum) const;
    //TMatrixF transformCov6ToCov5(TMatrixF& cov6) const; //handled inside cartesianToPerigee

    float calcXFromPerigee() const;
    float calcYFromPerigee() const;
    float calcZFromPerigee() const;
    float calcPxFromPerigee(const float bField) const;
    float calcPyFromPerigee(const float bField) const;
    float calcPzFromPerigee(const float bField) const;
    TMatrixF transformCov5ToCov6(const TMatrixF& cov5, const float bField) const;

    /** Cartesian to Perigee conversion
     * everything happens internally, m_tau and m_cov5 will be set and cartesian values dropped
     */
    void cartesianToPerigee(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                            const short int charge, const float bField);

    /** PDG Code for hypothesis with which the corresponding fit was performed. */
    const unsigned int m_pdg;

    /** Chi2 Probability of the fit. */
    const float m_pValue;

    /** perigee helix parameters
     * tau = d0, phi, omega, z0, cotTheta
     */
    std::vector<float> m_tau;

    /** covariance matrix elements
     * (0,0), (0,1) ... (1,1), (1,2) ... (2,2) ...
     */
    std::vector<float> m_cov5;

    ///--------------------------------------------------------------------------------------------------------------------------
    /** Hit Pattern of the corresponding Hit.
     *
     *  Bits 0-1:   PXD <br>
     *  Bits 2-5:   SVD <br>
     *  Bits 6-61:  CDC <br>
     *  Bits 62-63: unused.
     */
//    std::bitset<64> m_hitPattern;

    ClassDef(TrackFitResult, 2);
  };
}

