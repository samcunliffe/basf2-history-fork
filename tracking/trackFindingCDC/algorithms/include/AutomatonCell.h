/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/algorithms/CellWeight.h>
#include <tracking/trackFindingCDC/algorithms/CellState.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Cell used by the cellular automata.
    /** This class represents a cell in the cellular automata algorithm the local tracking is build on.
     *  Different to the ordinary cellular automaton algorihms this class stores the state not as a interger number but \n
     *  as a float value. This enables to have fractional measures of quality attached to the cell if say one new item \n
     *  is not worth a full point, but a probability measure which is additiv. The points that can be gained by picking \n
     *  this cell are stored in the cell weight property. Moreover the class defines status flags to be set and/or read by\n
     *  the cellular automaton. */
    class AutomatonCell  {

    public:
      /// Type for the additional status flags of cells in the cellular automata
      typedef unsigned CellFlags;

      /// Constant for a already updated cell
      static const CellFlags ASSIGNED = 1;

      /// Constant for a cell that marks the start of a path
      static const CellFlags START = 2;

      /// Constant marker for the detection of cycles in the cellular automata
      static const CellFlags CYCLE = 4;

      /// Constant for a cell that has been masked for the current run of the cellular automaton.
      static const CellFlags MASKED = 8;

      /// Constant for a cell that has been taken by track
      static const CellFlags TAKEN = 16;

      /// Constant for a cell that is marked as background.
      static const CellFlags BACKGROUND = 32;

      /// Constant summing all possible cell flags
      static const CellFlags ALL_FLAGS = ASSIGNED + START + CYCLE + MASKED + TAKEN + BACKGROUND;


    public:

      /// Default constructor for ROOT compatibility. Cell weight defaults to 1
      AutomatonCell() :
        m_weight(0),
        m_flags(0),
        m_state(0) {;}

      /// Constructor with a certain cell weight
      AutomatonCell(const CellState& weight) :
        m_weight(weight),
        m_flags(0),
        m_state(0) {;}

      /// Constructor with a certain cell weight and initial flags to be set.
      AutomatonCell(const CellState& weight, const CellFlags& initialFlags) :
        m_weight(weight),
        m_flags(initialFlags),
        m_state(0) {;}

      /// Empty deconstructor
      ~AutomatonCell() {;}

      /// Getter for the cell state
      const CellState& getCellState() const { return m_state; }

      /// Setter for the cell state
      void setCellState(CellState state) const { m_state = state; }

      /// Getter for the cell weight. See details
      /** The cell might carry more than one unit of information to be added to the path. \n
       *  The weight discribes an additiv constant to be gained when picking up this cell. \n
       *  For instance segments may provide their number of hits as weight. \n
       *  Or they could have a reduced number of hits deminshed by the quality of a fit to the segment. \n
       *  For on the other side wire hits this should be one. */
      const CellState& getCellWeight() const { return m_weight; }

      /// Setter for the cell weight
      void setCellWeight(CellState weight) const { m_weight = weight; }



      /// Sets the already assigned marker flag to the given value. Default value true.
      void setAssignedFlag(bool setTo = true) const
      { setFlags<ASSIGNED>(setTo); }

      /// Resets the already assigned marker flag to false.
      void unsetAssignedFlag() const
      { setFlags<ASSIGNED>(false); }

      /// Gets the current state of the already assigned marker flag.
      bool hasAssignedFlag() const
      { return hasAnyFlags(ASSIGNED); }



      /// Sets the start marker flag to the given value. Default value true.
      void setStartFlag(bool setTo = true) const
      { setFlags<START>(setTo); }

      /// Resets the start marker flag to false.
      void unsetStartFlag() const
      { setFlags<START>(false); }

      /// Gets the current state of the start marker flag.
      bool hasStartFlag() const
      { return hasAnyFlags(START); }



      /// Sets the cycle marker flag to the given value. Default value true.
      void setCycleFlag(bool setTo = true) const
      { setFlags<CYCLE>(setTo); }

      /// Resets the cycle marker flag to false.
      void unsetCycleFlag() const
      { setFlags<CYCLE>(false); }

      /// Gets the current state of the cycle marker flag.
      bool hasCycleFlag() const
      { return hasAnyFlags(CYCLE); }


      /// Resets the assigned, start and cycle marker flag.
      void unsetTemporaryFlags() const
      { setFlags < ASSIGNED + START + CYCLE > (false); }



      /// Sets the masked flag to the given value. Default value true.
      void setMaskedFlag(bool setTo = true) const
      { setFlags<MASKED>(setTo); }

      /// Resets the masked flag to false.
      void unsetMaskedFlag() const
      { setFlags<MASKED>(false); }

      /// Gets the current state of the masked marker flag.
      bool hasMaskedFlag() const
      { return hasAnyFlags(MASKED); }



      /// Sets the taken flag to the given value. Default value true.
      void setTakenFlag(bool setTo = true) const
      { setFlags<TAKEN>(setTo); }

      /// Resets the taken flag to false.
      void unsetTakenFlag() const
      { setFlags<TAKEN>(false); }

      /// Gets the current state of the taken marker flag.
      bool hasTakenFlag() const
      { return hasAnyFlags(TAKEN); }



      /// Sets the background flag to the given value. Default value true.
      void setBackgroundFlag(bool setTo = true) const
      { setFlags<BACKGROUND>(setTo); }

      /// Resets the background flag to false.
      void unsetBackgroundFlag() const
      { setFlags<BACKGROUND>(false); }

      /// Gets the current state of the do not use flag marker flag.
      bool hasBackgroundFlag() const
      { return hasAnyFlags(BACKGROUND); }



      /// Setting accessing the flag by tag.
      template<CellFlags cellFlag>
      void setFlags(bool setTo) const
      {
        if (setTo) setFlags(cellFlag);
        else clearFlags(cellFlag);
      }

    private:

      /** getter and setter for the cell flags */
      /// Getter for the additional cell flags to mark some status of the cell
      /** Give the sum of all cell flags, therefor providing a bit pattern. \n
       *  Cell flags can be \n
       *  const CellFlags ASSIGNED = 1; \n
       *  const CellFlags START = 2; \n
       *  const CellFlags CYCLE = 4; \n
       *  const CellFlags MASKED = 8; \n
       *  const CellFlags TAKEN = 16; \n
       *  const CellFlags BACKGROUND = 32; \n
       *  Use rather hasAnyFlags() to retrieve stats even for single state values.
       */
      const CellFlags& getFlags() const { return m_flags; }

      /// Setter for the cell flags
      void setFlags(CellFlags flags) const { m_flags |= flags; }

      /// Clear all flags
      void clearFlags(CellFlags flags = ALL_FLAGS) const { m_flags &= ~flags; }

      /// Checks if a cell has any of a sum of given flags.
      bool hasAnyFlags(CellFlags flags) const { return  m_flags bitand flags; }



    private:
      /// Storage for the cell weight
      mutable CellWeight m_weight;

      /// Storage for the cell status flags
      mutable CellFlags m_flags;

      /// Storage for the cell state set by the cellular automata
      mutable CellState m_state;


    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
