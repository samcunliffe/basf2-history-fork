#include <calibration/TestCalibrationAlgorithm.h>

#include <TProfile.h>
#include <TTree.h>
#include <TRandom.h>
#include <TClonesArray.h>
#include <calibration/dbobjects/TestCalibObject.h>
#include <calibration/dbobjects/TestCalibMean.h>
#include <memory>


using namespace std;
using namespace Belle2;

TestCalibrationAlgorithm::TestCalibrationAlgorithm(): CalibrationAlgorithmNew("CaTest")
{
  setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which just gets mean of a test histogram collected by      \n"
    "  CaTest module and provides a DB object with another histogram with one       \n"
    "  entry at calibrated value.                                                   \n"
    " ------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithmNew::EResult TestCalibrationAlgorithm::calibrate()
{
  // Pulling in data from collector output. It now returns shared_ptr<T> so the underlying pointer
  // will delete itself automatically at the end of this scope unless you do something
  auto ttree = getTreeObjectPtr("MyTree");
  auto hist = getObjectPtr<TH1F>("MyHisto");
  B2INFO("Number of Entries in MyTree was " << ttree->GetEntries());
  B2INFO("Number of Entries in MyHisto was " << hist->GetEntries());

  float mean = hist->GetMean();
  float meanError = hist->GetMeanError();

  B2INFO("Mean of MyHisto was " << mean);
  B2INFO("Mean Error of MyHisto was " << meanError);
//  auto& mille = getObject<MilleData>("test_mille");

  if (hist->GetEntries() < 100 || ttree->GetEntries() < 100)
    return c_NotEnoughData;

  // Example of saving a DBObject.
  TestCalibMean* correction = new TestCalibMean(mean, meanError);
  saveCalibration(correction, "TestCalibMean");

  // Example of saving a Belle2 DBArray of DBObjects defined in the dbobjects directory
  TClonesArray* exampleDBArrayConstants = new TClonesArray("Belle2::TestCalibObject", 2);
  float val = 0.0;
  for (int i = 0; i < 2; i++) {
    val += 1.0;
    new((*exampleDBArrayConstants)[i]) TestCalibObject(val);
  }
  saveCalibration(exampleDBArrayConstants, "TestCalibObjects");
  return c_OK;

//  // Iterate until we find answer to the most fundamental question...
//  B2INFO("mean: " << mean);
//  B2INFO("meanError: " << meanError);
//  if (mean - 42. >= 1.)
//    return c_Iterate;
//
}
