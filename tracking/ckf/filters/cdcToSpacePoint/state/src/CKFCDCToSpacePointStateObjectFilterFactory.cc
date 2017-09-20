/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectFilterFactory.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateObjectFilter.h>
#include <tracking/ckf/filters/base/CKFStateTruthVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectBasicVarSet.h>

#include <tracking/ckf/filters/cdcToSVDSpacePoint/SimpleCKFCDCToSVDStateFilter.h>
#include <tracking/ckf/filters/pxdSpacePoint/SimpleCKFPXDStateFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/SloppyMCFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCKFCDCToSpacePointStateObjectFilter = MCFilter<CKFStateTruthVarSet<RecoTrack, SpacePoint>>;

  /// MC-ordering filter for VXD - CDC relations.
  using MCOrderingCKFCDCToSpacePointStateObjectFilter = NamedChoosableVarSetFilter<CKFStateTruthVarSet<RecoTrack, SpacePoint>>;

  /// Sloppy MC filter for VXD - CDC relations.
  using SloppyMCCKFCDCToSpacePointStateObjectFilter = SloppyMCFilter<CKFStateTruthVarSet<RecoTrack, SpacePoint>>;

  /// Prescaled recording filter for VXD - CDC relations.
  class SloppyRecordingCKFCDCToSpacePointStateObjectFilter : public
    RecordingFilter<VariadicUnionVarSet<CKFStateTruthVarSet<RecoTrack, SpacePoint>,
    CKFCDCToSpacePointStateObjectBasicVarSet, CKFCDCToSpacePointStateObjectVarSet>> {
    using Super = RecordingFilter<VariadicUnionVarSet<CKFStateTruthVarSet<RecoTrack, SpacePoint>,
          CKFCDCToSpacePointStateObjectBasicVarSet, CKFCDCToSpacePointStateObjectVarSet>>;
  public:
    SloppyRecordingCKFCDCToSpacePointStateObjectFilter(const std::string& defaultRootFileName) : Super(defaultRootFileName)
    {
      setSkimFilter(std::make_unique<SloppyMCCKFCDCToSpacePointStateObjectFilter>());
    }
  };

  /// Recording filter for VXD - CDC relations.
  using RecordingCKFCDCToSpacePointStateObjectFilter = RecordingFilter<VariadicUnionVarSet<CKFStateTruthVarSet<RecoTrack, SpacePoint>,
        CKFCDCToSpacePointStateObjectBasicVarSet, CKFCDCToSpacePointStateObjectVarSet>>;

  /// MVA filter for VXD - CDC relations.
  class MVACKFCDCToSpacePointStateObjectFilter : public
    MVAFilter<CKFCDCToSpacePointStateObjectBasicVarSet> {
    using Super = MVAFilter<CKFCDCToSpacePointStateObjectBasicVarSet>;
  public:
    using Super::Super;

    /// Function to object for its signalness
    Weight operator()(const CKFCDCToSpacePointStateObjectBasicVarSet::Object& obj) override
    {
      if (obj.getHit()) {
        return -Super::operator()(obj);
      } else {
        return 0;
      }
    }
  };

  /// All filter for VXD - CDC relations.
  using AllCKFCDCToSpacePointStateObjectFilter = AllFilter<BaseCKFCDCToSpacePointStateObjectFilter>;
}

CKFCDCToSpacePointStateObjectFilterFactory::CKFCDCToSpacePointStateObjectFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CKFCDCToSpacePointStateObjectFilterFactory::getIdentifier() const
{
  return "CKFCDCToSpacePointStateObject";
}

std::string CKFCDCToSpacePointStateObjectFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
CKFCDCToSpacePointStateObjectFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"truth", "monte carlo truth"},
    {"ordering_truth", "monte carlo truth ordering"},
    {"sloppy_truth", "sloppy monte carlo truth"},
    {"pxd_simple", "simple filter to be used in pxd"},
    {"svd_simple", "simple filter to be used in svd"},
    {"recording", "record variables to a TTree"},
    {"mva", "MVA filter"},
    {"sloppy_recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BaseCKFCDCToSpacePointStateObjectFilter>
CKFCDCToSpacePointStateObjectFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseCKFCDCToSpacePointStateObjectFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllCKFCDCToSpacePointStateObjectFilter>();
  } else if (filterName == "svd_simple") {
    return std::make_unique<SimpleCKFCDCToSVDStateFilter>();
  } else if (filterName == "pxd_simple") {
    return std::make_unique<SimpleCKFPXDStateFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCCKFCDCToSpacePointStateObjectFilter>();
  } else if (filterName == "ordering_truth") {
    return std::make_unique<MCOrderingCKFCDCToSpacePointStateObjectFilter>("truth_inverted");
  } else if (filterName == "sloppy_truth") {
    return std::make_unique<SloppyMCCKFCDCToSpacePointStateObjectFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingCKFCDCToSpacePointStateObjectFilter>("CKFCDCToSpacePointStateObjectFilter.root");
  } else if (filterName == "mva") {
    return std::make_unique<MVACKFCDCToSpacePointStateObjectFilter>("tracking/data/ckf_CDCSVDStateFilter.xml");
  } else if (filterName == "sloppy_recording") {
    return std::make_unique<SloppyRecordingCKFCDCToSpacePointStateObjectFilter>("CKFCDCToSpacePointStateObjectFilter.root");
  } else {
    return Super::create(filterName);
  }
}
