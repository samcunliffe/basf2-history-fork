/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch, Nils Braun, Oliver Frost           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/CurlerCloneRejecter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::ChooseableFilter<CurlerCloneFilterFactory>;

CurlerCloneRejecter::CurlerCloneRejecter(const std::string& defaultFilterName)
  : m_curlerCloneFilter(defaultFilterName)
{
  this->addProcessingSignalListener(&m_mcTrackCurlerCloneLookUpFiller);
  this->addProcessingSignalListener(&m_curlerCloneFilter);
}

std::string CurlerCloneRejecter::getDescription()
{
  return "Classifies tracks which are expected to be clones from curlers.";
}

void CurlerCloneRejecter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_curlerCloneFilter.exposeParameters(moduleParamList, prefix);
  m_mcTrackCurlerCloneLookUpFiller.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "markAsBackground"),
                                m_param_markAsBackground,
                                "Marking expected clones from curlers as background.",
                                m_param_markAsBackground);

  m_curlerCloneFilter.exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "deleteCurlerClones"),
                                m_param_deleteCurlerClones,
                                "Delete the tracks instead of marking them as background.",
                                m_param_deleteCurlerClones);
}

void CurlerCloneRejecter::apply(std::vector<CDCTrack>& tracks)
{
  m_mcTrackCurlerCloneLookUpFiller.apply(tracks);

  auto reject = [this](CDCTrack & track) {
    double filterWeight = m_curlerCloneFilter(track);
    track->setCellWeight(filterWeight);
    if (std::isnan(filterWeight) && m_param_markAsBackground) {
      track->setBackgroundFlag();
      track->setTakenFlag();
      return true;
    } else {
      return false;
    }
  };

  if (m_param_deleteCurlerClones) {
    erase_remove_if(tracks, reject);
  } else {
    std::for_each(tracks.begin(), tracks.end(), reject);
  }
}
