
#include <analysis/variables/Variables.h>
#include <analysis/variables/BasicParticleInformation.h>
#include <analysis/variables/VertexVariables.h>
#include <analysis/variables/PIDVariables.h>
#include <analysis/variables/TrackVariables.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <gtest/gtest.h>

#include <TMatrixFSym.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  /** test kinematic Variable. */
  TEST(KinematicVariableTest, Variable)
  {

    // Connect gearbox for CMS variables

    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    {
      Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);

      TMatrixFSym error(7);
      error.Zero();
      error(0, 0) = 0.05;
      error(1, 1) = 0.2;
      error(2, 2) = 0.4;
      error(0, 1) = -0.1;
      error(0, 2) = 0.9;
      p.setMomentumVertexErrorMatrix(error);

      EXPECT_FLOAT_EQ(0.9, particleP(&p));
      EXPECT_FLOAT_EQ(1.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.1, particlePx(&p));
      EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
      EXPECT_FLOAT_EQ(0.8, particlePz(&p));
      EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
      EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

      EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
      EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
      EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
      EXPECT_FLOAT_EQ(sqrt(0.4), particlePzErr(&p));
      EXPECT_FLOAT_EQ(0.488093530, particlePtErr(&p));
      EXPECT_FLOAT_EQ(0.156402664, particleCosThetaErr(&p));
      EXPECT_FLOAT_EQ(0.263066820, particlePhiErr(&p));


      {
        UseReferenceFrame<CMSFrame> dummy;
        EXPECT_FLOAT_EQ(0.68176979, particleP(&p));
        EXPECT_FLOAT_EQ(0.80920333, particleE(&p));
        EXPECT_FLOAT_EQ(0.061728548, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.40000001, particlePy(&p));
        EXPECT_FLOAT_EQ(0.54863429, particlePz(&p));
        EXPECT_FLOAT_EQ(0.404735, particlePt(&p));
        EXPECT_FLOAT_EQ(0.80472076, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.4176828, particlePhi(&p));

        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
      }

      {
        UseReferenceFrame<RestFrame> dummy(&p);
        EXPECT_ALL_NEAR(particleP(&p), 0.0, 1e-9);
        EXPECT_FLOAT_EQ(0.4358899, particleE(&p));
        EXPECT_ALL_NEAR(0.0, particlePx(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePy(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePz(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePt(&p), 1e-9);

      }

      {
        UseReferenceFrame<LabFrame> dummy;
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

        EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.4), particlePzErr(&p));
        EXPECT_FLOAT_EQ(0.488093530, particlePtErr(&p));
        EXPECT_FLOAT_EQ(0.156402664, particleCosThetaErr(&p));
        EXPECT_FLOAT_EQ(0.263066820, particlePhiErr(&p));
      }

      {
        UseReferenceFrame<RotationFrame> dummy(TVector3(1, 0, 0), TVector3(0, 1, 0), TVector3(0, 0, 1));
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

        EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.4), particlePzErr(&p));
        EXPECT_FLOAT_EQ(0.488093530, particlePtErr(&p));
        EXPECT_FLOAT_EQ(0.156402664, particleCosThetaErr(&p));
        EXPECT_FLOAT_EQ(0.263066820, particlePhiErr(&p));

        const auto& frame = ReferenceFrame::GetCurrent();
        EXPECT_FLOAT_EQ(-0.1, frame.getMomentumErrorMatrix(&p)(0, 1));
        EXPECT_FLOAT_EQ(0.9, frame.getMomentumErrorMatrix(&p)(0, 2));
      }

      {
        UseReferenceFrame<RotationFrame> dummy(TVector3(1, 0, 0), TVector3(0, 0, -1), TVector3(0, 1, 0));
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.8, particlePy(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePz(&p));

        EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.4), particlePyErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.2), particlePzErr(&p));

        const auto& frame = ReferenceFrame::GetCurrent();
        EXPECT_FLOAT_EQ(-0.9, frame.getMomentumErrorMatrix(&p)(0, 1));
        EXPECT_FLOAT_EQ(-0.1, frame.getMomentumErrorMatrix(&p)(0, 2));
      }

      {
        UseReferenceFrame<CMSRotationFrame> dummy(TVector3(1, 0, 0), TVector3(0, 1, 0), TVector3(0, 0, 1));
        EXPECT_FLOAT_EQ(0.68176979, particleP(&p));
        EXPECT_FLOAT_EQ(0.80920333, particleE(&p));
        EXPECT_FLOAT_EQ(0.061728548, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.40000001, particlePy(&p));
        EXPECT_FLOAT_EQ(0.54863429, particlePz(&p));
        EXPECT_FLOAT_EQ(0.404735, particlePt(&p));
        EXPECT_FLOAT_EQ(0.80472076, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.4176828, particlePhi(&p));

        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
      }

      {
        Particle pinv({ -0.1 , 0.4, -0.8, 1.0 }, 11);
        UseReferenceFrame<RestFrame> dummy(&pinv);
        Particle p2({ 0.0 , 0.0, 0.0, 0.4358899}, 11);
        EXPECT_FLOAT_EQ(0.9, particleP(&p2));
        EXPECT_FLOAT_EQ(1.0, particleE(&p2));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p2));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p2));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p2));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p2));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p2));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p2));
      }
    }

    {
      Particle p({ 0.0 , 0.0, 0.0, 0.0 }, 11);
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));

      UseReferenceFrame<CMSFrame> dummy;
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));
    }

    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle> particles;
      particles.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      PCmsLabTransform T;
      TLorentzVector vec0 = {0.0, 0.0, 0.0, T.getCMSEnergy()};
      TLorentzVector vec1 = {0.0, +0.332174566, 0.0, T.getCMSEnergy() / 2.};
      TLorentzVector vec2 = {0.0, -0.332174566, 0.0, T.getCMSEnergy() / 2.};
      Particle* p0 = particles.appendNew(Particle(T.rotateCmsToLab() * vec0, 22));
      Particle* p1 = particles.appendNew(Particle(T.rotateCmsToLab() * vec1, 22, Particle::c_Unflavored, Particle::c_Undefined, 1));
      Particle* p2 = particles.appendNew(Particle(T.rotateCmsToLab() * vec2, 22, Particle::c_Unflavored, Particle::c_Undefined, 2));

      p0->appendDaughter(p1->getArrayIndex());
      p0->appendDaughter(p2->getArrayIndex());

      EXPECT_ALL_NEAR(m2RecoilSignalSide(p0), 0.0, 1e-7);
    }


  }


  TEST(VertexVariableTest, Variable)
  {

    // Connect gearbox for CMS variables

    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setPValue(0.5);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    EXPECT_FLOAT_EQ(1.0, particleDX(&p));
    EXPECT_FLOAT_EQ(2.0, particleDY(&p));
    EXPECT_FLOAT_EQ(2.0, particleDZ(&p));
    EXPECT_FLOAT_EQ(std::sqrt(5.0), particleDRho(&p));
    EXPECT_FLOAT_EQ(3.0, particleDistance(&p));
    EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));

    {
      UseReferenceFrame<CMSFrame> dummy;
      EXPECT_FLOAT_EQ(1.0382183, particleDX(&p));
      EXPECT_FLOAT_EQ(2.0, particleDY(&p));
      EXPECT_FLOAT_EQ(2.2510159, particleDZ(&p));
      EXPECT_FLOAT_EQ(std::sqrt(2.0 * 2.0 + 1.0382183 * 1.0382183), particleDRho(&p));
      EXPECT_FLOAT_EQ(3.185117, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }

    {
      Particle p2({ 0.1 , -0.4, 0.8, 1.0 }, 11);
      p2.setPValue(0.5);
      p2.setVertex(TVector3(1.0, 2.0, 2.0));

      UseReferenceFrame<RestFrame> dummy(&p2);
      EXPECT_FLOAT_EQ(0.0, particleDX(&p));
      EXPECT_FLOAT_EQ(0.0, particleDY(&p));
      EXPECT_FLOAT_EQ(0.0, particleDZ(&p));
      EXPECT_FLOAT_EQ(0.0, particleDRho(&p));
      EXPECT_FLOAT_EQ(0.0, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }

    /* Test with a distance between mother and daughter vertex. One
     * has to calculate the result by hand to test the code....

    {
      Particle p2({ 0.0 , 1.0, 0.0, 1.0 }, 11);
      p2.setPValue(0.5);
      p2.setVertex(TVector3(1.0, 0.0, 2.0));

      UseReferenceFrame<RestFrame> dummy(&p2);
      EXPECT_FLOAT_EQ(0.0, particleDX(&p));
      EXPECT_FLOAT_EQ(2.0, particleDY(&p));
      EXPECT_FLOAT_EQ(0.0, particleDZ(&p));
      EXPECT_FLOAT_EQ(2.0, particleDRho(&p));
      EXPECT_FLOAT_EQ(2.0, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }
         */

  }

  TEST(TrackVariablesTest, Variable)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult> myResults;
    StoreArray<Track> myTracks;
    StoreArray<Particle> myParticles;
    myResults.registerInDataStore();
    myTracks.registerInDataStore();
    myParticles.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    TRandom3 generator;

    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);

    // Generate a random put orthogonal pair of vectors in the r-phi plane
    TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());

    // Add a random z component
    TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
    TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    myResults.appendNew(position, momentum, cov6, charge, Const::electron, pValue, bField, CDCValue, 16777215);
    Track mytrack;
    mytrack.setTrackFitResultIndex(Const::electron, 0);
    Track* savedTrack = myTracks.appendNew(mytrack);

    Particle* part = myParticles.appendNew(savedTrack, Const::ChargedStable(11));

    const Manager::Var* vIsFromECL = Manager::Instance().getVariable("isFromECL");
    const Manager::Var* vIsFromKLM = Manager::Instance().getVariable("isFromKLM");
    const Manager::Var* vIsFromTrack = Manager::Instance().getVariable("isFromTrack");

    EXPECT_TRUE(vIsFromTrack->function(part));
    EXPECT_FALSE(vIsFromECL->function(part));
    EXPECT_FALSE(vIsFromKLM->function(part));
    EXPECT_FLOAT_EQ(0.5, trackPValue(part));
    EXPECT_FLOAT_EQ(position.Z(), trackZ0(part));
    EXPECT_FLOAT_EQ(sqrt(pow(position.X(), 2) + pow(position.Y(), 2)), trackD0(part));
    EXPECT_FLOAT_EQ(3, trackNCDCHits(part));
    EXPECT_FLOAT_EQ(24, trackNSVDHits(part));
    EXPECT_FLOAT_EQ(12, trackNPXDHits(part));

  }

  class MCTruthVariablesTest : public ::testing::Test {
  protected:
    virtual void SetUp()
    {
      // datastore things
      DataStore::Instance().reset();
      DataStore::Instance().setInitializeActive(true);

      // needed to mock up
      StoreArray<ECLCluster> clusters;
      StoreArray<MCParticle> mcparticles;
      StoreArray<Track> tracks;
      StoreArray<TrackFitResult> trackfits;
      StoreArray<Particle> particles;

      // register the arrays
      clusters.registerInDataStore();
      mcparticles.registerInDataStore();
      tracks.registerInDataStore();
      trackfits.registerInDataStore();
      particles.registerInDataStore();

      // register the relations for mock up mcmatching
      clusters.registerRelationTo(mcparticles);
      tracks.registerRelationTo(mcparticles);
      particles.registerRelationTo(mcparticles);

      // register the relation for mock up track <--> cluster matching
      //clusters.registerRelationTo(tracks);
      tracks.registerRelationTo(clusters);

      // end datastore things
      DataStore::Instance().setInitializeActive(false);

      /* mock up an electron (track with a cluster AND a track-cluster match)
       * and a photon (cluster, no track) and MCParticles for both
       *
       * this assumes that everything (tracking, clustering, track-cluster
       * matching *and* mcmatching all worked)
       *
       * this can be extended to pions, kaons, etc but leave it simple for now
       */

      // create the true underlying mcparticles
      auto* true_photon = mcparticles.appendNew(MCParticle());
      true_photon->setPDG(22);
      auto* true_electron = mcparticles.appendNew(MCParticle());
      true_electron->setPDG(11);
      auto* true_pion = mcparticles.appendNew(MCParticle());
      true_pion->setPDG(-211);

      // create the reco clusters
      auto* cl0 = clusters.appendNew(ECLCluster());
      cl0->setEnergy(1.0);
      cl0->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      cl0->setClusterId(0);

      auto* cl1 = clusters.appendNew(ECLCluster());
      cl1->setEnergy(0.5);
      cl1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      cl1->setClusterId(1);

      // create a reco track (one has to also mock up a track fit result)
      TMatrixDSym cov(6);
      trackfits.appendNew(
        TVector3(), TVector3(), cov, -1, Const::electron, 0.5, 1.5,
        static_cast<unsigned long long int>(0x300000000000000), 16777215);
      auto* electron_tr = tracks.appendNew(Track());
      electron_tr->setTrackFitResultIndex(Const::electron, 0);
      electron_tr->addRelationTo(cl1);  // a track <--> cluster match

      TMatrixDSym cov1(6);
      trackfits.appendNew(
        TVector3(), TVector3(), cov1, -1, Const::pion, 0.51, 1.5,
        static_cast<unsigned long long int>(0x300000000000000), 16777215);
      auto* pion_tr = tracks.appendNew(Track());
      pion_tr->setTrackFitResultIndex(Const::pion, 0);
      pion_tr->addRelationTo(cl1);  // a track <--> cluster match

      // now set mcmatch relations
      cl0->addRelationTo(true_photon,   12.3);
      cl0->addRelationTo(true_electron,  2.3);
      cl1->addRelationTo(true_electron, 45.6);
      cl1->addRelationTo(true_photon,    5.6);
      cl1->addRelationTo(true_pion,     15.6);

      electron_tr->addRelationTo(true_electron);
      pion_tr->addRelationTo(true_pion);

      // create belle2::Particles from the mdst objects
      const auto* photon = particles.appendNew(Particle(cl0));
      const auto* electron = particles.appendNew(Particle(electron_tr, Const::electron));
      const auto* pion = particles.appendNew(Particle(pion_tr, Const::pion));
      const auto* misid_photon = particles.appendNew(Particle(cl1));

      // now set mcmatch relations
      photon->addRelationTo(true_photon);
      electron->addRelationTo(true_electron);
      pion->addRelationTo(true_pion);
      misid_photon->addRelationTo(true_electron); // assume MC matching caught this
    }

    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(MCTruthVariablesTest, ECLMCMatchWeightVariable)
  {
    StoreArray<Particle> particles;
    const auto* photon = particles[0];
    const auto* electron = particles[1];
    const auto* pion = particles[2];

    const auto* weight = Manager::Instance().getVariable("clusterMCMatchWeight");
    EXPECT_FLOAT_EQ(weight->function(photon),   12.3);
    EXPECT_FLOAT_EQ(weight->function(electron), 45.6);
    EXPECT_FLOAT_EQ(weight->function(pion), 15.6);
  }

  TEST_F(MCTruthVariablesTest, ECLBestMCMatchVariables)
  {
    StoreArray<Particle> particles;
    const auto* photon = particles[0];
    const auto* electron = particles[1];
    const auto* pion = particles[2];
    const auto* misid_photon = particles[3];


    const auto* pdgcode = Manager::Instance().getVariable("clusterBestMCPDG");
    EXPECT_EQ(pdgcode->function(photon),       22);
    EXPECT_EQ(pdgcode->function(electron),     11);
    EXPECT_EQ(pdgcode->function(pion),     11);
    EXPECT_EQ(pdgcode->function(misid_photon), 11);

    const auto* weight = Manager::Instance().getVariable("clusterBestMCMatchWeight");
    EXPECT_FLOAT_EQ(weight->function(photon),       12.3);
    EXPECT_FLOAT_EQ(weight->function(electron),     45.6);
    EXPECT_FLOAT_EQ(weight->function(pion),     45.6);
    EXPECT_FLOAT_EQ(weight->function(misid_photon), 45.6);
  }

  class ROEVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {

      StoreObjPtr<ParticleList> pi0ParticleList("pi0:vartest");
      DataStore::Instance().setInitializeActive(true);
      pi0ParticleList.registerInDataStore(DataStore::c_DontWriteOut);
      StoreArray<ECLCluster> myECLClusters;
      StoreArray<KLMCluster> myKLMClusters;
      StoreArray<TrackFitResult> myTFRs;
      StoreArray<Track> myTracks;
      StoreArray<Particle> myParticles;
      StoreArray<RestOfEvent> myROEs;
      StoreArray<PIDLikelihood> myPIDLikelihoods;
      myECLClusters.registerInDataStore();
      myKLMClusters.registerInDataStore();
      myTFRs.registerInDataStore();
      myTracks.registerInDataStore();
      myParticles.registerInDataStore();
      myROEs.registerInDataStore();
      myPIDLikelihoods.registerInDataStore();
      myParticles.registerRelationTo(myROEs);
      myTracks.registerRelationTo(myPIDLikelihoods);
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };
//
// TODO: redo all ROE variable tests
//

  TEST_F(ROEVariablesTest, Variable)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);
    StoreObjPtr<ParticleList> pi0ParticleList("pi0:vartest");
    StoreArray<ECLCluster> myECLClusters;
    StoreArray<KLMCluster> myKLMClusters;
    StoreArray<TrackFitResult> myTFRs;
    StoreArray<Track> myTracks;
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    StoreArray<PIDLikelihood> myPIDLikelihoods;

    pi0ParticleList.create();
    pi0ParticleList->initialize(111, "pi0:vartest");

    // Neutral ECLCluster on reconstructed side
    ECLCluster myECL;
    myECL.setIsTrack(false);
    float eclREC = 0.5;
    myECL.setEnergy(eclREC);
    myECL.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    ECLCluster* savedECL = myECLClusters.appendNew(myECL);

    // Particle on reconstructed side from ECLCluster
    Particle p(savedECL);
    Particle* part = myParticles.appendNew(p);

    // Create ECLCluster on ROE side
    ECLCluster myROEECL;
    myROEECL.setIsTrack(false);
    float eclROE = 1.0;
    myROEECL.setEnergy(eclROE);
    myROEECL.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    ECLCluster* savedROEECL = myECLClusters.appendNew(myROEECL);
    Particle* roeECLParticle = myParticles.appendNew(savedROEECL);
    // Create KLMCluster on ROE side
    KLMCluster myROEKLM;
    KLMCluster* savedROEKLM = myKLMClusters.appendNew(myROEKLM);
    Particle* roeKLMParticle = myParticles.appendNew(savedROEKLM);

    // Create Track on ROE side
    // - create TFR

    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);

    TVector3 position(1.0, 0, 0);
    TVector3 momentum(0, 1.0, 0);

    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    myTFRs.appendNew(position, momentum, cov6, charge, Const::muon, pValue, bField, CDCValue, 16777215);

    // - create Track
    Track myROETrack;
    myROETrack.setTrackFitResultIndex(Const::muon, 0);
    Track* savedROETrack = myTracks.appendNew(myROETrack);
    // - create PID information, add relation
    PIDLikelihood myPID;
    myPID.setLogLikelihood(Const::TOP, Const::muon, 0.15);
    myPID.setLogLikelihood(Const::ARICH, Const::muon, 0.152);
    myPID.setLogLikelihood(Const::ECL, Const::muon, 0.154);
    myPID.setLogLikelihood(Const::CDC, Const::muon, 0.156);
    myPID.setLogLikelihood(Const::SVD, Const::muon, 0.158);
    myPID.setLogLikelihood(Const::TOP, Const::pion, 0.5);
    myPID.setLogLikelihood(Const::ARICH, Const::pion, 0.52);
    myPID.setLogLikelihood(Const::ECL, Const::pion, 0.54);
    myPID.setLogLikelihood(Const::CDC, Const::pion, 0.56);
    myPID.setLogLikelihood(Const::SVD, Const::pion, 0.58);
    PIDLikelihood* savedPID = myPIDLikelihoods.appendNew(myPID);

    savedROETrack->addRelationTo(savedPID);
    Particle* roeTrackParticle = myParticles.appendNew(savedROETrack, Const::muon);

    // Create ROE object, append tracks, clusters, add relation to particle
    //TODO: make particles
    RestOfEvent roe;
    vector<const Particle*> roeParticlesToAdd;
    roeParticlesToAdd.push_back(roeTrackParticle);
    roeParticlesToAdd.push_back(roeECLParticle);
    roeParticlesToAdd.push_back(roeKLMParticle);
    roe.addParticles(roeParticlesToAdd);
    RestOfEvent* savedROE = myROEs.appendNew(roe);
    /*
    std::map<std::string, std::map<unsigned int, bool>> tMasks;
    std::map<std::string, std::map<unsigned int, bool>> cMasks;
    std::map<std::string, std::vector<double>> fracs;

    std::map<unsigned int, bool> tMask1;
    std::map<unsigned int, bool> tMask2;
    tMask1[savedROETrack->getArrayIndex()] = true;
    tMask2[savedROETrack->getArrayIndex()] = false;

    std::map<unsigned int, bool> cMask1;
    std::map<unsigned int, bool> cMask2;
    cMask1[savedROEECL->getArrayIndex()] = true;
    cMask2[savedROEECL->getArrayIndex()] = false;

    std::vector<double> frac1 = {0, 0, 1, 0, 0, 0};
    std::vector<double> frac2 = {1, 1, 1, 1, 1, 1};

    tMasks["mask1"] = tMask1;
    tMasks["mask2"] = tMask2;

    cMasks["mask1"] = cMask1;
    cMasks["mask2"] = cMask2;

    fracs["mask1"] = frac1;
    fracs["mask2"] = frac2;

    savedROE->appendTrackMasks(tMasks);
    savedROE->appendECLClusterMasks(cMasks);
    savedROE->appendChargedStableFractionsSet(fracs);
    */
    savedROE->initializeMask("mask1", "test");
    std::shared_ptr<Variable::Cut> trackSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 2"));
    std::shared_ptr<Variable::Cut> eclSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 2"));
    savedROE->updateMaskWithCuts("mask1");
    savedROE->initializeMask("mask2", "test");
    savedROE->updateMaskWithCuts("mask2",  trackSelection,  eclSelection);
    part->addRelationTo(savedROE);

    // ROE variables
    PCmsLabTransform T;
    float E0 = T.getCMSEnergy() / 2;
    B2INFO("E0 is " << E0);
    //*/
    TLorentzVector pTrack_ROE_Lab(momentum, TMath::Sqrt(Const::muon.getMass()*Const::muon.getMass() + 1.0 /*momentum.Mag2()*/));
    pTrack_ROE_Lab = roeTrackParticle->get4Vector();
    TLorentzVector pECL_ROE_Lab(0, 0, eclROE, eclROE);
    TLorentzVector pECL_REC_Lab(0, 0, eclREC, eclREC);

    TLorentzVector rec4vec;
    rec4vec.SetE(pECL_REC_Lab.E());
    rec4vec.SetVect(pECL_REC_Lab.Vect());

    TLorentzVector roe4vec;
    roe4vec.SetE(pTrack_ROE_Lab.E() + pECL_ROE_Lab.E());
    roe4vec.SetVect(pTrack_ROE_Lab.Vect() + pECL_ROE_Lab.Vect());

    TLorentzVector rec4vecCMS = T.rotateLabToCms() * rec4vec;
    TLorentzVector roe4vecCMS = T.rotateLabToCms() * roe4vec;

    TVector3 pB = - roe4vecCMS.Vect();
    pB.SetMag(0.340);

    TLorentzVector m4v0;
    m4v0.SetE(2 * E0 - (rec4vecCMS.E() + roe4vecCMS.E()));
    m4v0.SetVect(- (rec4vecCMS.Vect() + roe4vecCMS.Vect()));

    TLorentzVector m4v1;
    m4v1.SetE(E0 - rec4vecCMS.E());
    m4v1.SetVect(- (rec4vecCMS.Vect() + roe4vecCMS.Vect()));

    TLorentzVector m4v2;
    m4v2.SetE(E0 - rec4vecCMS.E());
    m4v2.SetVect(- rec4vecCMS.Vect());

    TLorentzVector m4v3;
    m4v3.SetE(E0 - rec4vecCMS.E());
    m4v3.SetVect(pB - rec4vecCMS.Vect());

    TLorentzVector neutrino4vecCMS;
    neutrino4vecCMS.SetVect(- (roe4vecCMS.Vect() + rec4vecCMS.Vect()));
    neutrino4vecCMS.SetE(neutrino4vecCMS.Vect().Mag());

    TLorentzVector corrRec4vecCMS = rec4vecCMS + neutrino4vecCMS;
    B2INFO("roe4vecCMS.E() = " << roe4vecCMS.E());
    // TESTS FOR ROE STRUCTURE
    //EXPECT_B2FATAL(savedROE->getTrackMask("noSuchMask"));
    //EXPECT_B2FATAL(savedROE->getECLClusterMask("noSuchMask"));
    //double fArray[6];
    //EXPECT_B2FATAL(savedROE->fillFractions(fArray, "noSuchMask"));
    EXPECT_B2FATAL(savedROE->updateMaskWithCuts("noSuchMask"));
    EXPECT_B2FATAL(savedROE->updateMaskWithV0("noSuchMask", part));
    EXPECT_B2FATAL(savedROE->hasParticle(part, "noSuchMask"));

    // TESTS FOR ROE VARIABLES

    const Manager::Var* var = Manager::Instance().getVariable("nROE_Charged(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask1, 13)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask1, 211)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_Photons(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Photons(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_NeutralHadrons(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Tracks(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Tracks(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_ParticlesInList(pi0:vartest)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("roeCharge(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("roeCharge(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("roeEextra(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), savedROEECL->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons));

    var = Manager::Instance().getVariable("roeEextra(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("roeDeltae(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4vecCMS.E() - E0);

    var = Manager::Instance().getVariable("roeDeltae(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), -E0);

    var = Manager::Instance().getVariable("roeMbc(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - roe4vecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("roeMbc(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), E0);

    var = Manager::Instance().getVariable("weDeltae(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), corrRec4vecCMS.E() - E0);

    var = Manager::Instance().getVariable("weDeltae(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), rec4vecCMS.E() + rec4vecCMS.Vect().Mag() - E0);

    var = Manager::Instance().getVariable("weMbc(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - corrRec4vecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("weMbc(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), E0);

    var = Manager::Instance().getVariable("weMissM2(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), m4v0.Mag2());

    var = Manager::Instance().getVariable("weMissM2(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), (2 * E0 - rec4vecCMS.E()) * (2 * E0 - rec4vecCMS.E()) - rec4vecCMS.Vect().Mag2());

  }


  class EventVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(EventVariableTest, ExperimentRunEventDateAndTime)
  {
    const Manager::Var* exp = Manager::Instance().getVariable("expNum");
    const Manager::Var* run = Manager::Instance().getVariable("runNum");
    const Manager::Var* evt = Manager::Instance().getVariable("evtNum");
    const Manager::Var* date = Manager::Instance().getVariable("date");
    const Manager::Var* year = Manager::Instance().getVariable("year");
    const Manager::Var* time = Manager::Instance().getVariable("eventTimeSeconds");

    // there is no EventMetaData so expect nan
    EXPECT_FALSE(date->function(nullptr) == date->function(nullptr));
    EXPECT_FALSE(year->function(nullptr) == year->function(nullptr));
    EXPECT_FALSE(time->function(nullptr) == time->function(nullptr));

    DataStore::Instance().setInitializeActive(true);
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    evtMetaData.create();
    evtMetaData->setExperiment(1337);
    evtMetaData->setRun(12345);
    evtMetaData->setEvent(54321);
    evtMetaData->setTime(1288569600e9);
    // 01/11/2010 is the date TDR was uploaded to arXiv ... experiment's birthday?


    // -
    EXPECT_FLOAT_EQ(exp->function(NULL), 1337.);
    EXPECT_FLOAT_EQ(run->function(NULL), 12345.);
    EXPECT_FLOAT_EQ(evt->function(NULL), 54321.);
    EXPECT_FLOAT_EQ(date->function(NULL), 20101101.);
    EXPECT_FLOAT_EQ(year->function(NULL), 2010.);
    EXPECT_FLOAT_EQ(time->function(NULL), 1288569600);
  }

  TEST_F(EventVariableTest, TestGlobalCounters)
  {
    StoreArray<MCParticle> mcParticles; // empty
    const Manager::Var* var = Manager::Instance().getVariable("nMCParticles");
    EXPECT_FLOAT_EQ(var->function(NULL), 0.0);

    for (unsigned i = 0; i < 10; ++i)
      mcParticles.appendNew();

    EXPECT_FLOAT_EQ(var->function(NULL), 10.0);

    // TODO: add other counters nTracks etc in here
  }

  TEST_F(EventVariableTest, TestIfContinuumEvent_ForContinuumEvent)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p2->addRelationTo(mcParticle);

    const Manager::Var* var = Manager::Instance().getVariable("isContinuumEvent");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 1.0);
    EXPECT_FLOAT_EQ(var->function(p2), 1.0);
    const Manager::Var* varN = Manager::Instance().getVariable("isNotContinuumEvent");
    ASSERT_NE(varN, nullptr);
    EXPECT_FLOAT_EQ(varN->function(p1), 0.0);
    EXPECT_FLOAT_EQ(varN->function(p2), 0.0);
  }

  TEST_F(EventVariableTest, TestIfContinuumEvent_ForUpsilon4SEvent)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles2;
    StoreArray<Particle> particles2;
    particles2.registerRelationTo(mcParticles2);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles2.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p3 = particles2.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p3->addRelationTo(mcParticle);

    mcParticle = mcParticles2.appendNew();
    mcParticle->setPDG(300553);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p4 = particles2.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 300553);
    p4->addRelationTo(mcParticle);

    const Manager::Var* var2 = Manager::Instance().getVariable("isContinuumEvent");
    ASSERT_NE(var2, nullptr);
    EXPECT_FLOAT_EQ(var2->function(p3), 0.0);
    EXPECT_FLOAT_EQ(var2->function(p4), 0.0);
    const Manager::Var* var2N = Manager::Instance().getVariable("isNotContinuumEvent");
    ASSERT_NE(var2N, nullptr);
    EXPECT_FLOAT_EQ(var2N->function(p3), 1.0);
    EXPECT_FLOAT_EQ(var2N->function(p4), 1.0);
  }

  TEST_F(EventVariableTest, TestIfContinuumEvent_ForWrongReconstructedUpsilon4SEvent)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles3;
    StoreArray<Particle> particles3;
    particles3.registerRelationTo(mcParticles3);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles3.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p5 = particles3.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p5->addRelationTo(mcParticle);

    mcParticle = mcParticles3.appendNew();
    mcParticle->setPDG(300553);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p6 = particles3.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 15);
    p6->addRelationTo(mcParticle);

    const Manager::Var* var3 = Manager::Instance().getVariable("isContinuumEvent");
    ASSERT_NE(var3, nullptr);
    EXPECT_FLOAT_EQ(var3->function(p5), 0.0);
    EXPECT_FLOAT_EQ(var3->function(p6), 0.0);
    const Manager::Var* var3N = Manager::Instance().getVariable("isNotContinuumEvent");
    ASSERT_NE(var3N, nullptr);
    EXPECT_FLOAT_EQ(var3N->function(p5), 1.0);
    EXPECT_FLOAT_EQ(var3N->function(p6), 1.0);
  }


  class MetaVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap>().registerInDataStore();
      StoreObjPtr<EventExtraInfo>().registerInDataStore();
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(MetaVariableTest, countDaughters)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 0, 0, 3.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("countDaughters(charge > 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 3.0);

    var = Manager::Instance().getVariable("countDaughters(abs(charge) > 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 6.0);

  }

  TEST_F(MetaVariableTest, useRestFrame)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useRestFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.0, 1e-9);

    var = Manager::Instance().getVariable("useRestFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.4358899);

    var = Manager::Instance().getVariable("useRestFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.0);
  }

  TEST_F(MetaVariableTest, useLabFrame)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useLabFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("useLabFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("useLabFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);
  }

  TEST_F(MetaVariableTest, useCMSFrame)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useCMSFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.68176979);

    var = Manager::Instance().getVariable("useCMSFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.80920333);

    var = Manager::Instance().getVariable("useCMSFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.185117);
  }

  TEST_F(MetaVariableTest, useTagSideRecoilRestFrame)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    PCmsLabTransform T;
    TLorentzVector vec0 = {0.0, 0.0, 0.0, T.getCMSEnergy()};
    TLorentzVector vec1 = {0.0, +0.332174566, 0.0, T.getCMSEnergy() / 2.};
    TLorentzVector vec2 = {0.0, -0.332174566, 0.0, T.getCMSEnergy() / 2.};
    Particle* p0 = particles.appendNew(Particle(T.rotateCmsToLab() * vec0, 300553));
    Particle* p1 = particles.appendNew(Particle(T.rotateCmsToLab() * vec1, 511, Particle::c_Unflavored, Particle::c_Undefined, 1));
    Particle* p2 = particles.appendNew(Particle(T.rotateCmsToLab() * vec2, -511, Particle::c_Unflavored, Particle::c_Undefined, 2));

    p0->appendDaughter(p1->getArrayIndex());
    p0->appendDaughter(p2->getArrayIndex());

    const Manager::Var* var = Manager::Instance().getVariable("useTagSideRecoilRestFrame(daughter(1, p), 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_NEAR(var->function(p0), 0., 1e-6);

    var = Manager::Instance().getVariable("useTagSideRecoilRestFrame(daughter(1, px), 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_NEAR(var->function(p0), 0., 1e-6);

    var = Manager::Instance().getVariable("useTagSideRecoilRestFrame(daughter(1, py), 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_NEAR(var->function(p0), 0., 1e-6);

    var = Manager::Instance().getVariable("useTagSideRecoilRestFrame(daughter(1, pz), 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_NEAR(var->function(p0), 0., 1e-6);

    var = Manager::Instance().getVariable("useTagSideRecoilRestFrame(daughter(1, E), 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_NEAR(var->function(p0), p1->getMass(), 1e-6);
  }


  TEST_F(MetaVariableTest, extraInfo)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.addExtraInfo("pi", 3.14);

    const Manager::Var* var = Manager::Instance().getVariable("extraInfo(pi)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.14);

    // If nullptr is given, -999. is returned
    EXPECT_TRUE(std::isnan(var->function(nullptr)));
  }

  TEST_F(MetaVariableTest, eventExtraInfo)
  {
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    if (not eventExtraInfo.isValid())
      eventExtraInfo.create();
    eventExtraInfo->addExtraInfo("pi", 3.14);
    const Manager::Var* var = Manager::Instance().getVariable("eventExtraInfo(pi)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 3.14);
  }

  TEST_F(MetaVariableTest, eventCached)
  {
    const Manager::Var* var = Manager::Instance().getVariable("eventCached(constant(3.14))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 3.14);
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    EXPECT_TRUE(eventExtraInfo.isValid());
    EXPECT_TRUE(eventExtraInfo->hasExtraInfo("__constant__bo3__pt14__bc"));
    EXPECT_FLOAT_EQ(eventExtraInfo->getExtraInfo("__constant__bo3__pt14__bc"), 3.14);
    eventExtraInfo->addExtraInfo("__eventExtraInfo__bopi__bc", 3.14);
    var = Manager::Instance().getVariable("eventCached(eventExtraInfo(pi))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 3.14);
  }

  TEST_F(MetaVariableTest, particleCached)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    const Manager::Var* var = Manager::Instance().getVariable("particleCached(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.1);
    EXPECT_TRUE(p.hasExtraInfo("__px"));
    EXPECT_FLOAT_EQ(p.getExtraInfo("__px"), 0.1);
    p.addExtraInfo("__py", -0.5); // NOT -0.4 because we want to see if the cache is used instead of py!
    var = Manager::Instance().getVariable("particleCached(py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.5);
  }

  TEST_F(MetaVariableTest, basicMathTest)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("abs(py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.4);

    var = Manager::Instance().getVariable("min(E, pz)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.8);

    var = Manager::Instance().getVariable("max(E, pz)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 2.0);

    var = Manager::Instance().getVariable("log10(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -1.0);
  }

  TEST_F(MetaVariableTest, formula)
  {
    // see also unit tests in framework/formula_parser.cc
    //
    // keep particle-based tests here, and operator precidence tests (etc) in
    // framework with the parser itself

    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("formula(px + py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.3);

    var = Manager::Instance().getVariable("formula(px - py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.5);

    var = Manager::Instance().getVariable("formula(px * py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.04);

    var = Manager::Instance().getVariable("formula(py / px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -4.0);

    var = Manager::Instance().getVariable("formula(px ^ E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.01);

    var = Manager::Instance().getVariable("formula(px * py + pz)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.76, 1e-6);

    var = Manager::Instance().getVariable("formula(pz + px * py)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.76, 1e-6);

    var = Manager::Instance().getVariable("formula(pt)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.41231057);
    double pt = var->function(&p);

    var = Manager::Instance().getVariable("formula((px**2 + py**2)**(1/2))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), pt);

    var = Manager::Instance().getVariable("formula(charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -1.0);

    var = Manager::Instance().getVariable("formula(charge**2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("formula(charge^2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("formula(PDG * charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -11.0);

    var = Manager::Instance().getVariable("formula(PDG**2 * charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -121.0);

    var = Manager::Instance().getVariable("formula(10.58 - (px + py + pz - E)**2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 8.33);

    var = Manager::Instance().getVariable("formula(-10.58 + (px + py + pz - E)**2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -8.33);

    var = Manager::Instance().getVariable("formula(-1.0 * PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -11);
  }

  TEST_F(MetaVariableTest, passesCut)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("passesCut(E < 3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1);
    EXPECT_FLOAT_EQ(var->function(&p2), 0);
    EXPECT_TRUE(std::isnan(var->function(nullptr)));

  }

  TEST_F(MetaVariableTest, conditionalVariableSelector)
  {
    Particle p({ 0.1, -0.4, 0.8, 2.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("conditionalVariableSelector(E>1, px, py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.1);

    var = Manager::Instance().getVariable("conditionalVariableSelector(E<1, px, py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.4);

  }

  TEST_F(MetaVariableTest, nCleanedTracks)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult> track_fit_results;
    StoreArray<Track> tracks;
    track_fit_results.registerInDataStore();
    tracks.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.1, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);
    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.15, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);
    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.4, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);
    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.6, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);

    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 0);
    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 1);
    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 2);
    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 3);

    const Manager::Var* var1 = Manager::Instance().getVariable("nCleanedTracks(p > 0.5)");
    EXPECT_FLOAT_EQ(var1->function(nullptr), 1);

    const Manager::Var* var2 = Manager::Instance().getVariable("nCleanedTracks(p > 0.2)");
    EXPECT_FLOAT_EQ(var2->function(nullptr), 2);

    const Manager::Var* var3 = Manager::Instance().getVariable("nCleanedTracks()");
    EXPECT_FLOAT_EQ(var3->function(nullptr), 4);


  }

  TEST_F(MetaVariableTest, NumberOfMCParticlesInEvent)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    StoreArray<MCParticle> mcParticles;
    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);


    const Manager::Var* var = Manager::Instance().getVariable("NumberOfMCParticlesInEvent(11)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 2);

  }

  TEST_F(MetaVariableTest, daughterInvM)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(2, 2, 2, 4.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterInvM(6,5)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(p)));

    var = Manager::Instance().getVariable("daughterInvM(0, 1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 4.0);

    var = Manager::Instance().getVariable("daughterInvM(0, 1, 2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 6.0);
  }

  TEST_F(MetaVariableTest, daughter)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(i * 1.0, 1, 1, 1), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughter(6, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(p)));

    var = Manager::Instance().getVariable("daughter(0, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(p), 0.0, 1e-6);

    var = Manager::Instance().getVariable("daughter(1, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 1.0);

    var = Manager::Instance().getVariable("daughter(2, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 2.0);
  }

  TEST_F(MetaVariableTest, mcDaughter)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    mcParticles.registerInDataStore();
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    // Create MC graph for B -> (muon -> electron + muon_neutrino) + anti_muon_neutrino
    MCParticleGraph mcGraph;

    MCParticleGraph::GraphParticle& graphParticleGrandMother = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleMother = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleAunt = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleDaughter1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleDaughter2 = mcGraph.addParticle();

    graphParticleGrandMother.setPDG(-521);
    graphParticleMother.setPDG(13);
    graphParticleAunt.setPDG(-14);
    graphParticleDaughter1.setPDG(11);
    graphParticleDaughter2.setPDG(14);

    graphParticleMother.comesFrom(graphParticleGrandMother);
    graphParticleAunt.comesFrom(graphParticleGrandMother);
    graphParticleDaughter1.comesFrom(graphParticleMother);
    graphParticleDaughter2.comesFrom(graphParticleMother);
    mcGraph.generateList();

    // Get MC Particles from StoreArray
    auto* mcGrandMother = mcParticles[0];
    mcGrandMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcMother = mcParticles[1];
    mcMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcAunt = mcParticles[2];
    mcAunt->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter1 = mcParticles[3];
    mcDaughter1->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter2 = mcParticles[4];
    mcDaughter2->setStatus(MCParticle::c_PrimaryParticle);

    auto* pGrandMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), -521);
    pGrandMother->addRelationTo(mcGrandMother);

    auto* pMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 13);
    pMother->addRelationTo(mcMother);

    // Test for particle that has no MC match
    auto* p_noMC = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 13);

    // Test for particle that has MC match, but MC match has no daughter
    auto* p_noDaughter = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p_noDaughter->addRelationTo(mcDaughter1);

    const Manager::Var* var = Manager::Instance().getVariable("mcDaughter(0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(pGrandMother), 13);
    EXPECT_FLOAT_EQ(var->function(pMother), 11);
    EXPECT_TRUE(std::isnan(var->function(p_noMC)));
    EXPECT_TRUE(std::isnan(var->function(p_noDaughter)));
    var = Manager::Instance().getVariable("mcDaughter(1, PDG)");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), -14);
    EXPECT_FLOAT_EQ(var->function(pMother), 14);
    // Test for particle where mc daughter index is out of range of mc daughters
    var = Manager::Instance().getVariable("mcDaughter(2, PDG)");
    EXPECT_TRUE(std::isnan(var->function(pGrandMother)));
    EXPECT_TRUE(std::isnan(var->function(pMother)));
    // Test nested application of mcDaughter
    var = Manager::Instance().getVariable("mcDaughter(0, mcDaughter(0, PDG))");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), 11);
    EXPECT_TRUE(std::isnan(var->function(pMother)));
    var = Manager::Instance().getVariable("mcDaughter(0, mcDaughter(1, PDG))");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), 14);
    var = Manager::Instance().getVariable("mcDaughter(0, mcDaughter(2, PDG))");
    EXPECT_TRUE(std::isnan(var->function(pGrandMother)));
    var = Manager::Instance().getVariable("mcDaughter(1, mcDaughter(0, PDG))");
    EXPECT_TRUE(std::isnan(var->function(pGrandMother)));
  }

  TEST_F(MetaVariableTest, mcMother)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    mcParticles.registerInDataStore();
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    // Create MC graph for B -> (muon -> electron + muon_neutrino) + anti_muon_neutrino
    MCParticleGraph mcGraph;

    MCParticleGraph::GraphParticle& graphParticleGrandMother = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleMother = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleAunt = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleDaughter1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleDaughter2 = mcGraph.addParticle();

    graphParticleGrandMother.setPDG(-521);
    graphParticleMother.setPDG(13);
    graphParticleAunt.setPDG(-14);
    graphParticleDaughter1.setPDG(11);
    graphParticleDaughter2.setPDG(14);

    graphParticleMother.comesFrom(graphParticleGrandMother);
    graphParticleAunt.comesFrom(graphParticleGrandMother);
    graphParticleDaughter1.comesFrom(graphParticleMother);
    graphParticleDaughter2.comesFrom(graphParticleMother);

    mcGraph.generateList();

    // Get MC Particles from StoreArray
    auto* mcGrandMother = mcParticles[0];
    mcGrandMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcMother = mcParticles[1];
    mcMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcAunt = mcParticles[2];
    mcAunt->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter1 = mcParticles[3];
    mcDaughter1->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter2 = mcParticles[4];
    mcDaughter2->setStatus(MCParticle::c_PrimaryParticle);

    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcDaughter1);

    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 14);
    p2->addRelationTo(mcDaughter2);

    auto* pMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 13);
    pMother->addRelationTo(mcMother);

    // For test of particle that has no MC match
    auto* p_noMC = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);

    // For test of particle that has MC match, but MC match has no mother
    auto* p_noMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), -521);
    p_noMother->addRelationTo(mcGrandMother);

    const Manager::Var* var = Manager::Instance().getVariable("mcMother(PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 13);
    EXPECT_FLOAT_EQ(var->function(p2), 13);
    EXPECT_FLOAT_EQ(var->function(pMother), -521);
    EXPECT_TRUE(std::isnan(var->function(p_noMC)));
    EXPECT_TRUE(std::isnan(var->function(p_noMother)));

    // Test if nested calls of mcMother work correctly
    var = Manager::Instance().getVariable("mcMother(mcMother(PDG))");
    EXPECT_FLOAT_EQ(var->function(p1), -521);
  }

  TEST_F(MetaVariableTest, genParticle)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    mcParticles.registerInDataStore();
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    // Create MC graph for Upsilon(4S) -> (B^- -> electron + anti_electron_neutrino) + B^+
    MCParticleGraph mcGraph;

    MCParticleGraph::GraphParticle& graphParticleGrandMother = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleMother = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleAunt = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleDaughter1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleDaughter2 = mcGraph.addParticle();

    graphParticleGrandMother.setPDG(300553);
    graphParticleMother.setPDG(-521);
    graphParticleAunt.setPDG(521);
    graphParticleDaughter1.setPDG(11);
    graphParticleDaughter2.setPDG(-12);

    graphParticleGrandMother.setMomentum(0.0, 0.0, 0.4);
    graphParticleMother.setMomentum(1.1, 1.3, 1.5);

    graphParticleMother.comesFrom(graphParticleGrandMother);
    graphParticleAunt.comesFrom(graphParticleGrandMother);
    graphParticleDaughter1.comesFrom(graphParticleMother);
    graphParticleDaughter2.comesFrom(graphParticleMother);

    mcGraph.generateList();

    // Get MC Particles from StoreArray
    auto* mcGrandMother = mcParticles[0];
    mcGrandMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcMother = mcParticles[1];
    mcMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcAunt = mcParticles[2];
    mcAunt->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter1 = mcParticles[3];
    mcDaughter1->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter2 = mcParticles[4];
    mcDaughter2->setStatus(MCParticle::c_PrimaryParticle);

    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcDaughter1);

    // For test of particle that has no MC match
    auto* p_noMC = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 211);

    const Manager::Var* var = Manager::Instance().getVariable("genParticle(0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 300553);
    EXPECT_FLOAT_EQ(var->function(p_noMC), 300553);

    var = Manager::Instance().getVariable("genParticle(0, matchedMC(pz))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 0.4);
    EXPECT_FLOAT_EQ(var->function(p_noMC), 0.4);

    var = Manager::Instance().getVariable("genParticle(0, mcDaughter(0, PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -521);
    EXPECT_FLOAT_EQ(var->function(p_noMC), -521);

    var = Manager::Instance().getVariable("genParticle(0, mcDaughter(0, matchedMC(px)))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 1.1);
    EXPECT_FLOAT_EQ(var->function(p_noMC), 1.1);

    var = Manager::Instance().getVariable("genParticle(1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -521);
    EXPECT_FLOAT_EQ(var->function(p_noMC), -521);

    var = Manager::Instance().getVariable("genParticle(4, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -12);
    EXPECT_FLOAT_EQ(var->function(p_noMC), -12);

    var = Manager::Instance().getVariable("genParticle(5, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(p1)));
    EXPECT_TRUE(std::isnan(var->function(p_noMC)));
  }

  TEST_F(MetaVariableTest, genUpsilon4S)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    mcParticles.registerInDataStore();
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    // Create MC graph for Upsilon(4S) -> (B^- -> electron + anti_electron_neutrino) + B^+
    MCParticleGraph mcGraph;

    MCParticleGraph::GraphParticle& graphParticleGrandMother = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleMother = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleAunt = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleDaughter1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleDaughter2 = mcGraph.addParticle();

    graphParticleGrandMother.setPDG(300553);
    graphParticleMother.setPDG(-521);
    graphParticleAunt.setPDG(521);
    graphParticleDaughter1.setPDG(11);
    graphParticleDaughter2.setPDG(-12);

    graphParticleGrandMother.setMomentum(0.0, 0.0, 0.4);
    graphParticleMother.setMomentum(1.1, 1.3, 1.5);

    graphParticleMother.comesFrom(graphParticleGrandMother);
    graphParticleAunt.comesFrom(graphParticleGrandMother);
    graphParticleDaughter1.comesFrom(graphParticleMother);
    graphParticleDaughter2.comesFrom(graphParticleMother);

    mcGraph.generateList();

    // Get MC Particles from StoreArray
    auto* mcGrandMother = mcParticles[0];
    mcGrandMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcMother = mcParticles[1];
    mcMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcAunt = mcParticles[2];
    mcAunt->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter1 = mcParticles[3];
    mcDaughter1->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter2 = mcParticles[4];
    mcDaughter2->setStatus(MCParticle::c_PrimaryParticle);

    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcDaughter1);

    // For test of particle that has no MC match
    auto* p_noMC = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 211);

    const Manager::Var* var = Manager::Instance().getVariable("genUpsilon4S(PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 300553);
    EXPECT_FLOAT_EQ(var->function(p_noMC), 300553);

    var = Manager::Instance().getVariable("genUpsilon4S(matchedMC(pz))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 0.4);
    EXPECT_FLOAT_EQ(var->function(p_noMC), 0.4);

    var = Manager::Instance().getVariable("genUpsilon4S(mcDaughter(0, PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -521);
    EXPECT_FLOAT_EQ(var->function(p_noMC), -521);

    var = Manager::Instance().getVariable("genUpsilon4S(mcDaughter(0, matchedMC(px)))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 1.1);
    EXPECT_FLOAT_EQ(var->function(p_noMC), 1.1);

    /// Test for event without generator level Upsilon(4S)
    mcParticles.clear();
    particles.clear();
    MCParticleGraph mcGraph2;

    MCParticleGraph::GraphParticle& graphParticle1 = mcGraph2.addParticle();
    MCParticleGraph::GraphParticle& graphParticle2 = mcGraph2.addParticle();

    graphParticle1.setPDG(11);
    graphParticle2.setPDG(-11);

    graphParticle1.setMomentum(1.1, 1.3, 1.4);
    graphParticle1.setMomentum(-1.1, -1.3, 1.4);

    mcGraph2.generateList();

    auto* mcP1 = mcParticles[0];
    mcP1->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcP2 = mcParticles[1];
    mcP2->setStatus(MCParticle::c_PrimaryParticle);

    auto* someParticle = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    someParticle->addRelationTo(mcP1);

    var = Manager::Instance().getVariable("genUpsilon4S(PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(someParticle)));
  }

  TEST_F(MetaVariableTest, daughterProductOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterProductOf(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 24.0);
  }

  TEST_F(MetaVariableTest, daughterSumOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterSumOf(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 10.0);

  }

  TEST_F(MetaVariableTest, daughterDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterDiffOf(0, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -222);

    var = Manager::Instance().getVariable("daughterDiffOf(1, 0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 222);

    var = Manager::Instance().getVariable("daughterDiffOf(0, 1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -200);

    var = Manager::Instance().getVariable("daughterDiffOf(1, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0);

    var = Manager::Instance().getVariable("daughterDiffOf(1, 3, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0);

    var = Manager::Instance().getVariable("daughterDiffOf(0, 2, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0);

  }

  TEST_F(MetaVariableTest, daughterClusterAngleInBetween)
  {
    // declare all the array we need
    StoreArray<Particle> particles, particles_noclst;
    std::vector<int> daughterIndices, daughterIndices_noclst;

    //proxy initialize where to declare the needed array
    DataStore::Instance().setInitializeActive(true);
    StoreArray<ECLCluster> eclclusters;
    eclclusters.registerInDataStore();
    particles.registerRelationTo(eclclusters);
    DataStore::Instance().setInitializeActive(false);

    // create two Lorentz vectors that are back to back in the CMS and boost them to the Lab frame
    const float px_CM = 2.;
    const float py_CM = 1.;
    const float pz_CM = 3.;
    float E_CM;
    E_CM = sqrt(pow(px_CM, 2) + pow(py_CM, 2) + pow(pz_CM, 2));
    TLorentzVector momentum, momentum_noclst;
    TLorentzVector dau0_4vec_CM(px_CM, py_CM, pz_CM, E_CM), dau1_4vec_CM(-px_CM, -py_CM, -pz_CM, E_CM);
    TLorentzVector dau0_4vec_Lab, dau1_4vec_Lab;
    dau0_4vec_Lab = PCmsLabTransform::cmsToLab(
                      dau0_4vec_CM); //why is eveybody using the extendend method when there are the functions that do all the steps for us?
    dau1_4vec_Lab = PCmsLabTransform::cmsToLab(dau1_4vec_CM);

    // add the two photons (now in the Lab frame) as the two daughters of some particle and create the latter
    Particle dau0_noclst(dau0_4vec_Lab, 22);
    momentum += dau0_noclst.get4Vector();
    Particle* newDaughter0_noclst = particles.appendNew(dau0_noclst);
    daughterIndices_noclst.push_back(newDaughter0_noclst->getArrayIndex());
    Particle dau1_noclst(dau1_4vec_Lab, 22);
    momentum += dau1_noclst.get4Vector();
    Particle* newDaughter1_noclst = particles.appendNew(dau1_noclst);
    daughterIndices_noclst.push_back(newDaughter1_noclst->getArrayIndex());
    const Particle* par_noclst = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices_noclst);

    // grab variables
    const Manager::Var* var = Manager::Instance().getVariable("daughterClusterAngleInBetween(0, 1)");
    const Manager::Var* varCMS = Manager::Instance().getVariable("useCMSFrame(daughterClusterAngleInBetween(0, 1))");

    // when no relations are set between the particles and the eclClusters, nan is expected to be returned
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(par_noclst)));

    // set relations between particles and eclClusters
    ECLCluster* eclst0 = eclclusters.appendNew(ECLCluster());
    eclst0->setEnergy(dau0_4vec_Lab.E());
    eclst0->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst0->setClusterId(1);
    eclst0->setTheta(dau0_4vec_Lab.Theta());
    eclst0->setPhi(dau0_4vec_Lab.Phi());
    eclst0->setR(148.4);
    ECLCluster* eclst1 = eclclusters.appendNew(ECLCluster());
    eclst1->setEnergy(dau1_4vec_Lab.E());
    eclst1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst1->setClusterId(2);
    eclst1->setTheta(dau1_4vec_Lab.Theta());
    eclst1->setPhi(dau1_4vec_Lab.Phi());
    eclst1->setR(148.5);

    const Particle* newDaughter0 = particles.appendNew(Particle(eclclusters[0]));
    daughterIndices.push_back(newDaughter0->getArrayIndex());
    const Particle* newDaughter1 = particles.appendNew(Particle(eclclusters[1]));
    daughterIndices.push_back(newDaughter1->getArrayIndex());

    const Particle* par = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices);

    //now we expect non-nan results
    EXPECT_FLOAT_EQ(var->function(par), 2.8638029);
    EXPECT_FLOAT_EQ(varCMS->function(par), M_PI);
  }

  TEST_F(MetaVariableTest, grandDaughterDiffOfs)
  {
    // declare all the array we need
    StoreArray<Particle> particles, particles_noclst;
    std::vector<int> daughterIndices0_noclst, daughterIndices1_noclst, daughterIndices2_noclst;
    std::vector<int> daughterIndices0, daughterIndices1, daughterIndices2;

    //proxy initialize where to declare the needed array
    DataStore::Instance().setInitializeActive(true);
    StoreArray<ECLCluster> eclclusters;
    eclclusters.registerInDataStore();
    particles.registerRelationTo(eclclusters);
    DataStore::Instance().setInitializeActive(false);

    // create two Lorentz vectors
    const float px_0 = 2.;
    const float py_0 = 1.;
    const float pz_0 = 3.;
    const float px_1 = 1.5;
    const float py_1 = 1.5;
    const float pz_1 = 2.5;
    float E_0, E_1;
    E_0 = sqrt(pow(px_0, 2) + pow(py_0, 2) + pow(pz_0, 2));
    E_1 = sqrt(pow(px_1, 2) + pow(py_1, 2) + pow(pz_1, 2));
    TLorentzVector momentum_0, momentum_1, momentum;
    TLorentzVector dau0_4vec(px_0, py_0, pz_0, E_0), dau1_4vec(px_1, py_1, pz_1, E_1);

    // add the two photons as the two daughters of some particle and create the latter
    // Particle dau0_noclst(dau0_4vec, 22);
    // momentum += dau0_noclst.get4Vector();
    // Particle* newDaughter0_noclst = particles.appendNew(dau0_noclst);
    // daughterIndices_noclst.push_back(newDaughter0_noclst->getArrayIndex());
    // Particle dau1_noclst(dau1_4vec, 22);
    // momentum += dau1_noclst.get4Vector();
    // Particle* newDaughter1_noclst = particles.appendNew(dau1_noclst);
    // daughterIndices_noclst.push_back(newDaughter1_noclst->getArrayIndex());
    // const Particle* par_noclst = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices_noclst);

    Particle dau0_noclst(dau0_4vec, 22);
    momentum_0 = dau0_4vec;
    Particle* newDaughter0_noclst = particles.appendNew(dau0_noclst);
    daughterIndices0_noclst.push_back(newDaughter0_noclst->getArrayIndex());
    const Particle* par0_noclst = particles.appendNew(momentum_0, 111, Particle::c_Unflavored, daughterIndices0_noclst);
    Particle dau1_noclst(dau1_4vec, 22);
    momentum_1 = dau1_4vec;
    Particle* newDaughter1_noclst = particles.appendNew(dau1_noclst);
    daughterIndices1_noclst.push_back(newDaughter1_noclst->getArrayIndex());
    const Particle* par1_noclst = particles.appendNew(momentum_1, 111, Particle::c_Unflavored, daughterIndices1_noclst);

    momentum = momentum_0 + momentum_1;
    daughterIndices2_noclst.push_back(par0_noclst->getArrayIndex());
    daughterIndices2_noclst.push_back(par1_noclst->getArrayIndex());
    const Particle* parGranny_noclst = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices2_noclst);

    // grab variables
    const Manager::Var* var_Theta = Manager::Instance().getVariable("grandDaughterDiffOf(0,1,0,0,theta)");
    const Manager::Var* var_ClusterTheta = Manager::Instance().getVariable("grandDaughterDiffOf(0,1,0,0,clusterTheta)");
    const Manager::Var* var_E = Manager::Instance().getVariable("grandDaughterDiffOf(0,1,0,0,E)");
    const Manager::Var* var_ClusterE = Manager::Instance().getVariable("grandDaughterDiffOf(0,1,0,0,clusterE)");
    const Manager::Var* var_E_wrongIndexes = Manager::Instance().getVariable("grandDaughterDiffOf(0,1,2,3,E)");
    const Manager::Var* var_ClusterE_wrongIndexes = Manager::Instance().getVariable("grandDaughterDiffOf(0,1,2,3,clusterE)");

    const Manager::Var* var_ClusterPhi = Manager::Instance().getVariable("grandDaughterDiffOfClusterPhi(0,1,0,0)");
    const Manager::Var* var_Phi = Manager::Instance().getVariable("grandDaughterDiffOfPhi(0,1,0,0)");
    const Manager::Var* var_ClusterPhi_wrongIndexes = Manager::Instance().getVariable("grandDaughterDiffOfClusterPhi(0,1,2,3)");
    const Manager::Var* var_Phi_wrongIndexes = Manager::Instance().getVariable("grandDaughterDiffOfPhi(0,1,2,3)");

    // when no relations are set between the particles and the eclClusters, nan is expected to be returned for the Cluster- vars
    // no problems are supposed to happen for non-Cluster- vars
    // also, we expect NaN when we pass wrong indexes
    ASSERT_NE(var_ClusterPhi, nullptr);
    EXPECT_TRUE(std::isnan(var_ClusterPhi->function(parGranny_noclst)));
    EXPECT_TRUE(std::isnan(var_ClusterTheta->function(parGranny_noclst)));
    EXPECT_TRUE(std::isnan(var_ClusterE->function(parGranny_noclst)));
    EXPECT_FLOAT_EQ(var_Phi->function(parGranny_noclst), 0.32175055);
    EXPECT_FLOAT_EQ(var_Theta->function(parGranny_noclst), 0.06311664);
    EXPECT_FLOAT_EQ(var_E->function(parGranny_noclst), -0.46293831);
    EXPECT_TRUE(std::isnan(var_ClusterPhi_wrongIndexes->function(parGranny_noclst)));
    EXPECT_TRUE(std::isnan(var_Phi_wrongIndexes->function(parGranny_noclst)));
    EXPECT_TRUE(std::isnan(var_ClusterE_wrongIndexes->function(parGranny_noclst)));
    EXPECT_TRUE(std::isnan(var_E_wrongIndexes->function(parGranny_noclst)));

    // set relations between particles and eclClusters
    ECLCluster* eclst0 = eclclusters.appendNew(ECLCluster());
    eclst0->setEnergy(dau0_4vec.E());
    eclst0->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst0->setClusterId(1);
    eclst0->setTheta(dau0_4vec.Theta());
    eclst0->setPhi(dau0_4vec.Phi());
    eclst0->setR(148.4);
    ECLCluster* eclst1 = eclclusters.appendNew(ECLCluster());
    eclst1->setEnergy(dau1_4vec.E());
    eclst1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst1->setClusterId(2);
    eclst1->setTheta(dau1_4vec.Theta());
    eclst1->setPhi(dau1_4vec.Phi());
    eclst1->setR(148.5);

    const Particle* newDaughter0 = particles.appendNew(Particle(eclclusters[0]));
    daughterIndices0.push_back(newDaughter0->getArrayIndex());
    const Particle* par0 = particles.appendNew(momentum_0, 111, Particle::c_Unflavored, daughterIndices0);

    const Particle* newDaughter1 = particles.appendNew(Particle(eclclusters[1]));
    daughterIndices1.push_back(newDaughter1->getArrayIndex());
    const Particle* par1 = particles.appendNew(momentum_1, 111, Particle::c_Unflavored, daughterIndices1);

    daughterIndices2.push_back(par0->getArrayIndex());
    daughterIndices2.push_back(par1->getArrayIndex());
    const Particle* parGranny = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices2);
    //const Particle* par = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices);

    //now we expect non-nan results
    EXPECT_FLOAT_EQ(var_ClusterPhi->function(parGranny), 0.32175055);
    EXPECT_FLOAT_EQ(var_Phi->function(parGranny), 0.32175055);
    EXPECT_FLOAT_EQ(var_ClusterTheta->function(parGranny), 0.06311664);
    EXPECT_FLOAT_EQ(var_Theta->function(parGranny), 0.06311664);
    EXPECT_FLOAT_EQ(var_ClusterE->function(parGranny), -0.46293831);
    EXPECT_FLOAT_EQ(var_E->function(parGranny), -0.46293813);
  }

  TEST_F(MetaVariableTest, daughterNormDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterNormDiffOf(0, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -222 / 200.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(1, 0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 222 / 200.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(0, 1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -200 / 222.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(1, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -0 / 22.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(1, 3, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0 / 22.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(0, 2, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0 / 422.);

  }

  TEST_F(MetaVariableTest, daughterMotherDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterMotherDiffOf(1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 422);

    var = Manager::Instance().getVariable("daughterMotherDiffOf(1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 400);

    var = Manager::Instance().getVariable("daughterMotherDiffOf(0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 200);

  }

  TEST_F(MetaVariableTest, daughterMotherNormDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterMotherNormDiffOf(1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 422 / 400.);

    var = Manager::Instance().getVariable("daughterMotherNormDiffOf(1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 400 / 422.);

    var = Manager::Instance().getVariable("daughterMotherNormDiffOf(0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 200 / 622.);

  }

  TEST_F(MetaVariableTest, constant)
  {

    const Manager::Var* var = Manager::Instance().getVariable("constant(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 1.0);

    var = Manager::Instance().getVariable("constant(0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 0.0);

  }

  TEST_F(MetaVariableTest, abs)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ -0.1 , -0.4, 0.8, 4.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("abs(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.1);
    EXPECT_FLOAT_EQ(var->function(&p2), 0.1);

  }

  TEST_F(MetaVariableTest, sin)
  {
    Particle p({ 3.14159265359 / 2.0 , -0.4, 0.8, 1.0}, 11);
    Particle p2({ 0.0 , -0.4, 0.8, 1.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("sin(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);
    EXPECT_ALL_NEAR(var->function(&p2), 0.0, 1e-6);

  }

  TEST_F(MetaVariableTest, cos)
  {
    Particle p({ 3.14159265359 / 2.0 , -0.4, 0.8, 1.0}, 11);
    Particle p2({ 0.0 , -0.4, 0.8, 1.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("cos(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.0, 1e-6);
    EXPECT_FLOAT_EQ(var->function(&p2), 1.0);

  }

  TEST_F(MetaVariableTest, NBDeltaIfMissingDeathTest)
  {
    //Variable got removed, test for absence
    EXPECT_B2FATAL(Manager::Instance().getVariable("NBDeltaIfMissing(TOP, 11)"));
    EXPECT_B2FATAL(Manager::Instance().getVariable("NBDeltaIfMissing(ARICH, 11)"));
  }

  TEST_F(MetaVariableTest, matchedMC)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p2->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p3 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p3->addRelationTo(mcParticle);

    // Test if matchedMC also works for particle which already is an MCParticle.
    auto* p4 = particles.appendNew(mcParticle);

    const Manager::Var* var = Manager::Instance().getVariable("matchedMC(charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -1);
    EXPECT_FLOAT_EQ(var->function(p2), 1);
    EXPECT_FLOAT_EQ(var->function(p3), 0);
    EXPECT_FLOAT_EQ(var->function(p4), 0);
  }

  TEST_F(MetaVariableTest, countInList)
  {
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    StoreObjPtr<ParticleList> outputList("pList1");
    DataStore::Instance().setInitializeActive(true);
    outputList.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList.create();
    outputList->initialize(22, "pList1");

    particles.appendNew(Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2));
    particles.appendNew(Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3));
    particles.appendNew(Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4));
    particles.appendNew(Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 5));
    particles.appendNew(Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 6));

    outputList->addParticle(0, 22, Particle::c_Unflavored);
    outputList->addParticle(1, 22, Particle::c_Unflavored);
    outputList->addParticle(2, 22, Particle::c_Unflavored);
    outputList->addParticle(3, 22, Particle::c_Unflavored);
    outputList->addParticle(4, 22, Particle::c_Unflavored);

    const Manager::Var* var = Manager::Instance().getVariable("countInList(pList1, E < 0.85)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 2);

    var = Manager::Instance().getVariable("countInList(pList1)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 5);

    var = Manager::Instance().getVariable("countInList(pList1, E > 5)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 0);

    var = Manager::Instance().getVariable("countInList(pList1, E < 5)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 5);
  }

  TEST_F(MetaVariableTest, isInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // mock up two photons
    Particle goingin({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0);
    Particle notgoingin({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1);
    auto* inthelist = particles.appendNew(goingin);
    auto* notinthelist = particles.appendNew(notgoingin);

    // put the the zeroth one in the list the first on not in the list
    gammalist->addParticle(0, 22, Particle::c_Unflavored);

    // get the variables
    const Manager::Var* vnonsense = Manager::Instance().getVariable("isInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable("isInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(notinthelist));
    EXPECT_FLOAT_EQ(vsensible->function(inthelist), 1.0);
    EXPECT_FLOAT_EQ(vsensible->function(notinthelist), 0.0);
  }

  TEST_F(MetaVariableTest, sourceObjectIsInList)
  {
    // datastore things
    DataStore::Instance().reset();
    DataStore::Instance().setInitializeActive(true);

    // needed to mock up
    StoreArray<ECLCluster> clusters;
    StoreArray<Particle> particles;
    StoreObjPtr<ParticleList> gammalist("testGammaList");

    clusters.registerInDataStore();
    particles.registerInDataStore();
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;
    gammalist.registerInDataStore(flags);

    // end datastore things
    DataStore::Instance().setInitializeActive(false);

    // of course we have to create the list...
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // mock up two clusters from the ECL let's say they both came from true Klongs
    // but one looked a little bit photon-like
    auto* cl0 = clusters.appendNew(ECLCluster());
    cl0->setEnergy(1.0);
    cl0->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    cl0->addHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
    cl0->setClusterId(0);
    auto* cl1 = clusters.appendNew(ECLCluster());
    cl1->setEnergy(1.0);
    cl1->setHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
    cl1->setClusterId(1);

    // create particles from the clusters
    Particle myphoton(cl0, Const::photon);
    Particle iscopiedin(cl0, Const::Klong);
    Particle notcopiedin(cl1, Const::Klong);

    // add the particle created from cluster zero to the gamma list
    auto* myphoton_ = particles.appendNew(myphoton);
    gammalist->addParticle(myphoton_);

    auto* iscopied = particles.appendNew(iscopiedin); // a clone of this guy is now in the gamma list
    auto* notcopied = particles.appendNew(notcopiedin);

    // get the variables
    const Manager::Var* vnonsense = Manager::Instance().getVariable("sourceObjectIsInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable("sourceObjectIsInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(iscopied));
    EXPECT_FLOAT_EQ(vsensible->function(iscopied), 1.0);
    EXPECT_FLOAT_EQ(vsensible->function(notcopied), 0.0);

    // now mock up some other type particles
    Particle composite({0.5 , 0.4 , 0.5 , 0.8}, 512, Particle::c_Unflavored, Particle::c_Composite, 0);
    Particle undefined({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1);
    auto* composite_ = particles.appendNew(undefined);
    auto* undefined_ = particles.appendNew(composite);
    EXPECT_FLOAT_EQ(vsensible->function(composite_), -1.0);
    EXPECT_FLOAT_EQ(vsensible->function(undefined_), -1.0);
  }

  TEST_F(MetaVariableTest, mostB2BAndClosestParticles)
  {
    /* Mock up an event with a "photon" and an "electron" which are nearly back to
     * back, and second "photon" which is close-ish to the "electron".
     *
     * Other test of non-existent / empty lists and variables also included.
     */

    // Connect gearbox for CMS variables
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    StoreObjPtr<ParticleList> emptylist("testEmptyList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    emptylist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");
    emptylist.create();
    emptylist->initialize(22, "testEmptyList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({ -1.0 , -1.0 , 0.8, 1.2}, // this should be the most b2b to our reference particle
      22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.2 , 0.7 , 0.9, 3.4},    // should be the closest
      22, Particle::c_Unflavored, Particle::c_Undefined, 1),
    };
    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // add the reference particle (electron) to the StoreArray
    const auto* electron = particles.appendNew(
                             Particle({1.0 , 1.0 , 0.5, 0.8},  // somewhere in the +ve quarter of the detector
                                      11, Particle::c_Unflavored, Particle::c_Undefined, 2) // needs to be incremented if we add to gamma vector
                           );

    {
      EXPECT_B2FATAL(Manager::Instance().getVariable("angleToClosestInList"));
      EXPECT_B2FATAL(Manager::Instance().getVariable("angleToClosestInList(A, B)"));

      const auto* nonexistant = Manager::Instance().getVariable("angleToClosestInList(NONEXISTANTLIST)");
      EXPECT_B2FATAL(nonexistant->function(electron));

      const auto* empty = Manager::Instance().getVariable("angleToClosestInList(testEmptyList)");
      EXPECT_TRUE(std::isnan(empty->function(electron)));

      const auto* closest = Manager::Instance().getVariable("angleToClosestInList(testGammaList)");
      EXPECT_FLOAT_EQ(closest->function(electron), 0.68014491);

      const auto* closestCMS = Manager::Instance().getVariable("useCMSFrame(angleToClosestInList(testGammaList))");
      EXPECT_FLOAT_EQ(closestCMS->function(electron), 0.72592634);
    }

    {
      EXPECT_B2FATAL(Manager::Instance().getVariable("closestInList"));
      EXPECT_B2FATAL(Manager::Instance().getVariable("closestInList(A, B, C)"));

      const auto* nonexistant = Manager::Instance().getVariable("closestInList(NONEXISTANTLIST, E)");
      EXPECT_B2FATAL(nonexistant->function(electron));

      const auto* empty = Manager::Instance().getVariable("closestInList(testEmptyList, E)");
      EXPECT_TRUE(std::isnan(empty->function(electron)));

      const auto* closest = Manager::Instance().getVariable("closestInList(testGammaList, E)");
      EXPECT_FLOAT_EQ(closest->function(electron), 3.4);

      const auto* closestCMS = Manager::Instance().getVariable("useCMSFrame(closestInList(testGammaList, E))");
      EXPECT_FLOAT_EQ(closestCMS->function(electron), 3.2732551); // the energy gets smeared because of boost

      const auto* closestCMSLabE = Manager::Instance().getVariable("useCMSFrame(closestInList(testGammaList, useLabFrame(E)))");
      EXPECT_FLOAT_EQ(closestCMSLabE->function(electron), 3.4); // aaand should be back to the lab frame value
    }

    {
      EXPECT_B2FATAL(Manager::Instance().getVariable("angleToMostB2BInList"));
      EXPECT_B2FATAL(Manager::Instance().getVariable("angleToMostB2BInList(A, B)"));

      const auto* nonexistant = Manager::Instance().getVariable("angleToMostB2BInList(NONEXISTANTLIST)");
      EXPECT_B2FATAL(nonexistant->function(electron));

      const auto* empty = Manager::Instance().getVariable("angleToMostB2BInList(testEmptyList)");
      EXPECT_TRUE(std::isnan(empty->function(electron)));

      const auto* mostB2B = Manager::Instance().getVariable("angleToMostB2BInList(testGammaList)");
      EXPECT_FLOAT_EQ(mostB2B->function(electron), 2.2869499);

      const auto* mostB2BCMS = Manager::Instance().getVariable("useCMSFrame(angleToMostB2BInList(testGammaList))");
      EXPECT_FLOAT_EQ(mostB2BCMS->function(electron), 2.6054888);
    }

    {
      EXPECT_B2FATAL(Manager::Instance().getVariable("mostB2BInList"));
      EXPECT_B2FATAL(Manager::Instance().getVariable("mostB2BInList(A, B, C)"));

      const auto* nonexistant = Manager::Instance().getVariable("mostB2BInList(NONEXISTANTLIST, E)");
      EXPECT_B2FATAL(nonexistant->function(electron));

      const auto* empty = Manager::Instance().getVariable("mostB2BInList(testEmptyList, E)");
      EXPECT_TRUE(std::isnan(empty->function(electron)));

      const auto* mostB2B = Manager::Instance().getVariable("mostB2BInList(testGammaList, E)");
      EXPECT_FLOAT_EQ(mostB2B->function(electron), 1.2);

      const auto* mostB2BCMS = Manager::Instance().getVariable("useCMSFrame(mostB2BInList(testGammaList, E))");
      EXPECT_FLOAT_EQ(mostB2BCMS->function(electron), 1.0647389); // the energy gets smeared because of boost

      const auto* mostB2BCMSLabE = Manager::Instance().getVariable("useCMSFrame(mostB2BInList(testGammaList, useLabFrame(E)))");
      EXPECT_FLOAT_EQ(mostB2BCMSLabE->function(electron), 1.2); // aaand should be back to the lab frame value
    }
  }

  TEST_F(MetaVariableTest, totalEnergyOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalEnergyOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalEnergyOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 4.3);
  }
  TEST_F(MetaVariableTest, totalPxOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalPxOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalPxOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 2.2);
  }
  TEST_F(MetaVariableTest, totalPyOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalPyOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalPyOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 1.5);
  }
  TEST_F(MetaVariableTest, totalPzOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalPzOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalPzOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 3.1);
  }
  TEST_F(MetaVariableTest, maxPtInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "maxPtInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "maxPtInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), sqrt(0.5 * 0.5 + 0.4 * 0.4));
  }


  TEST_F(MetaVariableTest, numberOfNonOverlappingParticles)
  {
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    StoreObjPtr<ParticleList> outputList("pList1");
    DataStore::Instance().setInitializeActive(true);
    outputList.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList.create();
    outputList->initialize(22, "pList1");

    auto* p1 = particles.appendNew(Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2));
    auto* p2 = particles.appendNew(Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3));
    auto* p3 = particles.appendNew(Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4));

    outputList->addParticle(0, 22, Particle::c_Unflavored);
    outputList->addParticle(1, 22, Particle::c_Unflavored);

    const Manager::Var* var = Manager::Instance().getVariable("numberOfNonOverlappingParticles(pList1)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p1), 1);
    EXPECT_DOUBLE_EQ(var->function(p2), 1);
    EXPECT_DOUBLE_EQ(var->function(p3), 2);

  }

  TEST_F(MetaVariableTest, veto)
  {
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    const Particle* p = particles.appendNew(Particle({0.8 , 0.8 , 1.131370849898476039041351 , 1.6}, 22,
                                                     Particle::c_Unflavored, Particle::c_Undefined, 1));

    StoreObjPtr<ParticleList> outputList("pList1");
    DataStore::Instance().setInitializeActive(true);
    outputList.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList.create();
    outputList->initialize(22, "pList1");

    particles.appendNew(Particle({0.5 , 0.4953406774856531014212777 , 0.5609256753154148484773173 , 0.9}, 22,
                                 Particle::c_Unflavored, Particle::c_Undefined, 2));         //m=0.135
    particles.appendNew(Particle({0.5 , 0.2 , 0.72111 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3));    //m=0.3582
    particles.appendNew(Particle({0.4 , 0.2 , 0.78102 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4));    //m=0.3908
    particles.appendNew(Particle({0.5 , 0.4 , 0.89443 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 5));    //m=0.2369
    particles.appendNew(Particle({0.3 , 0.3 , 0.42426 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 6));    //m=0.0036

    outputList->addParticle(1, 22, Particle::c_Unflavored);
    outputList->addParticle(2, 22, Particle::c_Unflavored);
    outputList->addParticle(3, 22, Particle::c_Unflavored);
    outputList->addParticle(4, 22, Particle::c_Unflavored);
    outputList->addParticle(5, 22, Particle::c_Unflavored);

    StoreObjPtr<ParticleList> outputList2("pList2");
    DataStore::Instance().setInitializeActive(true);
    outputList2.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList2.create();
    outputList2->initialize(22, "pList2");

    particles.appendNew(Particle({0.5 , -0.4 , 0.63246 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 7));    //m=1.1353
    particles.appendNew(Particle({0.5 , 0.2 , 0.72111 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 8));     //m=0.3582
    particles.appendNew(Particle({0.4 , 0.2 , 0.78102 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 9));     //m=0.3908
    particles.appendNew(Particle({0.5 , 0.4 , 0.89443 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 10));    //m=0.2369
    particles.appendNew(Particle({0.3 , 0.3 , 0.42426 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 11));    //m=0.0036

    outputList2->addParticle(6, 22, Particle::c_Unflavored);
    outputList2->addParticle(7, 22, Particle::c_Unflavored);
    outputList2->addParticle(8, 22, Particle::c_Unflavored);
    outputList2->addParticle(9, 22, Particle::c_Unflavored);
    outputList2->addParticle(10, 22, Particle::c_Unflavored);

    const Manager::Var* var = Manager::Instance().getVariable("veto(pList1, 0.130 < M < 0.140)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 1);

    var = Manager::Instance().getVariable("veto(pList2, 0.130 < M < 0.140)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 0);

  }

  TEST_F(MetaVariableTest, averageValueInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get the average px, py, pz, E of the gammas in the list
    const Manager::Var* vmeanpx = Manager::Instance().getVariable(
                                    "averageValueInList(testGammaList, px)");
    const Manager::Var* vmeanpy = Manager::Instance().getVariable(
                                    "averageValueInList(testGammaList, py)");
    const Manager::Var* vmeanpz = Manager::Instance().getVariable(
                                    "averageValueInList(testGammaList, pz)");
    const Manager::Var* vmeanE = Manager::Instance().getVariable(
                                   "averageValueInList(testGammaList, E)");

    EXPECT_FLOAT_EQ(vmeanpx->function(nullptr), 0.44);
    EXPECT_FLOAT_EQ(vmeanpy->function(nullptr), 0.3);
    EXPECT_FLOAT_EQ(vmeanpz->function(nullptr), 0.62);
    EXPECT_FLOAT_EQ(vmeanE->function(nullptr), 0.86);

    // wrong number of arguments (no variable provided)
    EXPECT_B2FATAL(Manager::Instance().getVariable("averageValueInList(testGammaList)"));

    // non-existing variable
    EXPECT_B2FATAL(Manager::Instance().getVariable("averageValueInList(testGammaList, NONEXISTANTVARIABLE)"));

    // non-existing list
    const Manager::Var* vnolist = Manager::Instance().getVariable(
                                    "averageValueInList(NONEXISTANTLIST, px)");

    EXPECT_B2FATAL(vnolist->function(nullptr));
  }

  TEST_F(MetaVariableTest, medianValueInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create two photon lists for testing (one with odd and one with even number of particles)
    StoreObjPtr<ParticleList> oddgammalist("oddGammaList");
    DataStore::Instance().setInitializeActive(true);
    oddgammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    oddgammalist.create();
    oddgammalist->initialize(22, "oddGammaList");
    StoreObjPtr<ParticleList> evengammalist("evenGammaList");
    DataStore::Instance().setInitializeActive(true);
    evengammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    evengammalist.create();
    evengammalist->initialize(22, "evenGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto& g : gammavector)
      particles.appendNew(g);

    // put the photons in the test lists
    if (gammavector.size() % 2 == 0) {
      evengammalist->addParticle(0, 22, Particle::c_Unflavored);
    } else
      oddgammalist->addParticle(0, 22, Particle::c_Unflavored);
    for (size_t i = 1; i < gammavector.size(); i++) {
      oddgammalist->addParticle(i, 22, Particle::c_Unflavored);
      evengammalist->addParticle(i, 22, Particle::c_Unflavored);
    }

    // get the median px, py, pz, E of the gammas in the list with odd number of particles
    const Manager::Var* voddmedianpx = Manager::Instance().getVariable(
                                         "medianValueInList(oddGammaList, px)");
    const Manager::Var* voddmedianpy = Manager::Instance().getVariable(
                                         "medianValueInList(oddGammaList, py)");
    const Manager::Var* voddmedianpz = Manager::Instance().getVariable(
                                         "medianValueInList(oddGammaList, pz)");
    const Manager::Var* voddmedianE = Manager::Instance().getVariable(
                                        "medianValueInList(oddGammaList, E)");

    EXPECT_FLOAT_EQ(voddmedianpx->function(nullptr), 0.5);
    EXPECT_FLOAT_EQ(voddmedianpy->function(nullptr), 0.3);
    EXPECT_FLOAT_EQ(voddmedianpz->function(nullptr), 0.7);
    EXPECT_FLOAT_EQ(voddmedianE->function(nullptr), 0.9);

    // get the median px, py, pz, E of the gammas in the list with odd number of particles
    const Manager::Var* vevenmedianpx = Manager::Instance().getVariable(
                                          "medianValueInList(evenGammaList, px)");
    const Manager::Var* vevenmedianpy = Manager::Instance().getVariable(
                                          "medianValueInList(evenGammaList, py)");
    const Manager::Var* vevenmedianpz = Manager::Instance().getVariable(
                                          "medianValueInList(evenGammaList, pz)");
    const Manager::Var* vevenmedianE = Manager::Instance().getVariable(
                                         "medianValueInList(evenGammaList, E)");

    EXPECT_FLOAT_EQ(vevenmedianpx->function(nullptr), 0.45);
    EXPECT_FLOAT_EQ(vevenmedianpy->function(nullptr), 0.25);
    EXPECT_FLOAT_EQ(vevenmedianpz->function(nullptr), 0.7);
    EXPECT_FLOAT_EQ(vevenmedianE->function(nullptr), 0.9);

    // wrong number of arguments (no variable provided)
    EXPECT_B2FATAL(Manager::Instance().getVariable("medianValueInList(oddGammaList)"));

    // non-existing variable
    EXPECT_B2FATAL(Manager::Instance().getVariable("medianValueInList(oddGammaList, NONEXISTANTVARIABLE)"));

    // non-existing list
    const Manager::Var* vnolist = Manager::Instance().getVariable(
                                    "medianValueInList(NONEXISTANTLIST, px)");

    EXPECT_B2FATAL(vnolist->function(nullptr));
  }

  TEST_F(MetaVariableTest, pValueCombination)
  {
    TLorentzVector momentum;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    Particle KS(TLorentzVector(1.164, 1.55200, 0, 2), 310, Particle::c_Unflavored, Particle::c_Composite, 0);
    KS.setPValue(0.1);
    momentum += KS.get4Vector();
    Particle* newDaughters = particles.appendNew(KS);
    daughterIndices.push_back(newDaughters->getArrayIndex());
    Particle Jpsi(TLorentzVector(-1, 1, 1, 3.548), 443, Particle::c_Unflavored, Particle::c_Composite, 1);
    Jpsi.setPValue(0.9);
    momentum += Jpsi.get4Vector();
    newDaughters = particles.appendNew(Jpsi);
    daughterIndices.push_back(newDaughters->getArrayIndex());
    Particle* B = particles.appendNew(momentum, 521, Particle::c_Flavored, daughterIndices);
    B->setPValue(0.5);

    const Manager::Var* singlePvalue = Manager::Instance().getVariable("pValueCombination(chiProb)");
    ASSERT_NE(singlePvalue, nullptr);
    EXPECT_FLOAT_EQ(singlePvalue->function(B), 0.5);

    const Manager::Var* twoPvalues = Manager::Instance().getVariable("pValueCombination(chiProb, daughter(0, chiProb))");
    ASSERT_NE(twoPvalues, nullptr);
    EXPECT_FLOAT_EQ(twoPvalues->function(B), 0.05 * (1 - log(0.05)));

    const Manager::Var* threePvalues =
      Manager::Instance().getVariable("pValueCombination(chiProb, daughter(0, chiProb), daughter(1, chiProb))");
    ASSERT_NE(threePvalues, nullptr);
    EXPECT_FLOAT_EQ(threePvalues->function(B), 0.045 * (1 - log(0.045) + 0.5 * log(0.045) * log(0.045)));

    // wrong number of arguments
    EXPECT_B2FATAL(Manager::Instance().getVariable("pValueCombination()"));

    // non-existing variable
    EXPECT_B2FATAL(Manager::Instance().getVariable("pValueCombination(chiProb, NONEXISTANTVARIABLE)"));
  }


  TEST_F(MetaVariableTest, daughterCombinationOneGeneration)
  {
    const int nDaughters = 5;
    TLorentzVector momentum(0, 0, 0, 0);
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    std::vector<TLorentzVector> daughterMomenta;

    for (int i = 0; i < nDaughters; i++) {
      TLorentzVector mom(1, i * 0.5, 1, i * 1.0 + 2.0);
      Particle d(mom, (i % 2) ? -11 : 211);
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
      daughterMomenta.push_back(mom);
      momentum = momentum + mom;
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Flavored, daughterIndices);

    // Test the invariant mass of several combinations
    const Manager::Var* var = Manager::Instance().getVariable("daughterCombination(M, 0,1,2)");
    double M_test = (daughterMomenta[0] + daughterMomenta[1] + daughterMomenta[2]).Mag();
    EXPECT_FLOAT_EQ(var->function(p), M_test);

    var = Manager::Instance().getVariable("daughterCombination(M, 0,4)");
    M_test = (daughterMomenta[0] + daughterMomenta[4]).Mag();
    EXPECT_FLOAT_EQ(var->function(p), M_test);


    // Try with a non-lorentz invariant quantity
    var = Manager::Instance().getVariable("daughterCombination(p, 1, 0, 4)");
    double p_test = (daughterMomenta[0] + daughterMomenta[1] + daughterMomenta[4]).Vect().Mag();
    EXPECT_FLOAT_EQ(var->function(p), p_test);


    // errors and bad stuff
    EXPECT_B2FATAL(Manager::Instance().getVariable("daughterCombination(aVeryNonExistingVariableSillyName, 1, 0, 4)"));

    var = Manager::Instance().getVariable("daughterCombination(M, 1, 0, 100)");
    EXPECT_B2WARNING(var->function(p));
    EXPECT_TRUE(std::isnan(var->function(p)));


    var = Manager::Instance().getVariable("daughterCombination(M, 1, -1)");
    EXPECT_B2WARNING(var->function(p));
    EXPECT_TRUE(std::isnan(var->function(p)));


    var = Manager::Instance().getVariable("daughterCombination(M, 1, 0:1:0:0:1)");
    EXPECT_B2WARNING(var->function(p));
    EXPECT_TRUE(std::isnan(var->function(p)));

  }


  TEST_F(MetaVariableTest, daughterCombinationTwoGenerations)
  {
    StoreArray<Particle> particles;

    // make a 1 -> 3 particle

    TLorentzVector momentum_1(0, 0, 0, 0);
    std::vector<TLorentzVector> daughterMomenta_1;
    std::vector<int> daughterIndices_1;

    for (int i = 0; i < 3; i++) {
      TLorentzVector mom(i * 0.2, 1, 1, i * 1.0 + 2.0);
      Particle d(mom, (i % 2) ? -11 : 211);
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices_1.push_back(newDaughters->getArrayIndex());
      daughterMomenta_1.push_back(mom);
      momentum_1 = momentum_1 + mom;
    }

    const Particle* compositeDau_1 = particles.appendNew(momentum_1, 411, Particle::c_Flavored, daughterIndices_1);


    // make a 1 -> 2 particle

    TLorentzVector momentum_2(0, 0, 0, 0);
    std::vector<TLorentzVector> daughterMomenta_2;
    std::vector<int> daughterIndices_2;

    for (int i = 0; i < 2; i++) {
      TLorentzVector mom(1, 1, i * 0.3, i * 1.0 + 2.0);
      Particle d(mom, (i % 2) ? -11 : 211);
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices_2.push_back(newDaughters->getArrayIndex());
      daughterMomenta_2.push_back(mom);
      momentum_2 = momentum_2 + mom;
    }

    const Particle* compositeDau_2 = particles.appendNew(momentum_2, 411, Particle::c_Flavored, daughterIndices_2);


    // make the composite particle
    std::vector<int> daughterIndices = {compositeDau_1->getArrayIndex(), compositeDau_2->getArrayIndex()};
    const Particle* p = particles.appendNew(momentum_2 + momentum_1, 111, Particle::c_Unflavored, daughterIndices);


    // Test the invariant mass of several combinations
    const Manager::Var* var = Manager::Instance().getVariable("daughterCombination(M, 0,1)");
    double M_test = (momentum_1 + momentum_2).Mag();
    EXPECT_FLOAT_EQ(var->function(p), M_test);

    // this should be the mass of the first daughter
    var = Manager::Instance().getVariable("daughterCombination(M, 0:0, 0:1, 0:2)");
    M_test = (momentum_1).Mag();
    EXPECT_FLOAT_EQ(var->function(p), M_test);

    // this should be a generic combinations
    var = Manager::Instance().getVariable("daughterCombination(M, 0:0, 0:1, 1:0)");
    M_test = (daughterMomenta_1[0] + daughterMomenta_1[1] + daughterMomenta_2[0]).Mag();
    EXPECT_FLOAT_EQ(var->function(p), M_test);

  }


  TEST_F(MetaVariableTest, useAlternativeDaughterHypothesis)
  {
    const int nDaughters = 5;
    StoreArray<Particle> particles;

    // Build a first Particle
    TLorentzVector momentum(0, 0, 0, 0);
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      double px =  i * 0.1;
      double py =  i * 0.3;
      double pz =  -i * 0.1 - 0.2;

      TLorentzVector mom(px, py, pz, 1);
      // all pions
      int pdgCode = Const::pion.getPDGCode();
      Particle d(mom, pdgCode);
      d.updateMass(pdgCode);
      mom.SetXYZM(px, py, pz, d.getMass());

      Particle* daughters = particles.appendNew(d);
      daughterIndices.push_back(daughters->getArrayIndex());
      momentum = momentum + mom;
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Flavored, daughterIndices);


    // Build a second Particle with same momenta, but different mass hyp.
    TLorentzVector momentumAlt(0, 0, 0, 0);
    std::vector<int> daughterIndicesAlt;
    for (int i = 0; i < nDaughters; i++) {
      double px =  i * 0.1;
      double py =  i * 0.3;
      double pz =  -i * 0.1 - 0.2;

      TLorentzVector mom(px, py, pz, 1);
      // all pions but the first two
      int pdgCode = Const::pion.getPDGCode();
      if (i == 0)
        pdgCode = Const::proton.getPDGCode(); // a proton
      if (i == 1)
        pdgCode = Const::kaon.getPDGCode(); // a K
      Particle d(mom, pdgCode);
      d.updateMass(pdgCode);
      mom.SetXYZM(px, py, pz, d.getMass());

      Particle* daughters = particles.appendNew(d);
      daughterIndicesAlt.push_back(daughters->getArrayIndex());
      momentumAlt = momentumAlt + mom;
    }
    const Particle* pAlt = particles.appendNew(momentumAlt, 411, Particle::c_Flavored, daughterIndicesAlt);


    // Test the invariant mass under the alternative hypothesis
    std::cout << "mass test" << std::endl;
    const Manager::Var* var = Manager::Instance().getVariable("useAlternativeDaughterHypothesis(M, 0:p+,1:K+)");
    const Manager::Var* varAlt = Manager::Instance().getVariable("M");
    EXPECT_FLOAT_EQ(var->function(p), varAlt->function(pAlt));

    // check it's really charge-insensitive...
    std::cout << "charge test" << std::endl;
    var = Manager::Instance().getVariable("useAlternativeDaughterHypothesis(M, 0:p+,1:K-)");
    EXPECT_FLOAT_EQ(var->function(p), varAlt->function(pAlt));

    // check the variable is not changing the 3-momentum
    std::cout << "momentum test" << std::endl;
    var = Manager::Instance().getVariable("useAlternativeDaughterHypothesis(p, 0:p+,1:K-)");
    varAlt = Manager::Instance().getVariable("p");
    EXPECT_FLOAT_EQ(var->function(p), varAlt->function(pAlt));
    EXPECT_FLOAT_EQ(var->function(p), varAlt->function(p));
    EXPECT_FLOAT_EQ(var->function(pAlt), varAlt->function(pAlt));
  }




  TEST_F(MetaVariableTest, daughterAngleInBetween)
  {
    StoreArray<Particle> particles;

    // make a 1 -> 3 particle

    TLorentzVector momentum_1(0, 0, 0, 0);
    std::vector<TLorentzVector> daughterMomenta_1;
    std::vector<int> daughterIndices_1;

    for (int i = 0; i < 3; i++) {
      TLorentzVector mom(i * 0.2, 1, 1, i * 1.0 + 2.0);
      Particle d(mom, (i % 2) ? -11 : 211);
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices_1.push_back(newDaughters->getArrayIndex());
      daughterMomenta_1.push_back(mom);
      momentum_1 = momentum_1 + mom;
    }

    const Particle* compositeDau_1 = particles.appendNew(momentum_1, 411, Particle::c_Flavored, daughterIndices_1);


    // make a 1 -> 2 particle

    TLorentzVector momentum_2(0, 0, 0, 0);
    std::vector<TLorentzVector> daughterMomenta_2;
    std::vector<int> daughterIndices_2;

    for (int i = 0; i < 2; i++) {
      TLorentzVector mom(1, 1, i * 0.3, i * 1.0 + 2.0);
      Particle d(mom, (i % 2) ? -11 : 211);
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices_2.push_back(newDaughters->getArrayIndex());
      daughterMomenta_2.push_back(mom);
      momentum_2 = momentum_2 + mom;
    }

    const Particle* compositeDau_2 = particles.appendNew(momentum_2, 411, Particle::c_Flavored, daughterIndices_2);


    // make the composite particle
    std::vector<int> daughterIndices = {compositeDau_1->getArrayIndex(), compositeDau_2->getArrayIndex()};
    const Particle* p = particles.appendNew(momentum_2 + momentum_1, 111, Particle::c_Unflavored, daughterIndices);


    // Test the invariant mass of several combinations
    const Manager::Var* var = Manager::Instance().getVariable("daughterAngleInBetween(0, 1)");
    double v_test = momentum_1.Vect().Angle(momentum_2.Vect());
    EXPECT_FLOAT_EQ(var->function(p), v_test);

    // this should be a generic combinations
    var = Manager::Instance().getVariable("daughterAngleInBetween(0:0, 1:0)");
    v_test = daughterMomenta_1[0].Vect().Angle(daughterMomenta_2[0].Vect());
    EXPECT_FLOAT_EQ(var->function(p), v_test);

    var = Manager::Instance().getVariable("daughterAngleInBetween( 1, -1)");
    EXPECT_B2WARNING(var->function(p));
    EXPECT_TRUE(std::isnan(var->function(p)));

    var = Manager::Instance().getVariable("daughterAngleInBetween(1, 0:1:0:0:1)");
    EXPECT_B2WARNING(var->function(p));
    EXPECT_TRUE(std::isnan(var->function(p)));

  }


  TEST_F(MetaVariableTest, isDescendantOfList)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreObjPtr<ParticleList> DList("D0:vartest");
    DList.registerInDataStore();
    DList.create();
    DList->initialize(421, "D0:vartest");
    StoreObjPtr<ParticleList> BList("B:vartest");
    BList.registerInDataStore();
    BList.create();
    BList->initialize(521, "B:vartest");
    DataStore::Instance().setInitializeActive(false);

    TLorentzVector momentum;
    TLorentzVector momentum_0;
    TLorentzVector momentum_1;
    StoreArray<Particle> particles;
    std::vector<int> D_daughterIndices;
    std::vector<int> D_grandDaughterIndices_0;
    std::vector<int> D_grandDaughterIndices_1;
    std::vector<int> B_daughterIndices;
    std::vector<int> B_grandDaughterIndices;
    std::vector<int> B_grandGrandDaughterIndices;

    // Creation of D decay: D->K0s(->pi pi) K0s(->pi pi)

    const Particle* D_gd_0_0 = particles.appendNew(TLorentzVector(0.0, 1, 1, 1), 211);
    const Particle* D_gd_0_1 = particles.appendNew(TLorentzVector(1.0, 1, 1, 1), -211);
    const Particle* D_gd_1_0 = particles.appendNew(TLorentzVector(2.0, 1, 1, 1), 211);
    const Particle* D_gd_1_1 = particles.appendNew(TLorentzVector(3.0, 1, 1, 1), -211);

    D_grandDaughterIndices_0.push_back(D_gd_0_0->getArrayIndex());
    D_grandDaughterIndices_0.push_back(D_gd_0_1->getArrayIndex());
    D_grandDaughterIndices_1.push_back(D_gd_1_0->getArrayIndex());
    D_grandDaughterIndices_1.push_back(D_gd_1_1->getArrayIndex());
    momentum_0 = D_gd_0_0->get4Vector() + D_gd_0_1->get4Vector();
    momentum_1 = D_gd_1_0->get4Vector() + D_gd_1_1->get4Vector();


    const Particle* D_d_0 = particles.appendNew(momentum_0, 310, Particle::c_Unflavored, D_grandDaughterIndices_0);
    const Particle* D_d_1 = particles.appendNew(momentum_1, 310, Particle::c_Unflavored, D_grandDaughterIndices_1);


    momentum = D_d_0->get4Vector() + D_d_1->get4Vector();
    D_daughterIndices.push_back(D_d_0->getArrayIndex());
    D_daughterIndices.push_back(D_d_1->getArrayIndex());

    const Particle* D_m = particles.appendNew(momentum, 421, Particle::c_Unflavored, D_daughterIndices);
    DList->addParticle(D_m);

    // Creation of B decay B -> D(->K0s(->pi pi) pi) pi

    const Particle* B_d_1 = particles.appendNew(TLorentzVector(0.0, 1, 1, 1), 211);
    const Particle* B_gd_0_1 = particles.appendNew(TLorentzVector(1.0, 1, 1, 1), -211);
    const Particle* B_ggd_0_0_0 = particles.appendNew(TLorentzVector(2.0, 1, 1, 1), 211);
    const Particle* B_ggd_0_0_1 = particles.appendNew(TLorentzVector(3.0, 1, 1, 1), -211);

    B_grandGrandDaughterIndices.push_back(B_ggd_0_0_0->getArrayIndex());
    B_grandGrandDaughterIndices.push_back(B_ggd_0_0_1->getArrayIndex());
    momentum_0 = B_ggd_0_0_0->get4Vector() + B_ggd_0_0_1->get4Vector();
    const Particle* B_gd_0_0 = particles.appendNew(momentum_0, 310, Particle::c_Unflavored, B_grandGrandDaughterIndices);

    B_grandDaughterIndices.push_back(B_gd_0_0->getArrayIndex());
    B_grandDaughterIndices.push_back(B_gd_0_1->getArrayIndex());
    momentum_1 = B_gd_0_0->get4Vector() + B_gd_0_1->get4Vector();
    const Particle* B_d_0 = particles.appendNew(momentum_1, -411, Particle::c_Unflavored, B_grandDaughterIndices);

    B_daughterIndices.push_back(B_d_0->getArrayIndex());
    B_daughterIndices.push_back(B_d_1->getArrayIndex());
    momentum = B_d_0->get4Vector() + B_d_1->get4Vector();
    const Particle* B_m = particles.appendNew(momentum, 521, Particle::c_Unflavored, B_daughterIndices);
    BList->addParticle(B_m);

    // Particle that is not an child
    const Particle* not_child = particles.appendNew(TLorentzVector(5.0, 1, 1, 1), 211);



    const Manager::Var* var_0 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest)");
    ASSERT_NE(var_0, nullptr);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_1_0), 1.);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_1_1), 1.);
    EXPECT_FLOAT_EQ(var_0->function(D_d_0), 1.);
    EXPECT_FLOAT_EQ(var_0->function(D_d_1), 1.);
    EXPECT_FLOAT_EQ(var_0->function(B_ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0->function(B_ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0->function(B_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0->function(B_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0->function(B_d_0), 0.);
    EXPECT_FLOAT_EQ(var_0->function(B_d_1), 0.);
    EXPECT_FLOAT_EQ(var_0->function(not_child), 0.);

    const Manager::Var* var_0a = Manager::Instance().getVariable("isDaughterOfList(D0:vartest)");
    ASSERT_NE(var_0a, nullptr);
    EXPECT_FLOAT_EQ(var_0a->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(D_d_0), 1.);
    EXPECT_FLOAT_EQ(var_0a->function(D_d_1), 1.);
    EXPECT_FLOAT_EQ(var_0a->function(B_ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(B_ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(B_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(B_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(B_d_0), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(B_d_1), 0.);
    EXPECT_FLOAT_EQ(var_0a->function(not_child), 0.);

    const Manager::Var* var_0b = Manager::Instance().getVariable("isGrandDaughterOfList(D0:vartest)");
    ASSERT_NE(var_0b, nullptr);
    EXPECT_FLOAT_EQ(var_0b->function(D_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_0b->function(D_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_0b->function(D_gd_1_0), 1.);
    EXPECT_FLOAT_EQ(var_0b->function(D_gd_1_1), 1.);
    EXPECT_FLOAT_EQ(var_0b->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(B_ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(B_ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(B_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(B_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(B_d_0), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(B_d_1), 0.);
    EXPECT_FLOAT_EQ(var_0b->function(not_child), 0.);

    const Manager::Var* var_1 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest, 1)");
    ASSERT_NE(var_1, nullptr);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_d_0), 1.);
    EXPECT_FLOAT_EQ(var_1->function(D_d_1), 1.);
    EXPECT_FLOAT_EQ(var_1->function(B_ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(B_ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(B_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(B_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(B_d_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(B_d_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(not_child), 0.);

    const Manager::Var* var_2 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest, 2)");
    ASSERT_NE(var_2, nullptr);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_1_0), 1.);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_1_1), 1.);
    EXPECT_FLOAT_EQ(var_2->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_2->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_2->function(B_ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_2->function(B_ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_2->function(B_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_2->function(B_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_2->function(B_d_0), 0.);
    EXPECT_FLOAT_EQ(var_2->function(B_d_1), 0.);
    EXPECT_FLOAT_EQ(var_2->function(not_child), 0.);

    const Manager::Var* var_3 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest, B:vartest)");
    ASSERT_NE(var_3, nullptr);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_1_0), 1.);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_1_1), 1.);
    EXPECT_FLOAT_EQ(var_3->function(D_d_0), 1.);
    EXPECT_FLOAT_EQ(var_3->function(D_d_1), 1.);
    EXPECT_FLOAT_EQ(var_3->function(B_ggd_0_0_0), 1.);
    EXPECT_FLOAT_EQ(var_3->function(B_ggd_0_0_1), 1.);
    EXPECT_FLOAT_EQ(var_3->function(B_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_3->function(B_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_3->function(B_d_0), 1.);
    EXPECT_FLOAT_EQ(var_3->function(B_d_1), 1.);
    EXPECT_FLOAT_EQ(var_3->function(not_child), 0.);

    const Manager::Var* var_4 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest, B:vartest, -1)");
    ASSERT_NE(var_4, nullptr);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_1_0), 1.);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_1_1), 1.);
    EXPECT_FLOAT_EQ(var_4->function(D_d_0), 1.);
    EXPECT_FLOAT_EQ(var_4->function(D_d_1), 1.);
    EXPECT_FLOAT_EQ(var_4->function(B_ggd_0_0_0), 1.);
    EXPECT_FLOAT_EQ(var_4->function(B_ggd_0_0_1), 1.);
    EXPECT_FLOAT_EQ(var_4->function(B_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_4->function(B_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_4->function(B_d_0), 1.);
    EXPECT_FLOAT_EQ(var_4->function(B_d_1), 1.);
    EXPECT_FLOAT_EQ(var_4->function(not_child), 0.);


    const Manager::Var* var_5 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest, B:vartest, 1)");
    ASSERT_NE(var_5, nullptr);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_d_0), 1.);
    EXPECT_FLOAT_EQ(var_5->function(D_d_1), 1.);
    EXPECT_FLOAT_EQ(var_5->function(B_ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(B_ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(B_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(B_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(B_d_0), 1.);
    EXPECT_FLOAT_EQ(var_5->function(B_d_1), 1.);
    EXPECT_FLOAT_EQ(var_5->function(not_child), 0.);


    const Manager::Var* var_6 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest, B:vartest, 2)");
    ASSERT_NE(var_6, nullptr);
    EXPECT_FLOAT_EQ(var_6->function(D_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_6->function(D_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_6->function(D_gd_1_0), 1.);
    EXPECT_FLOAT_EQ(var_6->function(D_gd_1_1), 1.);
    EXPECT_FLOAT_EQ(var_6->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_6->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_6->function(B_ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_6->function(B_ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_6->function(B_gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_6->function(B_gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_6->function(B_d_0), 0.);
    EXPECT_FLOAT_EQ(var_6->function(B_d_1), 0.);
    EXPECT_FLOAT_EQ(var_6->function(not_child), 0.);

    const Manager::Var* var_7 = Manager::Instance().getVariable("isDescendantOfList(D0:vartest, B:vartest, 3)");
    ASSERT_NE(var_7, nullptr);
    EXPECT_FLOAT_EQ(var_7->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_7->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_7->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_7->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_7->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_7->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_7->function(B_ggd_0_0_0), 1.);
    EXPECT_FLOAT_EQ(var_7->function(B_ggd_0_0_1), 1.);
    EXPECT_FLOAT_EQ(var_7->function(B_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_7->function(B_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_7->function(B_d_0), 0.);
    EXPECT_FLOAT_EQ(var_7->function(B_d_1), 0.);
    EXPECT_FLOAT_EQ(var_7->function(not_child), 0.);
  }


  TEST_F(MetaVariableTest, isMCDescendantOfList)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    mcParticles.registerInDataStore();
    particles.registerRelationTo(mcParticles);
    StoreObjPtr<ParticleList> BList("B:vartest");
    BList.registerInDataStore();
    BList.create();
    BList->initialize(521, "B:vartest");
    StoreObjPtr<ParticleList> DList("D0:vartest");
    DList.registerInDataStore();
    DList.create();
    DList->initialize(421, "D0:vartest");
    DataStore::Instance().setInitializeActive(false);
    TLorentzVector momentum;
    TLorentzVector momentum_0;
    TLorentzVector momentum_1;
    std::vector<int> daughterIndices;
    std::vector<int> grandDaughterIndices;
    std::vector<int> grandGrandDaughterIndices;
    std::vector<int> D_daughterIndices;
    std::vector<int> D_grandDaughterIndices_0;
    std::vector<int> D_grandDaughterIndices_1;


    // Create MC graph for B+ -> (D -> (K0s -> pi+ + pi-) pi-)  + pi+
    MCParticleGraph mcGraph;

    MCParticleGraph::GraphParticle& mcg_m = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& mcg_d_0 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& mcg_d_1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& mcg_gd_0_0 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& mcg_gd_0_1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& mcg_ggd_0_0_0 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& mcg_ggd_0_0_1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& mcg_not_child = mcGraph.addParticle();

    mcg_m.setPDG(521);
    mcg_d_0.setPDG(-411);
    mcg_d_1.setPDG(211);
    mcg_gd_0_0.setPDG(310);
    mcg_gd_0_1.setPDG(-211);
    mcg_ggd_0_0_0.setPDG(211);
    mcg_ggd_0_0_1.setPDG(-211);
    mcg_not_child.setPDG(211);

    mcg_d_0.comesFrom(mcg_m);
    mcg_d_1.comesFrom(mcg_m);
    mcg_gd_0_0.comesFrom(mcg_d_0);
    mcg_gd_0_1.comesFrom(mcg_d_0);
    mcg_ggd_0_0_0.comesFrom(mcg_gd_0_1);
    mcg_ggd_0_0_1.comesFrom(mcg_gd_0_1);

    mcGraph.generateList();

    // Get MC Particles from StoreArray
    auto* mc_m = mcParticles[0];
    auto* mc_d_0 = mcParticles[1];
    auto* mc_d_1 = mcParticles[2];
    auto* mc_gd_0_0 = mcParticles[3];
    auto* mc_gd_0_1 = mcParticles[4];
    auto* mc_ggd_0_0_0 = mcParticles[5];
    auto* mc_ggd_0_0_1 = mcParticles[6];
    auto* mc_not_child = mcParticles[7];

    mc_m->setStatus(MCParticle::c_PrimaryParticle);
    mc_d_0->setStatus(MCParticle::c_PrimaryParticle);
    mc_d_1->setStatus(MCParticle::c_PrimaryParticle);
    mc_gd_0_0->setStatus(MCParticle::c_PrimaryParticle);
    mc_gd_0_1->setStatus(MCParticle::c_PrimaryParticle);
    mc_ggd_0_0_0->setStatus(MCParticle::c_PrimaryParticle);
    mc_ggd_0_0_1->setStatus(MCParticle::c_PrimaryParticle);
    mc_not_child->setStatus(MCParticle::c_PrimaryParticle);

    // Creation of D decay: D->K0s(->pi pi) K0s(->pi pi) (not matched)

    const Particle* D_gd_0_0 = particles.appendNew(TLorentzVector(0.0, 1, 1, 1), 211);
    const Particle* D_gd_0_1 = particles.appendNew(TLorentzVector(1.0, 1, 1, 1), -211);
    const Particle* D_gd_1_0 = particles.appendNew(TLorentzVector(2.0, 1, 1, 1), 211);
    const Particle* D_gd_1_1 = particles.appendNew(TLorentzVector(3.0, 1, 1, 1), -211);

    D_grandDaughterIndices_0.push_back(D_gd_0_0->getArrayIndex());
    D_grandDaughterIndices_0.push_back(D_gd_0_1->getArrayIndex());
    D_grandDaughterIndices_1.push_back(D_gd_1_0->getArrayIndex());
    D_grandDaughterIndices_1.push_back(D_gd_1_1->getArrayIndex());
    momentum_0 = D_gd_0_0->get4Vector() + D_gd_0_1->get4Vector();
    momentum_1 = D_gd_1_0->get4Vector() + D_gd_1_1->get4Vector();


    const Particle* D_d_0 = particles.appendNew(momentum_0, 310, Particle::c_Unflavored, D_grandDaughterIndices_0);
    const Particle* D_d_1 = particles.appendNew(momentum_1, 310, Particle::c_Unflavored, D_grandDaughterIndices_1);


    momentum = D_d_0->get4Vector() + D_d_1->get4Vector();
    D_daughterIndices.push_back(D_d_0->getArrayIndex());
    D_daughterIndices.push_back(D_d_1->getArrayIndex());

    const Particle* D_m = particles.appendNew(momentum, 421, Particle::c_Unflavored, D_daughterIndices);
    DList->addParticle(D_m);

    // Creating B decay
    const Particle* d_1 = particles.appendNew(TLorentzVector(0.0, 1, 1, 1), 211);
    const Particle* gd_0_1 = particles.appendNew(TLorentzVector(1.0, 1, 1, 1), -211);
    const Particle* ggd_0_0_0 = particles.appendNew(TLorentzVector(2.0, 1, 1, 1), 211);
    const Particle* ggd_0_0_1 = particles.appendNew(TLorentzVector(3.0, 1, 1, 1), -211);

    grandGrandDaughterIndices.push_back(ggd_0_0_0->getArrayIndex());
    grandGrandDaughterIndices.push_back(ggd_0_0_1->getArrayIndex());
    momentum_0 = ggd_0_0_0->get4Vector() + ggd_0_0_1->get4Vector();
    const Particle* gd_0_0 = particles.appendNew(momentum_0, 310, Particle::c_Unflavored, grandGrandDaughterIndices);

    grandDaughterIndices.push_back(gd_0_0->getArrayIndex());
    grandDaughterIndices.push_back(gd_0_1->getArrayIndex());
    momentum_1 = gd_0_0->get4Vector() + gd_0_1->get4Vector();
    const Particle* d_0 = particles.appendNew(momentum_1, -411, Particle::c_Unflavored, grandDaughterIndices);

    daughterIndices.push_back(d_0->getArrayIndex());
    daughterIndices.push_back(d_1->getArrayIndex());
    momentum = d_0->get4Vector() + d_1->get4Vector();
    const Particle* m = particles.appendNew(momentum, 521, Particle::c_Unflavored, daughterIndices);
    BList->addParticle(m);

    // Particle that is not an child
    const Particle* not_child = particles.appendNew(TLorentzVector(5.0, 1, 1, 1), 211);

    // Particle that is not an child and doesn't have MC particle
    const Particle* not_child_2 = particles.appendNew(TLorentzVector(6.0, 1, 1, 1), 211);

    gd_0_0->addRelationTo(mc_gd_0_0);
    gd_0_1->addRelationTo(mc_gd_0_1);
    ggd_0_0_0->addRelationTo(mc_ggd_0_0_0);
    ggd_0_0_1->addRelationTo(mc_ggd_0_0_1);
    d_0->addRelationTo(mc_d_0);
    d_1->addRelationTo(mc_d_1);
    m->addRelationTo(mc_m);
    not_child->addRelationTo(mc_not_child);

    const Manager::Var* var_0 = Manager::Instance().getVariable("isMCDescendantOfList(B:vartest)");
    ASSERT_NE(var_0, nullptr);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_0->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_0->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_0->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_0->function(ggd_0_0_0), 1.);
    EXPECT_FLOAT_EQ(var_0->function(ggd_0_0_1), 1.);
    EXPECT_FLOAT_EQ(var_0->function(gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_0->function(gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_0->function(d_0), 1.);
    EXPECT_FLOAT_EQ(var_0->function(d_1), 1.);
    EXPECT_FLOAT_EQ(var_0->function(not_child), 0.);
    EXPECT_FLOAT_EQ(var_0->function(not_child_2), 0.);

    const Manager::Var* var_1 = Manager::Instance().getVariable("isMCDescendantOfList(B:vartest, D0:vartest)");
    ASSERT_NE(var_1, nullptr);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_1->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_1->function(ggd_0_0_0), 1.);
    EXPECT_FLOAT_EQ(var_1->function(ggd_0_0_1), 1.);
    EXPECT_FLOAT_EQ(var_1->function(gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_1->function(gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_1->function(d_0), 1.);
    EXPECT_FLOAT_EQ(var_1->function(d_1), 1.);
    EXPECT_FLOAT_EQ(var_1->function(not_child), 0.);
    EXPECT_FLOAT_EQ(var_1->function(not_child_2), 0.);

    const Manager::Var* var_2 = Manager::Instance().getVariable("isMCDescendantOfList(B:vartest, -1)");
    ASSERT_NE(var_2, nullptr);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_2->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_2->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_2->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_2->function(ggd_0_0_0), 1.);
    EXPECT_FLOAT_EQ(var_2->function(ggd_0_0_1), 1.);
    EXPECT_FLOAT_EQ(var_2->function(gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_2->function(gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_2->function(d_0), 1.);
    EXPECT_FLOAT_EQ(var_2->function(d_1), 1.);
    EXPECT_FLOAT_EQ(var_2->function(not_child), 0.);
    EXPECT_FLOAT_EQ(var_2->function(not_child_2), 0.);

    const Manager::Var* var_3 = Manager::Instance().getVariable("isMCDescendantOfList(B:vartest, 1)");
    ASSERT_NE(var_3, nullptr);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_3->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_3->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_3->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_3->function(ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_3->function(ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_3->function(gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_3->function(gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_3->function(d_0), 1.);
    EXPECT_FLOAT_EQ(var_3->function(d_1), 1.);
    EXPECT_FLOAT_EQ(var_3->function(not_child), 0.);
    EXPECT_FLOAT_EQ(var_3->function(not_child_2), 0.);

    const Manager::Var* var_4 = Manager::Instance().getVariable("isMCDescendantOfList(B:vartest, 2)");
    ASSERT_NE(var_4, nullptr);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_4->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_4->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_4->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_4->function(ggd_0_0_0), 0.);
    EXPECT_FLOAT_EQ(var_4->function(ggd_0_0_1), 0.);
    EXPECT_FLOAT_EQ(var_4->function(gd_0_0), 1.);
    EXPECT_FLOAT_EQ(var_4->function(gd_0_1), 1.);
    EXPECT_FLOAT_EQ(var_4->function(d_0), 0.);
    EXPECT_FLOAT_EQ(var_4->function(d_1), 0.);
    EXPECT_FLOAT_EQ(var_4->function(not_child), 0.);
    EXPECT_FLOAT_EQ(var_4->function(not_child_2), 0.);


    const Manager::Var* var_5 = Manager::Instance().getVariable("isMCDescendantOfList(B:vartest, 3)");
    ASSERT_NE(var_5, nullptr);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_1_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_gd_1_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_d_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(D_d_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(ggd_0_0_0), 1.);
    EXPECT_FLOAT_EQ(var_5->function(ggd_0_0_1), 1.);
    EXPECT_FLOAT_EQ(var_5->function(gd_0_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(gd_0_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(d_0), 0.);
    EXPECT_FLOAT_EQ(var_5->function(d_1), 0.);
    EXPECT_FLOAT_EQ(var_5->function(not_child), 0.);
    EXPECT_FLOAT_EQ(var_5->function(not_child_2), 0.);
  }





  class PIDVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap> peim;
      StoreArray<TrackFitResult> tfrs;
      StoreArray<MCParticle> mcparticles;
      StoreArray<PIDLikelihood> likelihood;
      StoreArray<Particle> particles;
      StoreArray<Track> tracks;
      peim.registerInDataStore();
      tfrs.registerInDataStore();
      mcparticles.registerInDataStore();
      likelihood.registerInDataStore();
      particles.registerInDataStore();
      tracks.registerInDataStore();
      particles.registerRelationTo(likelihood);
      tracks.registerRelationTo(likelihood);
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(PIDVariableTest, LogLikelihood)
  {
    StoreArray<PIDLikelihood> likelihood;
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> tfrs;

    // create tracks and trackFitResutls
    TRandom3 generator;
    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);
    // Generate a random put orthogonal pair of vectors in the r-phi plane
    TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
    // Add a random z component
    TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
    TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);
    tfrs.appendNew(position, momentum, cov6, charge, Const::electron, pValue, bField, CDCValue, 16777215);
    Track mytrack;
    mytrack.setTrackFitResultIndex(Const::electron, 0);
    Track* allTrack = tracks.appendNew(mytrack);
    Track* noPIDTrack = tracks.appendNew(mytrack);
    Track* dEdxTrack = tracks.appendNew(mytrack);

    // Fill by hand likelihood values for all the detectors and hypothesis
    // This is clearly not a phyisical case, since a particle cannot leave good
    // signals in both TOP and ARICH
    auto* lAll = likelihood.appendNew();
    lAll->setLogLikelihood(Const::TOP, Const::electron, 0.18);
    lAll->setLogLikelihood(Const::ARICH, Const::electron, 0.16);
    lAll->setLogLikelihood(Const::ECL, Const::electron, 0.14);
    lAll->setLogLikelihood(Const::CDC, Const::electron, 0.12);
    lAll->setLogLikelihood(Const::SVD, Const::electron, 0.1);

    lAll->setLogLikelihood(Const::TOP, Const::pion, 0.2);
    lAll->setLogLikelihood(Const::ARICH, Const::pion, 0.22);
    lAll->setLogLikelihood(Const::ECL, Const::pion, 0.24);
    lAll->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    lAll->setLogLikelihood(Const::SVD, Const::pion, 0.28);

    lAll->setLogLikelihood(Const::TOP, Const::kaon, 0.3);
    lAll->setLogLikelihood(Const::ARICH, Const::kaon, 0.32);
    lAll->setLogLikelihood(Const::ECL, Const::kaon, 0.34);
    lAll->setLogLikelihood(Const::CDC, Const::kaon, 0.36);
    lAll->setLogLikelihood(Const::SVD, Const::kaon, 0.38);

    lAll->setLogLikelihood(Const::TOP, Const::proton, 0.4);
    lAll->setLogLikelihood(Const::ARICH, Const::proton, 0.42);
    lAll->setLogLikelihood(Const::ECL, Const::proton, 0.44);
    lAll->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    lAll->setLogLikelihood(Const::SVD, Const::proton, 0.48);

    lAll->setLogLikelihood(Const::TOP, Const::muon, 0.5);
    lAll->setLogLikelihood(Const::ARICH, Const::muon, 0.52);
    lAll->setLogLikelihood(Const::ECL, Const::muon, 0.54);
    lAll->setLogLikelihood(Const::CDC, Const::muon, 0.56);
    lAll->setLogLikelihood(Const::SVD, Const::muon, 0.58);

    lAll->setLogLikelihood(Const::TOP, Const::deuteron, 0.6);
    lAll->setLogLikelihood(Const::ARICH, Const::deuteron, 0.62);
    lAll->setLogLikelihood(Const::ECL, Const::deuteron, 0.64);
    lAll->setLogLikelihood(Const::CDC, Const::deuteron, 0.66);
    lAll->setLogLikelihood(Const::SVD, Const::deuteron, 0.68);


    // Likelihoods for a dEdx only case
    auto* ldEdx = likelihood.appendNew();
    ldEdx->setLogLikelihood(Const::CDC, Const::electron, 0.12);
    ldEdx->setLogLikelihood(Const::SVD, Const::electron, 0.1);

    ldEdx->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    ldEdx->setLogLikelihood(Const::SVD, Const::pion, 0.28);

    ldEdx->setLogLikelihood(Const::CDC, Const::kaon, 0.36);
    ldEdx->setLogLikelihood(Const::SVD, Const::kaon, 0.38);

    ldEdx->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    ldEdx->setLogLikelihood(Const::SVD, Const::proton, 0.48);

    ldEdx->setLogLikelihood(Const::CDC, Const::muon, 0.56);
    ldEdx->setLogLikelihood(Const::SVD, Const::muon, 0.58);

    ldEdx->setLogLikelihood(Const::CDC, Const::deuteron, 0.66);
    ldEdx->setLogLikelihood(Const::SVD, Const::deuteron, 0.68);


    allTrack->addRelationTo(lAll);
    dEdxTrack->addRelationTo(ldEdx);

    // Table with the sum(LogL) for several cases
    //      All  dEdx
    // e    0.7  0.22
    // mu   2.7  1.14
    // pi   1.2  0.54
    // k    1.7  0.74
    // p    2.2  0.94
    // d    3.2  1.34

    auto* particleAll = particles.appendNew(allTrack, Const::pion);
    auto* particledEdx = particles.appendNew(dEdxTrack, Const::pion);
    auto* particleNoID = particles.appendNew(noPIDTrack, Const::pion);

    double numsumexp = std::exp(0.7) + std::exp(2.7) + std::exp(1.2) + std::exp(1.7) + std::exp(2.2) + std::exp(3.2);

    // Basic PID quantities. Currently just wrappers for global probability.
    EXPECT_FLOAT_EQ(electronID(particleAll), std::exp(0.7) / numsumexp);
    EXPECT_FLOAT_EQ(muonID(particleAll),     std::exp(2.7) / numsumexp);
    EXPECT_FLOAT_EQ(pionID(particleAll),     std::exp(1.2) / numsumexp);
    EXPECT_FLOAT_EQ(kaonID(particleAll),     std::exp(1.7) / numsumexp);
    EXPECT_FLOAT_EQ(protonID(particleAll),   std::exp(2.2) / numsumexp);
    EXPECT_FLOAT_EQ(deuteronID(particleAll), std::exp(3.2) / numsumexp);

    // smart PID that takes the hypothesis into account
    auto* particleMuonAll = particles.appendNew(allTrack, Const::muon);
    auto* particleKaonAll = particles.appendNew(allTrack, Const::kaon);
    auto* particleElectronAll = particles.appendNew(allTrack, Const::electron);
    auto* particleProtonAll = particles.appendNew(allTrack, Const::proton);
    auto* particleDeuteronAll = particles.appendNew(allTrack, Const::deuteron);
    EXPECT_FLOAT_EQ(particleID(particleAll), std::exp(1.2) / numsumexp); // there's already a pion
    EXPECT_FLOAT_EQ(particleID(particleMuonAll), std::exp(2.7) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleKaonAll), std::exp(1.7) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleElectronAll), std::exp(0.7) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleProtonAll),   std::exp(2.2) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleDeuteronAll), std::exp(3.2) / numsumexp);

    // Check what hapens if no Likelihood is available
    EXPECT_TRUE(std::isnan(electronID(particleNoID)));
    EXPECT_TRUE(std::isnan(muonID(particleNoID)));
    EXPECT_TRUE(std::isnan(pionID(particleNoID)));
    EXPECT_TRUE(std::isnan(kaonID(particleNoID)));
    EXPECT_TRUE(std::isnan(protonID(particleNoID)));
    EXPECT_TRUE(std::isnan(deuteronID(particleNoID)));

    //expert stuff: LogL values
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, TOP)")->function(particleAll), 0.18);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, ALL)")->function(particleAll), 0.70);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(2212, TOP, CDC)")->function(particleAll), 0.86);

    // global probability
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(1000010020, ALL)")->function(particleAll),
                    std::exp(3.2) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(2212, ALL)")->function(particleAll),
                    std::exp(2.2) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(particleAll),
                    std::exp(1.2) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(321, ALL)")->function(particleAll),
                    std::exp(1.7) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(13, ALL)")->function(particleAll),
                    std::exp(2.7) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(11, ALL)")->function(particleAll),
                    std::exp(0.7) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(particledEdx),
                    std::exp(0.54) / (std::exp(0.22) + std::exp(1.14) + std::exp(0.54) + std::exp(0.74) + std::exp(0.94) + std::exp(1.34)));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(particledEdx),
                    Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, SVD)")->function(particleAll));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, CDC)")->function(particledEdx),
                    Manager::Instance().getVariable("pidProbabilityExpert(211, CDC)")->function(particleAll));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(321, CDC)")->function(particleAll),
                    std::exp(0.36) / (std::exp(0.12) + std::exp(0.26) + std::exp(0.36) + std::exp(0.46) + std::exp(0.56) + std::exp(0.66)));

    // binary probability
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ALL)")->function(particleAll),
                    1.0 / (1.0 + std::exp(2.2 - 1.7)));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ALL)")->function(particledEdx),
                    1.0 / (1.0 + std::exp(0.94 - 0.74)));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, SVD)")->function(particleAll),
                    1.0 / (1.0 + std::exp(0.94 - 0.74)));

    // No likelihood available
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, KLM)")->function(particledEdx)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, TOP, CDC, SVD)")->function(particleNoID)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, TOP)")->function(particledEdx)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, KLM)")->function(particledEdx)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ECL, TOP, ARICH)")->function(
                             particledEdx)));
    EXPECT_FALSE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ECL, TOP, ARICH, SVD)")->function(
                              particledEdx)));
    //Mostlikely PDG tests:
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG()")->function(particledEdx), 1.00001e+09);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG(0.5, 0.1, 0.1, 0.1, 0.1, 0.1)")->function(particledEdx), 11);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG(0.1, 0.5, 0.1, 0.1, 0.1, 0.1)")->function(particledEdx), 11);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG(0.1, 0.1, 0.5, 0.1, 0.1, 0.1)")->function(particledEdx), 11);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG(0.1, 0.1, 0.1, 0.5, 0.1, 0.1)")->function(particledEdx), 11);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG(0.1, 0.1, 0.1, 0.1, 0.5, 0.1)")->function(particledEdx), 11);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG(0.1, 0.1, 0.1, 0.1, 0.1, 0.5)")->function(particledEdx), 11);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidMostLikelyPDG(0, 1., 0, 0, 0, 0)")->function(particledEdx), 13);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidIsMostLikely(0.5,0.1,0.1,0.1,0.1,0.1)")->function(particleElectronAll), 1.0);
  }

  TEST_F(PIDVariableTest, MissingLikelihood)
  {
    StoreArray<PIDLikelihood> likelihood;
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> tfrs;

    // create tracks and trackFitResutls
    TRandom3 generator;
    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);
    // Generate a random put orthogonal pair of vectors in the r-phi plane
    TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
    // Add a random z component
    TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
    TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);
    tfrs.appendNew(position, momentum, cov6, charge, Const::electron, pValue, bField, CDCValue, 16777215);
    Track mytrack;
    mytrack.setTrackFitResultIndex(Const::electron, 0);
    Track* savedTrack1 = tracks.appendNew(mytrack);
    Track* savedTrack2 = tracks.appendNew(mytrack);
    Track* savedTrack3 = tracks.appendNew(mytrack);
    Track* savedTrack4 = tracks.appendNew(mytrack);

    auto* l1 = likelihood.appendNew();
    l1->setLogLikelihood(Const::TOP, Const::electron, 0.18);
    l1->setLogLikelihood(Const::ECL, Const::electron, 0.14);
    savedTrack1->addRelationTo(l1);

    auto* electron = particles.appendNew(savedTrack1, Const::electron);

    auto* l2 = likelihood.appendNew();
    l2->setLogLikelihood(Const::TOP, Const::pion, 0.2);
    l2->setLogLikelihood(Const::ARICH, Const::pion, 0.22);
    l2->setLogLikelihood(Const::ECL, Const::pion, 0.24);
    l2->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    l2->setLogLikelihood(Const::SVD, Const::pion, 0.28);
    savedTrack2->addRelationTo(l2);

    auto* pion = particles.appendNew(savedTrack2, Const::pion);

    auto* l3 = likelihood.appendNew();
    l3->setLogLikelihood(Const::TOP, Const::kaon, 0.3);
    l3->setLogLikelihood(Const::ARICH, Const::kaon, 0.32);
    savedTrack3->addRelationTo(l3);

    auto* kaon = particles.appendNew(savedTrack3, Const::kaon);

    auto* l4 = likelihood.appendNew();
    l4->setLogLikelihood(Const::ARICH, Const::proton, 0.42);
    l4->setLogLikelihood(Const::ECL, Const::proton, 0.44);
    l4->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    l4->setLogLikelihood(Const::SVD, Const::proton, 0.48);
    savedTrack4->addRelationTo(l4);

    auto* proton = particles.appendNew(savedTrack4, Const::proton);

    const Manager::Var* varMissECL = Manager::Instance().getVariable("pidMissingProbabilityExpert(ECL)");
    const Manager::Var* varMissTOP = Manager::Instance().getVariable("pidMissingProbabilityExpert(TOP)");
    const Manager::Var* varMissARICH = Manager::Instance().getVariable("pidMissingProbabilityExpert(ARICH)");


    EXPECT_FLOAT_EQ(varMissTOP->function(electron), 0.0);
    EXPECT_FLOAT_EQ(varMissTOP->function(pion), 0.0);
    EXPECT_FLOAT_EQ(varMissTOP->function(kaon), 0.0);
    EXPECT_FLOAT_EQ(varMissTOP->function(proton), 1.0);

    EXPECT_FLOAT_EQ(varMissARICH->function(electron), 1.0);
    EXPECT_FLOAT_EQ(varMissARICH->function(pion), 0.0);
    EXPECT_FLOAT_EQ(varMissARICH->function(kaon), 0.0);
    EXPECT_FLOAT_EQ(varMissARICH->function(proton), 0.0);

    EXPECT_FLOAT_EQ(varMissECL->function(electron), 0.0);
    EXPECT_FLOAT_EQ(varMissECL->function(pion), 0.0);
    EXPECT_FLOAT_EQ(varMissECL->function(kaon), 1.0);
    EXPECT_FLOAT_EQ(varMissECL->function(proton), 0.0);
  }

  class FlightInfoTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      StoreArray<MCParticle> mcParticles;
      StoreArray<Particle> particles;
      particles.registerRelationTo(mcParticles);
      StoreObjPtr<ParticleExtraInfoMap>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);


      // Insert MC particle logic here
      MCParticle mcKs;
      mcKs.setPDG(310);
      mcKs.setProductionVertex(1.0, 1.0, 0.0);
      mcKs.setDecayVertex(4.0, 5.0, 0.0);
      mcKs.setProductionTime(0);
      mcKs.setMassFromPDG();
      mcKs.setMomentum(1.164, 1.55200, 0);
      float decayTime = 5 * mcKs.getMass() / mcKs.getEnergy();
      mcKs.setDecayTime(decayTime);
      mcKs.setStatus(MCParticle::c_PrimaryParticle);
      MCParticle* newMCKs = mcParticles.appendNew(mcKs);



      MCParticle mcDp;
      mcDp.setPDG(411);
      mcDp.setDecayVertex(1.0, 1.0, 0.0);
      mcDp.setMassFromPDG();
      mcDp.setStatus(MCParticle::c_PrimaryParticle);
      MCParticle* newMCDp = mcParticles.appendNew(mcDp);

      // Insert Reco particle logic here
      TLorentzVector momentum;
      TMatrixFSym error(7);
      error.Zero();
      error(0, 0) = 0.05;
      error(1, 1) = 0.2;
      error(2, 2) = 0.4;
      error(3, 3) = 0.01;
      error(4, 4) = 0.04;
      error(5, 5) = 0.00875;
      error(6, 6) = 0.01;
      Particle pi(TLorentzVector(1.59607, 1.19705, 0, 2), 211);
      momentum += pi.get4Vector();
      Particle* newpi = particles.appendNew(pi);


      Particle Ks(TLorentzVector(1.164, 1.55200, 0, 2), 310, Particle::c_Unflavored, Particle::c_Composite, 0);
      Ks.setVertex(TVector3(4.0, 5.0, 0.0));
      Ks.setMomentumVertexErrorMatrix(error);   // (order: px,py,pz,E,x,y,z)
      momentum += Ks.get4Vector();
      Ks.addExtraInfo("prodVertX", 1.0);
      Ks.addExtraInfo("prodVertY", 1.0);
      Ks.addExtraInfo("prodVertZ", 0.0);
      Ks.addExtraInfo("prodVertSxx", 0.04);
      Ks.addExtraInfo("prodVertSxy", 0.0);
      Ks.addExtraInfo("prodVertSxz", 0.0);
      Ks.addExtraInfo("prodVertSyx", 0.0);
      Ks.addExtraInfo("prodVertSyy", 0.00875);
      Ks.addExtraInfo("prodVertSyz", 0.0);
      Ks.addExtraInfo("prodVertSzx", 0.0);
      Ks.addExtraInfo("prodVertSzy", 0.0);
      Ks.addExtraInfo("prodVertSzz", 0.01);
      Particle* newKs = particles.appendNew(Ks);
      newKs->addRelationTo(newMCKs);


      Particle Dp(momentum, 411, Particle::c_Flavored, Particle::c_Composite, 0);
      Dp.appendDaughter(newpi);
      Dp.appendDaughter(newKs);
      TVector3 motherVtx(1.0, 1.0, 0.0);
      Dp.setVertex(motherVtx);
      Dp.setMomentumVertexErrorMatrix(error);   // (order: px,py,pz,E,x,y,z)
      Dp.addExtraInfo("prodVertX", 0.0);
      Dp.addExtraInfo("prodVertY", 1.0);
      Dp.addExtraInfo("prodVertZ", -2.0);
      Dp.addExtraInfo("prodVertSxx", 0.04);
      Dp.addExtraInfo("prodVertSxy", 0.0);
      Dp.addExtraInfo("prodVertSxz", 0.0);
      Dp.addExtraInfo("prodVertSyx", 0.0);
      Dp.addExtraInfo("prodVertSyy", 0.01);
      Dp.addExtraInfo("prodVertSyz", 0.0);
      Dp.addExtraInfo("prodVertSzx", 0.0);
      Dp.addExtraInfo("prodVertSzy", 0.0);
      Dp.addExtraInfo("prodVertSzz", 0.1575);
      Particle* newDp = particles.appendNew(Dp);
      newDp->addRelationTo(newMCDp);

    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };
  TEST_F(FlightInfoTest, flightDistance)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("flightDistance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 5.0);
  }
  TEST_F(FlightInfoTest, flightDistanceErr)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("flightDistanceErr");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newKs), 0.0);
  }
  TEST_F(FlightInfoTest, flightTime)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks had flight time of 0.0427 us (t = d/c * m/p)

    const Manager::Var* var = Manager::Instance().getVariable("flightTime");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 5.0 / Const::speedOfLight * newKs->getPDGMass() / newKs->getP());
  }

  TEST_F(FlightInfoTest, flightTimeErr)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks should have positive flight distance uncertainty

    const Manager::Var* var = Manager::Instance().getVariable("flightTimeErr");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newKs), 0.0);
  }


  TEST_F(FlightInfoTest, flightDistanceOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("flightDistanceOfDaughter(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 5.0);

    var = Manager::Instance().getVariable("flightDistanceOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(newDp)));
  }
  TEST_F(FlightInfoTest, flightDistanceOfDaughterErr)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks should have positive flight distance uncertainty

    const Manager::Var* var = Manager::Instance().getVariable("flightDistanceOfDaughterErr(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newDp), 0.0);

    var = Manager::Instance().getVariable("flightDistanceOfDaughterErr(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(newDp)));
  }
  TEST_F(FlightInfoTest, flightTimeOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight time of 0.0427 us (t = d/c * m/p)

    const Manager::Var* var = Manager::Instance().getVariable("flightTimeOfDaughter(1)");
    ASSERT_NE(var, nullptr);
    const Particle* Ks = newDp->getDaughter(1);

    EXPECT_FLOAT_EQ(var->function(newDp), 5.0 / Const::speedOfLight * Ks->getPDGMass() / Ks->getP());

    var = Manager::Instance().getVariable("flightTimeOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(newDp)));
  }
  TEST_F(FlightInfoTest, flightTimeOfDaughterErr)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks should have positive flight time uncertainty

    const Manager::Var* var = Manager::Instance().getVariable("flightTimeOfDaughterErr(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newDp), 0.0);

    var = Manager::Instance().getVariable("flightTimeOfDaughterErr(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(newDp)));
  }
  TEST_F(FlightInfoTest, mcFlightDistanceOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcFlightDistanceOfDaughter(1)");
    ASSERT_NE(var, nullptr);

    EXPECT_FLOAT_EQ(var->function(newDp), 5.0);

    var = Manager::Instance().getVariable("mcFlightDistanceOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(newDp)));
  }
  TEST_F(FlightInfoTest, mcFlightTimeOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight time of 0.0427 us (t = d/c * m/p)

    const Manager::Var* var = Manager::Instance().getVariable("mcFlightTimeOfDaughter(1)");
    ASSERT_NE(var, nullptr);
    auto* Ks = newDp->getDaughter(1)->getRelatedTo<MCParticle>();
    //    double p = Ks->getMomentum().Mag();
    //    EXPECT_FLOAT_EQ(var->function(newDp), 5.0 / Const::speedOfLight * Ks->getMass() / p);

    EXPECT_FLOAT_EQ(var->function(newDp), Ks->getLifetime() / Ks->getEnergy()*Ks->getMass());

    var = Manager::Instance().getVariable("mcFlightTimeOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(newDp)));
  }

  TEST_F(FlightInfoTest, vertexDistance)
  {
    StoreArray<Particle> particles;
    const Particle* newKS = particles[1]; // Get KS, as it has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKS), 5.0);
  }

  TEST_F(FlightInfoTest, vertexDistanceError)
  {
    StoreArray<Particle> particles;
    const Particle* newKS = particles[1]; // Get KS, as it has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistanceErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKS), 0.2);
  }

  TEST_F(FlightInfoTest, vertexDistanceSignificance)
  {
    StoreArray<Particle> particles;
    const Particle* newKS = particles[1]; // Get KS, as it has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistanceSignificance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKS), 25);
  }

  TEST_F(FlightInfoTest, vertexDistanceOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter KS has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistanceOfDaughter(1, noIP)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 5.0);

    var = Manager::Instance().getVariable("vertexDistanceOfDaughter(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 6.0);

    var = Manager::Instance().getVariable("vertexDistanceOfDaughter(2)");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(newDp)));
  }

  TEST_F(FlightInfoTest, vertexDistanceOfDaughterError)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter KS has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistanceOfDaughterErr(1, noIP)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 0.2);

    var = Manager::Instance().getVariable("vertexDistanceOfDaughterErr(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 0.25);
  }

  TEST_F(FlightInfoTest, vertexDistanceOfDaughterSignificance)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter KS has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistanceOfDaughterSignificance(1, noIP)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 25);

    var = Manager::Instance().getVariable("vertexDistanceOfDaughterSignificance(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 24);
  }

  class VertexVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      StoreArray<MCParticle> mcParticles;
      StoreArray<Particle> particles;
      particles.registerRelationTo(mcParticles);
      StoreObjPtr<ParticleExtraInfoMap>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);


      // Insert MC particle logic here
      MCParticle mcKs;
      mcKs.setPDG(310);
      mcKs.setDecayVertex(4.0, 5.0, 0.0);
      mcKs.setProductionVertex(TVector3(1.0, 2.0, 3.0));
      mcKs.setMassFromPDG();
      mcKs.setMomentum(1.164, 1.55200, 0);
      mcKs.setStatus(MCParticle::c_PrimaryParticle);
      MCParticle* newMCKs = mcParticles.appendNew(mcKs);

      Particle Ks(TLorentzVector(1.164, 1.55200, 0, 2), 310);
      Ks.setVertex(TVector3(4.0, 5.0, 0.0));
      Ks.addExtraInfo("prodVertX", 1.0);
      Ks.addExtraInfo("prodVertY", 2.0);
      Ks.addExtraInfo("prodVertZ", 3.0);
      Ks.addExtraInfo("prodVertSxx", 0.1);
      Ks.addExtraInfo("prodVertSxy", 0.2);
      Ks.addExtraInfo("prodVertSxz", 0.3);
      Ks.addExtraInfo("prodVertSyx", 0.4);
      Ks.addExtraInfo("prodVertSyy", 0.5);
      Ks.addExtraInfo("prodVertSyz", 0.6);
      Ks.addExtraInfo("prodVertSzx", 0.7);
      Ks.addExtraInfo("prodVertSzy", 0.8);
      Ks.addExtraInfo("prodVertSzz", 0.9);
      Particle* newKs = particles.appendNew(Ks);
      newKs->addRelationTo(newMCKs);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  // MC vertex tests
  TEST_F(VertexVariablesTest, mcX)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth decay x is 4.0

    const Manager::Var* var = Manager::Instance().getVariable("mcX");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 4.0);
  }

  TEST_F(VertexVariablesTest, mcY)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth decay y is 5.0

    const Manager::Var* var = Manager::Instance().getVariable("mcY");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 5.0);
  }

  TEST_F(VertexVariablesTest, mcZ)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth decay z is 0.0

    const Manager::Var* var = Manager::Instance().getVariable("mcZ");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 0.0);
  }


  TEST_F(VertexVariablesTest, mcDistance)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth distance of sqrt(41)

    const Manager::Var* var = Manager::Instance().getVariable("mcDistance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(4.0 * 4.0 + 5.0 * 5.0));
  }

  TEST_F(VertexVariablesTest, mcRho)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth rho of sqrt(41)

    const Manager::Var* var = Manager::Instance().getVariable("mcRho");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(4.0 * 4.0 + 5.0 * 5.0));
  }

  TEST_F(VertexVariablesTest, mcProdVertexX)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex x of 1.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcProdVertexX");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 1.0);
  }

  TEST_F(VertexVariablesTest, mcProdVertexY)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex y of 2.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcProdVertexY");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 2.0);
  }

  TEST_F(VertexVariablesTest, mcProdVertexZ)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex z of 3.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcProdVertexZ");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 3.0);
  }

  // Production position tests

  TEST_F(VertexVariablesTest, prodVertexX)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex x of 1.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("prodVertexX");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 1.0);
  }
  TEST_F(VertexVariablesTest, prodVertexY)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex y of 2.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("prodVertexY");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 2.0);
  }
  TEST_F(VertexVariablesTest, prodVertexZ)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex z of 3.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("prodVertexZ");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 3.0);
  }

  // Production Covariance tests

  TEST_F(VertexVariablesTest, prodVertexCov)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex covariance xx of .1 cm

    //const Manager::Var* var = Manager::Instance().getVariable("prodVertexCovXX");
    const Manager::Var* var = Manager::Instance().getVariable("prodVertexCov(0,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 0.1);
    var = Manager::Instance().getVariable("prodVertexCov(0,1)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.2);
    var = Manager::Instance().getVariable("prodVertexCov(0,2)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.3);
    var = Manager::Instance().getVariable("prodVertexCov(1,0)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.4);
    var = Manager::Instance().getVariable("prodVertexCov(1,1)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.5);
    var = Manager::Instance().getVariable("prodVertexCov(1,2)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.6);
    var = Manager::Instance().getVariable("prodVertexCov(2,0)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.7);
    var = Manager::Instance().getVariable("prodVertexCov(2,1)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.8);
    var = Manager::Instance().getVariable("prodVertexCov(2,2)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.9);
    var = Manager::Instance().getVariable("prodVertexXErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(0.1));
    var = Manager::Instance().getVariable("prodVertexYErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(0.5));
    var = Manager::Instance().getVariable("prodVertexZErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(0.9));
  }
  TEST_F(MetaVariableTest, KSFWVariables)
  {
    // simple tests that do not require the ROE builder nor the CS builder

    // check that garbage input throws helpful B2FATAL
    EXPECT_B2FATAL(Manager::Instance().getVariable("KSFWVariables(NONSENSE)"));

    // check for NaN if we don't have a CS object for this particle
    StoreArray<Particle> myParticles;
    const Particle* particle_with_no_cs = myParticles.appendNew();
    const Manager::Var* var = Manager::Instance().getVariable("KSFWVariables(mm2)");
    EXPECT_TRUE(std::isnan(var->function(particle_with_no_cs)));

  }
}
