/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <vector>
#include <map>
#include <utility>
#include <tuple>
#include <TVector3.h>

#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include "Splitter.h"
#endif

namespace Belle2 {
  namespace BoostVectorCalib {
    /** track parameters (neglecting curvature) */
    struct Track {
      TVector3 p; ///< momentum vector of the track
      double pid; ///< particle ID of mu/e separation
    };

    /** Event containing two tracks */
    struct Event {
      int exp;   ///< experiment number
      int run;   ///< run number
      int evtNo; ///< event number


      Track mu0; ///< track in the event
      Track mu1; ///< other track in the event

      int nBootStrap; ///< random bootstrap weight (n=1 -> original sample)
      bool isSig;     ///< is not removed by quality cuts?
      double t;       ///< time of event [hours]

    };


    /** Convert events from TTree to std::vector */
    std::vector<Event> getEvents(TTree* tr);


    /** Run the BoostVector analysis with short calibration intervals defined by the splitPoints
      @param evts: vector of events
      @param splitPoints: the vector containing times of the edges of the short calibration intervals [hours]
      @return A tuple containing vector with BV-centers, vector with BV-centers stat. errors and a BV-spread covariance matrix
    */
    std::tuple<std::vector<Eigen::VectorXd>, std::vector<Eigen::MatrixXd>, Eigen::MatrixXd>  runBoostVectorAnalysis(
      std::vector<Event> evts,
      const std::vector<double>& splitPoints);

  }
}
