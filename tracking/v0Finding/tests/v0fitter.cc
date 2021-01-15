#include <tracking/v0Finding/fitter/V0Fitter.h>

#include <framework/gearbox/Const.h>

#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/ConstField.h>
#include <genfit/TGeoMaterialInterface.h>

#include <utility>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class V0FitterTest : public ::testing::Test {
  };

  /// Test getter for track hypotheses.
  TEST_F(V0FitterTest, GetTrackHypotheses)
  {
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
    genfit::FieldManager::getInstance()->init(new genfit::ConstField(0., 0., 1.5));
    V0Fitter v0Fitter;
    const auto kShortTracks = v0Fitter.getTrackHypotheses(Const::Kshort);
    const auto photonTracks = v0Fitter.getTrackHypotheses(Const::photon);
    const auto lambdaTracks = v0Fitter.getTrackHypotheses(Const::Lambda);
    const auto antiLambdaTracks = v0Fitter.getTrackHypotheses(Const::antiLambda);

    EXPECT_EQ(Const::pion, kShortTracks.first);
    EXPECT_EQ(Const::pion, kShortTracks.second);

    EXPECT_EQ(Const::electron, photonTracks.first);
    EXPECT_EQ(Const::electron, photonTracks.second);

    EXPECT_EQ(Const::proton, lambdaTracks.first);
    EXPECT_EQ(Const::pion, lambdaTracks.second);

    EXPECT_EQ(Const::pion, antiLambdaTracks.first);
    EXPECT_EQ(Const::proton, antiLambdaTracks.second);
  }

  /// Test initialization of cuts.
  TEST_F(V0FitterTest, InitializeCuts)
  {
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
    genfit::FieldManager::getInstance()->init(new genfit::ConstField(0., 0., 1.5));
    V0Fitter v0Fitter;
    v0Fitter.initializeCuts(1.0, 52.);
    EXPECT_EQ(1.0, v0Fitter.m_beamPipeRadius);
    EXPECT_EQ(52., v0Fitter.m_vertexChi2CutOutside);
  }

}  // namespace
