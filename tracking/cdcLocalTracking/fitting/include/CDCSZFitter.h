/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCSZFITTER_H
#define CDCSZFITTER_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the z coordinate over travel distance fit.
    class CDCSZFitter : public CDCLocalTracking::UsedTObject {
    public:

      /** Constructor. */
      /** This is as well the parameter free I/O constructor.
       */
      CDCSZFitter();

      /** Destructor. */
      ~CDCSZFitter();

      /// Getter for a standard sz line fitter instance.
      static const CDCSZFitter& getFitter();

      /// Returns a fitted trajectory
      CDCTrajectorySZ fit(const CDCStereoRecoSegment2D& stereoSegment,
                          const CDCTrajectory2D& axialTrajectory2D) const {
        CDCTrajectorySZ trajectorySZ;
        update(trajectorySZ, stereoSegment, axialTrajectory2D);
        return trajectorySZ;
      }

      /// Update the given sz trajectory reconstructing the stereo segment with a near by axial segment
      void update(CDCTrajectorySZ& trajectorySZ,
                  const CDCStereoRecoSegment2D& stereoSegment,
                  const CDCTrajectory2D& axialTrajectory2D) const;


      /// Update the trajectory with a fit in the sz direction to the three dimensional hits
      void update(CDCTrajectorySZ& trajectory, const CDCRecoHit3DVector& recohits) const {
        std::vector<FloatType> observations;
        fillObservations(recohits, observations);
        update(trajectory, observations);
      }

    private:
      /// Appends the s and z values of all given hits to the given vector.
      void fillObservations(const CDCRecoHit3DVector& recoHits3D, std::vector<FloatType>& observations) const {
        for (const CDCRecoHit3D & recoHit3D : recoHits3D) {
          fillObservation(recoHit3D, observations);
        }
      }

      /// Appends the s and z value of the given hits to the given vector.
      void fillObservation(const Belle2::CDCLocalTracking::CDCRecoHit3D& recohit,
                           std::vector<FloatType>& observations) const {
        observations.push_back(recohit.getPerpS());
        observations.push_back(recohit.getRecoPos3D().z());
      }


    private:
      /// Update the trajectory with a fit in the sz direction to the vector of s and z values setup by fillObservations.
      void update(CDCTrajectorySZ& trajectorySZ,
                  std::vector<FloatType>& observations) const {

        size_t nObservations = observations.size() / 2;
        FloatType* rawObservations = &(observations.front());
        updateOptimizeZDistance(trajectorySZ, rawObservations, nObservations);

      }

      /// Update the trajectory with a fit in the sz direction to the vector of s and z values setup by fillObservations optimizing the direct euclidian distance in sz space.
      void updateOptimizeSZDistance(CDCTrajectorySZ& trajectorySZ,
                                    FloatType* observations,
                                    size_t nObservations) const;

      /// Update the trajectory with a fit in the sz direction to the vector of s and z values setup by fillObservations optimizing the z distance in sz space.
      void updateOptimizeZDistance(CDCTrajectorySZ& trajectorySZ,
                                   FloatType* observations,
                                   size_t nObservations) const;

      /// ROOT Macro to make CDCSZFitter a ROOT class.
      ClassDefInCDCLocalTracking(CDCSZFitter, 1);

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCSZFITTER
