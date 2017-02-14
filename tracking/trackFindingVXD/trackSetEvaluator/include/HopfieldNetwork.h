/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//basf2
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>
#include <framework/logging/Logger.h>

//STL
#include <numeric>

//ROOT
#include <TMatrixD.h>
#include <TRandom.h>


namespace Belle2 {

  /** Hopfield Algorithm with number based inputs.
   *
   *  This class encapsulates the actual algorithm with pure number inputs
   *  (instead of objects of TrackCandidates or whatever).
   *  The reason for this is mainly better testability and potentially as well
   *  reusability.
   *  The development of this class started as a copy from Jakob's implementation for
   *  the VXDTF2.
   *
   *  Relevant resources:
   *  [1] R. Frühwirth, "Selection of optimal subsets of tracks with a feedback neural network",
   *  Comput. Phys. Commun., vol. 78, pp. 23–28, 1993.
   */
  class HopfieldNetwork {
  public:
    /** Constructor taking parameters for the algorithm.
     *
     *  @param omega  Should be between 0 and 1; small values lead to large number of nodes, large ones to large sums of quality indicators.
     *  @param T      Temperature for annealing.
     *  @param Tmin   Minimal reached temperature in the annealing scheme.
     *  @param cmax   Maximum change of weights between iterations, so we accept the network as converged.
     */
    HopfieldNetwork(float omega = 0.5, float T = 3.1, float Tmin = 0.1, float cmax = 0.01):
      m_omega(omega), m_T(T), m_Tmin(Tmin), m_cmax(cmax)
    {}

    /** Performance of the actual algorithm.
     *
     *  HINT FOR SPEED OPTIMIZATION: A lot of time is spend for checking the Logger,
     *  if you don't have LOG_NO_B2DEBUG defined. If you have done that,
     *  a lot of time is taken by the tanh function and drawing random
     *  numbers.<br>
     *
     *  Currently this algorithm can only be used once for each instance,
     *  as the algorithm parameter variables are changed during the performance.
     *  @sa OverlapResolverNodeInfo
     */
    unsigned short doHopfield(std::vector<OverlapResolverNodeInfo>& overlapResolverNodeInfos, unsigned short nIterations = 20)
    {
      //Start value for neurons if they are compatible.
      //Each compatible connection activates a node with this value.
      //As the sum of all the activations shall be less than one, we divide the
      //activiation by the total number of Nodes.
      //Incompatible Nodes get later minus one, which counteracts all activation,
      //if the incompatible Node is active.
      float compatibilityValue = (1.0 - m_omega) / static_cast<float>(overlapResolverNodeInfos.size() - 1);

      //Weight matrix; knows compatibility between each possible pair of Nodes
      TMatrixD W(overlapResolverNodeInfos.size(), overlapResolverNodeInfos.size());
      //A): Set all elements to compatible:
      for (unsigned int i = 0; i < overlapResolverNodeInfos.size(); i++) {
        for (unsigned int j = 0; i < overlapResolverNodeInfos.size(); i++) {
          W(i, j) = compatibilityValue;
        }
      }
      //B): Inform weight matrix elements of incompatible neurons:
      for (const auto& aTC : overlapResolverNodeInfos) {
        for (unsigned int overlapIndex : aTC.overlaps) {
          W(aTC.trackIndex, overlapIndex) = -1.0;
        }
      }

      // Neuron values
      TMatrixD xMatrix(1, overlapResolverNodeInfos.size());
      // randomize neuron values for first iteration:
      for (unsigned int i = 0; i < overlapResolverNodeInfos.size(); i++) {
        xMatrix(0, i) = gRandom->Uniform(1.0); // WARNING: original does Un(0;0.1) not Un(0;1)!
      }

      //Store for results from last round:
      TMatrixD xMatrixOld(1, overlapResolverNodeInfos.size());

      //Order of execution for neuron values:
      std::vector<unsigned short> sequenceVector(overlapResolverNodeInfos.size());
      //iota fills the vector with 0, 1, 2, ... , (size-1)
      std::iota(sequenceVector.begin(), sequenceVector.end(), 0);

      //The following block will be evaluated to empty, if LOG_NO_B2DEBUG is defined:
      B2DEBUG(100, "sequenceVector with length " << sequenceVector.size());
      B2DEBUG(100, "Entries are from begin to end:");
      for (auto && entry : sequenceVector) {
        B2DEBUG(100, std::to_string(entry) + ", ");
      }

      //Store all values of c for protocolling:
      std::vector<float> cValues(nIterations);
      //Store for maximum change of weights between iterations.
      float c = 1.0;

      //Iterate until change in weights is small:
      unsigned iIterations = 0;
      while (c > m_cmax) {

        std::shuffle(sequenceVector.begin(), sequenceVector.end(), TRandomWrapper());

        xMatrixOld = xMatrix;

        for (unsigned int i : sequenceVector) {
          float aTempVal = 0.0;
          for (unsigned int a = 0; a < overlapResolverNodeInfos.size(); a++) {
            aTempVal += W(i, a) * xMatrix(0, a);
          }

          float act = aTempVal + m_omega * overlapResolverNodeInfos[i].qualityIndex;

          xMatrix(0, i) = 0.5 * (1. + tanh(act / m_T));
        }

        m_T = 0.5 * (m_T + m_Tmin);

        //Determine maximum change in weights:
        c = ((xMatrix - xMatrixOld).Abs()).Max();
        B2DEBUG(10, "c value is " << c << " at iteration " << iIterations);
        cValues.at(iIterations) = c;

        if (iIterations + 1 == nIterations) {
          B2INFO("Hopfield reached maximum number of iterations without convergence. cValues are:");
          for (auto && entry : cValues) {
            B2INFO(std::to_string(entry));
          }
          break;
        }
        iIterations++;
      }

      //Copy Node values into the activity state of the OverlapResolverNodeInfo objects:
      for (unsigned int i = 0; i < overlapResolverNodeInfos.size(); i++) {
        overlapResolverNodeInfos[i].activityState = xMatrix(0, i);
      }

      return iIterations;
    }

  private:
    float m_omega;
    float m_T;
    float m_Tmin;
    float m_cmax;

    //--- Structs to help simplify the process ------------------------------------------------------------------------
    /** Wrap TRandom to be useable as a uniform random number generator with std algorithms like std::shuffle. */
    struct TRandomWrapper {
      /** Define the result type to be a normal unsigned int. */
      typedef unsigned int result_type;

      /** Minimum value returned by the random number generator. */
      static constexpr result_type min() { return 0; }

      /** Maximum value returned by the random number generator. */
      static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

      /** Return a random value in the range [min(), max()]. */
      result_type operator()() { return gRandom->Integer(max()); }
    };
  };
}
