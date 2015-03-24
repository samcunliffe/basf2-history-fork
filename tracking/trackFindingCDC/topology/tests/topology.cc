/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCWInwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCWInwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCWOutwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCWOutwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCW();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCW();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}


TEST_F(TrackFindingCDCTestWithTopology, topology_WireSkew)
{
  // Test if the all wires in the same superlayer have similar skew parameters.

  FloatType skewByICLayer[CDCWireTopology::N_LAYERS];
  FloatType stereoAngleByICLayer[CDCWireTopology::N_LAYERS];
  FloatType refCylindricalRByICLayer[CDCWireTopology::N_LAYERS];

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  for (const CDCWireLayer& wireLayer : theWireTopology.getWireLayers()) {
    const ILayerType iCLayer = wireLayer.getICLayer();

    const CDCWire& firstWire = wireLayer.first();
    skewByICLayer[iCLayer] = firstWire.getSkew();
    stereoAngleByICLayer[iCLayer] = firstWire.getStereoAngle();
    refCylindricalRByICLayer[iCLayer] = firstWire.getRefCylindricalR();

    for (const CDCWire& wire : wireLayer) {
      EXPECT_NEAR(skewByICLayer[iCLayer], wire.getSkew(), 10e-6);
      EXPECT_NEAR(stereoAngleByICLayer[iCLayer], wire.getStereoAngle(), 10e-6);
      EXPECT_NEAR(refCylindricalRByICLayer[iCLayer], wire.getRefCylindricalR(), 10e-6);
    }

    B2INFO("ICLayer : " << iCLayer <<
           " Skew : " << skewByICLayer[iCLayer] <<
           " Stereo angle : " << stereoAngleByICLayer[iCLayer] <<
           " Ref. cylindricalR : " << refCylindricalRByICLayer[iCLayer]
          );

  }

}



TEST_F(TrackFindingCDCTestWithTopology, topology_RefCylindricalRVersusZInSuperLayers)
{
  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  for (const CDCWireSuperLayer& wireSuperLayer : theWireTopology.getWireSuperLayers()) {
    if (wireSuperLayer.getStereoType() == AXIAL) {
      EXPECT_EQ(0.0, wireSuperLayer.getRefTanLambda());
    }

    B2INFO("ISuperLayer : " << wireSuperLayer.getISuperLayer() <<
           " Inner ref. z : " << wireSuperLayer.getInnerRefZ() <<
           " Outer ref. z : " << wireSuperLayer.getOuterRefZ() <<
           " CylindricalR Z slope : " << wireSuperLayer.getRefTanLambda()
          );

  }



}
