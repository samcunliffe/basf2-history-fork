#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  /** test kinematic Variable. */
  TEST(KinematicVariableTest, Variable)
  {
    {
      Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
      EXPECT_FLOAT_EQ(0.9, particleP(&p));
      EXPECT_FLOAT_EQ(1.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.1, particlePx(&p));
      EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
      EXPECT_FLOAT_EQ(0.8, particlePz(&p));
      EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
      EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

      EXPECT_FLOAT_EQ(0.0, particleP_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particleE_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt_CMS(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi_CMS(&p));
    }

    {
      Particle p({ 0.0 , 0.0, 0.0, 0.0 }, 11);
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));

      EXPECT_FLOAT_EQ(0.0, particleP_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particleE_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt_CMS(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi_CMS(&p));
    }
  }

}