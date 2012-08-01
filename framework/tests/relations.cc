#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/RunMetaData.h>
#include <gtest/gtest.h>
#include <boost/foreach.hpp>
#include <iostream>
using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** The fixture for testing class Foo. */
  class RelationTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    virtual void SetUp() {
      for (int i = 0; i < 10; ++i) {
        new(evtData->AddrAt(i)) EventMetaData();
        new(runData->AddrAt(i)) RunMetaData();
      }
    }

    /** clear datastore */
    virtual void TearDown() {
      for (int i = 0; i < DataStore::c_NDurabilityTypes; ++i) {
        DataStore::Instance().clearMaps((DataStore::EDurability) i);
      }
    }

    StoreArray<EventMetaData> evtData; /**< event data array */
    StoreArray<RunMetaData> runData; /**< run data array */
  };

  /** Tests the creation of a Relation. */
  TEST_F(RelationTest, RelationCreate)
  {
    RelationArray relation(evtData, runData);
    EXPECT_TRUE(relation);
  }

  /** Check finding of relations. */
  TEST_F(RelationTest, RelationFind)
  {
    EXPECT_FALSE(RelationArray(evtData, runData, "", DataStore::c_Event, false));
    RelationArray relation(evtData, runData);
    EXPECT_TRUE(RelationArray(evtData, runData, "", DataStore::c_Event, false));
    string name = relation.getName();
    EXPECT_TRUE(RelationArray(name));

    StoreArray<EventMetaData> evtData2("OwnName");
    //check for OwnNameToRunMetaDatas
    EXPECT_FALSE(RelationArray(evtData2, runData, "", DataStore::c_Event, false));
    EXPECT_FALSE(RelationArray("OwnNameToRunMetaDatas", DataStore::c_Event));
    RelationArray relation2(evtData2, runData);
    EXPECT_TRUE(relation2.getName() == "OwnNameToRunMetaDatas");
    EXPECT_TRUE(RelationArray(evtData2, runData, "", DataStore::c_Event, false));
  }

  /** Test that adding to an invalid relation yields a FATAL */
  TEST_F(RelationTest, AddInvalidDeathTest)
  {
    RelationArray relation(evtData, runData, "", DataStore::c_Event, false);
    EXPECT_FALSE(relation);
    EXPECT_FATAL(relation.add(0, 0, 1.0));
    EXPECT_FATAL(relation[0]);
    EXPECT_FATAL(relation.getFromAccessorParams());
    EXPECT_FATAL(relation.getToAccessorParams());
    EXPECT_FATAL(relation.getModified());
  }

  /** Test that Relations wich points to the wrong arrays yields a FATAL. */
  TEST_F(RelationTest, RelationWrongDeathTest)
  {
    RelationArray relation1(evtData, runData, "test");
    EXPECT_FATAL(RelationArray relation2(runData, evtData, "test"));
  }

  /** Check consolidation of RelationElements. */
  TEST_F(RelationTest, RelationConsolidate)
  {
    RelationArray relation(evtData, runData);
    relation.add(0, 0, 1.0);
    relation.add(0, 1, 2.0);
    relation.add(0, 1, 3.0);
    relation.add(1, 0, 1.0);
    ASSERT_EQ(relation.getEntries(), 4);
    relation.consolidate();
    ASSERT_EQ(relation.getEntries(), 2);
    EXPECT_EQ(relation[0].getWeight(0), 1.0);
    EXPECT_EQ(relation[0].getWeight(1), 5.0);
    EXPECT_EQ(relation[1].getWeight(0), 1.0);

    std::map<unsigned int, unsigned int> replace;
    replace[0] = 1;
    RelationArray::ReplaceMap<> replaceMap(replace);
    relation.consolidate(replaceMap, replaceMap);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getWeight(0), 7.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(), 1u);
  }

  /** Check creation of an index. */
  TEST_F(RelationTest, BuildIndex)
  {
    RelationArray relation(evtData, runData);
    relation.add(0, 0, 1.0);
    relation.add(0, 1, 2.0);
    relation.add(0, 2, 3.0);
    relation.consolidate();
    EXPECT_EQ(relation.getEntries(), 1);

    RelationIndex<EventMetaData, RunMetaData> relIndex;
    EXPECT_EQ(relIndex.size(), 3u);

    relation.add(1, 0, 1.0);
    RelationIndex<EventMetaData, RunMetaData> relIndex2;
    EXPECT_EQ(relIndex2.size(), 4u);
    //Rebuilding index will affect old index. Should we copy index?
    //Copy could be expensive and this should be a corner-case anyway
    EXPECT_EQ(relIndex.size(), 4u);

    typedef const RelationIndex<EventMetaData, RunMetaData>::Element el_t;
    //check elements of last relation (both from objects point to to_obj)
    const EventMetaData* first_from_obj = evtData[0];
    const EventMetaData* from_obj = evtData[1];
    const RunMetaData* to_obj = runData[0];
    EXPECT_TRUE(first_from_obj == relIndex.getFirstElementTo(to_obj)->from);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementTo(to_obj)->to);
    EXPECT_TRUE(1.0 == relIndex.getFirstElementTo(to_obj)->weight);
    EXPECT_TRUE(first_from_obj == relIndex.getFirstElementTo(*to_obj)->from);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementFrom(from_obj)->to);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementFrom(*from_obj)->to);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementFrom(first_from_obj)->to);

    //check search for non-existing relations
    EXPECT_TRUE(relIndex.getFirstElementTo(0) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementFrom(0) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementFrom(0) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementFrom(evtData[4]) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementTo(runData[3]) == NULL);

    //check size of found element lists
    {
      int size(0);
      double allweights(0);
      BOOST_FOREACH(el_t & e, relIndex.getElementsFrom(evtData[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 3);
      EXPECT_DOUBLE_EQ(allweights, 6.0);
    }
    {
      int size(0);
      double allweights(0);
      BOOST_FOREACH(el_t & e, relIndex.getElementsTo(runData[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 2);
      EXPECT_DOUBLE_EQ(allweights, 2.0);
    }
    {
      int size(0);
      double allweights(0);
      BOOST_FOREACH(el_t & e, relIndex.getElementsTo(runData[4])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 0);
      EXPECT_DOUBLE_EQ(allweights, 0.0);
    }
  }

  /** Check wether out-of-bound indices are caught by RelationIndex. */
  TEST_F(RelationTest, InconsitentIndexDeathTest)
  {
    RelationArray relation(evtData, runData);
    relation.add(0, 10, 1.0);
    typedef RelationIndex<EventMetaData, RunMetaData> rel_t;
    EXPECT_FATAL(rel_t relIndex);
    relation.clear();
    relation.add(10, 0, 1.0);
    EXPECT_FATAL(rel_t relIndex);
  }


  /** Check behaviour when attaching to non-existing(=empty) relation */
  TEST_F(RelationTest, EmptyIndex)
  {
    RelationIndex<EventMetaData, RunMetaData> index;
    EXPECT_FALSE(index);
    EXPECT_EQ(index.size(), 0u);
    EXPECT_EQ(index.getFromAccessorParams().first, "");
    EXPECT_EQ(index.getToAccessorParams().first, "");
  }

  /** Attaching to relation with from and two swapped, and with different StoreArray of same type. */
  TEST_F(RelationTest, WrongRelationIndexDeathTest)
  {
    RelationArray relation(runData, evtData, "test");
    typedef RelationIndex<EventMetaData, RunMetaData> rel_t;
    EXPECT_FATAL(rel_t(evtData, runData, "test"));
    EXPECT_FATAL(rel_t("test"));

    StoreArray<EventMetaData> eventData("evts");
    RelationArray relation2(evtData, runData, "test2");
    EXPECT_FATAL(rel_t(eventData, runData, "test2"));
    //This relation works and points to evtData, not eventData.
    //no check is performed, user is responsible to check
    //using getFromAccessorParams and getToAccessorParams
    EXPECT_TRUE(rel_t("test2"));
  }

}  // namespace
