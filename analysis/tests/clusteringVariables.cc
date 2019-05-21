/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe, Torben Ferber, Giacomo De Pietro           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Unit tests for all variables related to clustering and clustering
// subdetectors (KLM and ECL variables, track <--> cluster matching etc)

#include <gtest/gtest.h>
#include <TRandom3.h>

// VariableManager and particle(list)
#include <analysis/variables/Variables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// mdst dataobjects
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>

// framework - set up mock events
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Gearbox.h>

using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  /** ECL variable test fixture */
  class ECLVariableTest : public ::testing::Test {
  protected:
    /** register Particle and ECLCluster arrays. */
    void SetUp() override
    {
      // setup the DataStore
      DataStore::Instance().setInitializeActive(true);

      // particles (to be filled)
      StoreArray<Particle> particles;
      particles.registerInDataStore();

      // mock up mdst objects
      StoreArray<Track> tracks;
      tracks.registerInDataStore();
      StoreArray<TrackFitResult> trackFits;
      trackFits.registerInDataStore();
      StoreArray<ECLCluster> eclclusters;
      eclclusters.registerInDataStore();

      // tracks can be matched to clusters
      tracks.registerRelationTo(eclclusters);

      // we're done setting up the datastore
      DataStore::Instance().setInitializeActive(false);

      // add some tracks the zeroth one is not going to be matched
      tracks.appendNew(Track());
      const Track* t1 = tracks.appendNew(Track());
      const Track* t2 = tracks.appendNew(Track());
      const Track* t3 = tracks.appendNew(Track());
      const Track* t4 = tracks.appendNew(Track());
      tracks.appendNew(Track());
      tracks.appendNew(Track());

      // mock up some TrackFits for them (all pions)
      TRandom3 generator;
      TMatrixDSym cov6(6);
      auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

      for (int i = 0; i < tracks.getEntries(); ++i) {
        int charge = (i % 2 == 0) ? +1 : -1;
        TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
        TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
        d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
        TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
        TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));
        trackFits.appendNew(position, momentum, cov6, charge, Const::pion, 0.5, 1.5, CDCValue, 16777215);
        tracks[i]->setTrackFitResultIndex(Const::pion, i);
      }

      // add some ECL clusters
      ECLCluster* e1 = eclclusters.appendNew(ECLCluster());
      e1->setEnergy(0.3);
      e1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e1->setClusterId(1);
      // leave this guy with default theta and phi
      ECLCluster* e2 = eclclusters.appendNew(ECLCluster());
      e2->setEnergy(0.6);
      e2->setTheta(1.0); // somewhere in the barrel
      e2->setPhi(2.0);
      e2->setR(148.5);
      e2->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e2->setClusterId(2);
      ECLCluster* e3 = eclclusters.appendNew(ECLCluster());
      e3->setEnergy(0.15);
      e3->setTheta(0.2); // somewhere in the fwd encap
      e3->setPhi(1.5);
      e3->setR(200.0);
      e3->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e3->addHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
      // lets suppose this cluster could also be due to a neutral hadron. In
      // this case, the c_neuralHadron hypothesis bit would hopefully also have
      // been set by the reconstruction... arbirarily choose cluster 3
      e3->setClusterId(3);

      // aaand add clusters related to the tracks
      ECLCluster* e4 = eclclusters.appendNew(ECLCluster());
      e4->setEnergy(0.2);
      e4->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e4->setClusterId(4);
      t1->addRelationTo(e4);
      e4->setIsTrack(true);

      ECLCluster* e5 = eclclusters.appendNew(ECLCluster());
      e5->setEnergy(0.3);
      e5->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e5->setClusterId(5);
      t2->addRelationTo(e5);
      e5->setIsTrack(true);

      ECLCluster* e6 = eclclusters.appendNew(ECLCluster());
      e6->setEnergy(0.2);
      e6->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e6->setClusterId(6);
      t3->addRelationTo(e6);
      t4->addRelationTo(e6);
      // two tracks are related to this cluster this can happen due to real
      // physics and we should be able to cope
      e6->setIsTrack(true);

    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(ECLVariableTest, b2bKinematicsTest)
  {
    // we need the particles and ECLClusters arrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;
    StoreArray<Track> tracks;

    // connect gearbox for CMS boosting etc
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    // register in the datastore
    StoreObjPtr<ParticleList> gammalist("gamma:testGammaAllList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }

    // get the zeroth track in the array (is not associated to a cluster)
    const Particle* noclustertrack = particles.appendNew(Particle(tracks[0], Const::pion));

    // grab variables for testing
    const Manager::Var* b2bClusterTheta = Manager::Instance().getVariable("b2bClusterTheta");
    const Manager::Var* b2bClusterPhi = Manager::Instance().getVariable("b2bClusterPhi");

    EXPECT_EQ(gammalist->getListSize(), 3);

    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(0)), 3.0276606);
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(0)), 0.0);
    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(1)), 1.6036042);
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(1)), -1.0607308);
    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(2)), 2.7840068);
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(2)), -1.3155469);

    // track (or anything without a cluster) should be nan
    ASSERT_TRUE(std::isnan(b2bClusterTheta->function(noclustertrack)));
    ASSERT_TRUE(std::isnan(b2bClusterPhi->function(noclustertrack)));

    // the "normal" (not cluster based) variables should be the same for photons
    // (who have no track information)
    const Manager::Var* b2bTheta = Manager::Instance().getVariable("b2bTheta");
    const Manager::Var* b2bPhi = Manager::Instance().getVariable("b2bPhi");

    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(0)),
                    b2bTheta->function(gammalist->getParticle(0)));
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(0)),
                    b2bPhi->function(gammalist->getParticle(0)));
  }

  TEST_F(ECLVariableTest, KLMCMSHelpersTest)
  {
    // we need the particles and ECLClusters arrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;

    // connect gearbox for CMS boosting etc
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    // register in the datastore
    StoreObjPtr<ParticleList> gammalist("gamma:test");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }

    // grab variables for testing
    const Manager::Var* max = Manager::Instance().getVariable("maximumKLMAngleCMS");
    const Manager::Var* min = Manager::Instance().getVariable("minimumKLMAngleCMS");
    const Manager::Var* clx = Manager::Instance().getVariable("closestKLMCMS(klmClusterPositionX)");
    const Manager::Var* cly = Manager::Instance().getVariable("closestKLMCMS(klmClusterPositionY)");
    const Manager::Var* clz = Manager::Instance().getVariable("closestKLMCMS(klmClusterPositionZ)");
    const Manager::Var* clE = Manager::Instance().getVariable("closestKLMCMS(klmClusterEnergy)");
    const Manager::Var* frx = Manager::Instance().getVariable("furthestKLMCMS(klmClusterPositionX)");
    const Manager::Var* fry = Manager::Instance().getVariable("furthestKLMCMS(klmClusterPositionY)");
    const Manager::Var* frz = Manager::Instance().getVariable("furthestKLMCMS(klmClusterPositionZ)");
    const Manager::Var* frE = Manager::Instance().getVariable("furthestKLMCMS(klmClusterEnergy)");

    EXPECT_EQ(gammalist->getListSize(), 3);

    {
      // should all be NaN as there are no KLMClusters in the storearray yet
      ASSERT_TRUE(std::isnan(max->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(min->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(clx->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(cly->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(clz->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(clE->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(frx->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(fry->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(frz->function(gammalist->getParticle(0))));
      ASSERT_TRUE(std::isnan(frE->function(gammalist->getParticle(0))));
    }

    // now add KLMClusters to the datastore
    DataStore::Instance().setInitializeActive(true);
    StoreArray<KLMCluster> klmclusters;
    klmclusters.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    KLMCluster* k1 = klmclusters.appendNew(KLMCluster());
    k1->setClusterPosition(2, 2, 0.5); // somewhere in the ECL barrel - sameish as photon 1
    k1->setMomentumMag(3);
    EXPECT_EQ(klmclusters.getEntries(), 1);

    {
      // there is now one and only one KLMCluster, so the minimum should also be
      // the maximum for all photons
      EXPECT_FLOAT_EQ(max->function(gammalist->getParticle(0)), min->function(gammalist->getParticle(0)));
      EXPECT_FLOAT_EQ(max->function(gammalist->getParticle(1)), min->function(gammalist->getParticle(1)));
      EXPECT_FLOAT_EQ(max->function(gammalist->getParticle(2)), min->function(gammalist->getParticle(2)));

      EXPECT_FLOAT_EQ(clx->function(gammalist->getParticle(0)), frx->function(gammalist->getParticle(0)));
      EXPECT_FLOAT_EQ(clx->function(gammalist->getParticle(1)), frx->function(gammalist->getParticle(1)));
      EXPECT_FLOAT_EQ(clx->function(gammalist->getParticle(2)), frx->function(gammalist->getParticle(2)));

      EXPECT_FLOAT_EQ(cly->function(gammalist->getParticle(0)), fry->function(gammalist->getParticle(0)));
      EXPECT_FLOAT_EQ(cly->function(gammalist->getParticle(1)), fry->function(gammalist->getParticle(1)));
      EXPECT_FLOAT_EQ(cly->function(gammalist->getParticle(2)), fry->function(gammalist->getParticle(2)));

      EXPECT_FLOAT_EQ(clz->function(gammalist->getParticle(0)), frz->function(gammalist->getParticle(0)));
      EXPECT_FLOAT_EQ(clz->function(gammalist->getParticle(1)), frz->function(gammalist->getParticle(1)));
      EXPECT_FLOAT_EQ(clz->function(gammalist->getParticle(2)), frz->function(gammalist->getParticle(2)));

      EXPECT_FLOAT_EQ(clE->function(gammalist->getParticle(0)), frE->function(gammalist->getParticle(0)));
      EXPECT_FLOAT_EQ(clE->function(gammalist->getParticle(1)), frE->function(gammalist->getParticle(1)));
      EXPECT_FLOAT_EQ(clE->function(gammalist->getParticle(2)), frE->function(gammalist->getParticle(2)));

      // we should be able to retrieve the cluster's position
      EXPECT_FLOAT_EQ(clx->function(gammalist->getParticle(0)), 2);
      EXPECT_FLOAT_EQ(clx->function(gammalist->getParticle(1)), 2);
      EXPECT_FLOAT_EQ(clx->function(gammalist->getParticle(2)), 2);

      EXPECT_FLOAT_EQ(cly->function(gammalist->getParticle(0)), 2);
      EXPECT_FLOAT_EQ(cly->function(gammalist->getParticle(1)), 2);
      EXPECT_FLOAT_EQ(cly->function(gammalist->getParticle(2)), 2);

      EXPECT_FLOAT_EQ(clz->function(gammalist->getParticle(0)), 0.5);
      EXPECT_FLOAT_EQ(clz->function(gammalist->getParticle(1)), 0.5);
      EXPECT_FLOAT_EQ(clz->function(gammalist->getParticle(2)), 0.5);

      // and the energy
      EXPECT_FLOAT_EQ(clE->function(gammalist->getParticle(0)), 3.0409899);
      EXPECT_FLOAT_EQ(clE->function(gammalist->getParticle(1)), 3.0409899);
      EXPECT_FLOAT_EQ(clE->function(gammalist->getParticle(2)), 3.0409899);
    }

    KLMCluster* k2 = klmclusters.appendNew(KLMCluster());
    k2->setClusterPosition(-1.9, -1.5, -0.15); // opposite! far from photon 1 (and a bit of boosting by eye)
    k2->setMomentumMag(3);
    EXPECT_EQ(klmclusters.getEntries(), 2);

    {
      // there are now two clusters, one close to photon 1
      EXPECT_TRUE(min->function(gammalist->getParticle(1)) < min->function(gammalist->getParticle(0)));
      EXPECT_TRUE(min->function(gammalist->getParticle(1)) < min->function(gammalist->getParticle(2)));

      // and one far from photon 1
      EXPECT_TRUE(max->function(gammalist->getParticle(1)) < max->function(gammalist->getParticle(0)));
      EXPECT_TRUE(max->function(gammalist->getParticle(1)) < max->function(gammalist->getParticle(2)));

      // not particularly intelligent, but also check the precise values are reproducable
      EXPECT_FLOAT_EQ(max->function(gammalist->getParticle(0)), 1.8632624);
      EXPECT_FLOAT_EQ(min->function(gammalist->getParticle(0)), 1.7109571);
      EXPECT_FLOAT_EQ(max->function(gammalist->getParticle(1)), 1.8251470);
      EXPECT_FLOAT_EQ(min->function(gammalist->getParticle(1)), 1.2793235);
      EXPECT_FLOAT_EQ(max->function(gammalist->getParticle(2)), 2.0248148);
      EXPECT_FLOAT_EQ(min->function(gammalist->getParticle(2)), 1.5056936);
    }
  }

  TEST_F(ECLVariableTest, clusterKinematicsTest)
  {
    // we need the particles and ECLClusters arrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;
    StoreArray<Track> tracks;

    // connect gearbox for CMS boosting etc
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    // register in the datastore
    StoreObjPtr<ParticleList> gammalist("gamma:testGammaAllList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }

    // grab variables for testing
    const Manager::Var* clusterPhi = Manager::Instance().getVariable("clusterPhi");
    const Manager::Var* clusterPhiCMS = Manager::Instance().getVariable("useCMSFrame(clusterPhi)");
    const Manager::Var* clusterTheta = Manager::Instance().getVariable("clusterTheta");
    const Manager::Var* clusterThetaCMS = Manager::Instance().getVariable("useCMSFrame(clusterTheta)");

    EXPECT_FLOAT_EQ(clusterPhi->function(gammalist->getParticle(1)), 2.0);
    EXPECT_FLOAT_EQ(clusterPhiCMS->function(gammalist->getParticle(1)), 2.0442522);
    EXPECT_FLOAT_EQ(clusterTheta->function(gammalist->getParticle(1)), 1.0);
    EXPECT_FLOAT_EQ(clusterThetaCMS->function(gammalist->getParticle(1)), 1.2625268);

    // test cluster quantities directly (lab system only)
    EXPECT_FLOAT_EQ(clusterPhi->function(gammalist->getParticle(0)), eclclusters[0]->getPhi());
    EXPECT_FLOAT_EQ(clusterTheta->function(gammalist->getParticle(0)), eclclusters[0]->getTheta());
  }

  TEST_F(ECLVariableTest, HypothesisVariables)
  {
    // we need the particles and ECLClusters arrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;

    // register in the datastore
    StoreObjPtr<ParticleList> gammalist("gamma");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i)
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }

    // grab variables for testing
    const Manager::Var* vHasNPhotons = Manager::Instance().getVariable("clusterHasNPhotons");
    const Manager::Var* vHasNeutHadr = Manager::Instance().getVariable("clusterHasNeutralHadron");
    const Manager::Var* vHypothsisID = Manager::Instance().getVariable("clusterHypothesis");
    // TODO: remove hypothesis id after release-04 (should be gone by -05)

    // check that the hypotheses are correcltly propagated to the VM.
    for (size_t i = 0; i < gammalist->getListSize(); ++i) {
      EXPECT_FLOAT_EQ(vHasNPhotons->function(gammalist->getParticle(i)), 1.0);
      if (i == 2) { // third cluster arbitrarily chosen to test the behaviour of dual hypothesis clusters
        EXPECT_FLOAT_EQ(vHasNeutHadr->function(gammalist->getParticle(i)), 1.0);
        EXPECT_FLOAT_EQ(vHypothsisID->function(gammalist->getParticle(i)), 56.0);
      } else {
        EXPECT_FLOAT_EQ(vHasNeutHadr->function(gammalist->getParticle(i)), 0.0);
        EXPECT_FLOAT_EQ(vHypothsisID->function(gammalist->getParticle(i)), 5.0);
      }
    } // end loop over test list
  }

  TEST_F(ECLVariableTest, IsFromECL)
  {
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;

    const Manager::Var* vIsFromECL = Manager::Instance().getVariable("isFromECL");
    const Manager::Var* vIsFromKLM = Manager::Instance().getVariable("isFromKLM");
    const Manager::Var* vIsFromTrack = Manager::Instance().getVariable("isFromTrack");

    for (int i = 0; i < eclclusters.getEntries(); ++i)
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        EXPECT_TRUE(vIsFromECL->function(p));
        EXPECT_FALSE(vIsFromKLM->function(p));
        EXPECT_FALSE(vIsFromTrack->function(p));
      }
  }

  TEST_F(ECLVariableTest, WholeEventClosure)
  {
    // we need the particles, tracks, and ECLClusters StoreArrays
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<ECLCluster> eclclusters;

    // create a photon (clusters) and pion (tracks) lists
    StoreObjPtr<ParticleList> gammalist("gamma:testGammaAllList");
    StoreObjPtr<ParticleList> pionslist("pi+:testPionAllList");
    StoreObjPtr<ParticleList> apionslist("pi-:testPionAllList");

    // register the lists in the datastore
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    pionslist.registerInDataStore(DataStore::c_DontWriteOut);
    apionslist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());
    pionslist.create();
    pionslist->initialize(211, pionslist.getName());
    apionslist.create();
    apionslist->initialize(-211, apionslist.getName());
    apionslist->bindAntiParticleList(*(pionslist));

    // make the photons from clusters (and sum up the total ecl energy)
    double eclEnergy = 0.0;
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      eclEnergy += eclclusters[i]->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }


    // make the pions from tracks
    for (int i = 0; i < tracks.getEntries(); ++i) {
      const Particle* p = particles.appendNew(Particle(tracks[i], Const::pion));
      pionslist->addParticle(p);
    }

    // grab variables
    const Manager::Var* vClusterE = Manager::Instance().getVariable("clusterE");
    const Manager::Var* vClNTrack = Manager::Instance().getVariable("nECLClusterTrackMatches");

    // calculate the total neutral energy from the particle list --> VM
    double totalNeutralClusterE = 0.0;
    for (size_t i = 0; i < gammalist->getListSize(); ++i)
      totalNeutralClusterE += vClusterE->function(gammalist->getParticle(i));

    // calculate the total track-matched cluster energy from the particle list --> VM
    double totalTrackClusterE = 0.0;
    for (size_t i = 0; i < pionslist->getListSize(); ++i) { // includes antiparticles
      double clusterE = vClusterE->function(pionslist->getParticle(i));
      double nOtherCl = vClNTrack->function(pionslist->getParticle(i));
      if (nOtherCl > 0)
        totalTrackClusterE += clusterE / nOtherCl;
    }

    EXPECT_FLOAT_EQ(totalNeutralClusterE + totalTrackClusterE, eclEnergy);
  }

  TEST_F(ECLVariableTest, eclClusterOnlyInvariantMass)
  {
    // declare all the array we need
    StoreArray<Particle> particles, particles_noclst;
    std::vector<int> daughterIndices, daughterIndices_noclst;

    //proxy initialize where to declare the needed array
    DataStore::Instance().setInitializeActive(true);
    StoreArray<ECLCluster> eclclusters_new;
    eclclusters_new.registerInDataStore();
    particles.registerRelationTo(eclclusters_new);
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
    TLorentzVector momentum;
    TLorentzVector dau0_4vec(px_0, py_0, pz_0, E_0), dau1_4vec(px_1, py_1, pz_1, E_1);

    // add the two photons as the two daughters of some particle and create the latter
    Particle dau0_noclst(dau0_4vec, 22);
    momentum += dau0_noclst.get4Vector();
    Particle* newDaughter0_noclst = particles.appendNew(dau0_noclst);
    daughterIndices_noclst.push_back(newDaughter0_noclst->getArrayIndex());
    Particle dau1_noclst(dau1_4vec, 22);
    momentum += dau1_noclst.get4Vector();
    Particle* newDaughter1_noclst = particles.appendNew(dau1_noclst);
    daughterIndices_noclst.push_back(newDaughter1_noclst->getArrayIndex());
    const Particle* par_noclst = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices_noclst);

    // grab variables
    const Manager::Var* var = Manager::Instance().getVariable("eclClusterOnlyInvariantMass");

    // when no relations are set between the particles and the eclClusters, nan is expected to be returned
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(par_noclst)));

    // set relations between particles and eclClusters
    ECLCluster* eclst0 = eclclusters_new.appendNew(ECLCluster());
    eclst0->setEnergy(dau0_4vec.E());
    eclst0->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst0->setClusterId(1);
    eclst0->setTheta(dau0_4vec.Theta());
    eclst0->setPhi(dau0_4vec.Phi());
    eclst0->setR(148.4);
    ECLCluster* eclst1 = eclclusters_new.appendNew(ECLCluster());
    eclst1->setEnergy(dau1_4vec.E());
    eclst1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst1->setClusterId(2);
    eclst1->setTheta(dau1_4vec.Theta());
    eclst1->setPhi(dau1_4vec.Phi());
    eclst1->setR(148.5);

    // use these new-created clusters rather than the 6 default ones
    const Particle* newDaughter0 = particles.appendNew(Particle(eclclusters_new[6]));
    daughterIndices.push_back(newDaughter0->getArrayIndex());
    const Particle* newDaughter1 = particles.appendNew(Particle(eclclusters_new[7]));
    daughterIndices.push_back(newDaughter1->getArrayIndex());

    const Particle* par = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices);

    //now we expect non-nan results
    EXPECT_FLOAT_EQ(var->function(par), 0.73190731);
  }

  class KLMVariableTest : public ::testing::Test {
  protected:
    /** register Particle and KLMCluster arrays. */
    void SetUp() override
    {
      // setup the DataStore
      DataStore::Instance().setInitializeActive(true);

      // particles (to be filled)
      StoreArray<Particle> particles;
      particles.registerInDataStore();

      // mock up mdst objects
      StoreArray<Track> tracks;
      tracks.registerInDataStore();
      StoreArray<TrackFitResult> trackFits;
      trackFits.registerInDataStore();
      StoreArray<KLMCluster> klmClusters;
      klmClusters.registerInDataStore();

      // tracks can be matched to clusters
      tracks.registerRelationTo(klmClusters);

      // we're done setting up the datastore
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(KLMVariableTest, WholeEventClosure)
  {
    // we need the Particles, Tracks, TrackFitResults and KLMClusters StoreArrays
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFits;
    StoreArray<KLMCluster> klmClusters;

    // create a KLong (clusters) and muon (tracks) lists
    StoreObjPtr<ParticleList> kLongList("K0_L:testKLong");
    StoreObjPtr<ParticleList> muonsList("mu-:testMuons");
    StoreObjPtr<ParticleList> amuonsList("mu+:testMuons");

    // register the lists in the datastore
    DataStore::Instance().setInitializeActive(true);
    kLongList.registerInDataStore(DataStore::c_DontWriteOut);
    muonsList.registerInDataStore(DataStore::c_DontWriteOut);
    amuonsList.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    kLongList.create();
    kLongList->initialize(130, kLongList.getName());
    muonsList.create();
    muonsList->initialize(13, muonsList.getName());
    amuonsList.create();
    amuonsList->initialize(-13, amuonsList.getName());
    amuonsList->bindAntiParticleList(*(muonsList));

    // add some tracks
    const Track* t1 = tracks.appendNew(Track());
    const Track* t2 = tracks.appendNew(Track());
    const Track* t3 = tracks.appendNew(Track());
    tracks.appendNew(Track());
    tracks.appendNew(Track());

    // mock up some TrackFits for them (all muons)
    TRandom3 generator;
    TMatrixDSym cov6(6);
    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    for (int i = 0; i < tracks.getEntries(); ++i) {
      int charge = (i % 2 == 0) ? +1 : -1;
      TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
      TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
      TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));
      trackFits.appendNew(position, momentum, cov6, charge, Const::muon, 0.5, 1.5, CDCValue, 16777215);
      tracks[i]->setTrackFitResultIndex(Const::muon, i);
    }

    // add some clusters
    KLMCluster* klm1 = klmClusters.appendNew(KLMCluster());
    klm1->setTime(1.1);
    klm1->setClusterPosition(1.1, 1.1, 1.0);
    klm1->setLayers(1);
    klm1->setInnermostLayer(1);
    klm1->setMomentumMag(1.0);
    KLMCluster* klm2 = klmClusters.appendNew(KLMCluster());
    klm2->setTime(1.2);
    klm2->setClusterPosition(1.2, 1.2, 2.0);
    klm2->setLayers(2);
    klm2->setInnermostLayer(2);
    klm2->setMomentumMag(1.0);
    KLMCluster* klm3 = klmClusters.appendNew(KLMCluster());
    klm3->setTime(1.3);
    klm3->setClusterPosition(1.3, 1.3, 3.0);
    klm3->setLayers(3);
    klm3->setInnermostLayer(3);
    klm3->setMomentumMag(1.0);

    // and add clusters related to the tracks
    // case 1: 1 track --> 1 cluster
    KLMCluster* klm4 = klmClusters.appendNew(KLMCluster());
    klm4->setTime(1.4);
    klm4->setClusterPosition(-1.4, -1.4, 1.0);
    klm4->setLayers(4);
    klm4->setInnermostLayer(4);
    klm4->setMomentumMag(1.0);
    t1->addRelationTo(klm4);

    // case 2: 2 tracks --> 1 cluster
    KLMCluster* klm5 = klmClusters.appendNew(KLMCluster());
    klm5->setTime(1.5);
    klm5->setClusterPosition(-1.5, -1.5, 1.0);
    klm5->setLayers(5);
    klm5->setInnermostLayer(5);
    klm5->setMomentumMag(1.0);
    t2->addRelationTo(klm5);
    t3->addRelationTo(klm5);

    // case 3: 1 track --> 2 clusters
    // possible case, but not covered

    // make the KLong from clusters (and sum up the total KLM momentum magnitude)
    double klmMomentum = 0.0;
    for (int i = 0; i < klmClusters.getEntries(); ++i) {
      klmMomentum += klmClusters[i]->getMomentumMag();
      if (!klmClusters[i]->getAssociatedTrackFlag()) {
        const Particle* p = particles.appendNew(Particle(klmClusters[i]));
        kLongList->addParticle(p);
      }
    }

    // make the muons from tracks
    for (int i = 0; i < tracks.getEntries(); ++i) {
      const Particle* p = particles.appendNew(Particle(tracks[i], Const::muon));
      muonsList->addParticle(p);
    }

    // grab variables
    const Manager::Var* vClusterP = Manager::Instance().getVariable("klmClusterMomentum");
    const Manager::Var* vClNTrack = Manager::Instance().getVariable("nKLMClusterTrackMatches");

    // calculate the total KLM momentum from the KLong list --> VM
    double totalKLongMomentum = 0.0;
    for (size_t i = 0; i < kLongList->getListSize(); ++i)
      totalKLongMomentum += vClusterP->function(kLongList->getParticle(i));

    // calculate the total KLM momentum from muon-matched list --> VM
    double totalMuonMomentum = 0.0;
    for (size_t i = 0; i < muonsList->getListSize(); ++i) { // includes antiparticles
      double muonMomentum = vClusterP->function(muonsList->getParticle(i));
      double nOtherCl = vClNTrack->function(muonsList->getParticle(i));
      if (nOtherCl > 0)
        totalMuonMomentum += muonMomentum / nOtherCl;
    }

    EXPECT_FLOAT_EQ(5.0, klmMomentum);
    EXPECT_FLOAT_EQ(totalKLongMomentum + totalMuonMomentum, klmMomentum);
  }

  TEST_F(KLMVariableTest, MoreClustersToOneTrack)
  {
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFits;
    StoreArray<KLMCluster> klmClusters;

    // add a TrackFitResult
    TRandom3 generator;

    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);

    TVector3 position(1.0, 0, 0);
    TVector3 momentum(0, 1.0, 0);

    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    trackFits.appendNew(position, momentum, cov6, charge, Const::muon, pValue, bField, CDCValue, 16777215);

    // add one Track
    Track myTrack;
    myTrack.setTrackFitResultIndex(Const::muon, 0);
    Track* muonTrack = tracks.appendNew(myTrack);

    // add two KLMClusters
    KLMCluster* klm1 = klmClusters.appendNew(KLMCluster());
    klm1->setTime(1.1);
    klm1->setClusterPosition(1.1, 1.1, 1.0);
    klm1->setLayers(5);
    klm1->setInnermostLayer(1);
    klm1->setMomentumMag(1.0);
    KLMCluster* klm2 = klmClusters.appendNew(KLMCluster());
    klm2->setTime(1.2);
    klm2->setClusterPosition(1.2, 1.2, 2.0);
    klm2->setLayers(10);
    klm2->setInnermostLayer(2);
    klm2->setMomentumMag(1.0);

    // and add a relationship between the track and both clusters
    muonTrack->addRelationTo(klm1);
    muonTrack->addRelationTo(klm2);

    // add a Particle
    const Particle* muon = particles.appendNew(Particle(muonTrack, Const::muon));

    // grab variables
    const Manager::Var* vTrNClusters = Manager::Instance().getVariable("nMatchedKLMClusters");
    const Manager::Var* vClusterInnermostLayer = Manager::Instance().getVariable("klmClusterInnermostLayer");

    EXPECT_POSITIVE(vTrNClusters->function(muon));
    EXPECT_FLOAT_EQ(2.0, vClusterInnermostLayer->function(muon));

    // add a Pion - no clusters matched here
    trackFits.appendNew(position, momentum, cov6, charge, Const::pion, pValue, bField, CDCValue, 16777215);
    Track mySecondTrack;
    mySecondTrack.setTrackFitResultIndex(Const::pion, 0);
    Track* pionTrack = tracks.appendNew(mySecondTrack);
    const Particle* pion = particles.appendNew(Particle(pionTrack, Const::pion));

    EXPECT_FLOAT_EQ(0.0, vTrNClusters->function(pion));
  }
}
