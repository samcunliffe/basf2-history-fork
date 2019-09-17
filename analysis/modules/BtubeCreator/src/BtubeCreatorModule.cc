/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey, Abi Soffer                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/BtubeCreator/BtubeCreatorModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TMath.h>
#include <Eigen/Core>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BtubeCreator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BtubeCreatorModule::BtubeCreatorModule() : Module(),
  m_Bfield(0)
{
  // Set module properties
  setDescription("BtubeCreator : This module creates Btube object, an object geometrically similar to a very long ellipsoid along a particular direction and can be used as a constraint in vertexing B particles. Upsilon4S decays to two Bs. the user selects one B with a ^ in decaystring. The order of daughters in reconstructdecay of Upsilon4S and this decaystring should be same. Using selected B as reference, the module calculates the direction in which the other B should fly and constructs a Btube object along that direction. In semi-leptonic analyses, the selected B is usually the fully reconstructed one, while for time dependent studies, the selected B is usually signal B which can be partially reconstructed");

  // Parameter definitions
  addParam("listName", m_listName, "name of mother particle list", string(""));
  addParam("decayString", m_decayString,
           "decay string of the mother particle, the selected daughter specifies which daughter will be used as reference to create Btube",
           string(""));
  addParam("confidenceLevel", m_confidenceLevel,
           "required confidence level of fit to keep particles in the list. Note that even with confidenceLevel == 0.0, errors during the fit might discard Particles in the list. confidenceLevel = -1 if an error occurs during the fit",
           0.);
  addParam("verbosity", m_verbose, "print statements", false);
}

void BtubeCreatorModule::initialize()
{
  // magnetic field
  m_Bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;

  m_BeamSpotCenter = m_beamSpotDB->getIPPosition();
  m_beamSpotCov.ResizeTo(3, 3);
  m_beamSpotCov = m_beamSpotDB->getCovVertex();
  B2INFO("BtubeCreator : magnetic field = " << m_Bfield);

  tubeArray.registerInDataStore();
  particles.registerRelationTo(tubeArray);

  if (m_decayString != "") {
    m_decaydescriptor.init(m_decayString);
  }
}

void BtubeCreatorModule::event()
{
  StoreObjPtr<ParticleList> plist(m_listName);
  if (!plist) {
    B2ERROR("ParticleList " << m_listName << " not found");
    return;
  }

  if (m_decayString == "") {
    B2ERROR("decay descriptor cannot be empty");
    return;
  }
  analysis::RaveSetup::initialize(1, m_Bfield);

  std::vector<unsigned int> toRemove;
  unsigned int n = plist->getListSize();

  for (unsigned i = 0; i < n; i++) {
    Particle* particle = plist->getParticle(i);

    std::vector<Particle*> daughtervec = particle->getDaughters();
    std::vector<const Particle*> selParticles = m_decaydescriptor.getSelectionParticles(particle);
    if (selParticles.size() > 1) {
      B2FATAL("Please select only one daughter which will be used as reference to create Btube");
    }

    int selectindex = 1;
    if (daughtervec.at(0) == selParticles.at(0)) selectindex = 0 ;

    Particle* tubecreatorB = const_cast<Particle*>(particle->getDaughter(selectindex));
    Particle* otherB = const_cast<Particle*>(particle->getDaughter(1 - selectindex));

    if ((tubecreatorB->getVertexErrorMatrix()(2, 2)) == 0.0) {
      B2FATAL("Please perform a vertex fit of the fully reconstructed B before calling this module");
    }

    //make a copy of tubecreatorB so as not to modify the original object

    Particle tubecreatorBCopy(tubecreatorB->get4Vector(), tubecreatorB->getPDGCode());
    tubecreatorBCopy.setVertex(tubecreatorB->getVertex());
    tubecreatorBCopy.setMomentumVertexErrorMatrix(tubecreatorB->getMomentumVertexErrorMatrix());

    if (m_verbose) {
      B2DEBUG(10, "fullreco B decay vertex: ");
      B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << tubecreatorBCopy.getVertex()[0] << "," << std::fixed << std::setprecision(
                20) << tubecreatorBCopy.getVertex()[1] << "," << std::fixed << std::setprecision(20) << tubecreatorBCopy.getVertex()[2] << "}");
    }
    bool ok0 = doVertexFit(&tubecreatorBCopy);

    if (ok0) {
      particle->setVertex(tubecreatorBCopy.getVertex());
      particle->setMomentumVertexErrorMatrix(tubecreatorBCopy.getMomentumVertexErrorMatrix());

      tubecreatorB->writeExtraInfo("prod_vtx_x", tubecreatorBCopy.getVertex()[0]);
      tubecreatorB->writeExtraInfo("prod_vtx_y", tubecreatorBCopy.getVertex()[1]);
      tubecreatorB->writeExtraInfo("prod_vtx_z", tubecreatorBCopy.getVertex()[2]);
      tubecreatorB->writeExtraInfo("prod_vtx_cov00", tubecreatorBCopy.getVertexErrorMatrix()(0, 0));
      tubecreatorB->writeExtraInfo("prod_vtx_cov01", tubecreatorBCopy.getVertexErrorMatrix()(0, 1));
      tubecreatorB->writeExtraInfo("prod_vtx_cov02", tubecreatorBCopy.getVertexErrorMatrix()(0, 2));
      tubecreatorB->writeExtraInfo("prod_vtx_cov10", tubecreatorBCopy.getVertexErrorMatrix()(1, 0));
      tubecreatorB->writeExtraInfo("prod_vtx_cov11", tubecreatorBCopy.getVertexErrorMatrix()(1, 1));
      tubecreatorB->writeExtraInfo("prod_vtx_cov12", tubecreatorBCopy.getVertexErrorMatrix()(1, 2));
      tubecreatorB->writeExtraInfo("prod_vtx_cov20", tubecreatorBCopy.getVertexErrorMatrix()(2, 0));
      tubecreatorB->writeExtraInfo("prod_vtx_cov21", tubecreatorBCopy.getVertexErrorMatrix()(2, 1));
      tubecreatorB->writeExtraInfo("prod_vtx_cov22", tubecreatorBCopy.getVertexErrorMatrix()(2, 2));

      tubecreatorB->writeExtraInfo("Px_after_avf", (tubecreatorBCopy.get4Vector()).Px());
      tubecreatorB->writeExtraInfo("Py_after_avf", (tubecreatorBCopy.get4Vector()).Py());
      tubecreatorB->writeExtraInfo("Pz_after_avf", (tubecreatorBCopy.get4Vector()).Pz());
      tubecreatorB->writeExtraInfo("E_after_avf", (tubecreatorBCopy.get4Vector()).E());

      Eigen::Matrix<double, 3, 1> tubecreatorBOriginpos(tubecreatorBCopy.getVertex()[0], tubecreatorBCopy.getVertex()[1],
                                                        tubecreatorBCopy.getVertex()[2]);
      TLorentzVector v4Final = tubecreatorBCopy.get4Vector();
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * v4Final;
      TLorentzVector vecNew(-1 * vec.Px(), -1 * vec.Py(), -1 * vec.Pz(), vec.E());
      TLorentzVector v4FinalNew = T.rotateCmsToLab() * vecNew;

      if (m_verbose) {
        B2DEBUG(10, "beamspot center :");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << m_BeamSpotCenter.X() << "," << std::fixed << std::setprecision(
                  20) << m_BeamSpotCenter.Y() << "," << std::fixed << std::setprecision(20) << m_BeamSpotCenter.Z() << "}");
        B2DEBUG(10, "beamspot cov :");

        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) <<  m_beamSpotCov(0,
                0) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(0,
                    1) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(0, 2) << "},");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) <<  m_beamSpotCov(1,
                0) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(1,
                    1) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(1, 2) << "},");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << m_beamSpotCov(2,
                0) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(2,
                    1) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(2, 2) << "}");
      }
      TMatrixFSym pp = (tubecreatorBCopy.getMomentumErrorMatrix()).GetSub(0, 2, 0, 2, "S");
      double pe = tubecreatorBCopy.getMomentumErrorMatrix()(2, 2);
      TMatrixFSym pv = tubecreatorBCopy.getVertexErrorMatrix();

      // start rotation

      double theta = v4FinalNew.Theta();
      double phi = v4FinalNew.Phi();

      double st = TMath::Sin(theta);
      double ct = TMath::Cos(theta);
      double sp = TMath::Sin(phi);
      double cp = TMath::Cos(phi);

      TMatrix r2z(3, 3);  r2z(2, 2) = 1;
      r2z(0, 0) = cp; r2z(0, 1) = -1 * sp;
      r2z(1, 0) = sp; r2z(1, 1) = cp;

      TMatrix r2y(3, 3);  r2y(1, 1) = 1;
      r2y(0, 0) = ct; r2y(0, 2) = st;
      r2y(2, 0) = -1 * st; r2y(2, 2) = ct;

      TMatrix r2(3, 3);  r2.Mult(r2z, r2y);
      TMatrix r2t(3, 3); r2t.Transpose(r2);

      TMatrix longerror(3, 3); longerror(2, 2) = 1000;
      TMatrix longerror_temp(3, 3); longerror_temp.Mult(r2, longerror);
      TMatrix longerrorRotated(3, 3); longerrorRotated.Mult(longerror_temp, r2t);

      TMatrix pvNew(3, 3);
      pvNew += pv;
      pvNew += longerrorRotated;

      TMatrixFSym errNew(7);
      errNew.SetSub(0, 0, pp);
      errNew.SetSub(4, 4, pvNew);
      errNew(3, 3) = pe;

      TMatrixFSym tubeMat(3);
      tubeMat.SetSub(0, 0, pvNew);

      if (m_verbose) {
        B2DEBUG(10, "B origin error matrix  :  ");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << pv(0, 0) << "," << std::fixed << std::setprecision(20) << pv(0,
                1) << "," << std::fixed << std::setprecision(20) << pv(0, 2) << "},");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << pv(1, 0) << "," << std::fixed << std::setprecision(20) << pv(1,
                1) << "," << std::fixed << std::setprecision(20) << pv(1, 2) << "},");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << pv(2, 0) << "," << std::fixed << std::setprecision(20) << pv(2,
                1) << "," << std::fixed << std::setprecision(20) << pv(2, 2) << "}");

        B2DEBUG(10, "B tube error matrix  :  ");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) <<  pvNew(0, 0) << "," << std::fixed << std::setprecision(20) << pvNew(0,
                1) << "," << std::fixed << std::setprecision(20) << pvNew(0, 2) << "},");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) <<  pvNew(1, 0) << "," << std::fixed << std::setprecision(20) << pvNew(1,
                1) << "," << std::fixed << std::setprecision(20) << pvNew(1, 2) << "},");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << pvNew(2, 0) << "," << std::fixed << std::setprecision(20) << pvNew(2,
                1) << "," << std::fixed << std::setprecision(20) << pvNew(2, 2) << "}");

        B2DEBUG(10, "B origin  ");
        B2DEBUG(10, "{" << std::fixed << std::setprecision(20) << tubecreatorBCopy.getVertex()[0] << "," << std::fixed << std::setprecision(
                  20) << tubecreatorBCopy.getVertex()[1] << "," << std::fixed << std::setprecision(20) << tubecreatorBCopy.getVertex()[2] << "}");
      }

      tubecreatorBCopy.setMomentumVertexErrorMatrix(errNew);

      Btube* tubeconstraint = tubeArray.appendNew(Btube());
      otherB->addRelationTo(tubeconstraint);
      tubeconstraint->setTubeCenter(tubecreatorBOriginpos);
      tubeconstraint->setTubeMatrix(tubeMat);

      otherB->writeExtraInfo("TubePosX", tubecreatorBCopy.getVertex()[0]);
      otherB->writeExtraInfo("TubePosY", tubecreatorBCopy.getVertex()[1]);
      otherB->writeExtraInfo("TubePosZ", tubecreatorBCopy.getVertex()[2]);

      otherB->writeExtraInfo("TubeCov00", pvNew(0, 0));
      otherB->writeExtraInfo("TubeCov01", pvNew(0, 1));
      otherB->writeExtraInfo("TubeCov02", pvNew(0, 2));
      otherB->writeExtraInfo("TubeCov10", pvNew(1, 0));
      otherB->writeExtraInfo("TubeCov11", pvNew(1, 1));
      otherB->writeExtraInfo("TubeCov12", pvNew(1, 2));
      otherB->writeExtraInfo("TubeCov20", pvNew(2, 0));
      otherB->writeExtraInfo("TubeCov21", pvNew(2, 1));
      otherB->writeExtraInfo("TubeCov22", pvNew(2, 2));

      otherB->writeExtraInfo("TubeDirX", v4FinalNew.Px());
      otherB->writeExtraInfo("TubeDirY", v4FinalNew.Py());
      otherB->writeExtraInfo("TubeDirZ", v4FinalNew.Pz());

    }
    if (!ok0) toRemove.push_back(particle->getArrayIndex());
  }
  plist->removeParticles(toRemove);

  analysis::RaveSetup::getInstance()->reset();
}

bool BtubeCreatorModule::doVertexFit(Particle* mother)
{
  analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);

  analysis::RaveVertexFitter rsg;
  rsg.addTrack(mother);
  int nvert = rsg.fit("avf");

  if (nvert == 1) {
    rsg.updateDaughters();
  } else {return false;}
  return true;
}
