/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/database/DBImportObjPtr.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    const std::string SoftwareTriggerDBHandler::s_dbPackageIdentifier = "software_trigger_cut";

    std::string SoftwareTriggerDBHandler::makeFullCutName(const std::string& baseCutIdentifier,
                                                          const std::string& cutIdentifier)
    {
      assert(baseCutIdentifier.find("&") == std::string::npos);
      assert(cutIdentifier.find("&") == std::string::npos);

      return s_dbPackageIdentifier + "&" + baseCutIdentifier + "&" + cutIdentifier;
    }

    std::string SoftwareTriggerDBHandler::makeFullTriggerMenuName(const std::string& baseIdentifier)
    {
      assert(baseIdentifier.find("&") == std::string::npos);

      return s_dbPackageIdentifier + "&" + baseIdentifier;
    }

    void SoftwareTriggerDBHandler::checkForChangedDBEntries(const std::string& baseIdentifier)
    {
      // In case the whole trigger menu has changed, we start from scratch and reload all triggers.
      if (m_softwareTriggerMenu.hasChanged()) {
        initialize(baseIdentifier);
        return;
      }

      // In all other cases we just check each downloaded cut, if it has changed.
      for (auto& databaseCutEntry : m_databaseObjects) {
        if (databaseCutEntry.hasChanged()) {
          B2ASSERT("The name of the database entry changed! This is not handled properly by the module.",
                   m_cutsWithIdentifier.find(databaseCutEntry.getName()) != m_cutsWithIdentifier.end());
          m_cutsWithIdentifier[databaseCutEntry.getName()] = databaseCutEntry->getCut();
        }
      }
    }

    void SoftwareTriggerDBHandler::initialize(const std::string& baseIdentifier)
    {
      m_databaseObjects.clear();
      m_cutsWithIdentifier.clear();

      m_softwareTriggerMenu = DBObjPtr<SoftwareTriggerMenu>(makeFullTriggerMenuName(baseIdentifier));

      const auto& cutIdentifiers = m_softwareTriggerMenu->getCutIdentifiers();
      m_databaseObjects.reserve(cutIdentifiers.size());

      B2DEBUG(100, "Initializing SoftwareTrigger DB with baseIdentifier " << baseIdentifier << " and " << cutIdentifiers.size() <<
              " cutIdentifiers");

      for (const std::string& cutIdentifier : cutIdentifiers) {
        B2DEBUG(100, "-> with CutIndentifier " << cutIdentifier);

        const std::string& fullIdentifier = makeFullCutName(baseIdentifier, cutIdentifier);
        m_databaseObjects.emplace_back(fullIdentifier);
        if (m_databaseObjects.back()) {
          m_cutsWithIdentifier[fullIdentifier] = m_databaseObjects.back()->getCut();
        } else {
          B2FATAL("There is no DB object with the name " << fullIdentifier);
        }
      }
    }

    bool SoftwareTriggerDBHandler::getAcceptOverridesReject() const
    {
      return m_softwareTriggerMenu->isAcceptMode();
    }

    void SoftwareTriggerDBHandler::upload(const std::unique_ptr<SoftwareTriggerCut>& cut, const std::string& baseCutIdentifier,
                                          const std::string& cutIdentifier, const IntervalOfValidity& iov)
    {
      const std::string& fullCutName = makeFullCutName(baseCutIdentifier, cutIdentifier);
      DBImportObjPtr<DBRepresentationOfSoftwareTriggerCut> cutToUpload(fullCutName);
      cutToUpload.construct(cut);
      cutToUpload.import(iov);
    }

    void SoftwareTriggerDBHandler::uploadTriggerMenu(const std::string& baseCutIdentifier,
                                                     const std::vector<std::string>& cutIdentifiers,
                                                     bool acceptMode,
                                                     const IntervalOfValidity& iov)
    {
      const std::string& fullMenuName = makeFullTriggerMenuName(baseCutIdentifier);
      DBImportObjPtr<SoftwareTriggerMenu> menuToUpload(fullMenuName);
      menuToUpload.construct(cutIdentifiers, acceptMode);
      menuToUpload.import(iov);
    }

    std::unique_ptr<SoftwareTriggerCut> SoftwareTriggerDBHandler::download(const std::string& baseCutIdentifier,
        const std::string& cutIdentifier)
    {
      const std::string& fullCutName = makeFullCutName(baseCutIdentifier, cutIdentifier);
      DBObjPtr<DBRepresentationOfSoftwareTriggerCut> downloadedCut(fullCutName);
      if (downloadedCut) {
        return downloadedCut->getCut();
      } else {
        return std::unique_ptr<SoftwareTriggerCut>();
      }
    }

    const std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>>& SoftwareTriggerDBHandler::getCutsWithNames() const
    {
      return m_cutsWithIdentifier;
    };
  }
}
