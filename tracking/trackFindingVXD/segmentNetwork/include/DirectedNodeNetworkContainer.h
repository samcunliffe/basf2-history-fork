/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationsObject.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>

#include <tracking/trackFindingVXD/segmentNetwork/StaticSectorDummy.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>
#include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>
#include <tracking/trackFindingVXD/segmentNetwork/VoidMetaInfo.h>
#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>


// C++-std:
#include <vector>

namespace Belle2 {


  /** The Container stores the output produced by the SegmentNetworkProducerModule.
   *
   * Is coupled to an assigned static sectorMap, which influences the structure of the DirectedNodeNetworks stored in this container.
   * is intended to be filled event-wise and not to be stored in root-files.
   *
   * TODO: create constructor for vIPs in SpacePoint. What about activeSectors for vIP? -> solution dependent of treatment in static sectorMap.
   */
  class DirectedNodeNetworkContainer : public RelationsObject {
  protected:
    /** ************************* DATA MEMBERS ************************* */


    /** Stores the full network of activeSectors, which contain hits in that event and have compatible Sectors with hits too*/
    DirectedNodeNetwork<ActiveSector<StaticSectorDummy, TrackNode>, Belle2::VoidMetaInfo > m_ActiveSectorNetwork;

    /** stores the actual ActiveSectors, since the ActiveSectorNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<ActiveSector<StaticSectorDummy, TrackNode>* > m_activeSectors;

    /** Stores the full network of TrackNode< SpaacePoint>, which were accepted by activated two-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<TrackNode, Belle2::VoidMetaInfo> m_HitNetwork;

    /** Stores the full network of Segments, which were accepted by activated three-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<Segment<TrackNode>, Belle2::CACell > m_SegmentNetwork;

    /** stores the actual Segments, since the SegmentNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<Segment<TrackNode>* > m_segments;

    /** stores the actual trackNodes, since the SegmentNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<TrackNode* > m_trackNodes;

    /** stores a SpacePoint representing the virtual interaction point if set, NULL if not. */
    Belle2::TrackNode* m_VirtualInteractionPoint;

    /** stores the SpacePoint needed for the virtual IP */
    SpacePoint* m_VIPSpacePoint;

  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** standard constructor */
    DirectedNodeNetworkContainer() :
      m_ActiveSectorNetwork(DirectedNodeNetwork<ActiveSector<Belle2::StaticSectorDummy, Belle2::TrackNode>, Belle2::VoidMetaInfo >()),
      m_HitNetwork(DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>()),
      m_SegmentNetwork(DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >()),
      m_VirtualInteractionPoint(NULL),
      m_VIPSpacePoint(NULL) {}


    /** destructor */
    ~DirectedNodeNetworkContainer()
    {
      if (m_VirtualInteractionPoint != NULL) { delete m_VirtualInteractionPoint; }
      if (m_VIPSpacePoint != NULL) { delete m_VIPSpacePoint; }
      for (auto* aSector : m_activeSectors) { delete aSector; }
      for (auto* aSegment : m_segments) { delete aSegment; }
      for (auto* aNode : m_trackNodes) { delete aNode; }
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters

    /** returns reference to the ActiveSectorNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::ActiveSector<Belle2::StaticSectorDummy, Belle2::TrackNode>, Belle2::VoidMetaInfo >&
    accessActiveSectorNetwork() { return m_ActiveSectorNetwork; }


    /** returns reference to the actual ActiveSectors stored in this container, intended for read and write access */
    std::vector<Belle2::ActiveSector<Belle2::StaticSectorDummy, Belle2::TrackNode>* >& accessActiveSectors() { return m_activeSectors; }


    /** returns reference to the HitNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>& accessHitNetwork() { return m_HitNetwork; }


    /** returns reference to the SegmentNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >& accessSegmentNetwork() { return m_SegmentNetwork; }


    /** returns reference to the actual segments stored in this container, intended for read and write access */
    std::vector<Belle2::Segment<Belle2::TrackNode>* >& accessSegments() { return m_segments; }


    /** returns reference to the actual trackNodes stored in this container, intended for read and write access */
    std::vector<Belle2::TrackNode* >& accessTrackNodes() { return m_trackNodes; }


    /** passes parameters for creating a virtual interaction point TODO pass coordinates-parameter (and all the other stuff needed for a proper vIP. like ActiveSector-Pointer and Stuff! */
    void setVirtualInteractionPoint()
    {
      m_VIPSpacePoint = new SpacePoint();
      m_VirtualInteractionPoint = new Belle2::TrackNode();
      m_VirtualInteractionPoint->spacePoint = m_VIPSpacePoint;
    }


    /** returns reference to the Virtual interactionPoint stored here */
    Belle2::TrackNode* getVirtualInteractionPoint() { return m_VirtualInteractionPoint; }


    // last member changed: added metaInfo for DirectedNodeNetwork
    ClassDef(DirectedNodeNetworkContainer, 8)
  };

} //Belle2 namespace