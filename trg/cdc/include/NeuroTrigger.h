#ifndef NEUROTRIGGER_H
#define NEUROTRIGGER_H

#include <trg/cdc/dataobjects/CDCTriggerMLP.h>

namespace Belle2 {

  class CDCTriggerSegmentHit;
  class CDCTriggerTrack;

  /** Class to represent the CDC Neurotrigger.
   *
   * The Neurotrigger consists of one or several Multi Layer Perceptrons.
   * The input values are calculated from track segment hits and a 2D track estimate.
   * The output is a scaled estimate of the z-vertex of the track.
   * In case of several MLPs, each is an expert for a different track parameter region.
   *
   * @sa Neurotrigger Modules:
   * @sa NeuroTriggerTrainerModule for preparing training data and training,
   * @sa NeuroTriggerModule for loading trained networks and using them.
   */
  class NeuroTrigger {
  public:
    /** Struct to keep neurotrigger parameters.
     * Contains all information that is needed to initialize several
     * expert MLPs (not including values determined during training).
     */
    struct Parameters {
      /** Number of networks.
       * For network specific parameters you can give either a list with
       * values for each network, or a single value that will be used for all.
       * The ranges are also valid if nPhi * nPt * nTheta * nPattern = nMLPs.
       */
      unsigned nMLP = 1;
      /** Number of nodes in each hidden layer for all networks
       * or factor to multiply with number of inputs.
       * The number of layers is derived from the shape.
       */
      std::vector<std::vector<float>> nHidden = {{3.}};
      /** train z as output */
      bool targetZ = true;
      /** train theta as output */
      bool targetTheta = false;
      /** If true, multiply nHidden with number of input nodes. */
      bool multiplyHidden = true;
      /** Output scale for all networks. */
      std::vector<std::vector<float>> outputScale = {{ -1., 1.}};
      /** Phi region for which MLP is used in degree for all networks. */
      std::vector<std::vector<float>> phiRange = {{0., 360.}};
      /** Charge / Pt region for which MLP is used in 1/GeV for all networks. */
      std::vector<std::vector<float>> invptRange = {{ -5., 5.}};
      /** Theta region for which MLP is used in degree for all networks. */
      std::vector<std::vector<float>> thetaRange = {{17., 150.}};
      /** Phi region for which MLP is trained in degree for all networks.
       *  Can be larger than use range to avoid edge effects. */
      std::vector<std::vector<float>> phiRangeTrain = {{0., 360.}};
      /** Charge / Pt region for which MLP is trained in 1/GeV for all networks.
       *  Can be larger than use range to avoid edge effects. */
      std::vector<std::vector<float>> invptRangeTrain = {{ -5., 5.}};
      /** Theta region for which MLP is trained in degree for all networks.
       *  Can be larger than use range to avoid edge effects. */
      std::vector<std::vector<float>> thetaRangeTrain = {{17., 150.}};
      /** Maximum number of hits in a single super layer for all networks. */
      std::vector<unsigned short> maxHitsPerSL = {1};
      /** Super layer pattern for which MLP is trained for all networks.
       *  Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
       *  0 in bit <i>: hits from super layer <i> are not used.
       *  1 in bit <i>: hits from super layer <i> are used.
       *  SLpattern = 0: use any hits present, don't check the pattern. */
      std::vector<unsigned long> SLpattern = {0};
      /** Super layer pattern mask for which MLP is trained for all networks.
       *  Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
       *  0 in bit <i>: super layer <i> may or may not have a hit.
       *  1 in bit <i>: super layer <i>
       *                - must have a hit if SLpattern bit <i> = 1
       *                - must not have a hit if SLpattenr bit <i> = 0 */
      std::vector<unsigned long> SLpatternMask = {0};
      /** Maximal drift time, identical for all networks. */
      unsigned tMax = 256;
    };

    /** Default constructor. */
    NeuroTrigger() {}

    /** Default destructor. */
    virtual ~NeuroTrigger() {}

    /** Set parameters and get some network independent parameters. */
    void initialize(const Parameters& p);

    /** Get indices for sector ranges in parameter lists. */
    std::vector<unsigned> getRangeIndices(const Parameters& p, unsigned isector);

    /** Save MLPs to file.
     * @param filename name of the TFile to write to
     * @param arrayname name of the TObjArray holding the MLPs in the file
     */
    void save(const std::string& filename, const std::string& arrayname = "MLPs");
    /** Load MLPs from file.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the MLPs in the file
     * @return true if the MLPs were loaded correctly
     */
    bool load(const std::string& filename, const std::string& arrayname = "MLPs");

    /** Loads parameters from the geometry and precalculates some constants
     * that will be needed. */
    void setConstants();

    /** set fixed point precision */
    void setPrecision(std::vector<unsigned> precision) { m_precision = precision; }

    /** set name of the data store elements */
    void setCollectionNames(std::string hitCollectionName, std::string eventTimeName)
    {
      m_hitCollectionName = hitCollectionName;
      m_eventTimeName = eventTimeName;
    }

    /** return reference to a neural network */
    CDCTriggerMLP& operator[](unsigned index) { return m_MLPs[index]; }
    /** return const reference to a neural network */
    const CDCTriggerMLP& operator[](unsigned index) const { return m_MLPs[index]; }

    /** return number of neural networks */
    unsigned nSectors() const { return m_MLPs.size(); }

    /** add an MLP to the list of networks */
    void addMLP(const CDCTriggerMLP& newMLP) { m_MLPs.push_back(newMLP); }

    /** Select all matching expert MLPs based on the given track parameters.
     * If the sectors are overlapping, there may be more than one matching expert.
     * During training this is intended, afterwards sectors should be redefined to be unique.
     * For unique geometrical sectors, this function can still find several experts
     * with different sector patterns.
     * @return indices of the selected MLPs, empty if the track does not fit any sector
     */
    std::vector<int> selectMLPs(float phi0, float invpt, float theta);

    /** Select one MLP from a list of sector indices.
     * The selected expert either matches the given sector pattern,
     * or has no pattern restriction. An unrestricted expert is returned only
     * if there is no exactly matching expert.
     * @return index of the selected MLP, -1 if no matching MLP is found */
    int selectMLPbyPattern(std::vector<int>& MLPs, unsigned long pattern);

    /** Calculate 2D phi position and arclength for the given track and store them. */
    void updateTrack(const CDCTriggerTrack& track);

    /** Calculate 2D phi position and arclength for the given track and store them. */
    void updateTrackFix(const CDCTriggerTrack& track);

    /** Calculate phi position of a hit relative to 2D track
     * (scaled to number of wires). */
    double getRelId(const CDCTriggerSegmentHit& hit);

    /** Determine the event time from the shortest priority time of all hit candidates. */
    int getEventTime(unsigned isector, const CDCTriggerTrack& track);

    /** Calculate input pattern for MLP.
     * @param isector index of the MLP that will use the input
     * @param track   axial hit relations are taken from given track
     * @return super layer pattern of hits in the current track
     */
    unsigned long getInputPattern(unsigned isector, const CDCTriggerTrack& track);

    /** Select best hits for each super layer
     * @param isector              index of the MLP that will use the input
     * @param track                axial hit relations are taken from given track
     * @param returnAllRelevant    if true, return all relevant hits instead of
     *                             selecting the best (for making relations)
     * @return list of selected hit indices */
    std::vector<unsigned> selectHits(unsigned isector, const CDCTriggerTrack& track,
                                     bool returnAllRelevant = false);

    /** Calculate input values for MLP.
     * @param isector index of the MLP that will use the input
     * @param hitIds hit indices to be used for the input
     * @return scaled vector of input values (1 for each input node)
     */
    std::vector<float> getInputVector(unsigned isector, const std::vector<unsigned>& hitIds);

    /** Run an expert MLP.
     * @param isector index of the MLP
     * @param input vector of input values
     * @return unscaled output values (z vertex in cm and/or theta in radian) */
    std::vector<float> runMLP(unsigned isector, std::vector<float> input);

    /** Run an expert MLP with fixed point arithmetic. */
    std::vector<float> runMLPFix(unsigned isector, std::vector<float> input);

  private:
    /** List of networks */
    std::vector<CDCTriggerMLP> m_MLPs = {};
    /** Radius of the CDC layers with priority wires (2 per super layer) */
    double m_radius[9][2] = {};
    /** Number of track segments up to super layer */
    unsigned m_TSoffset[10] = {};
    /** 2D phi position of current track scaled to number of wires */
    double m_idRef[9][2] = {};
    /** 2D crossing angle of current track */
    double m_alpha[9][2] = {};
    /** Fixed point precision in bit after radix point.
     *  8 values:
     *  - 2D track parameters: omega, phi
     *  - geometrical values derived from track: crossing angle, reference wire ID
     *  - scale factor: radian to wire ID
     *  - MLP values: nodes, weights, activation function LUT input (LUT output = nodes)
     */
    std::vector<unsigned> m_precision;
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** Name of the StoreObjPtr containing the event time. */
    std::string m_eventTimeName;
  };
}
#endif
