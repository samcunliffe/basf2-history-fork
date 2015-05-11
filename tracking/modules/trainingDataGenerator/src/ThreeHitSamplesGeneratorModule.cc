/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/trainingDataGenerator/ThreeHitSamplesGeneratorModule.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/spacePointCreation/PurityCalculatorTools.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <vector>
#include <utility> // pair

using namespace std;
using namespace Belle2;

REG_MODULE(ThreeHitSamplesGenerator)

ThreeHitSamplesGeneratorModule::ThreeHitSamplesGeneratorModule() :
  Module()
{
  setDescription("Module for generating training data samples. TODO: complete documentation!");
  addParam("containerName", m_PARAMcontainerName, "Name of the SpacePoint container in the StoreArray");
}

// =========================================== INITIALIZE ==============================================================
void ThreeHitSamplesGeneratorModule::initialize()
{
  B2INFO("ThreeHitSamplesGenerator ----------------- initialize() -----------------------");
  StoreArray<SpacePointTrackCand>::required(m_PARAMcontainerName);
}

// ============================================== EVENT ===============================================================
void ThreeHitSamplesGeneratorModule::event()
{
  StoreArray<SpacePointTrackCand> spacePointTCs(m_PARAMcontainerName);

  for (int iTC = 0; iTC < spacePointTCs.getEntries(); ++iTC) {
    B2DEBUG(150, "Calculating purities for SP container " << iTC << ", name: " << spacePointTCs.getName());
    vector<pair<int, double> > purities = calculatePurity(spacePointTCs[iTC]);
  }



}

// ============================================== TERMINATE ===========================================================
void ThreeHitSamplesGeneratorModule::terminate()
{

}

