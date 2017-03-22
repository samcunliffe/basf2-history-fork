/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/ca/CellularAutomaton.h>
#include <tracking/trackFindingCDC/ca/CellularPathFollower.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/ca/Path.h>

#include <framework/logging/Logger.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     *  Class to combine the run of the cellular automaton and the repeated path extraction.
     *  Execute the cellular automaton and extracting paths interatively blocking the already used
     *  knots until there is no more path fullfilling the minimal length / energy requirement given
     *  as minStateToFollow to the constructor.
     */
    template<class ACellHolder>
    class  MultipassCellularPathFinder {

    public:
      /// Constructor setting up the minimal length / energy cut off.
      explicit MultipassCellularPathFinder(Weight minStateToFollow = -std::numeric_limits<Weight>::infinity())
        : m_minStateToFollow(minStateToFollow)
      {}

      /// Applies the cellular automaton to the collection and its neighborhood
      template<class ACellHolderRange>
      void apply(const ACellHolderRange& cellHolders,
                 const WeightedNeighborhood<ACellHolder>& cellHolderNeighborhood,
                 std::vector<Path<ACellHolder> >& paths) const
      {

        // multiple passes of the cellular automat
        // one segment is created at a time denying all knots it picked up,
        // applying the cellular automaton again
        // and so on
        // no best candidate analysis needed
        // (only makes sense with minimal clusters to avoid evaluating of uncommon paths)

        for (ACellHolder& cellHolder : cellHolders) {
          cellHolder.unsetAndForwardMaskedFlag();
        }

        B2DEBUG(100, "Apply multipass cellular automat");
        do {
          m_cellularAutomaton.applyTo(cellHolders, cellHolderNeighborhood);

          auto lessStartCellState = [this](ACellHolder & lhs, ACellHolder & rhs) {
            AutomatonCell& lhsCell = lhs.getAutomatonCell();
            AutomatonCell& rhsCell = rhs.getAutomatonCell();

            // Cells with state lower than the minimal cell state are one lowest category
            if (rhsCell.getCellState() < m_minStateToFollow) return false;
            if (lhsCell.getCellState() < m_minStateToFollow) return true;

            return (std::make_tuple(lhsCell.hasPriorityPathFlag(),
                                    lhsCell.hasStartFlag(),
                                    lhsCell.getCellState()) <
                    std::make_tuple(rhsCell.hasPriorityPathFlag(),
                                    rhsCell.hasStartFlag(),
                                    rhsCell.getCellState()));
          };

          auto itStartCellHolder =
            std::max_element(cellHolders.begin(), cellHolders.end(), lessStartCellState);
          if (itStartCellHolder == cellHolders.end()) break;
          else if (not itStartCellHolder->getAutomatonCell().hasStartFlag()) break;
          else if (itStartCellHolder->getAutomatonCell().getCellState() < m_minStateToFollow) break;

          const ACellHolder* highestCellHolder = &*itStartCellHolder;

          Path<ACellHolder> newPath = m_cellularPathFollower.followSingle(highestCellHolder,
                                      cellHolderNeighborhood,
                                      m_minStateToFollow);

          if (newPath.empty()) break;

          // Block the used items
          for (ACellHolder* cellHolderPtr : newPath) {
            cellHolderPtr->setAndForwardMaskedFlag();
          }

          // Block the items that have already used components
          for (ACellHolder& cellHolder : cellHolders) {
            cellHolder.receiveMaskedFlag();
          }

          paths.push_back(std::move(newPath));

        } while (true);

      }

    private:
      /// The minimal path length / energy to be followed.
      Weight m_minStateToFollow;

      /// The cellular automaton to be used.
      CellularAutomaton<ACellHolder> m_cellularAutomaton;

      /// The path follower used to extract the path from the graph processed by the cellular automaton.
      CellularPathFollower<ACellHolder> m_cellularPathFollower;
    };
  }
}
