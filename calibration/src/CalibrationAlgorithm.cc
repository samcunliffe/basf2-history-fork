#include <calibration/CalibrationAlgorithm.h>

using namespace std;
using namespace Belle2;
using namespace calibration;

const std::string CalibrationAlgorithm::RUN_RANGE_OBJ_NAME = "__ca_data_range";

CalibrationAlgorithm::E_Result CalibrationAlgorithm::execute(vector< Belle2::calibration::CalibrationAlgorithm::ExpRun > runs)
{
  m_payloads.clear();

  // Let's check that we have the data by accessing an object
  // created by all collector modules by their base class
  StoreObjPtr<CalibRootObj<RunRange>> storeobj(m_prefix + "_" + RUN_RANGE_OBJ_NAME, DataStore::c_Persistent);
  if (!storeobj.isValid()) {
    B2ERROR("Could not access collected data in datastore.");
    return calibration::CalibrationAlgorithm::c_Failure;
  }

  // If no runs are provided, just take all collected
  if (runs.empty())
    runs = getRunListFromAllData();

  if (runs.empty()) {
    B2ERROR("No data available at all.");
    return calibration::CalibrationAlgorithm::c_Failure;
  }

  std::sort(runs.begin(), runs.end());

  // After this line, the getObject<...>(...) helpers start to work
  m_runs = runs;

  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);

  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
    // Data collected with granularity=all
    if (m_runs != getRunListFromAllData()) {
      B2ERROR("The data is collected with granularity=all (exp=-1,run=-1), but you seem to request calibration for specific runs.");
      // Take the (-1,-1)
      m_runs = getRunListFromAllData();
      caRange = getIovFromData();
    }
  }

  IntervalOfValidity dataRange = getIovFromData();
  if (dataRange.empty()) {
    B2ERROR("No data available for selected runs.");
    return calibration::CalibrationAlgorithm::c_Failure;
  }

  if (!dataRange.contains(caRange)) {
    B2ERROR("The requested range for calibration is not contained within range of collected data.");
    // TODO: remove runs outside collected data range...?
    B2INFO("If you want to extend the validity range of calibration constants beyond data, you should do it before DB commit manually.");

    if (!dataRange.overlaps(caRange)) {
      B2ERROR("The requested calibration range does not even overlap with the collected data.");
      // We should get just c_NotEnoughData or c_Failure all times, so don't start and fail
      return calibration::CalibrationAlgorithm::c_Failure;
    }
  }


  return calibrate();
}

vector< CalibrationAlgorithm::ExpRun > CalibrationAlgorithm::string2RunList(string list)
{
  std::vector<ExpRun> result;

  if (list == "")
    return result;

  std::vector<std::string> runs;
  boost::algorithm::split(runs, list, boost::is_any_of(","));

  for (auto exprunstr : runs) {
    std::vector<std::string> exprun;
    boost::algorithm::split(exprun, exprunstr, boost::is_any_of("."));
    if (exprun.size() != 2)
      B2FATAL("Error in parsing object validity");
    result.push_back(std::make_pair(std::stoi(exprun[0]), std::stoi(exprun[1])));
  }
  return result;
}

string CalibrationAlgorithm::runList2String(vector< CalibrationAlgorithm::ExpRun >& list)
{
  std::string str("");
  for (auto run : list) {
    if (str != "")
      str = str + ",";

    str = str + std::to_string(run.first) + "." + std::to_string(run.second);
  }
  return str;
}

string CalibrationAlgorithm::runList2String(CalibrationAlgorithm::ExpRun run)
{
  std::vector<ExpRun> runlist;
  runlist.push_back(run);

  return runList2String(runlist);
}

IntervalOfValidity CalibrationAlgorithm::getIovFromData()
{
  auto& range = getObject<RunRange>(RUN_RANGE_OBJ_NAME);
  return range.getIntervalOfValidity();
}

void CalibrationAlgorithm::saveCalibration(TObject* data, string name, IntervalOfValidity iov)
{
  MyDBQuery query("dbstore", name, data, iov);
  m_payloads.push_back(query);
}

void CalibrationAlgorithm::saveCalibration(TObject* data, string name)
{
  if (m_runs.empty())
    return;

  IntervalOfValidity iov;
  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);

  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
    // For granularity=all, automatic IOV is defined by range of collected data
    iov = getIovFromData();
  } else {
    int expMin = m_runs[0].first;
    int runMin = m_runs[0].second;
    int expMax = m_runs[m_runs.size() - 1].first;
    int runMax = m_runs[m_runs.size() - 1].second;
    iov = IntervalOfValidity(expMin, runMin, expMax, runMax);
  }

  saveCalibration(data, name, iov);
}

bool CalibrationAlgorithm::commit()
{
  if (m_payloads.empty())
    return false;
  std::list<Database::DBQuery> output;
  for (auto pay : m_payloads) {
    Database::DBQuery query(pay.package, pay.module, pay.object, pay.iov);
    output.push_back(query);
  }
  return Database::Instance().storeData(output);
}

vector< CalibrationAlgorithm::ExpRun > CalibrationAlgorithm::getRunListFromAllData()
{
  string fullName(m_prefix + "_" + RUN_RANGE_OBJ_NAME);
  StoreObjPtr<CalibRootObj<RunRange>> storeobj(fullName, DataStore::c_Persistent);

  std::vector<ExpRun> list;
  if (!storeobj.isValid())
    return list;

  for (auto objAndIov : storeobj->getObjects()) {
    std::vector<ExpRun> sublist = string2RunList(objAndIov.second);
    for (auto exprun : sublist)
      list.push_back(exprun);
  }
  std::unique(list.begin(), list.end());
  return list;
}
