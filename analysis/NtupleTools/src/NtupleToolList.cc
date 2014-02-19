#include <framework/logging/Logger.h>
#include <analysis/NtupleTools/NtupleToolList.h>
#include <analysis/NtupleTools/NtupleKinematicsTool.h>
#include <analysis/NtupleTools/NtupleEventMetaDataTool.h>
#include <analysis/NtupleTools/NtupleDeltaEMbcTool.h>
#include <analysis/NtupleTools/NtupleMCTruthTool.h>
#include <analysis/NtupleTools/NtupleMCHierarchyTool.h>
#include <analysis/NtupleTools/NtupleMCKinematicsTool.h>
#include <analysis/NtupleTools/NtuplePIDTool.h>
#include <analysis/NtupleTools/NtupleTrackTool.h>
#include <analysis/NtupleTools/NtupleShowerTool.h>
#include <analysis/NtupleTools/NtupleECLPi0Tool.h>
#include <analysis/NtupleTools/NtupleRecoStatsTool.h>
#include <analysis/NtupleTools/NtupleDetectorStatsRecTool.h>
#include <analysis/NtupleTools/NtupleDetectorStatsSimTool.h>
#include <analysis/NtupleTools/NtupleMCReconstructibleTool.h>
#include <analysis/NtupleTools/NtupleVertexTool.h>
#include <analysis/NtupleTools/NtupleMCVertexTool.h>
#include <analysis/NtupleTools/NtupleCustomFloatsTool.h>
#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>
#include <analysis/NtupleTools/NtupleRecoilKinematicsTool.h>
#include <analysis/NtupleTools/NtupleExtraEnergyTool.h>

using namespace Belle2;
using namespace std;

NtupleFlatTool* NtupleToolList::create(string strName, TTree* tree, DecayDescriptor& d)
{

  size_t iPosBracket1 = strName.find("(");
  string strOption("");
  string strToolName("");

  B2INFO(strName);
  B2INFO("Length : " << strName.size());
  B2INFO("Position of ( : " << iPosBracket1);
  B2INFO("Position of ) : " << strName.find(")"));

  if (iPosBracket1 != string::npos) {
    if (strName.find(")") == strName.size() - 1) {
      strToolName = strName.substr(0, iPosBracket1);
      strOption = strName.substr(iPosBracket1 + 1, strName.size() - iPosBracket1 - 2);
    } else {
      B2WARNING("Incorrect NtupleTool name: " << strName);
      return NULL;
    }
  } else {
    strToolName = strName;
  }

  B2INFO("Options : " << strOption);

  if (strToolName.compare("Kinematics") == 0) return new NtupleKinematicsTool(tree, d);
  else if (strToolName.compare("EventMetaData") == 0) return new NtupleEventMetaDataTool(tree, d);
  else if (strToolName.compare("DeltaEMbc") == 0) return new NtupleDeltaEMbcTool(tree, d);
  else if (strToolName.compare("MCTruth") == 0) return new NtupleMCTruthTool(tree, d);
  else if (strToolName.compare("MCHierarchy") == 0) return new NtupleMCHierarchyTool(tree, d);
  else if (strToolName.compare("MCKinematics") == 0) return new NtupleMCKinematicsTool(tree, d);
  else if (strToolName.compare("PID") == 0) return new NtuplePIDTool(tree, d);
  else if (strToolName.compare("RecoStats") == 0) return new NtupleRecoStatsTool(tree, d);
  else if (strToolName.compare("DetectorStatsRec") == 0) return new NtupleDetectorStatsRecTool(tree, d);
  else if (strToolName.compare("DetectorStatsSim") == 0) return new NtupleDetectorStatsSimTool(tree, d);
  else if (strToolName.compare("MCReconstructible") == 0) return new NtupleMCReconstructibleTool(tree, d);
  else if (strToolName.compare("Track") == 0) return new NtupleTrackTool(tree, d);
  else if (strToolName.compare("Shower") == 0) return new NtupleShowerTool(tree, d);
  else if (strToolName.compare("ECLPi0") == 0) return new NtupleECLPi0Tool(tree, d);
  else if (strToolName.compare("Vertex") == 0) return new NtupleVertexTool(tree, d);
  else if (strToolName.compare("MCVertex") == 0) return new NtupleMCVertexTool(tree, d);
  else if (strToolName.compare("CustomFloats") == 0) return new NtupleCustomFloatsTool(tree, d, strOption);
  else if (strToolName.compare("ROEMultiplicities") == 0) return new NtupleROEMultiplicitiesTool(tree, d);
  else if (strToolName.compare("RecoilKinematics") == 0) return new NtupleRecoilKinematicsTool(tree, d);
  else if (strToolName.compare("ExtraEnergy") == 0) return new NtupleExtraEnergyTool(tree, d);

  B2WARNING("NtupleTool " << strToolName << " is not available!");
  return NULL;
}
