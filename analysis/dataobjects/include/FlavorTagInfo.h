/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca and Fernando Abudinen                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FLAVORTAGINFO_H
#define FLAVORTAGINFO_H

#include <framework/datastore/RelationsObject.h>
#include <analysis/dataobjects/Particle.h>

#include <vector>
#include <set>

namespace Belle2 {

  // forward declarations

  class Track;
  class MCParticle;

  /**
   * This class stores the relevant information for the TagV vertex fit, extracted mainly from the
   * Flavor Tagging Module. It also stores MC true information that could be used to check the goodness
   * of the fit.
   *
   *
   * The way the information is stored is into vectors with the same length. Every position
   * in all vectors correspond to the same event-category information, so that every category
   * is uniquely correlated to one, e.g, track. Variables stored:
   *
   * - Track's probability of being a Btag daughter for every category
   * - Highest probability track's pointer
   * - Track's probability to belong to a given category
   * - MC information obtained in the TagV module
   *
   * This class is still in an early phase, thus some changes may be needed.
   */

  class FlavorTagInfo : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0 (all vectors are empty).
     */
    FlavorTagInfo()
    {
      m_goodTracksPurityFT = 0;
      m_goodTracksPurityROE = 0;
      m_badTracksROE = 0;
      m_goodTracksROE = 0;
      m_badTracksFT = 0;
      m_goodTracksFT = 0;
    };


    /**
     * SETTERS
     * Some of the setters are expected to fill a vector whose each element corresponds to a given category
     * in the following order:
     * Electron, Muon, Kinetic Lepton, Kaon, Slow Pion, Fast Pion, Lambda, Highest Momentum
     * The setting of all elements need to be performed in a loop. For example, setTrack() need to be called
     * once per each category so that the vector is filled progressively. The same holds for all the other
     * setters described as Vector Fillers.
     */

    /**
    * Map filler: Set the category name and the pointer of the track with the highest target track probability
    * for the corresponding category in Event Level.
    * @param category string name of the given category
    * @param track pointer to track object
    */
    void setTargetTrackLevel(std::string category, Belle2::Track* track);

    /**
    * Map filler: Set the category name and the corresponding highest target track probability.
    * @param category string name of the given category
    * @param probability highest target track probability
    */
    void setProbTrackLevel(std::string category, float probability);

    /**
    * Map filler: Set the category name and the pointer of the track with the highest category probability
    * for the corresponding category in Event Level.
    * @param category string name of the given category
    * @param track pointer to track object
    */
    void setTargetEventLevel(std::string category, Belle2::Track* track);

    /**
    * Map filler: Set the category name and the highest category probability for the corresponding
    * category in Event Level.
    * @param category string name of the given category
    * @param probability highest category probability
    */
    void setProbEventLevel(std::string category, float probability);

    /**
    * Map filler: Set the category name and the corresponding qr Output, i.e. the Combiner input value.
    * They could be used for independent tags.
    * @param category string name of the given category
    * @param qr output of the given category
    */
    void setQrCategory(std::string category, float qr);

    /**
    * Saves the usemode of the FlavorTagger
    * @param mode "Teacher" or "Expert"
    */
    void setUseModeFlavorTagger(std::string mode);

    /**
    * Saves the method used for the FlavorTagger
    * @param method "TMVA" or "FANN".
    */
    void setMethod(std::string method);

    /**
    * Saves qr Output of the Combiner. Output of the FlavorTagger after the complete process.
    * @param qr final FlavorTagger output
    */
    void setQrCombined(float qr);

    /**
    * Saves the B0Probability output of the Combiner.
    * @param B0Probability probability of being a B0
    */
    void setB0Probability(float B0Probability);

    /**
    * Saves the B0barProbability output of the Combiner.
    * @param B0barProbability Probability of being a B0bar
    */
    void setB0barProbability(float B0barProbability);



    /**
    * Vector filler: Set the pointer of the track with the highest probability for each category.
    * @param track pointer to track object
    */
    void setTrack(Belle2::Track* track);

    /**
    * Vector filler: Set the pointer of the Partcile related to the track with the highest probability
    * for each category.
    * @param Particle pointer to Particle object related to the track
    */
    void setParticle(Particle* Particle);

    /**
    * Vector filler: Set the pointer of the MCParticle related related to track with the highest probability
    * for each category.
    * @param MCParticle pointer to MCParticle object
    */
    void setMCParticle(Belle2::MCParticle* MCParticle);

    /**
    * Vector filler: Set the pointer of the MCParticle's mother related to the track with the highest
    * probability for each category.
    * @param MCParticle pointer to MCParticle object
    */
    void setMCParticleMother(Belle2::MCParticle* MCParticle);

    /**
    * Vector filler: Set the probability of the event to correspond to a given category
    * probability for each category.
    * @param catProb category probability
    */
    void setCatProb(float catProb);

    /**
    * Vector filler: Set the probability of each category's track of being a direct daughter of the Btag
    * @param targProb target track's probability
    */
    void setTargProb(float);

    /**
    * Vector filler: Set the momentum absolute value of the corresponding category's track
    * @param momentum momentum
    */
    void setP(float momentum);

    /**
    * Vector filler: Set the MC matched code of the track depending on its mother (usually set in TagV module)
    * Quick reference:
    * 0 - does not come from the B
    * 1,2,3,4 - come from an immediately decaying daughter of the B (number depends on the PDG code of each meson)
    * 5 - come directly from the B
    * > 5 - come from a chain of immediately decaying daugthers of the B (11 - 14: one step, 21-24: two steps...)
    * @param isFromB MC matched code
    */
    void setIsFromB(int isFromB);

    /**
    * Vector filler: Set the name of the categories
    * @param catName name of each category
    */
    void setCategories(std::string catName);

    /**
    * Vector filler: Set the impact parameter D0 of each track in an accesible way
    * @param D0 impact parameter
    */
    void setD0(double D0);

    /**
    * Vector filler: Set the impact parameter Z0 of each track in an accesible way
    * @param Z0 impact parameter
    */
    void setZ0(double Z0);

    /**
    * Vector filler: Set resolution of the production point of each track, i.e
    * real production point - MC matched production point.
    * @param deltaProdZ production point difference
    */
    void setProdPointResolutionZ(float deltaProdZ);

//      /**
//      * Vector filler: Set the momentum absolute value of the corresponding category's track
//      * @param momentum momentum
//      */
//     void setTracks(std::vector<Belle2::Track*>); // Save a vector of tracks


    /**
    * Set the fraction of tracks coming from the Btag respect to the total amount of tracks within this class
    * @param goodTracksPurity  fraction E [0,1]
    */
    void setGoodTracksPurityFT(float goodTracksPurity);

    /**
    * Set the fraction of tracks coming from the Btag respect to the total amount of tracks within RoE
    * @param goodTracksPurity  fraction E [0,1]
    */
    void setGoodTracksPurityROE(float goodTracksPurity);

    /**
    * Set the number of tracks coming from the Btag in the RoE
    * @param numTracks
    */
    void setGoodTracksROE(int numTracks);

    /**
    * Set the number of tracks coming not from the Btag in the RoE
    * @param numTracks
    */
    void setBadTracksROE(int numTracks);

    /**
    * Set the number of tracks coming from the Btag within this class
    * @param numTracks
    */
    void setGoodTracksFT(int numTracks);

    /**
    * Set the number of tracks not coming from the Btag within this class
    * @param numTracks
    */
    void setBadTracksFT(int numTracks);




    /**
     * GETTERS
     */

    /**< map containing the category name and the corresponding pointer to the track with highest target probability in Track Level
    * @return map
    */
    std::map<std::string, Belle2::Track*> getTargetTrackLevel();

    /**< map containing the category name and thecorresponding highest target track probability in Track Level
    * @return map
    */
    std::map<std::string, float> getProbTrackLevel();

    /**< map containing the category name and the corresponding pointer to the track with highest category probability in Event Level
    * @return map
    */
    std::map<std::string, Belle2::Track*> getTargetEventLevel();

    /**< map containing the category name and the corresponding highest category probability in Event Level
    * @return map
    */
    std::map<std::string, float> getProbEventLevel();

    /**< map containing the category name and the corresponding qr Output
    * @return map
    */
    std::map<std::string, float> getQrCategory();

    /**
    * usemode of the FlavorTagger
    * @return mode "Teacher" or "Expert"
    */
    std::string getUseModeFlavorTagger();


    /** method used for the FlavorTagger
    * @return "TMVA" or "FANN".
    */
    std::string getMethod();

    /**< qr Output of the Combiner
    * @return qr
    */
    float getQrCombined();

    /**Probability of being a B0.
    * @return B0Probability
    */
    float getB0Probability();

    /**Probability of being a B0bar.
    * @return B0barProbability
    */
    float getB0barProbability(); /**< Direct Output of the Combiner: Probability of being a B0bar).*/


    /**
    * Get all the tracks
    * @return vector of tracks
    */
    std::vector<Belle2::Track*> getTracks();

    /**
    * Get a single track
    * @return track
    * @param position integer corresponding to the position of the desired track
    */
    Belle2::Track* getTrack(int position);

    /**
    * Get all the event probabilities to correspond to each category
    * @return vector of event probabilities
    */
    std::vector<float> getCatProb();

    /**
    * Get all the track probabilities to come directly from the Btag for each category
    * @return vector of track probabilities
    */
    std::vector<float> getTargProb();

    /**
    * Get the momentum of every category's track
    * @return vector momentum
    */
    std::vector<float> getP();

    /**
    * Get the particle's pointer related to each track
    * @return vector of particle pointers
    */
    std::vector<Particle*> getParticle();

    /**
    * Get the MC particle's pointer related to each track
    * @return vector of particle pointers
    */
    std::vector<Belle2::MCParticle*> getMCParticle();

    /**
    * Get the mother MC particle's pointer related to each track
    * @return vector of particle pointers
    */
    std::vector<Belle2::MCParticle*> getMCParticleMother();

    /**
    * Get the impact parameter D0 of every category's track
    * @return vector of D0
    */
    std::vector<float> getD0();

    /**
    * Get the impact parameter Z0 of every category's track
    * @return vector of Z0
    */
    std::vector<float> getZ0();

    /**
    *  Get the MC matched code of every track depending on its mother. Code explained in the setter
    * @return vector of codes
    */
    std::vector<int> getIsFromB();

    /**
    * Get the list of names of the categories of this object
    * @return vector of names
    */
    std::vector<std::string> getCategories();

    /**
    * Get resolution of the production point of each track, i.e
    * real production point - MC matched production point.
    * @return vector of production point resolutions
    */
    std::vector<float> getProdPointResolutionZ();

    /**
    * Get the ratio (purity) of tracks coming from the Btag out of the total amount of
    * tracks within this object.
    * @return Primary tracks purity
    */
    float getGoodTracksPurityFT();

    /**
    * Get the ratio (purity) of tracks coming from the Btag out of the total amount of
    * tracks within the correspondent RestOfEvent object.
    * @return Primary tracks purity
    */
    float getGoodTracksPurityROE();

    /**
    * Get the number of tracks not coming from the Btag within the correspondent RestOfEvent object
    * @return number of tracks
    */
    int getBadTracksROE();

    /**
    * Get the number of tracks coming from the Btag within the correspondent RestOfEvent object
    * @return number of tracks
    */
    int getGoodTracksROE();

    /**
    * Get the number of tracks not coming from the Btag within this object
    * @return number of tracks
    */
    int getBadTracksFT();

    /**
    * Get the number of tracks not coming from the Btag within this object
    * @return number of tracks
    */
    int getGoodTracksFT();


  private:

    // persistent data members

    // Track Level Flavor Tagger Info

    std::string m_useModeFlavorTagger; /**< Usemode of the FlavorTagger: "Teacher" or "Expert".*/

    std::string m_method; /**< Method used for the FlavorTagger: "TMVA" or "FANN".*/

    std::map<std::string, Belle2::Track*>
    m_targetTrackLevel; /**< map containing the category name and the corresponding pointer to the track with highest target probability in Track Level*/
    std::map<std::string, float>
    m_probTrackLevel; /**< map containing the category name and thecorresponding highest target track probability in Track Level*/
    std::map<std::string, Belle2::Track*>
    m_targetEventLevel; /**< map containing the category name and the corresponding pointer to the track with highest category probability in Event Level*/
    std::map<std::string, float>
    m_probEventLevel; /**< map containing the category name and the corresponding highest category probability in Event Level*/
    std::map<std::string, float>
    m_qrCategory; /**< map containing the category name and the corresponding qr Output, i.e. the Combiner input value. They could be used for independent tags.*/

    float m_qrCombined; /**< qr Output of the Combiner. Output of the FlavorTagger after the complete process*/
    float m_B0Probability; /**< Direct Output of the Combiner: Probability of being a B0.*/
    float m_B0barProbability; /**< Direct Output of the Combiner: Probability of being a B0bar).*/

    std::vector<Belle2::Track*> m_tracks;  /**< highest probability track for each category */
    std::vector<Particle*> m_particle; /**< particle pointer related to each track */
    std::vector<Belle2::MCParticle*> m_MCparticle; /**< MC particle pointer related to each track */
    std::vector<Belle2::MCParticle*> m_MCparticle_mother; /**< mother MC particle pointer related to each track */
    std::vector<float> m_D0; /**< impact parameter D0 related to each track */
    std::vector<float> m_Z0; /**< impact parameter Z0 related to each track */
    std::vector<std::string> m_categories;  /**< categories name */
    std::vector<float> m_prodPointResZ;  /**< production point resolution Z for each track */
    std::vector<float> m_categoryProb; /**< probability of the event to belong to each category */
    std::vector<float> m_targetProb; /**< probability of each track to be primary */
    std::vector<float> m_P; /**< absolute 3 momentum of each track */
    std::vector<int> m_isFromB; /**< MC matched code of each track depending on its mother */

    float m_goodTracksPurityFT; /**< purity of tracks coming from the Btag in this objecy */
    float m_goodTracksPurityROE; /**< purity of tracks coming from the Btag in the correspondent RestOfEvent object */
    int m_badTracksROE; /**< number of tracksc not coming from Btag in the correspondent RestOfEvent object*/
    int m_goodTracksROE; /**< number of tracksc coming from Btag in the correspondent RestOfEvent */
    int m_badTracksFT; /**< number of tracksc not coming from Btag in this object */
    int m_goodTracksFT; /**< number of tracksc coming from Btag in this object */

    ClassDef(FlavorTagInfo, 3) /**< class definition */

  };


} // end namespace Belle2

#endif
