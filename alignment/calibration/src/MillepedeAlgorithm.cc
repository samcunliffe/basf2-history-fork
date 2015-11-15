#include <alignment/calibration/MillepedeAlgorithm.h>

#include <alignment/PedeApplication.h>
#include <alignment/PedeResult.h>
#include <alignment/dataobjects/PedeSteering.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <alignment/GlobalLabel.h>

#include <TCanvas.h>

using namespace std;
using namespace Belle2;
using namespace alignment;

MillepedeAlgorithm::MillepedeAlgorithm() : CalibrationAlgorithm("MillepedeCollector")
{
  setDescription("Millepede calibration & alignment algorithm");
}

CalibrationAlgorithm::EResult MillepedeAlgorithm::calibrate()
{
  auto& mille = getObject<MilleData>("mille");
  B2INFO(" Mean of Chi2 / NDF of tracks before calibration: " << getObject<TH1F>("chi2/ndf").GetMean();)

  if (mille.getFiles().empty()) {
    B2INFO("No binary files.");
    return c_Failure;
  }

  m_steering.getFiles() = mille.getFiles();

  // Here the calibration is actually done
  PedeApplication pede;
  m_result = pede.calibrate(m_steering);

  if (!pede.success() || !m_result.isValid()) {
    B2INFO(pede.getExitMessage());
    return c_Failure;
  }

  if (m_result.getNoParameters() == 0) {
    B2INFO("No parameters to store. Failure.");
    return c_Failure;
  }

  int nVXDparams = 0;
  int nCDCparams = 0;
  int nBKLMparams = 0;
  for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
    if (!m_result.isParameterDetermined(ipar)) continue;

    GlobalLabel param(m_result.getParameterLabel(ipar));
    if (param.isVXD()) ++nVXDparams;
    if (param.isCDC()) ++nCDCparams;
    if (param.isKLM()) ++nBKLMparams;
  }
  if (!nVXDparams)
    B2WARNING("No VXD parameters determined");
  if (!nCDCparams)
    B2WARNING("No CDC parameters determined");
  if (!nBKLMparams)
    B2WARNING("No BKLM parameters determined");

  // Now here comes some experimental part...
  // The main reason to do this, we are only computing
  // CORRECTIONS to previous alignment, not absolute values.
  // Therefore we must add our result to all previous constants...

  // Set of of all (exp, run) occured in data collection
  auto& runSet = getObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME);
  // Objects in DB we are interested in
  std::list<Database::DBQuery> belle2Constants;
  if (nVXDparams)
    belle2Constants.push_back(Database::DBQuery("dbstore", "VXDAlignment"));
  if (nCDCparams)
    belle2Constants.push_back(Database::DBQuery("dbstore", "CDCCalibration"));
  if (nBKLMparams)
    belle2Constants.push_back(Database::DBQuery("dbstore", "BKLMAlignment"));
  // Maps (key is object address in databse cache - same objects are not added twice)
  std::map<string, VXDAlignment*> previousVXD;
  std::map<string, CDCCalibration*> previousCDC;
  std::map<string, BKLMAlignment*> previousBKLM;
  // Collect all distinct existing objects in DB:
  for (auto& exprun : runSet.getExpRunSet()) {
    // Ask DB for data at Event 1 in each run
    auto event1 = EventMetaData(1, exprun.second, exprun.first);
    Database::Instance().getData(event1, belle2Constants);
    for (auto& payload : belle2Constants) {
      if (auto vxd = dynamic_cast<VXDAlignment*>(payload.object)) {
        previousVXD[to_string(payload.iov)] = vxd;
      }
      if (auto cdc = dynamic_cast<CDCCalibration*>(payload.object)) {
        previousCDC[to_string(payload.iov)] = cdc;
      }
      if (auto bklm = dynamic_cast<BKLMAlignment*>(payload.object)) {
        previousBKLM[to_string(payload.iov)] = bklm;
      }
    }
  }
  // All objects have to be re-created, with new constant values...
  std::map<string, VXDAlignment*> newVXD;
  std::map<string, CDCCalibration*> newCDC;
  std::map<string, BKLMAlignment*> newBKLM;

  if (nVXDparams)
    for (auto& vxd : previousVXD)
      newVXD[vxd.first] = new VXDAlignment(*vxd.second);

  if (nCDCparams)
    for (auto& cdc : previousCDC)
      newCDC[cdc.first] = new CDCCalibration(*cdc.second);

  if (nBKLMparams)
    for (auto& bklm : previousBKLM)
      newBKLM[bklm.first] = new BKLMAlignment(*bklm.second);

  if (newVXD.empty() && nVXDparams) {
    B2INFO("No previous VXDAlignment found. First update from nominal.");
    newVXD.insert({to_string(getIovFromData()), new VXDAlignment()});
  }

  if (newCDC.empty() && nCDCparams) {
    B2INFO("No previous CDCCalibration found. First update from nominal.");
    newCDC.insert({to_string(getIovFromData()), new CDCCalibration()});
  }

  if (newBKLM.empty() && nBKLMparams) {
    B2INFO("No previous BKLMAlignment found. First update from nominal.");
    newBKLM.insert({to_string(getIovFromData()), new BKLMAlignment()});
  }

  double maxCorrectionPull = 0.;
  int maxCorrectionPullLabel = 0;
  int nParams = 0;
  double paramChi2 = 0.;

  // Loop over all determined parameters:
  for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
    if (!m_result.isParameterDetermined(ipar)) continue;

    GlobalLabel param(m_result.getParameterLabel(ipar));
    double correction = m_result.getParameterCorrection(ipar);
    double error = m_result.getParameterError(ipar);
    double pull = correction / error;

    ++nParams;
    paramChi2 += pull * pull;

    if (fabs(pull) > fabs(maxCorrectionPull)) {
      maxCorrectionPull = pull;
      maxCorrectionPullLabel = param.label();
    }

    if (param.isVXD()) {
      // Add correction to all objects
      for (auto& vxd : newVXD) {
        vxd.second->add(param.getVxdID(), param.getParameterId(), correction, m_invertSign);
      }
    }

    if (param.isCDC()) {
      // Add correction to all objects
      for (auto& cdc : newCDC) {
        cdc.second->add(param.getWireID(), param.getParameterId(), correction, m_invertSign);
      }
    }

    if (param.isKLM()) {
      // Add correction to all objects
      for (auto& bklm : newBKLM) {
        bklm.second->add(param.getWireID(), param.getParameterId(), correction, m_invertSign);
      }
    }
  }

  // Save (possibly updated) objects
  for (auto& vxd : newVXD)
    saveCalibration(vxd.second, "VXDAlignment", to_IOV(vxd.first));
  for (auto& cdc : newCDC)
    saveCalibration(cdc.second, "CDCCalibration", to_IOV(cdc.first));
  for (auto& bklm : newBKLM)
    saveCalibration(bklm.second, "BKLMAlignment", to_IOV(bklm.first));

  commit();

  if (paramChi2 / nParams > 1. || fabs(maxCorrectionPull) > 50.) {
    if (fabs(maxCorrectionPull) > 50.)
      B2INFO("Largest correction/error is " << maxCorrectionPull << " for parameter with label " << maxCorrectionPullLabel);
    if (paramChi2 / nParams >= 1.)
      B2INFO("Parameter corrections incosistent with small change, e.g. sum[(correction/error)^2]/#params = " << paramChi2 / nParams
             << " = " << paramChi2 << " / " << nParams << " > 1.");
    B2INFO("Requesting iteration.");
    return c_Iterate;
  }

  return c_OK;
}

