/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ORIGINALKARIMAKISMETHOD_H
#define ORIGINALKARIMAKISMETHOD_H

#include "TMatrixDSym.h"
#include "TVectorD.h"


#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class OriginalKarimakisMethod : public CDCLocalTracking::UsedTObject {

    public:
      ///Empty constructor
      OriginalKarimakisMethod();

      ///Empty destructor
      ~OriginalKarimakisMethod();

      /// Executes the fit and updates the trajectory parameters. This may render the information in the observation object.
      void update(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const;


    private:
      void simpleFitXY(bool, double, double) const;
      void addPoint(double, double, double) const;
      int fit(double&, int&) const;
      TVectorD getPar() const;
      TMatrixDSym getCov() const;

    private:
      const bool _curved;
      const int _npar;
      mutable double _xRef;
      mutable double _yRef;
      mutable int _numPoints;
      mutable double _sx;
      mutable double _sy;
      mutable double _sxx;
      mutable double _sxy;
      mutable double _syy;
      mutable double _sw;
      mutable double _sr;
      mutable double _sxr;
      mutable double _syr;
      mutable double _srr;
      mutable TVectorD _parameters;
      mutable TMatrixDSym _covariance;

    public:
      /** ROOT Macro to make OriginalKarimakisMethod a ROOT class.*/
      ClassDefInCDCLocalTracking(OriginalKarimakisMethod, 1);


    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // ORIGINALKARIMAKISMETHOD
