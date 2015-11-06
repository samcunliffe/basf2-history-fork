/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MUIDPAR_H
#define MUIDPAR_H

//! Outermost barrel layer number (zero-based counting)
#define MUID_MaxBarrelLayer 14

//! Outermost forward-endcap layer number (zero-based counting)
#define MUID_MaxForwardEndcapLayer 13

//! Outermost backward-endcap layer number (zero-based counting)
#define MUID_MaxBackwardEndcapLayer 11

/** Greatest value for outcome.
    0: never reached KLM
    1: stop in barrel
    2: stop in forward endcap (without crossing barrel)
    3: exit from barrel (without crossing endcap)
    4: exit from forward endcap (without crossing barrel)
    5: stop in backward endcap (without crossing barrel)
    6: exit from backward endcap (without crossing barrel)
    7-21: stop in forward endcap (after crossing barrel)
    22-36: stop in backward endcap (after crossing barrel)
    37-51: exit from forward endcap (after crossing barrel)
    52-66: exit from backward endcap (after crossing barrel)
**/
#define MUID_MaxOutcome 66

//! Greatest value for reduced-chi-squared struck-detector selector
#define MUID_MaxDetector 2

//! Greatest value for ndof/2 (half of # of degrees of freedom in transverse-scattering chi-squared)
#define MUID_MaxHalfNdof 18

//! Reduced-chi-squared array size for tabulated values
#define MUID_ReducedChiSquaredNbins 100

//! Maximum reduced-chi-squared tabulated value for transverse scattering
#define MUID_ReducedChiSquaredLimit 10.0

namespace Belle2 {

  class Muid;

  //! Provides muid parameters (from Gearbox)
  class MuidPar {

  public:

    //! Constructor with arguments (experiment #, particleID hypothesis)
    MuidPar(int, const char*);

    //! Destructor
    ~MuidPar();

    //! Get the PDF for a particular hypothesis
    double getPDF(const Muid*, bool isForward) const;

  private:

    //! Hidden constructor
    MuidPar();

    //! Hidden copy constructor
    MuidPar(MuidPar&);

    //! Hidden copy assignment
    MuidPar& operator=(const MuidPar&);

    //! Get probability density functions for this particleID hypothesis from Gearbox
    void fillPDFs(int, const char*);

    //! Construct spline interpolation coefficients (first, second, third derivatives)
    void spline(int, double, double*, double*, double*, double*);

    //! Get the per-layer PDF for a particular hypothesis
    double getPDFLayer(const Muid*, bool) const;

    //! Get the transverse-coordinate PDF for a particular hypothesis
    double getPDFRchisq(const Muid*) const;

    //! Per-layer (longitudinal) probability density function
    double m_LayerPDF[MUID_MaxOutcome + 1][MUID_MaxBarrelLayer + 1][MUID_MaxBarrelLayer + MUID_MaxForwardEndcapLayer + 2];

    //! Reduced chi-squared (transverse) probability density function (analytical): threshold
    double m_ReducedChiSquaredThreshold[MUID_MaxDetector + 1][MUID_MaxHalfNdof + 1];

    //! Reduced chi-squared (transverse) probability density function (analytical): horizontal scale ~ 1
    double m_ReducedChiSquaredScaleX[MUID_MaxDetector + 1][MUID_MaxHalfNdof + 1];

    //! Reduced chi-squared (transverse) probability density function (analytical): vertical scale
    double m_ReducedChiSquaredScaleY[MUID_MaxDetector + 1][MUID_MaxHalfNdof + 1];

    //! Reduced chi-squared (transverse) probability density function (overflows in last bin)
    double m_ReducedChiSquaredPDF[MUID_MaxDetector + 1][MUID_MaxHalfNdof + 1][MUID_ReducedChiSquaredNbins];

    //! First derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD1[MUID_MaxDetector + 1][MUID_MaxHalfNdof + 1][MUID_ReducedChiSquaredNbins];

    //! Second derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD2[MUID_MaxDetector + 1][MUID_MaxHalfNdof + 1][MUID_ReducedChiSquaredNbins];

    //! Third derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD3[MUID_MaxDetector + 1][MUID_MaxHalfNdof + 1][MUID_ReducedChiSquaredNbins];

    //! Reduced chi-squared (transverse) probability density function's bin size
    double m_ReducedChiSquaredDx;

  };

} // end of namespace Belle2

#endif // MUIDPAR_H
