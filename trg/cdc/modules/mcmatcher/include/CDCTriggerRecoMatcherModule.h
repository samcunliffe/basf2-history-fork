#ifndef CDCTRIGGERRECOMATCHERMODULE_H
#define CDCTRIGGERRECOMATCHERMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  /** A module to match CDCTriggerTracks to RecoTracks.
   */
  class CDCTriggerRecoMatcherModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerRecoMatcherModule();

    /** Destructor. */
    virtual ~CDCTriggerRecoMatcherModule() {}

    /** Initialize the module. */
    virtual void initialize();

    /** Called once for each event. */
    virtual void event();

  protected:
    /** Name of the RecoTrack StoreArray to be matched */
    std::string m_RecoTrackCollectionName;
    /** Name of the CDCTriggerTrack Store Array to be matched */
    std::string m_TrgTrackCollectionName;
    /** Name of the StoreArray containing the hits that are used for the matching. */
    std::string m_hitCollectionName;
    /** switch for 2D matching */
    bool m_axialOnly;
    /** minimum purity */
    double m_minPurity;
    /** minimum efficiency */
    double m_minEfficiency;
    /** switch for creating relations for clones and merged tracks */
    bool m_relateClonesAndMerged;
  };
}
#endif
