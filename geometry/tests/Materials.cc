#include <geometry/Materials.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <G4Element.hh>
#include <G4Material.hh>

using namespace std;

namespace Belle2 {
  namespace geometry {

    TEST(Materials, Element)
    {
      //Check that we can find hydrogen and that the basic Parameters
      //are correct.
      Materials &m = Materials::getInstance();
      G4Element* e1 = m.getElement("H");
      ASSERT_TRUE(e1);
      EXPECT_EQ("H", e1->GetName());
      EXPECT_EQ("H", e1->GetSymbol());
      EXPECT_EQ(1., e1->GetZ());
    }

    TEST(Materials, Material)
    {
      //Check if we find the Air Material which is named G4_AIR in Geant4
      //So check if Air and G4_AIR refer to the same material
      G4Material* m1 = Materials::get("Air");
      G4Material* m2 = Materials::get("G4_AIR");
      EXPECT_EQ(m1, m2);
      ASSERT_TRUE(m1);
      EXPECT_EQ("G4_AIR", m1->GetName());
      EXPECT_TRUE(Materials::get("Si"));
    }

    TEST(Materials, Create)
    {
      //Check creation of a simple mixture with fractions of other materials
      //The density of the new material should be equal to the weighted sum
      //of the original densities
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test'><state>Gas</state><Components>"
                         "<Material fraction='1.5'>Si</Material>"
                         "<Material fraction='2.5612'>Au</Material>"
                         "</Components></Material>");
      gb.setBackends(backends);
      gb.open();
      Materials &m = Materials::getInstance();
      ASSERT_EQ(0, gb.getNumberNodes("/Material/density"));
      ASSERT_EQ(1, gb.getNumberNodes("/Material/@name"));

      G4Material* au = Materials::get("Au");
      G4Material* si = Materials::get("Si");
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      EXPECT_EQ("Test", mat->GetName());
      EXPECT_DOUBLE_EQ((2.5612*au->GetDensity() + 1.5*si->GetDensity()) / 4.0612, mat->GetDensity());
      gb.close();
    }

    TEST(Materials, CreateDensityError)
    {
      //When adding elements one has to specify a density since elements do not
      //have a density
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test1'><state>Solid</state><components>"
                         "<material fraction='0.5'>Si</material>"
                         "<element fraction='0.5'>Au</element>"
                         "</components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    TEST(Materials, CreateDensity)
    {
      //Same as above, but with density so it should work
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test2'><state>Liquid</state><density>1</density><components>"
                         "<material>Si</material>"
                         "<element>Au</element>"
                         "</components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      gb.close();
    }

    TEST(Materials, CreateMaterialError)
    {
      //When adding unknown materials we should get NULL
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test3'><components>"
                         "<material>Foo</material>"
                         "</components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    TEST(Materials, CreateElementError)
    {
      //When adding unknown elements we should get NULL
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test4'><density>1</density><components>"
                         "<element>Foo</element>"
                         "</components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

  }
}  // namespace
