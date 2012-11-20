/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#include <eklm/geoeklm/G4PVPlacementGT.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  class G4PVPlacementGT;

  G4PVPlacementGT::G4PVPlacementGT(const G4Transform3D& globalTransform,
                                   const G4Transform3D& Transform3D,
                                   G4LogicalVolume* pCurrentLogical,
                                   const G4String& pName,
                                   G4LogicalVolume* pMotherLogical,
                                   int id,
                                   enum EKLMDetectorMode mode) :
    G4PVPlacement(Transform3D, pCurrentLogical, pName, pMotherLogical, false,
                  1, false),
    m_name(pName)
  {
    m_transform = globalTransform;
    m_id = id;
    m_mode = mode;
    m_mother = NULL;
    m_type = EKLM_NOT_SENSITIVE;
  }

  G4PVPlacementGT::G4PVPlacementGT(G4PVPlacementGT* motherPVPlacementGT,
                                   const G4Transform3D& Transform3D,
                                   G4LogicalVolume* pCurrentLogical,
                                   const G4String& pName,
                                   int id,
                                   enum EKLMDetectorMode mode) :
    G4PVPlacement(Transform3D, pCurrentLogical, pName,
                  motherPVPlacementGT->GetLogicalVolume(), false, 1, false),
    m_name(pName)
  {
    m_transform = motherPVPlacementGT->getTransform() * Transform3D;
    m_id = id;
    m_mode = mode;
    m_mother = motherPVPlacementGT;
    m_type = EKLM_NOT_SENSITIVE;
  }

  G4Transform3D G4PVPlacementGT::getTransform()
  {
    return m_transform;
  }

  int G4PVPlacementGT::getID() const
  {
    return m_id;
  }

  enum EKLMDetectorMode G4PVPlacementGT::getMode() const
  {
    return m_mode;
  }

  enum EKLMSensitiveType G4PVPlacementGT::getVolumeType() const
  {
    return m_type;
  }

  void G4PVPlacementGT::setVolumeType(enum EKLMSensitiveType t)
  {
    if (m_mode == EKLM_DETECTOR_NORMAL &&
        (t == EKLM_SENSITIVE_SIPM || t == EKLM_SENSITIVE_BOARD))
      B2WARNING("Attempt to create volume needed only during "
                "background studies in normal opeation mode.");
    m_type = t;
  }

  const G4PVPlacementGT* G4PVPlacementGT::getMother() const
  {
    return m_mother;
  }

  const std::string G4PVPlacementGT::getName() const
  {
    return m_name;
  }

}

