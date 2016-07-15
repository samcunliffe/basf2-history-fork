#include <framework/utilities/GeneralCut.h>
#include <gtest/gtest.h>

using namespace Belle2;
namespace {
  /// Class to mock objects for out variable manager.
  struct MockObjectType {
    double value = 4.2;
  };

  /**
   *  Class to mock variables for out variable manager.
   *  This is also the minimal interface a variable must have,
   *  to be used in the GeneralCut.
   */
  class MockVariableType {
  public:
    double function(const MockObjectType* object) const
    {
      return object->value;
    }

    const std::string name = "mocking_variable";
  };

  /**
   *  Mock a variable manager for tests. This is also the minimal interface a
   *  class must have to be used as a VariableManager in the GeneralCut tenplate.
   */
  class MockVariableManager {
  public:
    typedef MockObjectType Object;
    typedef MockVariableType Var;

    static MockVariableManager& Instance()
    {
      static MockVariableManager instance;
      return instance;
    }

    Var* getVariable(const std::string& name)
    {
      if (name == "mocking_variable") {
        return &m_mocking_variable;
      } else {
        return nullptr;
      }
    }

    Var m_mocking_variable;
  };

  /// Test for the general cut: Try to compile some cuts and check their result
  // using a mocked variable manager.
  TEST(GeneralCutTest, cutCheck)
  {
    typedef GeneralCut<MockVariableManager> MockGeneralCut;

    MockObjectType testObject;

    std::unique_ptr<MockGeneralCut> a = MockGeneralCut::compile("1.2 < 1.5 ");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile(" 1.5<1.2");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile(" 12.3 >1.5 ");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("12 > 15");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile("1.2 == 1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile(" 1.5!=1.2");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1.2 == 1.2 == 1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile(" 1.5 == 1.5!=1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile(" 1.5 == 1.5!=1.5");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile("1.0 < 1.2 == 1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile(" 1.5 < 1.6 != 1.6");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::compile(" 1.5 < 1.6 != 1.7");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile(" [12 >= 12 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("[ 15>= 16 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile(" [12 <= 12 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("  [ 17<= 16.7 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile(" [12 <= 12 < 13]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("  [ 17<= 16.7 < 18 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile(" [12 >= 12 < 13]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("  [ 15> 16.7 <= 18 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile("mocking_variable > 1.0");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("1.0 < mocking_variable <= mocking_variable");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("mocking_variable < 100.0");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("mocking_variable <= mocking_variable <= mocking_variable");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1 < 2 and 3 < 4");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("1 < 2 and 4 < 3");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::compile("2 < 1 and 4 < 3");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::compile("2 < 1 and 3 < 4");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile("1 < 2 or 3 < 4");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("1 < 2 or 4 < 3");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("2 < 1 or 4 < 3");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::compile("2 < 1 or 3 < 4");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1 < 2 and 3 < 4 and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("[1 < 2 < 3 or 3 > 4 ] and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("[1 < 2 < 3 or 3 > 4 ] or [ 5 < 6 and 7 > 6 ]");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1 < 2 and 3 < 4 or 5 > 6");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("1 < 2 or 3 < 4 and 5 > 6");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("1 < 2 and 4 < 3 or 6 > 5");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("1 < 2 or 4 < 3 and 6 > 5");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1 != 2 and 3 < 4 or 5 > 6");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::compile("1 < 2 or 3 != 4 and 5 > 6");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1 != 2 and 3 == 4 or 5 > 6");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::compile("1 < 2 or 3 == 4 and 5 > 6");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("15 != 0x15");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("15 == 0xF");
    EXPECT_TRUE(a->check(&testObject));

    // Should throw an exception
    EXPECT_THROW(a = MockGeneralCut::compile("15 == 15.0 bla"), std::runtime_error);
    EXPECT_TRUE(a->check(&testObject));
    EXPECT_THROW(a = MockGeneralCut::compile("15 == 15e1000"), std::out_of_range);
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1e-3 < 1e3");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1e-3 == 0.001");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1000 < infinity");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::compile("1000 > infinity");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile("1000 < nan");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::compile("1000 > nan");
    EXPECT_FALSE(a->check(&testObject));
  }


}  // namespace
