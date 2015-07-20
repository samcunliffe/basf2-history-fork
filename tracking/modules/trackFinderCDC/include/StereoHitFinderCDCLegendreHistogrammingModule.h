#pragma once
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
  }

  class StereoHitFinderCDCLegendreHistogrammingModule: public TrackFinderCDCBaseModule {

  public:
    StereoHitFinderCDCLegendreHistogrammingModule() : TrackFinderCDCBaseModule()
    {
      addParam("DebugOutput",
               m_param_debugOutput,
               "Flag to turn on debug output.",
               false);

      addParam("QuadTreeLevel",
               m_param_quadTreeLevel,
               "The number of levels for the quad tree search.",
               static_cast<unsigned int>(6));

      addParam("MinimumHitsInQuadtree",
               m_param_minimumHitsInQuadTree,
               "The minimum number of hits in a quad tree bin to be called as result.",
               static_cast<unsigned int>(5));

      addParam("UseOldImplementation",
               m_param_useOldImplementation,
               "Whether to use the old implementation o the quad tree.",
               true);
    }

  private:
    /**
     * Do a QuadTreeHistogramming with all the StereoHits
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /// Parameters
    bool m_param_debugOutput; /**< Flag to turn on debug output */
    unsigned int m_param_quadTreeLevel; /**< The number of levels for the quad tree search */
    unsigned int m_param_minimumHitsInQuadTree; /**< The minimum number of hits in a quad tree bin to be called as result. */
    bool m_param_useOldImplementation; /**< Whether to use the old implementation o the quad tree. */
  };

}
