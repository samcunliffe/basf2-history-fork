/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Thibaud Humair                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/TagVertex/TagVertexModule.h>

//to help printing out stuff
#include<sstream>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/TagVertex.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/utility/ParticleCopy.h>
#include <analysis/utility/CLHEPToROOT.h>
#include <analysis/utility/ROOTToCLHEP.h>
#include <analysis/utility/DistanceTools.h>
#include <analysis/utility/RotationTools.h>

// vertex fitting
#include <analysis/VertexFitting/KFit/VertexFitKFit.h>

// msdt dataobject
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternVXD.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TVector.h>
#include <TRotation.h>


using namespace std;

namespace Belle2 {

  // import tools from RotationTools.h
  using RotationTools::rotateTensor;
  using RotationTools::rotateTensorInv;
  using RotationTools::toSymMatrix;

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------
  REG_MODULE(TagVertex)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TagVertexModule::TagVertexModule() : Module(),
    m_Bfield(0), m_fitTruthStatus(0), m_fitPval(0), m_MCtagLifeTime(-1), m_mcPDG(0), m_MCLifeTimeReco(-1),
    m_deltaT(0), m_deltaTErr(0), m_MCdeltaT(0), m_MCdeltaTapprox(0),
    m_shiftZ(0), m_FitType(0), m_tagVl(0),
    m_truthTagVl(0), m_tagVlErr(0), m_tagVol(0), m_truthTagVol(0), m_tagVolErr(0), m_tagVNDF(0), m_tagVChi2(0), m_tagVChi2IP(0),
    m_verbose(true)
  {
    // Set module properties
    setDescription("Tag side Vertex Fitter for modular analysis");

    // Parameter definitions
    addParam("listName", m_listName, "name of particle list", string(""));
    addParam("confidenceLevel", m_confidenceLevel,
             "required confidence level of fit to keep particles in the list. Note that even with confidenceLevel == 0.0, errors during the fit might discard Particles in the list. confidenceLevel = -1 if an error occurs during the fit",
             0.001);
    addParam("MCAssociation", m_useMCassociation,
             "'': no MC association. breco: use standard Breco MC association. internal: use internal MC association", string("breco"));
    addParam("constraintType", m_constraintType,
             "Choose the type of the constraint: noConstraint, IP (tag tracks constrained to be within the beam spot), tube (long tube along the BTag line of flight, only for fully reconstruced B rec), boost (long tube along the Upsilon(4S) boost direction), (breco)",
             string("IP"));
    addParam("trackFindingType", m_trackFindingType,
             "Choose how to reconstruct the tracks on the tag side: standard, standard_PXD",
             string("standard_PXD"));
    addParam("maskName", m_roeMaskName,
             "Choose ROE mask to get particles from ", string(""));
    addParam("askMCInformation", m_MCInfo,
             "TRUE when requesting MC Information from the tracks performing the vertex fit", false);
    addParam("reqPXDHits", m_reqPXDHits,
             "Minium number of PXD hits for a track to be used in the vertex fit", 0);
    addParam("fitAlgorithm", m_fitAlgo,
             "Fitter used for the tag vertex fit: Rave or KFit", string("Rave"));
    addParam("useTruthInFit", m_useTruthInFit,
             "Use the true track parameters in the vertex fit", false);


  }

  void TagVertexModule::initialize()
  {
    // magnetic field
    m_Bfield = BFieldManager::getField(m_BeamSpotCenter).Z() / Unit::T;
    // RAVE setup
    analysis::RaveSetup::initialize(1, m_Bfield);
    B2INFO("TagVertexModule : magnetic field = " << m_Bfield);
    // truth fit status will be set to 2 only if the MC info cannot be recovered
    if (m_useTruthInFit) m_fitTruthStatus = 1;

    //TODO: this won't work with nonstandard name for Particle array (e.g. will fail when adding relations)
    //input
    StoreArray<Particle> particles;
    particles.isRequired();
    // output
    StoreArray<TagVertex> verArray;
    verArray.registerInDataStore();
    particles.registerRelationTo(verArray);
    //check if the fitting algorithm name  is set correctly
    if (m_fitAlgo != "Rave" && m_fitAlgo != "KFit")
      B2FATAL("TagVertexModule: invalid fitting algorithm (must be set to either Rave or KFit).");
    //temporary while the one track fit is broken
    if (m_trackFindingType == "singleTrack" || m_trackFindingType == "singleTrack_PXD")
      B2FATAL("TagVertexModule : the singleTrack option is temporarily broken.");
  }

  void TagVertexModule::beginRun()
  {
    //TODO: set magnetic field for each run
    //m_Bfield = BFieldMap::Instance().getBField(m_BeamSpotCenter).Z();
  }

  void TagVertexModule::event()
  {
    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("TagVertexModule: ParticleList " << m_listName << " not found");
      return;
    }

    // input
    StoreArray<Particle> Particles(plist->getParticleCollectionName());

    // output
    StoreArray<TagVertex> verArray;
    analysis::RaveSetup::initialize(1, m_Bfield);

    std::vector<unsigned int> toRemove;

    for (unsigned i = 0; i < plist->getListSize(); ++i) {
      resetReturnParams();

      Particle* particle =  plist->getParticle(i);
      if (m_useMCassociation == "breco" || m_useMCassociation == "internal") BtagMCVertex(particle);
      bool ok = doVertexFit(particle);
      if (ok) deltaT(particle);

      if ((m_fitPval < m_confidenceLevel && m_confidenceLevel != 0)
          || (m_fitPval <= m_confidenceLevel && m_confidenceLevel == 0)) {
        toRemove.push_back(particle->getArrayIndex());
      } else {
        // save information in the Vertex StoreArray
        TagVertex* ver = verArray.appendNew();
        // create relation: Particle <-> Vertex
        particle->addRelationTo(ver);
        // fill Vertex with content
        if (ok) {
          ver->setTagVertex(m_tagV);
          ver->setTagVertexErrMatrix(m_tagVErrMatrix);
          ver->setTagVertexPval(m_fitPval);
          ver->setDeltaT(m_deltaT);
          ver->setDeltaTErr(m_deltaTErr);
          ver->setMCTagVertex(m_MCtagV);
          ver->setMCTagBFlavor(m_mcPDG);
          ver->setMCDeltaT(m_MCdeltaT);
          ver->setMCDeltaTapprox(m_MCdeltaTapprox);
          ver->setFitType(m_FitType);
          ver->setNTracks(m_tagParticles.size());
          ver->setTagVl(m_tagVl);
          ver->setTruthTagVl(m_truthTagVl);
          ver->setTagVlErr(m_tagVlErr);
          ver->setTagVol(m_tagVol);
          ver->setTruthTagVol(m_truthTagVol);
          ver->setTagVolErr(m_tagVolErr);
          ver->setTagVNDF(m_tagVNDF);
          ver->setTagVChi2(m_tagVChi2);
          ver->setTagVChi2IP(m_tagVChi2IP);
          ver->setVertexFitParticles(m_raveParticles);
          ver->setVertexFitMCParticles(m_raveMCParticles);
          ver->setRaveWeights(m_raveWeights);
          ver->setConstraintType(m_constraintType);
          ver->setConstraintCenter(m_constraintCenter);
          ver->setConstraintCov(m_constraintCov);
          ver->setFitTruthStatus(m_fitTruthStatus);
        } else {
          ver->setTagVertex(m_tagV);
          ver->setTagVertexPval(-1.);
          ver->setDeltaT(m_deltaT);
          ver->setDeltaTErr(m_deltaTErr);
          ver->setMCTagVertex(m_MCtagV);
          ver->setMCTagBFlavor(0.);
          ver->setMCDeltaT(m_MCdeltaT);
          ver->setMCDeltaTapprox(m_MCdeltaTapprox);
          ver->setFitType(m_FitType);
          ver->setNTracks(m_tagParticles.size());
          ver->setTagVl(m_tagVl);
          ver->setTruthTagVl(m_truthTagVl);
          ver->setTagVlErr(m_tagVlErr);
          ver->setTagVol(m_tagVol);
          ver->setTruthTagVol(m_truthTagVol);
          ver->setTagVolErr(m_tagVolErr);
          ver->setTagVNDF(-1111.);
          ver->setTagVChi2(-1111.);
          ver->setTagVChi2IP(-1111.);
          ver->setVertexFitParticles(m_raveParticles);
          ver->setVertexFitMCParticles(m_raveMCParticles);
          ver->setRaveWeights(m_raveWeights);
          ver->setConstraintType(m_constraintType);
          ver->setConstraintCenter(m_constraintCenter);
          ver->setConstraintCov(m_constraintCov);
          ver->setFitTruthStatus(m_fitTruthStatus);
        }
      }

    }
    plist->removeParticles(toRemove);

    //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
    //
    analysis::RaveSetup::getInstance()->reset();

  }

  bool TagVertexModule::doVertexFit(const Particle* Breco)
  {
    //reset the fit truth status in case it was set to 2 in a previous fit

    if (m_useTruthInFit) m_fitTruthStatus = 1;

    //set constraint type, reset pVal and B field

    m_fitPval = 1;
    bool ok = false;

    if (!(Breco->getRelatedTo<RestOfEvent>())) {
      m_FitType = -1;
      return false;
    }

    if (m_Bfield == 0) {
      B2ERROR("TagVertex: No magnetic field");
      return false;
    }

    // recover beam spot info

    m_BeamSpotCenter = m_beamSpotDB->getIPPosition();
    m_BeamSpotCov.ResizeTo(3, 3);
    m_BeamSpotCov = m_beamSpotDB->getCovVertex();


    //make the beam spot bigger for the standard constraint

    double beta = PCmsLabTransform().getBoostVector().Mag();
    double bg = beta / sqrt(1 - beta * beta);

    //TODO: What's the origin of these numbers?
    double cut = 8.717575e-02 * bg;
    m_shiftZ = 4.184436e+02 * bg *  0.0001;

    //tube length here set to 20 * 2 * c tau beta gamma ~= 0.5 cm, should be enough to not bias the decay
    //time but should still help getting rid of some pions from kshorts
    if (m_constraintType == "IP")    ok = findConstraintBoost(cut);
    else if (m_constraintType == "tube")  ok = findConstraintBTube(Breco, 1000 * cut);
    else if (m_constraintType == "boost") ok = findConstraintBoost(cut * 200000.);
    else if (m_constraintType == "breco") ok = findConstraint(Breco, cut * 2000.);
    else if (m_constraintType == "noConstraint") ok = true;

    if (!ok) {
      B2ERROR("TagVertex: No correct fit constraint");
      return false;
    }

    /* Depending on the user's choice, one of the possible algorithms is chosen for the fit. In case the algorithm does not converge, in order to assure
       high efficiency, the next algorithm less restictive is used. I.e, if standard_PXD does not work, the program tries with standard.
    */

    m_FitType = 0;
    double minPVal = (m_fitAlgo != "KFit") ? 0.001 : 0.;

    if (m_trackFindingType == "standard_PXD") {
      m_tagParticles = getTagTracks_standardAlgorithm(Breco, 1);
      ok = (m_tagParticles.size() > 0);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 3;
      }
    }

    if ((ok == false || m_fitPval < minPVal) || m_trackFindingType == "standard") {
      m_tagParticles = getTagTracks_standardAlgorithm(Breco, m_reqPXDHits);
      ok = (m_tagParticles.size() > 0);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 4;
      }
    }

    if ((ok == false || (m_fitPval <= 0. && m_fitAlgo == "Rave")) && m_constraintType != "noConstraint") {
      ok = findConstraintBoost(cut * 200000.);
      if (ok) {
        m_tagParticles = getTagTracks_standardAlgorithm(Breco, m_reqPXDHits);
        ok = (m_tagParticles.size() > 0);
      }
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 5;
      }
    }

    return ok;

  }





  bool TagVertexModule::findConstraint(const Particle* Breco, double cut)
  {
    if (Breco->getPValue() < 0.) return false;

    TMatrixDSym beamSpotCov(3);
    beamSpotCov = m_beamSpotDB->getCovVertex();

    analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, beamSpotCov);

    double pmag = Breco->getMomentumMagnitude();
    double xmag = (Breco->getVertex() - m_BeamSpotCenter).Mag();

    TVector3 Pmom = (pmag / xmag) * (Breco->getVertex() - m_BeamSpotCenter);

    TMatrixDSym TerrMatrix = Breco->getMomentumVertexErrorMatrix();
    TMatrixDSym PerrMatrix(7);

    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (i == j) {
          PerrMatrix(i, j) = (beamSpotCov(i, j) + TerrMatrix(i, j)) * Pmom.Mag() / xmag;
        } else {
          PerrMatrix(i, j) = TerrMatrix(i, j);
        }
        PerrMatrix(i + 4, j + 4) = TerrMatrix(i + 4, j + 4);
      }
    }

    PerrMatrix(3, 3) = 0.;

    Particle* Breco2 = new Particle(Breco->get4Vector(), Breco->getPDGCode());
    Breco2->updateMomentum(Breco->get4Vector(), Breco->getVertex(), PerrMatrix, Breco->getPValue());

    analysis::RaveVertexFitter rsf;
    rsf.addTrack(Breco2);

    int nvert = rsf.fit("kalman");

    TVector3 pos;
    TMatrixDSym RerrMatrix(3);

    delete Breco2;


    if (nvert > 0) {
      pos = rsf.getPos(0);
      RerrMatrix = rsf.getCov(0);
    } else {return false;}


    // simpler version of momentum

    TVector3 pFinal = Breco->getVertex() - pos;
    TMatrixDSym errFinal = TMatrixDSym(Breco->getVertexErrorMatrix()) + RerrMatrix;

    // end simpler version

    // TODO : to be deveoped the extraction of the momentum from the rave fitted track

    // start rotation of the error matrix
    TLorentzVector v4Final(pFinal, Breco->getPDGMass());
    TLorentzVector vec = PCmsLabTransform::labToCms(v4Final);

    TMatrixD TubeZ = rotateTensorInv(v4Final.Vect(), errFinal);

    TubeZ(2, 2) = cut;
    TubeZ(2, 0) = 0; TubeZ(0, 2) = 0;
    TubeZ(2, 1) = 0; TubeZ(1, 2) = 0;

    vec.SetVect(-vec.Vect());

    TLorentzVector vecLab = PCmsLabTransform::cmsToLab(vec);
    TMatrixD Tube = rotateTensor(vecLab.Vect(), TubeZ);

    m_constraintCov.ResizeTo(3, 3);
    m_constraintCov = toSymMatrix(Tube);
    m_constraintCenter = m_BeamSpotCenter; // Standard algorithm needs no shift

    return true;

  }

  bool TagVertexModule::findConstraintBTube(const Particle* Breco, double cut)
  {
    //Use Breco as the creator of the B tube.

    bool ok0 = true;

    if ((Breco->getVertexErrorMatrix()(2, 2)) == 0.0) {
      B2WARNING("In TagVertexModule::findConstraintBTube: cannot get a proper vertex for BReco. BTube constraint replaced by Boost.");
      ok0 = findConstraintBoost(cut);
      return ok0;
    }

    //make a copy of tubecreatorB so as not to modify the original object

    Particle tubecreatorBCopy(Particle(Breco->get4Vector(), Breco->getPDGCode()));
    tubecreatorBCopy.updateMomentum(Breco->get4Vector(), Breco->getVertex(), Breco->getMomentumVertexErrorMatrix(),
                                    Breco->getPValue());

    //vertex fit will give the intersection between the beam spot and the trajectory of the B
    //(base of the BTube, or primary vtx cov matrix)

    ok0 = doVertexFitForBTube(&tubecreatorBCopy);

    if (!ok0) return false;

    //get direction of B tag = opposite direction of B rec in CMF

    TLorentzVector v4Final = tubecreatorBCopy.get4Vector();

    //if we want the true info, replace the 4vector by the true one

    if (m_useTruthInFit) {
      const MCParticle* mcBr = Breco->getRelated<MCParticle>();
      if (!mcBr)
        m_fitTruthStatus = 2;
      if (mcBr)
        v4Final = mcBr->get4Vector();
    }

    TLorentzVector vec = PCmsLabTransform::labToCms(v4Final);
    TLorentzVector vecNew(-vec.Vect(), vec.E());
    TLorentzVector v4FinalNew = PCmsLabTransform::cmsToLab(vecNew);

    //To creat the B tube, strategy is: take the primary vtx cov matrix, and add to it a cov
    //matrix corresponding to an very big error in the direction of the B tag

    TMatrixDSym pv = tubecreatorBCopy.getVertexErrorMatrix();

    //print some stuff if wanted
    if (m_verbose) {
      B2DEBUG(10, "Brec decay vertex before fit: " << printVector(Breco->getVertex()));
      B2DEBUG(10, "Brec decay vertex after fit: " << printVector(tubecreatorBCopy.getVertex()));
      B2DEBUG(10, "Brec direction before fit: " << printVector((1. / Breco->getP()) * Breco->getMomentum()));
      B2DEBUG(10, "Brec direction after fit: " << printVector((1. / tubecreatorBCopy.getP()) * tubecreatorBCopy.getMomentum()));
      B2DEBUG(10, "IP position: " << printVector(m_BeamSpotCenter));
      B2DEBUG(10, "IP covariance: " << printMatrix(m_BeamSpotCov));
      B2DEBUG(10, "Brec primary vertex: " << printVector(tubecreatorBCopy.getVertex()));
      B2DEBUG(10, "Brec PV covariance: " << printMatrix(pv));
      B2DEBUG(10, "BTag direction: " << printVector((1. / v4FinalNew.P())*v4FinalNew.Vect()));
      B2DEBUG(10, "BTag direction in CMF: " << printVector((1. / vecNew.P())*vecNew.Vect()));
      B2DEBUG(10, "Brec direction in CMF: " << printVector((1. / vec.P())*vec.Vect()));
    }


    //make a long error matrix along BTag direction
    TMatrixD longerror(3, 3); longerror(2, 2) = cut * cut;


    // make rotation matrix from z axis to BTag line of flight
    TMatrixD longerrorRotated = rotateTensor(v4FinalNew.Vect(), longerror);

    //pvNew will correspond to the covariance matrix of the B tube
    TMatrixD pvNew = TMatrixD(pv) + longerrorRotated;

    //set the constraint
    m_constraintCenter = tubecreatorBCopy.getVertex();

    //if we want the true info, set the centre of the constraint to the primary vertex

    if (m_useTruthInFit) {
      const MCParticle* mcBr = Breco->getRelated<MCParticle>();
      if (mcBr) {
        m_constraintCenter = mcBr->getProductionVertex();
      }
    }

    m_constraintCov.ResizeTo(3, 3);
    m_constraintCov = toSymMatrix(pvNew);

    if (m_verbose) {
      B2DEBUG(10, "IPTube covariance: " << printMatrix(m_constraintCov));
    }

    //The following is done to do the BTube constraint with a virtual track
    //(ie KFit way)

    m_tagMomentum = v4FinalNew;

    m_pvCov.ResizeTo(pv);
    m_pvCov = pv;

    return true;
  }


  bool TagVertexModule::findConstraintBoost(double cut, double shiftAlongBoost)
  {
    TVector3 boostDir = PCmsLabTransform().getBoostVector().Unit();

    TMatrixDSym beamSpotCov = m_beamSpotDB->getCovVertex();
    beamSpotCov(2, 2) = cut * cut; //cut on z-BeamSpot Cov

    TMatrixD Tube = rotateTensor(boostDir, beamSpotCov); //BeamSpot in CMS

    m_constraintCov.ResizeTo(3, 3);
    m_constraintCov = toSymMatrix(Tube);
    m_constraintCenter = m_BeamSpotCenter; // Standard algorithm needs no shift

    // The constraint used in the Single Track Fit needs to be shifted in the boost direction.

    if (shiftAlongBoost > -1000) {
      double boostAngle = atan2(boostDir[0] , boostDir[2]); // boost angle with respect from Z
      m_constraintCenter = m_BeamSpotCenter +
                           TVector3(shiftAlongBoost * sin(boostAngle), 0., shiftAlongBoost * cos(boostAngle)); // boost in the XZ plane
    }

    return true;


  }

  static double getProperLifeTime(const MCParticle* mc) //in ps
  {
    double beta = mc->getMomentum().Mag() / mc->getEnergy();
    return 1e3 * mc->getLifetime() * sqrt(1 - pow(beta, 2));
  }

  void TagVertexModule::BtagMCVertex(const Particle* Breco)
  {

    bool isBreco = false;
    int nReco = 0;

    TVector3 MCTagVert(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                       std::numeric_limits<float>::quiet_NaN());
    int mcPDG = 0;
    double mcTagLifeTime = -1;

    // Array of MC particles
    StoreArray<Belle2::MCParticle> mcParticles("");
    for (int i = 0; i < mcParticles.getEntries(); ++i) {
      const MCParticle* mc = mcParticles[i];
      if (abs(mc->getPDG()) != abs(Breco->getPDGCode()))
        continue;

      if (m_useMCassociation == "breco") {
        const MCParticle* mcBr = Breco->getRelated<MCParticle>();
        isBreco = (mcBr == mc) ? true : false;
      } else if (m_useMCassociation == "internal") {
        isBreco = compBrecoBgen(Breco, mc);
      }

      if (isBreco) {
        m_MCVertReco = mc->getDecayVertex();
        m_MCLifeTimeReco =  getProperLifeTime(mc);
        ++nReco;
      } else {
        MCTagVert = mc->getDecayVertex();
        mcTagLifeTime = getProperLifeTime(mc);
        mcPDG = mc->getPDG();
      }

    }


    if (nReco == 2) {
      double dref = 1000;
      for (int i = 0; i < mcParticles.getEntries(); ++i) {
        const MCParticle* mc = mcParticles[i];
        if (abs(mc->getPDG()) != abs(Breco->getPDGCode()))
          continue;
        //TODO is it correct?
        double dcalc = (mc->getDecayVertex() - Breco->getVertex()).Mag();
        m_MCVertReco = mc->getDecayVertex();
        m_MCLifeTimeReco  = getProperLifeTime(mc);
        if (dcalc < dref) {
          dref = dcalc;
          MCTagVert = mc->getDecayVertex();
          mcTagLifeTime = getProperLifeTime(mc);
          mcPDG = mc->getPDG();
        }
      }
    }

    m_MCtagV = MCTagVert;
    m_MCtagLifeTime = mcTagLifeTime;
    m_mcPDG = mcPDG;
  }


  // static
  bool TagVertexModule::compBrecoBgen(const Particle* Breco, const MCParticle* Bgen)
  {

    bool isDecMode = true;

    const std::vector<Belle2::Particle*> recDau = Breco->getDaughters();
    const std::vector<Belle2::MCParticle*> genDau = Bgen->getDaughters();

    if (recDau.size() > 0 && genDau.size() > 0) {
      for (auto dauRec : recDau) {
        bool isDau = false;
        for (auto dauGen : genDau) {
          if (dauGen->getPDG() == dauRec->getPDGCode())
            isDau = compBrecoBgen(dauRec, dauGen) ;
        }
        if (!isDau) isDecMode = false;
      }
    } else {
      if (recDau.size() == 0) { //&& genDau.size()==0){
        if (Bgen->getPDG() != Breco->getPDGCode()) isDecMode = false;;
      } else {isDecMode = false;}
    }

    return isDecMode;
  }

  // STANDARD FIT ALGORITHM
  /* This algorithm basically takes all the tracks coming from the Rest Of Events and send them to perform a multi-track fit
   The option of requestion PXD hits for the tracks can be chosen by the user.
   */
  std::vector<const Particle*> TagVertexModule::getTagTracks_standardAlgorithm(const Particle* Breco, int reqPXDHits) const
  {
    std::vector<const Particle*> fitParticles;
    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();
    if (!roe) return fitParticles;
    std::vector<const Particle*> ROEParticles = roe->getChargedParticles(m_roeMaskName, Const::pion.getPDGCode(), false);
    if (ROEParticles.size() == 0) return fitParticles;

    for (auto& ROEParticle : ROEParticles) {
      HitPatternVXD roeTrackPattern = ROEParticle->getTrackFitResult()->getHitPatternVXD();

      if (roeTrackPattern.getNPXDHits() >= reqPXDHits) {
        fitParticles.push_back(ROEParticle);
      }
    }
    //if (fitParticles.size() == 0) return false;
    //m_tagParticles = fitParticles;
    return fitParticles;
    //return true;
  }

  vector<ParticleAndWeight> TagVertexModule::getParticlesWithoutKS(const vector<const Particle*>& tagParticles,
      double massWindowWidth) const
  {
    vector<ParticleAndWeight> particleAndWeights;

    ParticleAndWeight particleAndWeight;
    particleAndWeight.mcParticle = 0;
    particleAndWeight.weight = -1111.;


    // remove tracks from KS
    for (unsigned i = 0; i < tagParticles.size(); ++i) {
      const Particle* particle1 = tagParticles.at(i);
      if (!particle1) continue;
      TLorentzVector mom1 = particle1->get4Vector();
      if (!isfinite(mom1.Mag2())) continue;

      //is from Ks decay?
      bool isKsDau = false;
      for (unsigned j = 0; j < tagParticles.size(); ++j) {
        if (i == j) continue;
        const Particle* particle2 = tagParticles.at(j);
        if (!particle2) continue;
        TLorentzVector mom2 = particle2->get4Vector();
        if (!isfinite(mom2.Mag2())) continue;
        double mass = (mom1 + mom2).M();
        if (abs(mass - Const::K0Mass) < massWindowWidth) {
          isKsDau = true;
          break;
        }
      }
      //if from Ks decay, skip
      if (isKsDau) continue;

      particleAndWeight.particle = particle1;

      if (m_useMCassociation == "breco" || m_useMCassociation == "internal")
        particleAndWeight.mcParticle = particle1->getRelatedTo<MCParticle>();

      particleAndWeights.push_back(particleAndWeight);

    }

    return particleAndWeights;
  }

  bool TagVertexModule::makeGeneralFit()
  {
    if (m_fitAlgo == "Rave") return makeGeneralFitRave();
    if (m_fitAlgo == "KFit") return makeGeneralFitKFit();

    return false;
  }

  bool TagVertexModule::makeGeneralFitRave()
  {
    // apply constraint
    analysis::RaveSetup::getInstance()->unsetBeamSpot();
    if (m_constraintType != "noConstraint") analysis::RaveSetup::getInstance()->setBeamSpot(m_constraintCenter, m_constraintCov);
    analysis::RaveVertexFitter rFit;

    //feed rave with tracks without Kshorts
    vector<ParticleAndWeight> particleAndWeights = getParticlesWithoutKS(m_tagParticles);

    for (const auto& pw : particleAndWeights) {
      try {
        if (m_useTruthInFit) {
          if (pw.mcParticle) {
            TrackFitResult tfr(getTrackWithTrueCoordinates(pw));
            rFit.addTrack(&tfr);
          } else
            m_fitTruthStatus = 2;
        } else {
          rFit.addTrack(pw.particle->getTrackFitResult());
        }

      } catch (const rave::CheckedFloatException&) {
        B2ERROR("Exception caught in TagVertexModule::makeGeneralFitRave(): Invalid inputs (nan/inf)?");
      }
    }

    //perform fit

    int isGoodFit = -1;
    try {
      isGoodFit = rFit.fit("avf");
    } catch (const rave::CheckedFloatException&) {
      B2ERROR("Exception caught in TagVertexModule::makeGeneralFitRave(): Invalid inputs (nan/inf)?");
      return false;
    }
    // if problems
    if (isGoodFit < 1) return false;

    //save the track info for later use
    //Tracks are sorted from highest rave weight to lowest

    unsigned n = particleAndWeights.size();
    for (unsigned i = 0; i < n; ++i)
      particleAndWeights.at(i).weight = rFit.getWeight(i);

    // sort by weight
    sort(particleAndWeights.begin(), particleAndWeights.end(),
    [](ParticleAndWeight & a, ParticleAndWeight & b) { return a.weight > b.weight; });

    m_raveParticles.resize(n);
    m_raveWeights.resize(n);
    m_raveMCParticles.resize(n);

    for (unsigned i = 0; i < n; ++i) {
      m_raveParticles.at(i) = particleAndWeights.at(i).particle;
      m_raveMCParticles.at(i) = particleAndWeights.at(i).mcParticle;
      m_raveWeights.at(i) = particleAndWeights.at(i).weight;
    }

    //if the fit is good, save the infos related to the vertex

    if (m_constraintType != "noConstraint") {
      TMatrixDSym tubeInv = m_constraintCov;
      tubeInv.Invert();
      TVector3 dTagV = rFit.getPos(0) - m_BeamSpotCenter;
      TVectorD dV(0, 2,
                  dTagV.X(),
                  dTagV.Y(),
                  dTagV.Z(),
                  "END");
      m_tagVChi2IP = tubeInv.Similarity(dV);
    }

    m_tagV = rFit.getPos(0);
    m_tagVErrMatrix.ResizeTo(rFit.getCov(0));
    m_tagVErrMatrix = rFit.getCov(0);
    m_tagVNDF = rFit.getNdf(0);
    m_tagVChi2 = rFit.getChi2(0);

    m_fitPval = rFit.getPValue();

    return true;
  }

  bool TagVertexModule::makeGeneralFitKFit()
  {
    //initialize KFit

    analysis::VertexFitKFit kFit;
    kFit.setMagneticField(m_Bfield);

    // apply constraint

    if (m_constraintType != "noConstraint" && m_constraintType != "tube")
      kFit.setIpProfile(ROOTToCLHEP::getPoint3D(m_constraintCenter), ROOTToCLHEP::getHepSymMatrix(m_constraintCov));

    if (m_constraintType == "tube") {
      CLHEP::HepSymMatrix err(7, 0);

      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
          err[i + 4][j + 4] = m_pvCov(i, j);
        }
      }

      kFit.setIpTubeProfile(
        ROOTToCLHEP::getHepLorentzVector(m_tagMomentum),
        ROOTToCLHEP::getPoint3D(m_constraintCenter),
        err,
        0.);
    }

    //feed KFit with tracks without Kshorts
    vector<ParticleAndWeight> particleAndWeights = getParticlesWithoutKS(m_tagParticles);

    int nTracksAdded = 0;
    for (auto& pawi : particleAndWeights) {
      int addedOK = 1;

      if (!m_useTruthInFit)
        addedOK = kFit.addParticle(pawi.particle);

      if (m_useTruthInFit && !pawi.mcParticle) {
        addedOK = 1;
        m_fitTruthStatus = 2;
      }

      if (m_useTruthInFit && pawi.mcParticle) {
        addedOK = kFit.addTrack(
                    ROOTToCLHEP::getHepLorentzVector(pawi.mcParticle->get4Vector()),
                    ROOTToCLHEP::getPoint3D(getTruePoca(pawi)),
                    ROOTToCLHEP::getHepSymMatrix(pawi.particle->getMomentumVertexErrorMatrix()),
                    pawi.particle->getCharge());
      }

      if (addedOK != 0) {
        B2WARNING("TagVertexModule::makeGeneralFitKFit: failed to add a track");
        pawi.weight = 0.;
      }

      if (addedOK == 0) {
        ++nTracksAdded;
        pawi.weight = 1.;
      }
    }

    //perform fit if there are enough tracks

    if ((nTracksAdded < 2 && m_constraintType == "noConstraint") || nTracksAdded < 1)
      return false;

    int isGoodFit = -1;

    isGoodFit = kFit.doFit();

    //save the track info for later use
    //Tracks are sorted by weight, ie pushing the tracks with 0 weight (from KS) to the end of the list

    unsigned n = particleAndWeights.size();
    sort(particleAndWeights.begin(), particleAndWeights.end(),
    [](ParticleAndWeight & a, ParticleAndWeight & b) { return a.weight > b.weight; });

    m_raveParticles.resize(n);
    m_raveWeights.resize(n);
    m_raveMCParticles.resize(n);

    for (unsigned i = 0; i < n; ++i) {
      m_raveParticles.at(i) = particleAndWeights.at(i).particle;
      m_raveMCParticles.at(i) = particleAndWeights.at(i).mcParticle;
      m_raveWeights.at(i) = particleAndWeights.at(i).weight;
    }

    //if the fit is good, save the infos related to the vertex

    if (isGoodFit != 0) return false;

    m_tagV = CLHEPToROOT::getTVector3(kFit.getVertex());

    if (m_constraintType != "noConstraint") {
      TMatrixDSym tubeInv = m_constraintCov;
      tubeInv.Invert();
      TVector3 dTagV = m_tagV - m_BeamSpotCenter;
      TVectorD dV(0, 2,
                  dTagV.X(),
                  dTagV.Y(),
                  dTagV.Z(),
                  "END");
      m_tagVChi2IP = tubeInv.Similarity(dV);
    }

    TMatrixDSym errMat(CLHEPToROOT::getTMatrixDSym(kFit.getVertexError()));

    m_tagVErrMatrix.ResizeTo(errMat);
    m_tagVErrMatrix = errMat;
    m_tagVNDF = kFit.getNDF();
    m_tagVChi2 = kFit.getCHIsq();

    m_fitPval = TMath::Prob(m_tagVChi2, m_tagVNDF);

    return true;
  }


  void TagVertexModule::deltaT(const Particle* Breco)
  {

    TVector3 boost = PCmsLabTransform().getBoostVector();
    TVector3 boostDir = boost.Unit();
    double bg = boost.Mag() / sqrt(1 - boost.Mag2());
    double c = Const::speedOfLight / 1000.; // cm ps-1



    //Reconstructed DeltaL & DeltaT in the boost direction
    TVector3 dVert = Breco->getVertex() - m_tagV; //reconstructed vtxReco - vtxTag
    double dl = dVert.Dot(boostDir);
    m_deltaT  = dl / (bg * c);

    //Truth DeltaL & approx DeltaT in the boost direction
    TVector3 MCdVert = m_MCVertReco - m_MCtagV;   //truth vtxReco - vtxTag
    double MCdl = MCdVert.Dot(boostDir);
    m_MCdeltaTapprox = MCdl / (bg * c);


    // MCdeltaT=tauRec-tauTag
    m_MCdeltaT = m_MCLifeTimeReco - m_MCtagLifeTime;
    if (m_MCLifeTimeReco  == -1 || m_MCtagLifeTime == -1)
      m_MCdeltaT =  std::numeric_limits<double>::quiet_NaN();


    // Calculate Delta t error
    TMatrixD RotErr = rotateTensor(boostDir, m_tagVErrMatrix);
    m_tagVlErr = sqrt(RotErr(2, 2));

    TMatrixD RR = (TMatrixD)Breco->getVertexErrorMatrix();
    TMatrixD RotErrBreco = rotateTensor(boostDir, RR);
    m_deltaTErr = sqrt(RotErr(2, 2) + RotErrBreco(2, 2)) / (bg * c);

    m_tagVl = m_tagV.Dot(boostDir);
    m_truthTagVl = m_MCtagV.Dot(boostDir);


    // calculate tagV component and error in the direction orthogonal to the boost
    TVector3 oboost(boostDir.Z(), boostDir.Y(), -boostDir.X());
    TMatrixD oRotErr = rotateTensor(oboost, m_tagVErrMatrix);
    m_tagVolErr = sqrt(oRotErr(2, 2));

    m_tagVol = m_tagV.Dot(oboost);
    m_truthTagVol = m_MCtagV.Dot(oboost);

  }

  bool TagVertexModule::doVertexFitForBTube(const Particle* mother) const
  {
    //Here rave is used to find the upsilon(4S) vtx as the intersection
    //between the mother B trajectory and the beam spot

    analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_BeamSpotCov);

    analysis::RaveVertexFitter rsg;
    rsg.addTrack(mother);
    int nvert = rsg.fit("avf");
    if (nvert != 1) return false;

    rsg.updateDaughters();
    return true;
  }

  TrackFitResult TagVertexModule::getTrackWithTrueCoordinates(ParticleAndWeight const& paw) const
  {
    if (!paw.mcParticle) {
      B2ERROR("In TagVertexModule::getTrackWithTrueCoordinate: no MC particle set");
      return TrackFitResult();
    }

    const TrackFitResult* tfr(paw.particle->getTrackFitResult());

    return TrackFitResult(getTruePoca(paw),
                          paw.mcParticle->getMomentum(),
                          tfr->getCovariance6(),
                          tfr->getChargeSign(),
                          tfr->getParticleType(),
                          tfr->getPValue(),
                          m_Bfield, 0, 0);
  }

  // static
  TVector3 TagVertexModule::getTruePoca(ParticleAndWeight const& paw)
  {
    if (!paw.mcParticle) {
      B2ERROR("In TagVertexModule::getTruePoca: no MC particle set");
      return TVector3(0., 0., 0.);
    }

    return DistanceTools::poca(paw.mcParticle->getProductionVertex(),
                               paw.mcParticle->getMomentum(),
                               paw.particle->getTrackFitResult()->getPosition());

  }

  void TagVertexModule::resetReturnParams()
  {
    m_raveParticles.resize(0);
    m_raveMCParticles.resize(0);
    m_tagParticles.resize(0);
    m_raveWeights.resize(0);

    double quietNaN(std::numeric_limits<double>::quiet_NaN());

    TMatrixDSym nanMatrix(3);
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) nanMatrix(i, j) = quietNaN;

    m_fitPval = quietNaN;
    m_tagV = TVector3(quietNaN, quietNaN, quietNaN);
    m_tagVErrMatrix.ResizeTo(nanMatrix);
    m_tagVErrMatrix = nanMatrix;
    m_MCtagV = TVector3(quietNaN, quietNaN, quietNaN);
    m_MCVertReco = TVector3(quietNaN, quietNaN, quietNaN);
    m_deltaT = quietNaN;
    m_deltaTErr = quietNaN;
    m_MCdeltaT = quietNaN;
    m_constraintCov.ResizeTo(nanMatrix);
    m_constraintCov = nanMatrix;
    m_constraintCenter = TVector3(quietNaN, quietNaN, quietNaN);
    m_tagVl = quietNaN;
    m_truthTagVl = quietNaN;
    m_tagVlErr = quietNaN;
    m_tagVol = quietNaN;
    m_truthTagVol = quietNaN;
    m_tagVolErr = quietNaN;
    m_tagVNDF = quietNaN;
    m_tagVChi2 = quietNaN;
    m_tagVChi2IP = quietNaN;
    m_pvCov.ResizeTo(nanMatrix);
    m_pvCov = nanMatrix;
    m_tagMomentum = TLorentzVector(quietNaN, quietNaN, quietNaN, quietNaN);


  }

  //The following functions are just here to help printing stuff

  // static
  std::string TagVertexModule::printVector(const TVector3& vec)
  {
    std::ostringstream oss;
    int w = 14;
    oss << "(" << std::setw(w) << vec[0] << ", " << std::setw(w) << vec[1] << ", " << std::setw(w) << vec[2] << ")" << std::endl;
    return oss.str();
  }

  // static
  std::string TagVertexModule::printMatrix(const TMatrixD& mat)
  {
    std::ostringstream oss;
    int w = 14;
    for (int i = 0; i < mat.GetNrows(); ++i) {
      for (int j = 0; j < mat.GetNcols(); ++j) {
        oss << std::setw(w) << mat(i, j) << " ";
      }
      oss << endl;
    }
    return oss.str();
  }


  // static
  std::string TagVertexModule::printMatrix(const TMatrixDSym& mat)
  {
    std::ostringstream oss;
    int w = 14;
    for (int i = 0; i < mat.GetNrows(); ++i) {
      for (int j = 0; j < mat.GetNcols(); ++j) {
        oss << std::setw(w) << mat(i, j) << " ";
      }
      oss << endl;
    }
    return oss.str();
  }



} // end Belle2 namespace
