/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <set>
#include <list>
#include <vector>

namespace Belle2 {

  class CDCLegendreTrackHit;
  class CDCLegendreQuadTree;
  class CDCLegendreTrackFitter;
  class CDCLegendreTrackCreator;


  class CDCLegendreQuadTreeCandidateCreator {

  public:
    CDCLegendreQuadTreeCandidateCreator() {};

    ~CDCLegendreQuadTreeCandidateCreator();

    static CDCLegendreQuadTreeCandidateCreator& Instance();

    /*
     * Set candidate fitter
     */
    void setFitter(CDCLegendreTrackFitter* cdcLegendreTrackFitter) {s_cdcLegendreTrackFitter = cdcLegendreTrackFitter;};

    /*
     * Set candidate fitter
     */
    void setCandidateCreator(CDCLegendreTrackCreator* cdcLegendreTrackCreator) {s_cdcLegendreTrackCreator = cdcLegendreTrackCreator;};

    /*
     * Add node to the list of nodes with candidated
     */
    inline void addNode(CDCLegendreQuadTree* node) {s_nodesWithCandidates.push_back(node);};

    /*
     * Creating candidates using information from nodes
     */
    void createCandidates();

    /*
     * Creating candidate using information from given node
     */
    void createCandidate(CDCLegendreQuadTree* node);

    /*
     * Creating candidates using information from nodes
     */
    std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > >& getCandidates()
    {return s_candidates;};

    /*
     * Clear list of candidates
     */
    void clearCandidates();

    /*
     * Creating candidates using information from nodes
     */
    void clearNodes();

    static bool sort_nodes(const CDCLegendreQuadTree* node_one, const CDCLegendreQuadTree* node_two);


  private:
    static std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > > s_candidates; /**< Holds list of track candidates */
    static std::list<CDCLegendreQuadTree*> s_nodesWithCandidates;
    static CDCLegendreTrackFitter* s_cdcLegendreTrackFitter;
    static CDCLegendreTrackCreator* s_cdcLegendreTrackCreator;

    static CDCLegendreQuadTreeCandidateCreator* s_cdcLegendreQuadTreeCandidateCreator;


  };
}


