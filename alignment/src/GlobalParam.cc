/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/GlobalParam.h>

#include <string>
#include <vector>
#include <map>

#include <framework/logging/Logger.h>

#include <framework/dbobjects/BeamParameters.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <eklm/dbobjects/EKLMAlignment.h>

#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCXtRelations.h>

#include <framework/database/Database.h>

#include <vxd/geometry/GeoCache.h>
#include <alignment/Hierarchy.h>

namespace Belle2 {
  namespace alignment {

    GlobalParamVector::GlobalParamVector(std::vector< std::string > components)
    {
      m_components = components;

      auto cdcInterface = std::shared_ptr<IGlobalParamInterface>(new CDCGlobalParamInterface());
      auto vxdInterface = std::shared_ptr<IGlobalParamInterface>(new VXDGlobalParamInterface());

      addDBObj<BeamParameters>();

      addDBObj<VXDAlignment>(vxdInterface);

      addDBObj<CDCAlignment>(cdcInterface);
      addDBObj<CDCTimeZeros>(cdcInterface);
      addDBObj<CDCTimeWalks>(cdcInterface);
      addDBObj<CDCXtRelations>(cdcInterface);

      addDBObj<BKLMAlignment>();
      addDBObj<EKLMAlignment>();
    }

    void GlobalParamVector::updateGlobalParam(double difference, short unsigned int uniqueID, short unsigned int element,
                                              short unsigned int param)
    {
      // TODO: niw can be replaced by other fcn updateGlobalParam of GlobalParamSetAccess
      auto prev = getGlobalParam(uniqueID, element, param);
      setGlobalParam(prev + difference, uniqueID, element, param);
    }
    void GlobalParamVector::setGlobalParam(double value, short unsigned int uniqueID, short unsigned int element,
                                           short unsigned int param)
    {
      auto dbObj = m_vector.find(uniqueID);
      if (dbObj != m_vector.end()) {
        dbObj->second->setGlobalParam(value, element, param);
      } else {
        B2WARNING("Did not found DB object with unique id " << uniqueID << " in global vector. Cannot set value for element " << element <<
                  " and parameter " << param);
      }
    }
    double GlobalParamVector::getGlobalParam(short unsigned int uniqueID, short unsigned int element, short unsigned int param)
    {
      auto dbObj = m_vector.find(uniqueID);
      if (dbObj != m_vector.end()) {
        return dbObj->second->getGlobalParam(element, param);
      } else {
        B2WARNING("Did not found DB object with unique id " << uniqueID << " in global vector. Cannot get value for element " << element <<
                  " and parameter " << param << ". Returning 0.");
        return 0.;
      }
    }
    std::vector< std::tuple< short unsigned int, short unsigned int, short unsigned int > > GlobalParamVector::listGlobalParams()
    {
      std::vector<std::tuple<unsigned short, unsigned short, unsigned short>> params;
      for (auto& uID_DBObj : m_vector) {
        for (auto element_param : uID_DBObj.second->listGlobalParams()) {
          params.push_back({uID_DBObj.first, element_param.first, element_param.second});
        }
      }
      return params;
    }
    std::vector< TObject* > GlobalParamVector::releaseObjects(bool onlyChanged)
    {
      std::vector<TObject*> result;
      for (auto& uID_DBObj : m_vector) {
        if (onlyChanged and not uID_DBObj.second->hasBeenChanged())
          continue;

        result.push_back({uID_DBObj.second->releaseObject()});
      }
      return result;
    }
    void GlobalParamVector::loadFromDB(EventMetaData event)
    {
      for (auto& uID_DBObj : m_vector) {
        uID_DBObj.second->loadFromDB(event);
      }
    }
    void GlobalParamVector::postReadFromResult(
      std::vector< std::tuple< short unsigned int, short unsigned int, short unsigned int, double > >& result)
    {
      for (auto interface : m_subDetectorInterfacesVector) {
        interface->readFromResult(result, *this);
      }
    }
    void CDCGlobalParamInterface::readFromResult(
      std::vector<std::tuple<short unsigned int, short unsigned int, short unsigned int, double> >& result,
      Belle2::alignment::GlobalParamVector& thisGlobalParamVector)
    {
      std::vector<double> dxLayer(56, 0.);
      std::vector<double> dyLayer(56, 0.);
      std::vector<double> dPhiLayer(56, 0.);

      for (auto entry : result) {
        unsigned short uid = std::get<0>(entry);
        if (uid != CDCAlignment::getGlobalUniqueID())
          continue;

        unsigned short layer = std::get<1>(entry);
        unsigned short param = std::get<2>(entry);
        double correction = std::get<3>(entry);

        if (param == 1)
          dxLayer[layer] = correction;
        if (param == 2)
          dyLayer[layer] = correction;
        if (param == 6)
          dPhiLayer[layer] = correction;

      }

      std::vector<double> R = {
        168.0, 178.0, 188.0, 198.0, 208.0, 218.0, 228.0, 238.0,
        257.0, 275.2, 293.4, 311.6, 329.8, 348.0,
        365.2, 383.4, 401.6, 419.8, 438.0, 455.7,
        476.9, 494.6, 512.8, 531.0, 549.2, 566.9,
        584.1, 601.8, 620.0, 638.2, 656.4, 674.1,
        695.3, 713.0, 731.2, 749.4, 767.6, 785.3,
        802.5, 820.2, 838.4, 856.6, 874.8, 892.5,
        913.7, 931.4, 949.6, 967.8, 986.0, 1003.7,
        1020.9, 1038.6, 1056.8, 1075.0, 1093.2, 1111.4
      };

      std::vector<int> nWires = {
        160, 160, 160, 160, 160, 160, 160, 160,
        160, 160, 160, 160, 160, 160,
        192, 192, 192, 192, 192, 192,
        224, 224, 224, 224, 224, 224,
        256, 256, 256, 256, 256, 256,
        288, 288, 288, 288, 288, 288,
        320, 320, 320, 320, 320, 320,
        352, 352, 352, 352, 352, 352,
        384, 384, 384, 384, 384, 384
      };

      std::vector<int> shiftHold = {
        0, 0, 0, 0, 0, 0, 0, 0,
        18, 18, 18, 18, 18, 18,
        0, 0, 0, 0, 0, 0,
        -18, -18, -18, -18, -18, -18,
        0, 0, 0, 0, 0, 0,
        19, 19, 19, 19, 19, 19,
        0, 0, 0, 0, 0, 0,
        -20, -20, -20, -20, -20, -20,
        0, 0, 0, 0, 0, 0
      };
      std::vector<double> offset = {
        0.5, 0., 0.5, 0., 0.5, 0.,  0.5, 0.,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5
      };

      std::vector<double> rotF(56, 0.);
      std::vector<double> rotB(56, 0.);

      std::vector<std::vector<std::vector<double>>> misBackward(56, std::vector<std::vector<double>>(385, std::vector<double>(3, 0.)));
      std::vector<std::vector<std::vector<double>>> misForward(56, std::vector<std::vector<double>>(385, std::vector<double>(3, 0.)));

      double PhiF, PhiB;
      for (int l = 0; l < 56; ++l) {
        double dPhi = 2 * 3.14159265358979323846 / nWires[l];
        for (int w = 0; w < nWires[l]; ++w) {

          PhiF = dPhi * (w + offset[l] + 0.5 * shiftHold[l]);
          misForward[l][w][0] = dxLayer[l] + R[l] * 0.1 * (cos(PhiF + rotF[l] + dPhiLayer[l]) - cos(PhiF));
          misForward[l][w][1] = dyLayer[l] + R[l] * 0.1 * (sin(PhiF + rotF[l] + dPhiLayer[l]) - sin(PhiF));
          misForward[l][w][2] = 0. ;

          PhiB = dPhi * (w + offset[l]);
          misBackward[l][w][0] = dxLayer[l] + R[l] * 0.1 * (cos(PhiB + rotB[l] + dPhiLayer[l]) - cos(PhiB));
          misBackward[l][w][1] = dyLayer[l] + R[l] * 0.1 * (sin(PhiB + rotB[l] + dPhiLayer[l]) - sin(PhiB));
          misBackward[l][w][2] = 0.;
        }
      }

      for (int l = 0; l < 56; ++l) {
        for (int w = 0; w < nWires[l]; ++w) {

          auto wire = WireID(l, w);
          // Test direct access
          if (thisGlobalParamVector.getDBObj<CDCAlignment>())
            thisGlobalParamVector.getDBObj<CDCAlignment>()->getWireTension(wire.getICLayer(), wire.getILayer());

          thisGlobalParamVector.updateGlobalParam(misForward[l][w][0], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireFwdX);
          thisGlobalParamVector.updateGlobalParam(misForward[l][w][1], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireFwdY);
          thisGlobalParamVector.updateGlobalParam(misForward[l][w][2], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireFwdZ);

          thisGlobalParamVector.updateGlobalParam(misBackward[l][w][0], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireBwdX);
          thisGlobalParamVector.updateGlobalParam(misBackward[l][w][0], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireBwdY);
          thisGlobalParamVector.updateGlobalParam(misBackward[l][w][0], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireBwdZ);
        }
      }
    }

    void VXDGlobalParamInterface::setupAlignmentHierarchy(RigidBodyHierarchy& hierarchy)
    {
      auto& geo = VXD::GeoCache::getInstance();
      // Set-up hierarchy
      DBObjPtr<VXDAlignment> vxdAlignments;
      /**
      So the hierarchy is as follows:
                  Belle 2
                / |     | \
            Ying  Yang Pat  Mat ... other sub-detectors
            / |   / |  |  \  | \
          ......  ladders ......
          / / |   / |  |  \  | \ \
        ......... sensors ........
      */

      for (auto& halfShellPlacement : geo.getHalfShellPlacements()) {
        TGeoHMatrix trafoHalfShell = halfShellPlacement.second;
        trafoHalfShell *= geo.getTGeoFromRigidBodyParams(
                            vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dU),
                            vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dV),
                            vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dW),
                            vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dAlpha),
                            vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dBeta),
                            vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dGamma)
                          );
        hierarchy.insertTGeoTransform<VXDAlignment, alignment::EmptyGlobaParamSet>(halfShellPlacement.first, 0, trafoHalfShell);

        for (auto& ladderPlacement : geo.getLadderPlacements(halfShellPlacement.first)) {
          // Updated trafo
          TGeoHMatrix trafoLadder = ladderPlacement.second;
          trafoLadder *= geo.getTGeoFromRigidBodyParams(
                           vxdAlignments->get(ladderPlacement.first, VXDAlignment::dU),
                           vxdAlignments->get(ladderPlacement.first, VXDAlignment::dV),
                           vxdAlignments->get(ladderPlacement.first, VXDAlignment::dW),
                           vxdAlignments->get(ladderPlacement.first, VXDAlignment::dAlpha),
                           vxdAlignments->get(ladderPlacement.first, VXDAlignment::dBeta),
                           vxdAlignments->get(ladderPlacement.first, VXDAlignment::dGamma)
                         );
          hierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(ladderPlacement.first, halfShellPlacement.first, trafoLadder);

          for (auto& sensorPlacement : geo.getSensorPlacements(ladderPlacement.first)) {
            // Updated trafo
            TGeoHMatrix trafoSensor = sensorPlacement.second;
            trafoSensor *= geo.getTGeoFromRigidBodyParams(
                             vxdAlignments->get(sensorPlacement.first, VXDAlignment::dU),
                             vxdAlignments->get(sensorPlacement.first, VXDAlignment::dV),
                             vxdAlignments->get(sensorPlacement.first, VXDAlignment::dW),
                             vxdAlignments->get(sensorPlacement.first, VXDAlignment::dAlpha),
                             vxdAlignments->get(sensorPlacement.first, VXDAlignment::dBeta),
                             vxdAlignments->get(sensorPlacement.first, VXDAlignment::dGamma)
                           );
            hierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(sensorPlacement.first, ladderPlacement.first, trafoSensor);


          }
        }
      }
    }
  }
}