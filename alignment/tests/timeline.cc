#include <alignment/GlobalTimeLine.h>

#include <alignment/PedeResult.h>

#include <vxd/dataobjects/VxdID.h>
#include <cdc/dataobjects/WireID.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include <framework/database/Database.h>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include <TClonesArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/Hierarchy.h>
#include <alignment/Manager.h>
#include <TMath.h>



using namespace std;
using namespace Belle2;
using namespace alignment;
using namespace timeline;

namespace {
  template<int UID>
  class MockDBObj {
  public:
    MockDBObj(double value = 0.) : m_param(value) {}
    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID()
    {
      static_assert(UID != 0, "Non-zero UID required");
      return UID;
    }
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short param) {return m_param;}
    /// Set global parameter
    void setGlobalParam(double value, unsigned short element, unsigned short param) {m_param = value;}
    /// TODO: list stored global parameters
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {{0, 0}};}
  private:
    double m_param{0.};
  };


  class TimeLineTest : public ::testing::Test {
  protected:

    IntraIoVPayloads beamspotRun0 = {
      {EventMetaData(0, 0, 7), std::make_shared<GlobalParamSet<BeamParameters>>()}
    };

    IntraIoVPayloads beamspotRun1 = {
      {EventMetaData(0, 1, 7), std::make_shared<GlobalParamSet<BeamParameters>>()}
    };

    IntraIoVPayloads beamspotRun2 = {
      {EventMetaData(0, 2, 7), std::make_shared<GlobalParamSet<BeamParameters>>()},
      {EventMetaData(530532, 2, 7), std::make_shared<GlobalParamSet<BeamParameters>>()}
    };

    IntraIoVPayloads beamspotRun3 = {
      {EventMetaData(0, 3, 7), std::make_shared<GlobalParamSet<BeamParameters>>()}
    };

    IntraIoVPayloads beamspotRun4 = {
      {EventMetaData(0, 4, 7), std::make_shared<GlobalParamSet<BeamParameters>>()}
    };

    PayloadIovBlockRow beamspotRow = {
      {IntervalOfValidity(7, 0, 7, 0), beamspotRun0},
      {IntervalOfValidity(7, 1, 7, 1), beamspotRun1},
      {IntervalOfValidity(7, 2, 7, 2), beamspotRun2},
      {IntervalOfValidity(7, 3, 7, 3), beamspotRun3},
      {IntervalOfValidity(7, 4, 7, 4), beamspotRun4}
    };

    IntraIoVPayloads vxdRun0to2 = {
      {EventMetaData(0, 0, 7), std::make_shared<GlobalParamSet<VXDAlignment>>()},
    };

    IntraIoVPayloads vxdRun3toInf = {
      {EventMetaData(0, 3, 7), std::make_shared<GlobalParamSet<VXDAlignment>>()},
    };

    PayloadIovBlockRow vxdRow = {
      {IntervalOfValidity(7, 0, 7, 2), vxdRun0to2},
      {IntervalOfValidity(7, 3, 7, 4), vxdRun3toInf}

    };

    IntraIoVPayloads cdcBlock = {
      {EventMetaData(0, 0, 7), std::make_shared<GlobalParamSet<CDCAlignment>>()}

    };
    PayloadIovBlockRow cdcRow = {
      {IntervalOfValidity(7, 0, 7, 4), cdcBlock}
    };

    PayloadsTable payloadsTable = {
      {BeamParameters::getGlobalUniqueID(), beamspotRow},
      {VXDAlignment::getGlobalUniqueID(), vxdRow},
      {CDCAlignment::getGlobalUniqueID(), cdcRow}
    };

    std::vector<int> head0 = {0, 1, 2, 3, 4, 5};
    std::vector<int> head1 = {0, 1, 2, 2, 3, 4};
    std::vector<int> row0  = {0, 1, 2, 3, 4, 5};
    std::vector<int> row1  = {0, 0, 0, 0, 1, 1};
    std::vector<int> row2  = {0, 0, 0, 0, 0, 0};

    EventHeader eventHeader = {
      EventMetaData(0, 0, 7),
      EventMetaData(0, 1, 7),
      EventMetaData(0, 2, 7),
      EventMetaData(530532, 2, 7),
      EventMetaData(0, 3, 7),
      EventMetaData(0, 4, 7)
    };

    RunHeader runHeader = {
      {7, 0},
      {7, 1},
      {7, 2},
      {7, 2},
      {7, 3},
      {7, 4}
    };

    TableData tableData = {
      { 1, row0},
      {10, row1},
      {27, row2}
    };

    TimeTable timeTable = std::make_tuple(eventHeader, runHeader, tableData);

    std::vector<int> irow0  = {0, 1, 1, 1, 1, 1};
    std::vector<int> irow1  = {0, 0, 0, 0, 1, 0};
    std::vector<int> irow2  = {0, 0, 0, 0, 0, 0};

    TableData initTableData = {
      { 1, irow0},
      {10, irow1},
      {27, irow2}
    };

    TimeTable initTimeTable = std::make_tuple(eventHeader, runHeader, initTableData);

    /// init
    virtual void SetUp()
    {
      GlobalLabel::clearTimeDependentParamaters();
      auto beamspotX = GlobalLabel::construct<BeamParameters>(0, 1);
      auto beamspotZ = GlobalLabel::construct<BeamParameters>(0, 3);
      auto ying = GlobalLabel::construct<VXDAlignment>(VxdID(1, 0, 0, 1).getID(), 3);

      ying.registerTimeDependent(4);

      beamspotX.registerTimeDependent(3);

      beamspotZ.registerTimeDependent(1);
      beamspotZ.registerTimeDependent(2);
      beamspotZ.registerTimeDependent(3);
      beamspotZ.registerTimeDependent(4);
      beamspotZ.registerTimeDependent(5);

    }

    /// cleanup
    virtual void TearDown()
    {
      GlobalLabel::clearTimeDependentParamaters();

    }

  };


  TEST_F(TimeLineTest, InitialTimeTable)
  {
    GlobalLabel label;
    auto table = makeInitialTimeTable(eventHeader, label);

    std::vector<int> myrow0  = {0, 1, 1, 1, 1, 1};
    std::vector<int> myrow1  = {0, 0, 0, 0, 1, 0};

    TableData myInitTableData = {
      { 1, myrow0},
      {10, myrow1}
    };

    TimeTable myInitTimeTable = std::make_tuple(eventHeader, runHeader, myInitTableData);


    EXPECT_EQ(table, myInitTimeTable);

  }

  TEST_F(TimeLineTest, TimeTable)
  {
    finalizeTimeTable(initTimeTable);
    EXPECT_EQ(initTimeTable, timeTable);

    EXPECT_EQ(getContinuousIndexByTimeID(timeTable, 1,  3), 3);
    EXPECT_EQ(getContinuousIndexByTimeID(timeTable, 10, 3), 0);
    EXPECT_EQ(getContinuousIndexByTimeID(timeTable, 27, 3), 0);

  }

  /// Test calculation of global derivatives
  TEST_F(TimeLineTest, PayloadTable)
  {

    getPayloadByContinuousIndex(payloadsTable, 1, 3).second->setGlobalParam(42., 0, 1);
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 3).second->getGlobalParam(0, 1), 42.);


    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 0).first, EventMetaData(0, 0, 7));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 1).first, EventMetaData(0, 1, 7));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 2).first, EventMetaData(0, 2, 7));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 3).first, EventMetaData(530532, 2, 7));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 4).first, EventMetaData(0, 3, 7));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 5).first, EventMetaData(0, 4, 7));

    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 10, 0).first, EventMetaData(0, 0, 7));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 10, 1).first, EventMetaData(0, 3, 7));

    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 27, 0).first, EventMetaData(0, 0, 7));


  }

  TEST_F(TimeLineTest, TimeTable2PayloadsTable)
  {
    GlobalParamVector vector({"BeamParameters", "VXDAlignment", "CDCAlignment"});
    GlobalCalibrationManager::initGlobalVector(vector);

    int timeid = 0;
    EventMetaData ev;

    // vxd
    timeid = 0;
    ev = gotoBeforeNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 0);
    EXPECT_EQ(ev, EventMetaData(0, 0, 7));

    timeid = 1;
    ev = gotoBeforeNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 1);
    EXPECT_EQ(ev, EventMetaData(0, 1, 7));

    timeid = 4;
    ev = gotoBeforeNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 4);
    EXPECT_EQ(ev, EventMetaData(0, 3, 7));

    timeid = 5;
    ev = gotoBeforeNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 7));

    // beam
    timeid = 0;
    ev = gotoBeforeNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 0);
    EXPECT_EQ(ev, EventMetaData(0, 0, 7));

    timeid = 1;
    ev = gotoBeforeNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 1);
    EXPECT_EQ(ev, EventMetaData(0, 1, 7));

    timeid = 2;
    ev = gotoBeforeNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 7));

    timeid = 3;
    ev = gotoBeforeNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 7));

    timeid = 4;
    ev = gotoBeforeNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 4);
    EXPECT_EQ(ev, EventMetaData(0, 3, 7));

    timeid = 5;
    ev = gotoBeforeNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 7));



    timeid = 0;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 0);
    EXPECT_EQ(ev, EventMetaData(0, 0, 7));

    timeid = 1;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 1);
    EXPECT_EQ(ev, EventMetaData(0, 1, 7));

    timeid = 2;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 7));

    timeid = 3;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 7));

    timeid = 4;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 4);
    EXPECT_EQ(ev, EventMetaData(0, 3, 7));

    timeid = 5;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 7));



    timeid = 0;
    ev = gotoNextChangeRunWise(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 7));
    ev = gotoNextChangeRunWise(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 7));

    timeid = 4;
    ev = gotoNextChangeRunWise(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 7));




    auto table = TimeIdsTable2PayloadsTable(timeTable, vector);


    EXPECT_EQ(getPayloadByContinuousIndex(table, 27, 0).first.getExperiment(), getPayloadByContinuousIndex(payloadsTable, 27,
              0).first.getExperiment());


    B2INFO(table.at(27).at(0).second.at(0).first.getEvent() << " " << table.at(27).at(0).second.at(0).first.getRun() << " " << table.at(
             27).at(0).second.at(0).first.getExperiment());
    EXPECT_EQ(table.at(27).at(0).second.at(0).first, payloadsTable.at(27).at(0).second.at(0).first);
    B2INFO(table.at(27).at(0).second.at(0).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(27).at(0).second.at(0).second->getGlobalUniqueID(),
              payloadsTable.at(27).at(0).second.at(0).second->getGlobalUniqueID());

    B2INFO(table.at(10).at(0).second.at(0).first.getEvent() << " " << table.at(10).at(0).second.at(0).first.getRun() << " " << table.at(
             10).at(0).second.at(0).first.getExperiment());
    EXPECT_EQ(table.at(10).at(0).second.at(0).first, payloadsTable.at(10).at(0).second.at(0).first);
    B2INFO(table.at(10).at(0).second.at(0).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(10).at(0).second.at(0).second->getGlobalUniqueID(),
              payloadsTable.at(10).at(0).second.at(0).second->getGlobalUniqueID());

    B2INFO(table.at(1).at(0).second.at(0).first.getEvent() << " " << table.at(1).at(0).second.at(0).first.getRun() << " " << table.at(
             1).at(0).second.at(0).first.getExperiment());
    EXPECT_EQ(table.at(1).at(0).second.at(0).first, payloadsTable.at(1).at(0).second.at(0).first);
    B2INFO(table.at(1).at(0).second.at(0).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(1).at(0).second.at(0).second->getGlobalUniqueID(),
              payloadsTable.at(1).at(0).second.at(0).second->getGlobalUniqueID());

    B2INFO(table.at(1).at(2).second.at(1).first.getEvent() << " " << table.at(1).at(2).second.at(1).first.getRun() << " " << table.at(
             1).at(2).second.at(1).first.getExperiment());
    EXPECT_EQ(table.at(1).at(2).second.at(1).first, payloadsTable.at(1).at(2).second.at(1).first);
    B2INFO(table.at(1).at(2).second.at(1).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(1).at(2).second.at(1).second->getGlobalUniqueID(),
              payloadsTable.at(1).at(2).second.at(1).second->getGlobalUniqueID());


  }

  TEST_F(TimeLineTest, GlobalParamTimeLine)
  {

  }
}  // namespace
