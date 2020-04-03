/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Thibaud Humair                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// DataStore
#include <framework/database/DBObjPtr.h>

// Rave
#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>
#include <analysis/VertexFitting/RaveInterface/RaveKinematicVertexFitter.h>

// KFit
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>

// DataObjects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>


namespace Belle2 {

  class Particle;
  struct ParticleAndWeight;

  /**
   * Tag side Vertex Fitter module for modular analysis
   *
   * This module fits the Btag Vertex    *
   */
  class TagVertexModule : public Module {

  public:

    /**
     * Constructor
     */
    TagVertexModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * returns the BeamSpot object
     */
    const BeamSpot& getBeamSpot() const { return *m_beamSpotDB; }


  private:

    //std::string m_EventType;      /**< Btag decay type */
    std::string m_listName;         /**< Breco particle list name */
    double m_confidenceLevel;       /**< required fit confidence level */
    std::string m_useMCassociation; /**< No MC association or standard Breco particle or internal MCparticle association */
    //std::string m_useFitAlgorithm;    /**< Choose constraint: from Breco or tube in the boost direction w/wo cut */
    std::string m_constraintType;   /**< Choose constraint: noConstraint, IP, tube, boost, (breco) */
    std::string m_trackFindingType;   /**< Choose how to find the tag tracks: standard, standard_PXD */
    int m_reqPXDHits;                /**< N of PXD hits for a track to be used */
    std::string m_roeMaskName;      /**< ROE particles from this mask will be used for vertex fitting */
    double m_Bfield;              /**< magnetic field from data base */
    std::vector<const Particle*> m_tagParticles;  /**< tracks of the rest of the event */
    std::vector<const Particle*> m_raveParticles; /**< tracks given to rave for the track fit (after removing Kshorts */
    std::vector<double> m_raveWeights; /**< Store the weights used by Rave in the vtx fit so that they can be accessed later */
    std::vector<const MCParticle*>
    m_raveMCParticles; /**< Store the MC particles corresponding to each track used by Rave in the vtx fit */
    bool m_useTruthInFit;  /**< Set to true if the tag fit is to be made with the TRUE tag track momentum and position */
    int m_fitTruthStatus; /**< Store info about whether the fit was performed with the truth info
                           * 0 fit performed with measured parameters
                           * 1 fit performed with true parameters
                           * 2 unable to recover truth parameters */
    double m_fitPval;             /**< P value of the tag side fit result */
    TVector3 m_tagV;              /**< tag side fit result */
    TMatrixDSym m_tagVErrMatrix;  /**< Error matrix of the tag side fit result */
    TVector3 m_MCtagV;            /**< generated tag side vertex */
    int m_mcPDG;                  /**< generated tag side B flavor */
    TVector3 m_MCVertReco;        /**< generated Breco decay vertex */
    double m_deltaT;              /**< reconstructed DeltaT */
    double m_deltaTErr;           /**< reconstructed DeltaT error */
    double m_MCdeltaT;            /**< generated DeltaT */
    TMatrixDSym m_constraintCov;  /**< constraint to be used in the tag vertex fit */
    TVector3 m_constraintCenter;  /**< centre position of the constraint for the tag Vertex fit */
    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_BeamSpotCov;    /**< size of the beam spot == covariance matrix on the beam spot position */
    bool m_MCInfo;                /**< true if user wants to retrieve MC information out from the tracks used in the fit */
    double m_shiftZ;              /**< parameter for testing the systematic error from the IP measurement*/
    DBObjPtr<BeamSpot> m_beamSpotDB;/**< Beam spot database object*/
    int m_FitType;                /**< fit algo used  */
    double m_tagVl;               /**< tagV component in the boost direction  */
    double m_truthTagVl;          /**< MC tagV component in the boost direction  */
    double m_tagVlErr;            /**< Error of the tagV component in the boost direction  */
    double m_tagVol;              /**< tagV component in the direction orthogonal to the boost */
    double m_truthTagVol;         /**< MC tagV component in the direction orthogonal to the boost */
    double m_tagVolErr;           /**< Error of the tagV component in the direction orthogonal to the boost */
    double m_tagVNDF;             /**< Number of degrees of freedom in the tag vertex fit */
    double m_tagVChi2;            /**< chi^2 value of the tag vertex fit result */
    double m_tagVChi2IP;          /**< IP component of the chi^2 of the tag vertex fit result */
    std::string m_fitAlgo;        /**< Algorithm used for the tag fit (Rave or KFit) */
    bool m_verbose;               /**< choose if you want to print extra infos */
    TMatrixDSym m_pvCov;          /**< covariance matrix of the PV (useful with tube and KFit) */
    TLorentzVector m_tagMomentum; /**< B tag momentum computed from fully reconstructed B sig */


    /** central method for the tag side vertex fit */
    bool doVertexFit(Particle* Breco);

    /** find intersection between B rec and beam spot (= origin of BTube) */
    bool doVertexFitForBTube(Particle* mother);

    /** calculate the constraint for the vertex fit on the tag side using Breco information*/
    bool findConstraint(Particle* Breco, double cut);

    /** calculate the standard constraint for the vertex fit on the tag side*/
    bool findConstraintBoost(double cut, double shiftAlongBoost = -2000.);

    /** calculate constraint for the vertex fit on the tag side using the B tube (cylinder along
    the expected BTag line of flights */
    bool findConstraintBTube(Particle* Breco, double cut);

    /** get the vertex of the MC B particle associated to Btag. It works anly with signal MC */
    void BtagMCVertex(Particle* Breco);

    /** compare Breco with the two MC B particles */
    bool compBrecoBgen(Particle* Breco, MCParticle* Bgen);

    /** performs the fit using the standard algorithm - using all tracks in RoE
    The user can specify a request on the PXD hits left by the tracks*/
    bool getTagTracks_standardAlgorithm(Particle* Breco, int nPXDHits);

    /**
     * Get a list of pions from a list of pions removing the Kshorts
     * Warning: this assumes all the particles are pions, which is fine are all the particles
     * are reconstructed as pions in the TagV module.
     */
    bool getParticlesWithoutKS(std::vector<const Particle*> const&  tagParticles, std::vector<ParticleAndWeight>& particleAndWeights,
                               double massWindowWidth = 0.01);

    /** TO DO: tag side vertex fit in the case of semileptonic tag side decay */
    //bool makeSemileptonicFit(Particle *Breco);


    /**
     * make the vertex fit on the tag side with chosen fit algorithm
     */
    bool makeGeneralFit();

    /**
     * make the vertex fit on the tag side:
     * RAVE AVF
     * tracks coming from Ks removed
     * all other tracks used
     */
    bool makeGeneralFitRave();

    /**
     * make the vertex fit on the tag side:
     * KFit
     * tracks coming from Ks removed
     * all other tracks used
     */
    bool makeGeneralFitKFit();

    /**
     * calculate DeltaT and MC-DeltaT (rec - tag) in ps from Breco and Btag vertices
     * DT = Dl / gamma beta c  ,  l = boost direction
     */
    void deltaT(Particle* Breco);

    /**
     * Reset all parameters that are computed in each event and then used to compute
     * tuple variables
     */
    void resetReturnParams();

    /**
     * Print a TVector3 (useful for debugging)
     */
    std::string printVector(TVector3 const& vec);

    /**
     * Print a TMatrix (useful for debugging)
     */
    std::string printMatrix(TMatrix const& mat);

    /**
     * Print a TMatrixFSym (useful for debugging)
     */
    std::string printMatrix(TMatrixFSym const& mat);

    /**
     * If the fit has to be done with the truth info, Rave is fed with a track
     * where the momentum is replaced by the true momentum and the position replaced
     * by the point on the true particle trajectory closest to the measured track position
     * The function below takes care of that.
     */
    TrackFitResult getTrackWithTrueCoordinates(ParticleAndWeight const& paw);

    /**
     * This finds the point on the true particle trajectory closest to the measured track position
     */
    TVector3 getTruePoca(ParticleAndWeight const& paw);
  };

  /**
   * this struct is used to store and sort the tag tracks
   */
  struct ParticleAndWeight {
    double weight; /**< rave weight associated to the track, for sorting purposes*/
    const Particle* particle;  /**< tag track fit result with pion mass hypo, for sorting purposes */
    const MCParticle* mcParticle;  /**< mc particle matched to the tag track, for sorting purposes*/
  };

  /**
   * This is used to sort the tag tracks by rave weight, to have the tracks having the
   * most significance contribution to the tag vertex fit appearing first
   */
  bool compare(ParticleAndWeight& a, ParticleAndWeight& b) { return a.weight > b.weight; }
}
