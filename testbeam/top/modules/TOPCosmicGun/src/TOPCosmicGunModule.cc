/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Matthew Barrett                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPCosmicGun/TOPCosmicGunModule.h>
#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <mdst/dataobjects/MCParticle.h>

// ROOT
#include <TRandom3.h>
#include <TVector3.h>
#include <TFile.h>
#include <TF1.h>
#include <TString.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCosmicGun)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCosmicGunModule::TOPCosmicGunModule() : Module()

  {
    // set module description
    setDescription("Cosmic ray gun for TOP cosmics tests");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    vector<double> defaultPad;
    std::string defaultString("Null");
    addParam("upperPad", m_upperPad, "Upper rectangular surface (z,x,Dz,Dx,y) in cm",
             defaultPad);
    addParam("lowerPad", m_lowerPad, "Lower rectangular surface (z,x,Dz,Dx,y) in cm",
             defaultPad);
    addParam("startTime", m_startTime, "Start time in nsec (time at upperPad)", 0.0);

    addParam("momentum", m_momentum, "Muon Momentum in GeV/c (for mono-energetic muons).", 3.14159);
    addParam("momentumCutOff", m_momentumCutOff, "Minimum muon momentum in GeV/c", 0.0);
    addParam("momentumDistributionType",  m_momentumDistributionType,  "Type of momentum distribution to use",
             std::string("monoEnergetic"));
    addParam("momentumHistogramFileName", m_momentumHistogramFileName, "Name of file containing momentum histogram", defaultString);
    addParam("momentumHistogramName",     m_momentumHistogramName,     "Name of momentum histogram in file", defaultString);

    addParam("angularDistributionType",  m_angularDistributionType,  "Type of angular distribution to use", std::string("None"));
    addParam("angularHistogramFileName", m_angularHistogramFileName, "Name of file containing angular histogram", defaultString);
    addParam("angularHistogramName",     m_angularHistogramName,     "Name of angular histogram in file", defaultString);


  }

  TOPCosmicGunModule::~TOPCosmicGunModule()
  {
  }

  void TOPCosmicGunModule::initialize()
  {
    // data store objects registration
    StoreArray<MCParticle>::registerPersistent();

    // parameter checks
    if (m_upperPad.size() != 5) {B2FATAL("upperPad not defined, 5 parameters needed.");}
    if (m_lowerPad.size() != 5) {B2FATAL("lowerPad not defined, 5 parameters needed.");}
    if (m_lowerPad[4] >= m_upperPad[4]) {B2INFO("lowerPad is not below UpperPad");}

    if ("monoEnergetic" == m_momentumDistributionType) {
      B2INFO("Generating mono-energetic cosmic ray muons with a momentum of " << m_momentum << " GeV/c");
    }

    if ("histogram" == m_momentumDistributionType) {
      B2INFO("Generating cosmic muon momentum distribution from a histogram.");

      B2INFO("Using histogram file" << m_momentumHistogramFileName << " to obtain momentum histogram.");
      m_momentumHistogramFile = new TFile(TString(m_momentumHistogramFileName));
      if (m_momentumHistogramFile->IsZombie()) {B2ERROR("Could not open ROOT file " << m_momentumHistogramFileName)};

      B2INFO("Reading histogram " << m_momentumHistogramName << " from ROOT file " << m_momentumHistogramFileName << ".");
      m_momentumDistribution = (TH1F*) m_momentumHistogramFile->Get(TString(m_momentumHistogramName));
      if (!m_momentumDistribution) {B2ERROR("Could not read histogram " << m_momentumHistogramName << " from ROOT file " << m_momentumHistogramFileName << ".");}
    }

    if ("monoEnergetic" != m_momentumDistributionType && "histogram" != m_momentumDistributionType) {
      B2ERROR("Unknown momentum distribution type: " << m_momentumDistributionType << std::endl <<
              "        Recognised types: \"monoEnergetic\", \"histogram\".");
    }

    if ("None" == m_angularDistributionType) {
      B2INFO("Generating cosmic ray muons with no angular dependence - geometrical acceptance only.");
    }

    if ("cosSquared" == m_angularDistributionType) {
      B2INFO("Generating cosmic ray muons with a cosine squared angular distribution.");
    }

    if ("histogram" == m_angularDistributionType) {
      B2INFO("Generating cosmic muon angular distribution from a histogram.");

      B2INFO("Using histogram file" << m_angularHistogramFileName << " to obtain angular histogram.");
      m_angularHistogramFile = new TFile(TString(m_angularHistogramFileName));
      if (m_angularHistogramFile->IsZombie()) {B2ERROR("Could not open ROOT file " << m_angularHistogramFileName)};

      B2INFO("Reading histogram " << m_angularHistogramName << " from ROOT file " << m_angularHistogramFileName << ".");
      m_angularDistribution = (TH1F*) m_angularHistogramFile->Get(TString(m_angularHistogramName));
      if (!m_angularDistribution) {B2ERROR("Could not read histogram " << m_angularHistogramName << " from ROOT file " << m_angularHistogramFileName << ".");}
    }

    if ("None" != m_angularDistributionType && "histogram" != m_angularDistributionType && "cosSquared" != m_angularDistributionType) {
      B2ERROR("Unknown angular distribution type: " << m_angularDistributionType << std::endl <<
              "        Recognised types: \"None\", \"histogram\", \"cosSquared\".");
    }

  }

  void TOPCosmicGunModule::beginRun()
  {
  }

  void TOPCosmicGunModule::event()
  {

    // generate points on upper and lower surfaces
    double z1 = m_upperPad[0] + m_upperPad[2] * (gRandom->Rndm() - 0.5);
    double x1 = m_upperPad[1] + m_upperPad[3] * (gRandom->Rndm() - 0.5);
    double y1 = m_upperPad[4];

    double z2 = m_lowerPad[0] + m_lowerPad[2] * (gRandom->Rndm() - 0.5);
    double x2 = m_lowerPad[1] + m_lowerPad[3] * (gRandom->Rndm() - 0.5);
    double y2 = m_lowerPad[4];

    if ("histogram" == m_angularDistributionType || "cosSquared" == m_angularDistributionType) {
      // Note: the theta used in this loop is NOT the theta parameter of the Belle II coordinate system!
      // Theta = 0 here corresponds to vertical cosmic ray muons.
      // The x, y, and z parameters are reordered in this loop to reflect this definition.

      // calculate momentum vector in this coordinate representation:
      TVector3 position1(x1, z1, y1);
      TVector3 position2(x2, z2, y2);

      TVector3 direction = position1 - position2; // Calculate direction muon is coming from.
      double   theta     = direction.Theta();
      double   cosTheta  = direction.CosTheta();

      double fAngle(0);
      if ("histogram"  == m_angularDistributionType) {fAngle = m_angularDistribution->Interpolate(theta);}
      if ("cosSquared" == m_angularDistributionType) {fAngle = cosTheta * cosTheta;}

      while (gRandom->Rndm() > fAngle) {
        //Regenerate muon direction until accepted  by angular distribution.
        //This loop could become inefficient for *very* steep muon selection setups.

        z1 = m_upperPad[0] + m_upperPad[2] * (gRandom->Rndm() - 0.5);
        x1 = m_upperPad[1] + m_upperPad[3] * (gRandom->Rndm() - 0.5);
        y1 = m_upperPad[4];
        z2 = m_lowerPad[0] + m_lowerPad[2] * (gRandom->Rndm() - 0.5);
        x2 = m_lowerPad[1] + m_lowerPad[3] * (gRandom->Rndm() - 0.5);
        y2 = m_lowerPad[4];

        position1 = TVector3(x1, z1, y1);
        position2 = TVector3(x2, z2, y2);
        direction = position1 - position2;
        theta     = direction.Theta();
        cosTheta  = direction.CosTheta();

        if ("histogram"  == m_angularDistributionType) {fAngle = m_angularDistribution->Interpolate(theta);}
        if ("cosSquared" == m_angularDistributionType) {fAngle = cosTheta * cosTheta;}

      }
    }
    TVector3 r1(x1, y1, z1);
    TVector3 r2(x2, y2, z2);

    // generate momentum
    double p = m_momentum; //default momentum

    if ("histogram" == m_momentumDistributionType) {
      p = m_momentumDistribution->GetRandom();
      while (p < m_momentumCutOff) {p = m_momentumDistribution->GetRandom();}
    }

    // calculate momentum vector
    TVector3 dr = r2 - r1;
    TVector3 momentum = (p / dr.Mag()) * dr;

    // create MCParticles data store
    StoreArray<MCParticle> MCParticles;
    MCParticles.create();

    // store generated muon
    MCParticle* part = MCParticles.appendNew();
    part->setPDG(13);
    part->setMassFromPDG();
    part->setStatus(MCParticle::c_PrimaryParticle);
    part->addStatus(MCParticle::c_StableInGenerator);
    part->setProductionVertex(r1);
    part->setProductionTime(m_startTime);
    part->setMomentum(momentum);
    double mass = part->getMass();
    double energy = sqrt(p * p + mass * mass);
    part->setEnergy(energy);

  }


  void TOPCosmicGunModule::endRun()
  {
    if ("histogram" == m_momentumDistributionType) {
      delete m_momentumDistribution;
      m_momentumHistogramFile->Close();
    }
    if ("histogram" == m_angularDistributionType) {
      delete m_angularDistribution;
      m_angularHistogramFile->Close();
    }



  }

  void TOPCosmicGunModule::terminate()
  {
  }


} // end Belle2 namespace

