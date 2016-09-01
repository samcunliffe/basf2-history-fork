#include <skim/softwaretrigger/modules/SoftwareTriggerModule.h>
#include <TFile.h>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SoftwareTrigger)


/// Create a new module instance and set the parameters.
SoftwareTriggerModule::SoftwareTriggerModule() : Module(), m_resultStoreObjectPointer("", DataStore::c_Event)
{
  setDescription("Module to perform cuts on various variables in the event. The cuts can be defined"
                 "by elements loaded from the database. Each cut is executed and its result stored."
                 "The return value of this module is an integer, which is:\n"
                 "if reject is more important than accept:"
                 "* 1: if one of the accept cuts has a true result and none of the reject cuts is false ( = accepted)\n"
                 "* 0: if neither one of the accept cuts is true nor one of the reject cuts false ( = don't know)\n"
                 "* -1: if one of the reject cuts is false ( = rejected)\n"
                 "Please note that the reject cuts override the accept cuts decision in this case!\n"
                 "if accept is more important than reject:\n"
                 "* 1: if one of the accept cuts has a true result. ( = accepted)\n"
                 "* 0: if neither one of the accept cuts is true nor one of the reject cuts false ( = don't know)\n"
                 "* -1: if one of the reject cuts is false and none of the accept cuts is true ( = rejected)\n"
                 "Please note that the accept cuts override the reject cuts decision in this case!"
                 "What is more important can be controlled by the flag acceptOverridesReject, which is off by default "
                 "(so reject is more important than accept by default).");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("baseIdentifier", m_param_baseIdentifier, "Base identifier for all cuts downloaded from database. "
           "The full db name of the cuts will be <base_identifier>/<cut_identifier>. You can only choose one identifier "
           "to make clear that all chosen cuts belong together (and should be calculated together).",
           m_param_baseIdentifier);

  addParam("cutIdentifiers", m_param_cutIdentifiers, "List of identifiers for the different cuts. The module will "
           "look for database entries with the form <base_identifier>/<cut_identifier> for each cut_identifier in the "
           "list of strings you provide here. Make sure to choose those wisely as the modules return value depends "
           "on these cuts.");

  addParam("acceptOverridesReject", m_param_acceptOverridesReject, "Flag to control which class of cuts is "
           "more \"important\": accept cuts or reject cuts.", m_param_acceptOverridesReject);

  addParam("resultStoreArrayName", m_param_resultStoreArrayName, "Store Object Pointer name for storing the "
           "trigger decision.", m_param_resultStoreArrayName);

  addParam("storeDebugOutput", m_param_storeDebugOutput, "Flag to save the results of the calculations leading "
           "to the the trigger decisions into a ROOT file. The file path and name of this file can be handled by the "
           "debugOutputFileName parameter.", m_param_storeDebugOutput);

  addParam("debugOutputFileName", m_param_debugOutputFileName, "File path and name of the ROOT "
           "file, in which the results of the calculation are stored, if storeDebugOutput is "
           "turned on. Please note that already present files will be overridden. "
           "ATTENTION: This debugging mode does not work in parallel processing.", m_param_debugOutputFileName);
}

/// Initialize/Require the DB object pointers and any needed store arrays.
void SoftwareTriggerModule::initialize()
{
  m_resultStoreObjectPointer.registerInDataStore(m_param_resultStoreArrayName);
  if (m_param_baseIdentifier == "fast_reco") {
    m_fastRecoCalculation.requireStoreArrays();
  } else if (m_param_baseIdentifier == "hlt") {
    m_hltCalculation.requireStoreArrays();
  } else {
    B2FATAL("You gave an invalid base identifier " << m_param_baseIdentifier << ".");
  }

  m_dbHandler.initialize(m_param_baseIdentifier, m_param_cutIdentifiers);

  if (m_param_storeDebugOutput) {
    m_debugOutputFile.reset(TFile::Open(m_param_debugOutputFileName.c_str(), "RECREATE"));
    if (not m_debugOutputFile) {
      B2ERROR("Could not open debug output file. Aborting.");
    }
    m_debugTTree.reset(new TTree("software_trigger_results", "software_trigger_results"));
    if (not m_debugTTree) {
      B2ERROR("Could not create debug output tree. Aborting.");
    }
  }
}

void SoftwareTriggerModule::terminate()
{
  if (m_debugTTree) {
    m_debugOutputFile->cd();
    m_debugOutputFile->Write();
    m_debugTTree.reset();
    m_debugOutputFile.reset();
  }
}

/// Run over all cuts and check them. If one of the cuts yields true, give a positive return value of the module.
void SoftwareTriggerModule::event()
{
  if (not m_resultStoreObjectPointer.isValid()) {
    m_resultStoreObjectPointer.construct();
  }

  B2DEBUG(100, "Doing the calculation...");
  SoftwareTriggerObject prefilledObject;

  if (m_param_baseIdentifier == "fast_reco") {
    prefilledObject = m_fastRecoCalculation.fillInCalculations();
  } else if (m_param_baseIdentifier == "hlt") {
    prefilledObject = m_hltCalculation.fillInCalculations();
  }
  B2DEBUG(100, "Successfully finished the calculation.");

  if (m_param_storeDebugOutput) {
    B2DEBUG(100, "Storing debug output as requested.");
    if (m_param_baseIdentifier == "fast_reco") {
      m_fastRecoCalculation.writeDebugOutput(m_debugTTree);
    } else if (m_param_baseIdentifier == "hlt") {
      m_hltCalculation.writeDebugOutput(m_debugTTree);
    }
    B2DEBUG(100, "Finished storing the debug output.");
  }

  for (const auto& cutWithName : m_dbHandler.getCutsWithNames()) {
    const std::string& cutIdentifier = cutWithName.first;
    const auto& cut = cutWithName.second;
    B2DEBUG(100, "Next processing cut " << cutIdentifier << " (" << cut->decompile() << "), with a prescale of " <<
            cut->getPreScaleFactor());
    const SoftwareTriggerCutResult& cutResult = cut->checkPreScaled(prefilledObject);
    //TODO: B2DEBUG(100, "The result if the trigger cut is " << cutResult);
    m_resultStoreObjectPointer->addResult(cutIdentifier, cutResult);
  }

  setReturnValue(m_resultStoreObjectPointer->getTotalResult(m_param_acceptOverridesReject));
}

/// Check if the cut representations in the database have changed and download newer ones if needed.
void SoftwareTriggerModule::beginRun()
{
  m_dbHandler.checkForChangedDBEntries();
}