/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACK_H
#define TRACK_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /** Track parameters. */
  /** This Track class is based on the perigee track parametrisation. The helix parameters according to track parametrisation used at Belle are also provided.
   *  It is not a permanent solution to have two simulateous track parametrisations, it seems to be conveneit for the moment, as it is still under discussion which parametrisation will be the one we want to use at the ende.
   *  It is also generally not fixed which variables this class should contain, so it is more or less a preliminary solution...
   */
  class Track : public TObject {
  public:

    /** Constructor. */
    /** This is as well the parameter free I/O constructor.
     */

    Track() {
      //set all member variables to some starting values
      m_d0 = -999;
      m_phi = -999;
      m_omega = -999;
      m_z0 = -999;
      m_cotTheta = -999;

      m_Belle_d0 = -999;
      m_Belle_phi = -999;
      m_Belle_kappa = -999;
      m_Belle_z0 = -999;
      m_Belle_tanLambda = -999;

      m_nHits = -999;
      m_chi2 = -999;
      m_pValue = -999;
      m_mcId = -999;
      m_pdgHypo = -999;
      m_purity = -999;
      m_fitFailed = false;
      m_extrapFailed = false;

      m_pErrors.SetX(-999);
      m_pErrors.SetY(-999);
      m_pErrors.SetZ(-999);

      m_vertexErrors.SetX(-999);
      m_vertexErrors.SetY(-999);
      m_vertexErrors.SetZ(-999);

      m_dEdx = -999;
      m_dEdxSigma = -999;
      m_radiusOfInnermostHit = -999;
    }

    /** Destructor. */
    ~Track() {
    }

    // Getters for all the parameters.
    //helix parameters: perigee
    /** Getter for impact parameter.
     */
    float getD0() const { return m_d0; }

    /** Getter for phi.
     */
    float getPhi() const { return m_phi; }

    /** Getter for omega.
      */
    float getOmega() const {return m_omega; }

    /** Getter for z0.
     */
    float getZ0() const {return m_z0; }

    /** Getter for cotTheta.
    */
    float getCotTheta() const { return m_cotTheta;}

    //helix parameters: Belle
    /** Getter for impact parameter.
      */
    float getBelleD0() const { return m_Belle_d0; }

    /** Getter for phi.
     */
    float getBellePhi() const { return m_Belle_phi; }

    /** Getter for kappa.
     */
    float getBelleKappa() const {return m_Belle_kappa; }

    /** Getter for z0.
     */
    float getBelleZ0() const {return m_Belle_z0; }

    /** Getter for tanLambda.
      */
    float getBelleTanLambda() const { return m_Belle_tanLambda;}

    //non-helix parameters.
    /** Getter for Chi2 of the track fit.
     */
    float getChi2() const {return m_chi2;}

    /** Getter for the probability value of the track fit.
     */
    float getPValue() const {return m_pValue;}
    /** Getter for total number of tracker hits.
     */
    int getNHits() const {return m_nHits;}

    /** Getter of the MCParticle Id.
     */
    int getMCId() const { return m_mcId; }

    /** Getter for the pdg hypothesis under which this track was fitted.
     */
    int getPDG() const {return m_pdgHypo;}

    /** Getter for the purity of the track.
     */
    float getPurity() const { return m_purity;}

    /** Returns true if the fit of this track failed.
     */
    bool getFitFailed() const {return m_fitFailed;}

    /** Return true if the extrapolation of this track failed.
     */
    bool getExtrapFailed() const {return m_extrapFailed;}

    /** Getter for momentum errors.
     */
    TVector3 getPErrors() const {return m_pErrors;}
    /** Getter for vertex errors.
     */
    TVector3 getVertexErrors() const {return m_vertexErrors;}

    /**Getter for energy loss in the tracking detectors.
     */
    float getdEdx() const { return m_dEdx;}

    /** Getter for uncertainty on energy loss in tracking detectors.
     */
    float getdEdxSigma() const { return m_dEdxSigma;}

    /** Getter for radius of innermost hit contributing to the track.
     */
    float getRadiusOfInnermostHit() const { return m_radiusOfInnermostHit;}


    // Setters for all the parameters.
    //helix parameters: perigee
    /** Setter for impact parameter.
     */
    void setD0(const float& d0) { m_d0 = d0;}

    /** Setter for phi
     */
    void setPhi(const float& phi) {m_phi = phi;}

    /** Setter for omega.
     */
    void setOmega(const float& omega) {m_omega = omega;}

    /** Setter for z0.
     */
    void setZ0(const float& z0) {m_z0 = z0;}

    /** Setter for cotTheta.
     */
    void setCotTheta(const float& cotTheta) {m_cotTheta = cotTheta;}

    //helix parameters: Belle
    /** Setter for impact parameter.
     */
    void setBelleD0(const float& d0) { m_Belle_d0 = d0;}

    /** Setter for phi
     */
    void setBellePhi(const float& phi) {m_Belle_phi = phi;}

    /** Setter for kappa.
     */
    void setBelleKappa(const float& kappa) {m_Belle_kappa = kappa;}

    /** Setter for z0.
     */
    void setBelleZ0(const float& z0) {m_Belle_z0 = z0;}

    /** Setter for tanLambda.
     */
    void setBelleTanLambda(const float& tanLambda) {m_Belle_tanLambda = tanLambda;}

    //non-helix parameters.
    /** Setter for Chi2 of the track fit.
     */
    void setChi2(const float& chi2) {m_chi2 = chi2;}

    /** Setter for the probabity value of the track fit.
     */
    void setPValue(const float& pValue) {m_pValue = pValue;};

    /** Setter for number of tracker hits.
     */
    void setNHits(const int& nHits) {
      m_nHits = nHits;
    }
    /** Setter for the MCParticle Id.
     */
    void setMCId(const int& mcId) {m_mcId = mcId;};

    /** Setter for the pdg hypothesis.
     */
    void setPDG(const int& pdg) { m_pdgHypo = pdg;};

    /** Setter for the purity of the track.
     */
    void setPurity(const float& purity) { m_purity = purity;};

    /** Set if the fit was successful or failed.
     */
    void setFitFailed(const bool& fitFailed) { m_fitFailed = fitFailed;};

    /** Set if the extrapolation was successful or failed.
     */
    void setExtrapFailed(const bool& extrapFailed) {m_extrapFailed = extrapFailed;};

    /** Setter for momentum errors.
     */
    void setPErrors(const TVector3& pErrors) { m_pErrors = pErrors  ;};

    /** Setter for momentum errors.
     */
    void setPErrors(const double& px, const double& py, const double& pz) {
      m_pErrors.SetX(px);
      m_pErrors.SetY(py);
      m_pErrors.SetZ(pz);
    }

    /** Setter for vertex errors.
    */
    void setVertexErrors(const TVector3& vertexErrors) { m_vertexErrors = vertexErrors  ;}
    /** Setter for vertex errors.
     */
    void setVertexErrors(const double& x, const double& y, const double& z) {
      m_vertexErrors.SetX(x);
      m_vertexErrors.SetY(y);
      m_vertexErrors.SetZ(z);
    }

    /** Setter for energy loss in the tracking detectors.
     */
    void setdEdx(const float& dEdx) { m_dEdx = dEdx;}

    /** Setter for uncertainty on energy loss in tracking detectors.
     */
    void setdEdxSigma(const float& dEdxSigma) { m_dEdxSigma = dEdxSigma; }

    /** Setter for radius of innermost hit contributing to the track.
     */
    void setRadiusOfInnermostHit(const float& radiusOfInnermostHit) {m_radiusOfInnermostHit = radiusOfInnermostHit;}



  private:

    //helix parameters
    //Perigee parametrisation
    //-------------------------------------------------------------------------------------------------------
    /** Impact Parameter.
     * The Impact Parameter is the two-dimensional point of closest approach of a track to the origin.
     * A sign is assigned such, that if the projection of the momentum on this distance points to the origin, d0 is negative.
     * (absolute value equals m_Belle_d0 but different sign convention)
     */
    float m_d0;

    /** Phi at the perigee [-pi, pi].
     * This is the angle of the track momentum at the perigee in the plane perpendicular to the detector axis.
     * (different angle definition as m_Belle_phi)
     */
    float m_phi;

    /** Signed curvature of the track.
     *  The curvature is 1/(R) where R is the radius of the track measured in cm.
     *  It is negative if we have a negatively charged track leaving the origin.
     *  (can be transformed to m_Belle_kappa by taking into account c and B)
     */
    float m_omega;

    /** z Position of the perigee.
     *  z is parallel to the detector axis.
     *  (same as m_Belle_z0)
     */
    float m_z0;

    /** Cotangens of polar angle theta.
     *  This equals dz/ds with s the path length in R-Phi at the perigee.
     *  (same as m_Belle_tanLambda)
     */
    float m_cotTheta;
    //---------------------------------------------------------------------------------------------------

    //Belle parametrisation
    //-------------------------------------------------------------------------------------------------------

    /** Impact Parameter.
     *  The Impact Parameter is the two-dimensional point of closest approach of a track to the origin.
     *  The sign convention is not quite clear to me, but seems to be consistent with the Belle track parametrisation...
     *  (absolute value equals m_d0 but different sign convention)
     */
    float m_Belle_d0;

    /** Azimuthal angle to specify the pivot with respect to the helix center [0, 2pi].
     * * (different angle definition as m_phi)
     */
    float m_Belle_phi;

    /** Signed reciprocal transvers momentum of the track (1/Pt).
     *  Pt is measured in GeV.
     *  It is negative if we have a negatively charged track leaving the origin.
     *  (can be transformed to m_omega by taking into account c and B)
     */
    float m_Belle_kappa;

    /** z Position of the perigee.
     *  z is parallel to the detector axis.
     *  (same as m_z0)
     */
    float m_Belle_z0;

    /** Tangens of the dip angle lambda.
     *  This equals dz/ds with s the path length in R-Phi at the perigee.
     *  (same as m_cotTheta)
     */
    float m_Belle_tanLambda;
    //--------------------------------------------------------------------------------------------

    //non-helix parameters

    /** Total number of tracking detectors (CDC, SVD, PXD) hits used for this track
     */
    int m_nHits;

    /** Chi2 of track fit.
        This is a measure for the quality of the track.
     */
    float m_chi2;

    /** Probability value of track fit.
        This is a measure for the quality of the track.
     */
    float m_pValue;

    /** ID of the MCParticle which created this tracks.
     *
     */
    int m_mcId;

    /** PDG hypothesis under which this track was fitted.
     *
     */
    int m_pdgHypo;

    /** Purity of the track (number of hits coming from original MCParticle/total number of hits  * 100)
     *  Is always 100 for MCTracks, but is useful to evaluate the performance of pattern recognition.
     */
    float m_purity;

    /** Mark is the fit of this track was successful;
     *  True if the fit failed.
     */
    bool m_fitFailed;

    /** Mark is the extrapolation of the fit results was successful;
     *  True if the extrapolation failed.
     */
    bool m_extrapFailed;

    /** Variances on momentum.
     *
     */
    TVector3 m_pErrors;

    /** Variances on vertex position.
     *
     */
    TVector3 m_vertexErrors;


    //these parameters are not used yet ...

    /** dE/dx in the tracking detectors.
        For the moment, this is intended to be used with the CDC.
        We have to see, how to handle the energy loss in the other detectors.
     */
    float m_dEdx;

    /** Uncertainty on energy loss in tracking detectors.
     */
    float m_dEdxSigma;

    /** Radius of innermost hit contributing to the track.
        This gives a hint, if we may have to do it with a track from a V0 particle.
    */
    float m_radiusOfInnermostHit;


    /** ROOT Macro to make Track a ROOT class.*/
    ClassDef(Track, 1);

  }; //class
} // namespace Belle2
#endif // TRACK
