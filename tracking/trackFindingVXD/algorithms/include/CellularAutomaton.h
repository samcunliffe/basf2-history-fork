/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/algorithms/TrackerAlgorithmBase.h>


namespace Belle2 {


  /** The CellularAutomaton class
   * This class serves as a functor for the algorithm itself
   */
  template<class ContainerType, class ValidatorType, class LoggerType>
  class CellularAutomaton : public TrackerAlgorithmBase<ContainerType, ValidatorType, LoggerType> {
  protected:

  public:

    /** typedef for the baseClass to get rid of the template arguments */
    typedef TrackerAlgorithmBase<ContainerType, ValidatorType, LoggerType> BaseClass;

    /** constructor */
    CellularAutomaton() : BaseClass() {}

    /** actual algorithm of Cellular Automaton, returns number of rounds needed to finish or -1 if CA was aborted */
    int apply(ContainerType& aNetworkContainer) override
    {
      /** REDESIGNCOMMENT CELLULARAUTOMATON:
       * optimization tip:
       * - at the moment for each round the number of active cells is reduced, but the loop runs over all nodes/cells. Maybe its faster to recreate/refill an ActiceCell-List each round.
       */
      unsigned int activeCells = 1, // is set 1 because of following while loop.
                   deadCells = 0,
                   caRound = 1,
                   goodNeighbours = 0,
                   highestCellState = 0;

      B2DEBUG(10, "starting ca-loop with network of" << aNetworkContainer.size() <<
              " nodes, which will be treated as cells. CA had " << BaseClass::m_log.nPasses <<
              " passes so far");

      // each iteration of following while loop is one CA-time-step
      while (activeCells != 0) {
        activeCells = 0;

        /// CAstep:
        // compare cells with inner neighbours:
        for (auto* aNode : aNetworkContainer) {
          auto& currentCell = aNode->getMetaInfo();
          if (currentCell.isActivated() == false) { continue; }

          for (auto* aNeighbour :  aNode->getInnerNodes()) {
            // skip if neighbour has not the same state
            if (currentCell != aNeighbour->getMetaInfo()) continue;

            goodNeighbours++;
            B2DEBUG(100, "neighbouring cell found!")
          }
          if (goodNeighbours != 0) {
            currentCell.setStateUpgrade(true);
            activeCells++;
            B2DEBUG(50, "CAstep: accepted cell found - had " << goodNeighbours <<
                    " good neighbours among " << aNode->getInnerNodes().size() <<
                    " neighbours in total!")
          } else { currentCell.setActivationState(false); deadCells++; }
          goodNeighbours = 0;
        }//CAStep

        B2DEBUG(25, "CA: before update-step: at round " << caRound <<
                ", there are " << activeCells <<
                " cells still alive, counted " << deadCells <<
                " dead cells so far")

        /// Updatestep:
        for (auto* aNode : aNetworkContainer) {
          auto& currentCell = aNode->getMetaInfo();
          if (currentCell.isActivated() == false or currentCell.isUpgradeAllowed() == false) { continue; }

          currentCell.setStateUpgrade(false);
          B2DEBUG(50, "good cell  with former state: " << currentCell.getState() << " upgraded!")
          currentCell.increaseState();
          if (currentCell.getState() > highestCellState) { highestCellState = currentCell.getState(); }
        } // Updatestep

        B2DEBUG(25, "CA: " << caRound <<
                ". round - " << activeCells <<
                " living cells remaining. Highest state: " << highestCellState);
        if (BaseClass::m_validator.isValidRound(caRound) == false) {
          B2ERROR("Pass of CA " << BaseClass::m_log.nPasses <<
                  ": more than " << BaseClass::m_validator.nMaxIterations <<
                  " ca rounds! " << activeCells <<
                  " living cells remaining");
          BaseClass::m_log.nFails++;
          break;
        } // catch bad case

        caRound++;
      } // CA main-loop

      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 15, PACKAGENAME()) == false) { return caRound; }

      /* Debugging section:
       * for each caRound count number of cells found with stateValue == caRound, print results. */
      std::vector<unsigned int> nCellsOfState;
      unsigned int nRounds = caRound < 1 ? BaseClass::m_validator.nMaxIterations : caRound;
      nCellsOfState.assign(nRounds, 0);

      unsigned int nRound = 0;
      if (nRound < nRounds) {
        for (auto* aNode : aNetworkContainer) {
          if (aNode->getMetaInfo().getState() == nRound) { nCellsOfState.at(nRound) += 1; }
        }
      }

      // small lambda function for printing the results
      auto miniPrint = [&]() -> std::string {
        std::string out = "";
        for (unsigned int i = 0; i < nCellsOfState.size(); i++)
        { out += "had " + std::to_string(nCellsOfState[i]) + " cells of state " + std::to_string(i) + "\n"; }
        return out;
      };

      BaseClass::m_log.passResults.push_back({ caRound, nCellsOfState});   // .first int, second vector<unsigned int>
      B2DEBUG(15, "Pass " << BaseClass::m_log.nPasses <<
              " is finished with " << caRound <<
              " rounds (negative numbers indicate fail)! Of " << aNetworkContainer.size() <<
              " cells total, their states were:\n" << miniPrint())
      return caRound;
    }



    /** checks network given for seeds, returns number of seeds found*/
    unsigned int findSeeds(ContainerType& aNetworkContainer) override
    {
      unsigned int nSeeds = 0;
      for (auto* aNode : aNetworkContainer) {
        if (BaseClass::m_validator.checkSeed(aNode->getMetaInfo()) == true) { nSeeds++; }
      }
      B2DEBUG(15, "Last Pass " << BaseClass::m_log.nPasses <<
              " is finished with " << BaseClass::m_log.passResults.back().first <<
              " rounds (negative numbers indicate fail)! Of " << aNetworkContainer.size() <<
              " cells total. findSeeds() found " << nSeeds << " seeds")
      return nSeeds;
    }



    /** returns current logging info of the algorithm (some stuff one wants to log about that algorithm */
    std::string printStatistics() override { return ""; }
  };

} //Belle2 namespace
