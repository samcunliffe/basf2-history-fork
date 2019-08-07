/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <alignment/GlobalLabel.h>
#include <alignment/GlobalParam.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/EventDependency.h>
#include <memory>

namespace Belle2 {
  namespace alignment {
    namespace timeline {

      typedef std::pair<int, int> ExpRun;
      typedef std::vector<EventMetaData> EventHeader;
      typedef std::vector<ExpRun> RunHeader;
      typedef std::unordered_map<int, std::vector<int>> TableData;
      typedef std::tuple<EventHeader, RunHeader, TableData> TimeTable;

      typedef std::vector< std::pair<EventMetaData, std::shared_ptr<GlobalParamSetAccess>>> IntraIoVPayloads;
      typedef std::vector<std::pair<IntervalOfValidity, IntraIoVPayloads>> PayloadIovBlockRow;
      typedef std::unordered_map<int, PayloadIovBlockRow> PayloadsTable;

      EventMetaData gotoNextChangeRunWise(TimeTable& timeTable, int uid, int& timeid)
      {
        auto& row = std::get<TableData>(timeTable).at(uid);

        auto lastIntervalStartEvent = std::get<EventHeader>(timeTable).at(row.size() - 1);

        if (timeid >= int(row.size())) {
          return lastIntervalStartEvent;
        }

        auto cell = row.at(timeid);
        auto cellRun = std::get<RunHeader>(timeTable).at(timeid);
        for (long unsigned int iCol = timeid + 1; iCol < row.size(); ++iCol) {
          if (row.at(iCol) != cell && std::get<RunHeader>(timeTable).at(iCol) != cellRun) {
            timeid = iCol - 1;
            return std::get<EventHeader>(timeTable).at(iCol - 1);
          }
        }
        timeid = row.size() - 1;

        return lastIntervalStartEvent;
      }

      EventMetaData gotoBeforeNextChangeInRun(TimeTable& timeTable, int uid, int& timeid)
      {
        auto& row = std::get<TableData>(timeTable).at(uid);
        auto cell = row.at(timeid);
        auto cellRun = std::get<RunHeader>(timeTable).at(timeid);
        for (long unsigned int iCol = timeid + 1; iCol < row.size(); ++iCol) {
          if (std::get<RunHeader>(timeTable).at(iCol) != cellRun) {
            timeid = iCol - 1;
            return std::get<EventHeader>(timeTable).at(iCol - 1);
          }
          if (row.at(iCol) != cell) {
            timeid = iCol;
            return std::get<EventHeader>(timeTable).at(iCol);
          }
        }
        return std::get<EventHeader>(timeTable).at(timeid);
      }

      PayloadsTable TimeIdsTable2PayloadsTable(TimeTable& timeTable, GlobalParamVector& vector)
      {
        PayloadsTable payloadsTable;

        for (auto& uid_obj : vector.getGlobalParamSets()) {
          auto uid = uid_obj.first;
          auto& obj = uid_obj.second;

          payloadsTable[uid] = {};

          if (std::get<TableData>(timeTable).find(uid) == std::get<TableData>(timeTable).end()) {
            auto firstEvent = std::get<EventHeader>(timeTable).at(0);
            auto lastEvent = std::get<EventHeader>(timeTable).at(std::get<EventHeader>(timeTable).size() - 1);

            auto iov = IntervalOfValidity(firstEvent.getExperiment(), firstEvent.getRun(), lastEvent.getExperiment(), lastEvent.getRun());
            auto objCopy = std::shared_ptr<GlobalParamSetAccess>(obj->clone());
            payloadsTable[uid].push_back({ iov, {{firstEvent, objCopy}} });

            continue;
          }
          int iCol = 0;
          // Now add PayloadIovBlockRow with run-spanning IoVs
          for (; iCol < int(std::get<EventHeader>(timeTable).size()); ++iCol) {
            auto event = std::get<EventHeader>(timeTable).at(iCol);
            auto exprun = std::get<RunHeader>(timeTable).at(iCol);
            auto exp = exprun.first;
            auto run = exprun.second;

            // Prepare intra run objects
            // 1st is always there (even for non-intra-run)
            auto objCopy = std::shared_ptr<GlobalParamSetAccess>(obj->clone());
            IntraIoVPayloads intraRunEntries;
            intraRunEntries.push_back({event, objCopy});
            // At each change in run, add new entry
            auto lastEvent = event;
            for (; iCol < int(std::get<EventHeader>(timeTable).size());) {
              auto nextEvent = gotoBeforeNextChangeInRun(timeTable, uid, iCol);
              if (nextEvent != lastEvent) {
                auto objIntraRunCopy = std::shared_ptr<GlobalParamSetAccess>(obj->clone());
                intraRunEntries.push_back({nextEvent, objIntraRunCopy});
                lastEvent = nextEvent;
              } else {
                break;
              }
            }

            // Move to next IoV block (for intra-run deps in just processed block, next block is always the next run)
            auto endEvent = gotoNextChangeRunWise(timeTable, uid, iCol);
            int endExp = endEvent.getExperiment();
            int endRun = endEvent.getRun();
            // Store finished block
            payloadsTable[uid].push_back({IntervalOfValidity(exp, run, endExp, endRun), intraRunEntries});

          }

        }

        return payloadsTable;
      }

      TimeTable makeInitialTimeTable(std::vector<EventMetaData> events, GlobalLabel& label)
      {
        TimeTable table;
        std::vector<int> nullRow(events.size(), 0);

        // event header
        std::get<EventHeader>(table) = events;

        // run header
        RunHeader runs;
        for (auto event : events) {
          runs.push_back({event.getExperiment(), event.getRun()});
        }
        std::get<RunHeader>(table) = runs;

        for (auto& eidpid_intervals : label.getTimeIntervals()) {
          auto uid = GlobalLabel(eidpid_intervals.first).getUniqueId();
          if (std::get<TableData>(table).find(uid) == std::get<TableData>(table).end()) {
            std::get<TableData>(table)[uid] = nullRow;
          }
          unsigned int lastTime = 0;
          for (long unsigned int timeid = 0; timeid < events.size(); ++timeid) {
            if (lastTime != eidpid_intervals.second.get(timeid)) {
              std::get<TableData>(table)[uid][timeid] = 1;
              lastTime = timeid;
            }
          }
        }

        return table;
      }

      void finalizeTimeTable(TimeTable& table)
      {
        for (auto& row : std::get<TableData>(table)) {
          auto& cells = row.second;

          int currIndex = 0;
          for (long unsigned int iCell = 0; iCell < cells.size(); ++iCell) {
            auto cell = cells.at(iCell);
            if (iCell == 0) {
              if (cell != 0) {
                B2FATAL("First cell (index 0) has to be zero (time id for const objects or 1st instance of time-dep objects) for each row.");
              }
              continue;
            }
            if (cell != 0 && cell != 1) {
              B2FATAL("In initial time table, only cells with 0 (=no change of object at beginning of cell) or 1 (object can change at beginning of this cell) are allowed");
            }
            // Now cell is not first and is either 0 or 1 (-> increment index)
            if (cell == 1) {
              ++currIndex;
            }
            cells.at(iCell) = currIndex;
          }
        }
      }

      std::pair<EventMetaData, std::shared_ptr<GlobalParamSetAccess>> getPayloadByContinuousIndex(PayloadsTable& payloadsTable, int uid,
          long unsigned int index)
      {
        auto& row = payloadsTable.at(uid);

        long unsigned int currentIndex = 0;
        for (long unsigned int iIovBlock = 0; iIovBlock < row.size(); ++iIovBlock) {
          if (currentIndex + row.at(iIovBlock).second.size() > index) {
            return row.at(iIovBlock).second.at(index - currentIndex);
          }
          currentIndex += row.at(iIovBlock).second.size();
        }

        return {EventMetaData(), {}};
      };

      int getContinuousIndexByTimeID(const TimeTable& timeTable, int uid, int timeid)
      {
        auto cIndex = std::get<TableData>(timeTable).at(uid)[timeid];
        return cIndex;
      }

      class GlobalParamTimeLine {

      private:
        TimeTable timeTable;
        PayloadsTable payloadsTable;

      public:

        GlobalParamTimeLine(std::vector<EventMetaData> events, GlobalLabel& label, GlobalParamVector& vector)
        {
          timeTable = makeInitialTimeTable(events, label);
          finalizeTimeTable(timeTable);

          payloadsTable = TimeIdsTable2PayloadsTable(timeTable, vector);

        };

        void loadFromDB()
        {
          for (auto& row : payloadsTable) {
            for (auto& iovBlock : row.second) {
              for (auto& payload : iovBlock.second) {
                payload.second->loadFromDB(payload.first);
              }
            }
          }
        }

        void updateGlobalParam(GlobalLabel label, double correction)
        {
          auto timeid = label.getTimeId();
          auto eov = label.getEndOfValidity();
          auto uid = label.getUniqueId();

          std::set<int> payloadIndices;
          // this is probably dangerous if we do not impose additional invariant
          //TODO: better to always loop over whole event header?
          for (int i = timeid; i < std::min(eov + 1, int(std::get<EventHeader>(timeTable).size())); ++i) {
            payloadIndices.insert(getContinuousIndexByTimeID(timeTable, uid, i));
          }

          for (auto payloadIndex : payloadIndices) {
            getPayloadByContinuousIndex(payloadsTable, label.getUniqueId(), payloadIndex).second->updateGlobalParam(correction,
                label.getElementId(), label.getParameterId());
          }

        }

        std::vector<std::pair<IntervalOfValidity, TObject*>> releaseObjects()
        {
          std::vector<std::pair<IntervalOfValidity, TObject*>> result;

          for (auto& row : payloadsTable) {
            for (auto& iovBlock : row.second) {
              auto iov = iovBlock.first;
              auto obj = iovBlock.second.at(0).second->releaseObject();

              // non-intra-run
              if (iovBlock.second.size() == 1) {
                result.push_back({iov, obj});
                continue;
              }

              // First obj in event dependency
              //TODO: how the lifetime of EventDependency is handled?
              // both work now -> have to check data storage in DB in real life scenario
              auto payloads = new EventDependency(obj);
              //auto payloads = EventDependency(obj);
              // Add others
              for (long unsigned int iObj = 1; iObj < iovBlock.second.size(); ++iObj) {
                auto nextEvent = iovBlock.second.at(iObj).first.getEvent();
                auto nextObj = iovBlock.second.at(iObj).second->releaseObject();

                payloads->add(nextEvent, nextObj);
                //payloads.add(nextEvent, nextObj);
              }
              result.push_back({iov, payloads});
              //result.push_back({iov, &payloads});
            }
          }
          return result;
        }

      };
    } // namespace timeline
  } // namespace alignment
} // namespace Belle2
