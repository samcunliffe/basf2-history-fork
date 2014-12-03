#include <framework/dataobjects/Helix.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TRandom3.h>
#include <TMath.h>

#include <gtest/gtest.h>

using namespace std;

// Additional tests
/// Expectation macro for combound structures like vectors to be close to each other
#define EXPECT_ALL_NEAR(expected, actual, delta) EXPECT_PRED3(allNearTemplate<decltype(expected)>, expected, actual, delta)

/// Assertation macro for combound structures like vectors to be close to each other
#define ASSERT_ALL_NEAR(expected, actual, delta) ASSERT_PRED3(allNearTemplate<decltype(expected)>, expected, actual, delta)

/// Expectation macro for angle values that should not care for a multiple of 2 * PI difference between the values
#define EXPECT_ANGLE_NEAR(expected, actual, delta) EXPECT_PRED3(angleNear, expected, actual, delta)

/// Assertation macro for angle values that should not care for a multiple of 2 * PI difference between the values
#define ASSERT_ANGLE_NEAR(expected, actual, delta) EXPECT_PRED3(angleNear, expected, actual, delta)

/// Sting output operator for a TVector3 for gtest print support.
std::ostream& operator<<(std::ostream& output, const TVector3& tVector3)
{
  return output
         << "TVector3("
         << tVector3.X() << ", "
         << tVector3.Y() << ", "
         << tVector3.Z() << ")";
}

/** Adds message to all EXCEPTS and ASSERT in the current and any called or nested scopes
 *
 *  The macro sets up for the following EXCEPTS and ASSERTS, hence it must be placed before the tests.
 *
 *  The message can be composed in a B2INFO style manner with addtional << between
 *  individual strings and values to be concatenated.
 *
 *  @example TEST_CONTEXT("for my value set to "  << myValue);
 */
#define TEST_CONTEXT(message) SCOPED_TRACE([&](){std::ostringstream messageStream; messageStream << message; return messageStream.str();}());

namespace {
  /** Predicate checking that all three components of TVector are close by a maximal error of absError. */
  bool allNear(const TVector3& expected, const TVector3& actual, const float& absError)
  {

    bool xNear = fabs(expected.X() - actual.X()) < absError;
    bool yNear = fabs(expected.Y() - actual.Y()) < absError;
    bool zNear = fabs(expected.Z() - actual.Z()) < absError;
    return xNear and yNear and zNear;
  }

  /** Predicate checking that all five components of the Helix are close by a maximal error of absError. */
  bool allNear(const Belle2::Helix& expected, const Belle2::Helix& actual, const float& absError)
  {
    bool d0Near = fabs(expected.getD0() - actual.getD0()) < absError;
    bool phi0Near = fabs(remainder(expected.getPhi0() - actual.getPhi0(), 2 * M_PI)) < absError; // Identical modulo 2 * PI
    bool omegaNear = fabs(expected.getOmega() - actual.getOmega()) < absError;
    bool z0Near = fabs(expected.getZ0() - actual.getZ0()) < absError;
    bool tanLambdaNear = fabs(expected.getTanLambda() - actual.getTanLambda()) < absError;

    return d0Near and phi0Near and omegaNear and z0Near and tanLambdaNear;
  }

  /** Templated version of predicate checking if two combound object containing some floating point are near each other by maximum deviation.
   *  Concrete implementations can be given as simple overloads of the allNear function.
   */
  template<class T>
  bool allNearTemplate(const T& expected, const T& actual, const float& absError)
  {
    return allNear(expected, actual, absError);
  }

  /** Predicate checking that two angular values are close to each other modulus a 2 * PI difference. */
  bool angleNear(const float& expected, const float& actual, const float& absError)
  {
    return fabs(remainder(expected - actual, 2 * M_PI)) < absError;
  }

  Belle2::Helix helixFromCenter(const TVector3& center, const float& radius, const float& tanLambda)
  {
    double omega = 1 / radius;
    double phi0 = center.Phi() + copysign(M_PI / 2.0, radius);
    double d0 = copysign(center.Perp(), radius) - radius;
    double z0 = center.Z();

    return Belle2::Helix(d0, phi0, omega, z0, tanLambda);
  }


  /** Returns n evenly spaced samples, calculated over the closed interval [start, stop ].*/
  vector<float> linspace(const float& start, const float& end, const int n)
  {
    std::vector<float> result(n);
    result[0] = start;
    result[n - 1] = end;

    for (int i = 1; i < n - 1; ++i) {
      float start_weight = (float)(n - 1 - i) / (n - 1);
      float end_weight = 1 - start_weight;
      result[i] = start * start_weight + end * end_weight;
    }

    return result;
  }

}


namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class HelixTest : public ::testing::Test {

  protected:
    // Common level precision for all tests.
    double absError = 1e-6;
    double nominalBz = 1.5;

    std::vector<float> omegas { -1, 0, 1};
    //std::vector<float> omegas {1};
    std::vector<float> phi0s = linspace(-M_PI, M_PI, 11);
    std::vector<float> d0s { -0.5, -0.2, 0, 0.2, 0.5};
    //std::vector<float> d0s {0.5};
    std::vector<float> chis = linspace(-5 * M_PI / 6, 5 * M_PI / 6, 11);

  };

  /** Test simple Setters and Getters. */
  TEST_F(HelixTest, Getters)
  {
    TRandom3 generator;
    unsigned int nCases = 1;
    double bField = nominalBz;

    for (unsigned int i = 0; i < nCases; ++i) {

      short int charge = generator.Uniform(-1, 1) > 0 ? 1 : -1;

      // Generate a random put orthogonal pair of vectors in the r-phi plane
      TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      d.Set(d.X(), -d.X() * pt.Px() / pt.Py());

      // Add a random z component
      TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
      TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

      // Set up class for testing
      Helix helix(position, momentum, charge, bField);

      // Test all vector elements
      EXPECT_ALL_NEAR(position, helix.getPosition(), absError);
      EXPECT_ALL_NEAR(momentum, helix.getMomentum(bField), absError);

      // Test getter for transverse momentum
      EXPECT_NEAR(momentum.Perp(), helix.getTransverseMomentum(bField), absError);

      // Test getter of kappa
      EXPECT_NEAR(charge / momentum.Perp(), helix.getKappa(bField), absError);

      // Test other variables
      EXPECT_EQ(charge, helix.getChargeSign());

    }
  } // Testcases for getters


  TEST_F(HelixTest, SignOfD0)
  {
    // This tests the assumption that the sign of d0 is given by the sign of position x momentum as a two dimensional cross product.

    const TVector3 position(1, 0, 0);
    const TVector3 momentum(0, 1, 0);
    const TVector3 oppositeMomentum(0, -1, 0);
    const float charge = 1;
    const float bField = nominalBz;

    Helix helix(position, momentum, charge, bField);
    EXPECT_NEAR(1, helix.getD0(), absError);

    // D0 does not change with the charge
    Helix helix2(position, momentum, -charge, bField);
    EXPECT_NEAR(1, helix2.getD0(), absError);

    // But with reversal of momentum
    Helix oppositeMomentumHelix(position, oppositeMomentum, charge, bField);
    EXPECT_NEAR(-1, oppositeMomentumHelix.getD0(), absError);

    Helix oppositeMomentumHelix2(position, oppositeMomentum, -charge, bField);
    EXPECT_NEAR(-1, oppositeMomentumHelix2.getD0(), absError);

  }


  TEST_F(HelixTest, Explicit)
  {
    /** Setup a helix
     *  for counterclockwise travel
     *  starting at -1.0, 0.0, 0.0
     *  heading in the negative y direction initially
     */

    TVector3 center(0.0, -2.0, 0.0);
    float radius = -1;
    // Keep it flat
    float tanLambda = 0;

    Helix helix = helixFromCenter(center, radius, tanLambda);
    EXPECT_NEAR(-1, helix.getD0(), absError);
    EXPECT_ANGLE_NEAR(-M_PI, helix.getPhi0(), absError);
    EXPECT_NEAR(-1, helix.getOmega(), absError);

    // Positions on the helix
    {
      // Start point
      float arcLength = 0;
      TVector3 position = helix.getPositionAtArcLength(arcLength);
      TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);

      EXPECT_ALL_NEAR(TVector3(0.0, -1.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(-M_PI, tangential.Phi(), absError);
    }

    {
      float arcLength = M_PI / 2;
      TVector3 position = helix.getPositionAtArcLength(arcLength);
      TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);
      EXPECT_ALL_NEAR(TVector3(-1.0, -2.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(-M_PI / 2, tangential.Phi(), absError);
    }

    {
      float arcLength = M_PI;
      TVector3 position = helix.getPositionAtArcLength(arcLength);
      TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);
      EXPECT_ALL_NEAR(TVector3(0.0, -3.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(0, tangential.Phi(), absError);
    }

    {
      float arcLength = 3 * M_PI / 2 ;
      TVector3 position = helix.getPositionAtArcLength(arcLength);
      TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);
      EXPECT_ALL_NEAR(TVector3(1.0, -2.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(M_PI / 2, tangential.Phi(), absError);
    }
  }

  TEST_F(HelixTest, Tangential)
  {
    float z0 = 0;
    float tanLambda = 2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          Helix helix(d0, phi0, omega, z0, tanLambda);
          TEST_CONTEXT("Failed for " << helix);

          TVector3 tangentialAtPerigee = helix.getUnitTangentialAtArcLength(0.0);

          EXPECT_ANGLE_NEAR(phi0, tangentialAtPerigee.Phi(), absError);
          EXPECT_FLOAT_EQ(1.0, tangentialAtPerigee.Mag());
          EXPECT_FLOAT_EQ(tanLambda, 1 / tan(tangentialAtPerigee.Theta()));

          for (const float chi : chis) {

            if (omega == 0) {
              // Use chi as the arc length in the straight line case
              float arcLength = chi;

              // Tangential vector shall not change along the line
              TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);
              EXPECT_ALL_NEAR(tangentialAtPerigee, tangential, absError);

            } else {
              float arcLength = -chi / omega;
              TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);

              float actualChi = tangential.DeltaPhi(tangentialAtPerigee);
              EXPECT_ANGLE_NEAR(chi, actualChi, absError);
              EXPECT_FLOAT_EQ(tangentialAtPerigee.Theta(), tangential.Theta());
              EXPECT_FLOAT_EQ(1, tangential.Mag());

            }

          }
        }
      }
    }
  }


  TEST_F(HelixTest, MomentumExtrapolation)
  {
    float z0 = 0;
    float tanLambda = -2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {
          if (omega != 0) {

            Helix helix(d0, phi0, omega, z0, tanLambda);
            TVector3 momentumAtPerigee = helix.getMomentum();
            for (const float chi : chis) {

              float arcLength = -chi / omega;
              TVector3 extrapolatedMomentum = helix.getMomentumAtArcLength(arcLength, nominalBz);

              float actualChi = extrapolatedMomentum.DeltaPhi(momentumAtPerigee);
              EXPECT_ANGLE_NEAR(chi, actualChi, absError);
              EXPECT_FLOAT_EQ(momentumAtPerigee.Theta(), extrapolatedMomentum.Theta());
              EXPECT_FLOAT_EQ(momentumAtPerigee.Mag(), extrapolatedMomentum.Mag());
            }
          }
        }
      }
    }
  }



  TEST_F(HelixTest, Extrapolation)
  {

    // z coordinates do not matter for this test.
    float z0 = 0;
    float tanLambda = 2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          Helix helix(d0, phi0, omega, z0, tanLambda);
          TVector3 perigee = helix.getPosition();

          TVector3 tangentialAtPerigee = helix.getUnitTangentialAtArcLength(0.0);
          TEST_CONTEXT("Failed for " << helix);

          //continue;

          for (const float chi : chis) {
            TEST_CONTEXT("Failed for chi = " << chi);

            // In the cases where omega is 0 (straight line case) chi become undefined.
            // Use chi sample as transverse travel distance instead.
            float expectedArcLength = omega != 0 ? -chi / omega : chi;
            TVector3 pointOnHelix = helix.getPositionAtArcLength(expectedArcLength);

            float polarR = pointOnHelix.Perp();
            float arcLength = helix.getArcLengthAtPolarR(polarR);

            // Only the absolute value is returned.
            EXPECT_NEAR(fabs(expectedArcLength), arcLength, absError);

            // Also check it the extrapolation lies in the forward direction.
            TVector3 secantVector = pointOnHelix - perigee;

            if (expectedArcLength == 0) {
              EXPECT_NEAR(0, secantVector.Mag(), absError);
            } else {
              TVector2 secantVectorXY = secantVector.XYvector();

              TVector2 tangentialXY = tangentialAtPerigee.XYvector();
              float coalignment = secantVectorXY * tangentialXY ;

              bool extrapolationIsForward = coalignment > 0;
              bool expectedIsForward = expectedArcLength > 0;
              EXPECT_EQ(expectedIsForward, extrapolationIsForward);
            }
          }
        }
      }
    }
  } // end TEST_F


  TEST_F(HelixTest, PerigeeExtrapolateRoundTrip)
  {
    float z0 = 0;
    float tanLambda = -2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          // Extrapolations involving the momentum only makes sense with finit momenta
          if (omega != 0) {
            Helix expectedHelix(d0, phi0, omega, z0, tanLambda);

            for (const float chi : chis) {
              float arcLength = -chi / omega;
              TVector3 position = expectedHelix.getPositionAtArcLength(arcLength);
              TVector3 momentum = expectedHelix.getMomentumAtArcLength(arcLength, nominalBz);
              int chargeSign = expectedHelix.getChargeSign();

              EXPECT_NEAR(tanLambda, 1 / tan(momentum.Theta()), absError);
              EXPECT_ANGLE_NEAR(phi0 + chi, momentum.Phi(), absError);
              EXPECT_NEAR(z0 + tanLambda * arcLength, position.Z(), absError);

              //B2INFO("chi out " << chi);
              Helix helix(position, momentum, chargeSign, nominalBz);

              EXPECT_NEAR(expectedHelix.getOmega(), helix.getOmega(), absError);
              EXPECT_ANGLE_NEAR(expectedHelix.getPhi0(), helix.getPhi0(), absError);
              EXPECT_NEAR(expectedHelix.getD0(), helix.getD0(), absError);
              EXPECT_NEAR(expectedHelix.getTanLambda(), helix.getTanLambda(), absError);
              EXPECT_NEAR(expectedHelix.getZ0(), helix.getZ0(), absError);
            }
          }

        }
      }
    }
  }





  TEST_F(HelixTest, CalcDrExplicit)
  {
    float tanLambda = 3;

    TVector3 center(0.0, -2.0, 0.0);
    float radius = -1;

    Helix helix = helixFromCenter(center, radius, tanLambda);
    EXPECT_NEAR(-1, helix.getD0(), absError);
    EXPECT_ANGLE_NEAR(-M_PI, helix.getPhi0(), absError);
    EXPECT_NEAR(-1, helix.getOmega(), absError);
    {
      TVector3 position(0.0, 0.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-1, newD0, absError);
    }

    {
      TVector3 position(2.0, -2.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-1, newD0, absError);
    }
    {
      TVector3 position(-2.0, -2.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-1, newD0, absError);
    }

    {
      TVector3 position(1.0, -1.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-(sqrt(2) - 1) , newD0, absError);
    }
  }

  TEST_F(HelixTest, CalcDr)
  {
    float z0 = 2;
    float tanLambda = 3;

    for (const float phi0 : phi0s) {
      for (const float omega : omegas) {
        for (const float d0 : d0s) {
          Helix helix(d0, phi0, omega, z0, tanLambda);
          TEST_CONTEXT("Failed for " << helix);

          EXPECT_NEAR(d0, helix.getDr(TVector3(0.0, 0.0, 0.0)), absError);

          for (const float chi : chis) {
            for (const float newD0 : d0s) {
              // In the line case use the chi value directly as the arc length

              float arcLength = omega == 0 ? chi : -chi / omega;
              TVector3 positionOnHelix = helix.getPositionAtArcLength(arcLength);

              TVector3 tangentialToHelix = helix.getUnitTangentialAtArcLength(arcLength);

              TVector3 perpendicularToHelix = tangentialToHelix;
              perpendicularToHelix.RotateZ(M_PI / 2.0);
              // Normalize the xy part
              perpendicularToHelix *= 1 / perpendicularToHelix.Perp();

              TVector3 displacementFromHelix = perpendicularToHelix * newD0;
              TVector3 testPosition = positionOnHelix + displacementFromHelix;

              TEST_CONTEXT("Failed for chi " << chi);
              TEST_CONTEXT("Failed for position on helix " << positionOnHelix);
              TEST_CONTEXT("Failed for tangential to helix " << tangentialToHelix);
              TEST_CONTEXT("Failed for perpendicular to helix " << perpendicularToHelix);
              TEST_CONTEXT("Failed for test position " << testPosition);

              float testDr = helix.getDr(testPosition);
              EXPECT_NEAR(newD0, testDr, absError);
            }
          }
        }
      }
    }
  }


  TEST_F(HelixTest, passiveMoveExplicit)
  {
    TVector3 center(0.0, 1.0, 0.0);
    float radius = -1;
    float tanLambda = 3;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    ASSERT_NEAR(0, helix.getD0(), absError);
    ASSERT_ANGLE_NEAR(0, helix.getPhi0(), absError);
    ASSERT_NEAR(-1, helix.getOmega(), absError);

    // Save the untransformed Helix
    Helix expectedHelix(helix);

    // Vector by which the coordinate system should move.
    // (To the top of the circle)
    TVector3 by(1.0, 1.0, 0.0);

    float arcLength = helix.passiveMoveBy(by);

    // The left of the circle lies in the counterclockwise direction
    // The forward direction is counterclockwise, so we expect to move forward.
    ASSERT_NEAR(M_PI / 2, arcLength, absError);

    ASSERT_NEAR(0, helix.getD0(), absError);
    ASSERT_ANGLE_NEAR(M_PI / 2, helix.getPhi0(), absError);
    ASSERT_NEAR(-1, helix.getOmega(), absError);

    ASSERT_NEAR(3 * M_PI / 2, helix.getZ0(), absError);
    ASSERT_NEAR(3, helix.getTanLambda(), absError);

    // Now transform back to the original point
    float arcLengthBackward = helix.passiveMoveBy(-by);

    ASSERT_NEAR(arcLength, -arcLengthBackward, absError);
    ASSERT_ALL_NEAR(expectedHelix, helix, absError);
  }


  TEST_F(HelixTest, passiveMove)
  {
    float z0 = 2;
    float tanLambda = 3;

    for (const float phi0 : phi0s) {
      for (const float omega : omegas) {
        for (const float d0 : d0s) {
          for (const float chi : chis) {
            for (const float newD0 : d0s) {
              Helix helix(d0, phi0, omega, z0, tanLambda);
              TEST_CONTEXT("Failed for " << helix);

              // In the line case use the chi value directly as the arc length

              float expectedArcLength = omega == 0 ? chi : -chi / omega;
              TVector3 positionOnHelix = helix.getPositionAtArcLength(expectedArcLength);
              TVector3 tangentialToHelix = helix.getUnitTangentialAtArcLength(expectedArcLength);

              TVector3 perpendicularToHelix = tangentialToHelix;
              perpendicularToHelix.RotateZ(M_PI / 2.0);
              // Normalize the xy part
              perpendicularToHelix *= 1 / perpendicularToHelix.Perp();

              TVector3 displacementFromHelix = -perpendicularToHelix * newD0;
              TVector3 testPosition = positionOnHelix + displacementFromHelix;

              TVector3 expectedPerigee = -displacementFromHelix;

              TEST_CONTEXT("Failed for chi " << chi);
              TEST_CONTEXT("Failed for position on helix " << positionOnHelix);
              TEST_CONTEXT("Failed for tangential to helix " << tangentialToHelix);
              TEST_CONTEXT("Failed for perpendicular to helix " << perpendicularToHelix);
              TEST_CONTEXT("Failed for test position " << testPosition);

              float arcLength = helix.passiveMoveBy(testPosition);

              ASSERT_NEAR(expectedArcLength, arcLength, absError);

              ASSERT_ALL_NEAR(expectedPerigee, helix.getPosition(), absError);

            }
          }
        }
      }
    }
  }




}  // namespace
