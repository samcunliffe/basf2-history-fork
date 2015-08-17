/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/WeightedFastHoughTree.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>

#include <tracking/trackFindingCDC/utilities/EvalVariadic.h>
#include <tracking/trackFindingCDC/utilities/GenIndices.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <tuple>
#include <array>
#include <memory>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A fast hough algorithm with rectangular boxes, which are split linearly by a fixed number of
     *  divisions in each coordinate up to a maximal level. */
    template<class ItemPtr,
             class HoughBox,
             size_t ... divisions>
    class BoxDivisionHoughTree {

    public:
      /// Type of the box division strategy
      using BoxDivision = LinearDivision<HoughBox, divisions...>;

      /// Type of the fast hough tree structure
      using HoughTree = WeightedFastHoughTree<ItemPtr, HoughBox, BoxDivision>;

      /// Type of the coordinate I.
      template<size_t I>
      using Type = typename HoughBox::template Type<I>;

      /// Predicate that the given type is indeed a coordinate of the hough space
      template<class T>
      using HasType = typename HoughBox::template HasType<T>;

      /// Function to get the coordinate index from its type
      template<class T>
      using TypeIndex = typename HoughBox::template TypeIndex<T>;

      /// Type of the width in coordinate I.
      template<size_t I>
      using Width = typename HoughBox::template Width<I>;

      /// Type of the nodes used in the tree for the search.
      using Node = typename  HoughTree::Node;

    public:
      /// Constructor using the given maximal level.
      template<class ... RangeSpecOverlap>
      BoxDivisionHoughTree(size_t maxLevel) :
        m_maxLevel(maxLevel),
        m_overlaps((divisions * 0) ...)
      {;}

    private:
      /// Type of the discrete value array to coordinate index I.
      template<size_t I>
      using Array = typename Type<I>::Array;

      /// Tuple type of the discrete value arrays
      using Arrays = MapGenIndices<Array, sizeof...(divisions)>;

    public:
      /// Getter the number of divisions at each level for coordinate index I.
      size_t getDivision(const size_t i) const
      { return m_divisions[i]; }

      /** Construct the discrete value array at coordinate index I
       *  @param lowerBound  Lower bound of the value range
       *  @param upperBound  Upper bound of the value range
       *  @param overlap     Overlap of neighboring bins. Default is no overlap.
       *                     Usuallly this is counted in number of discrete values
       *  @param width       Width of the bins at lowest level. Default is width of 1.
       *                     Usually this is counted in numbers of discrete values
       */
      template<size_t I>
      void constructArray(const double& lowerBound,
                          const double& upperBound,
                          const Width<I>& overlap = 0,
                          Width<I> width = 0)
      {
        const size_t division = getDivision(I);
        const size_t nBins = std::pow(division, m_maxLevel);

        if (width == 0) {
          width = overlap + 1;
        }
        B2ASSERT("Width " << width << "is not bigger than overlap " << overlap, overlap < width);

        const auto nPositions = (width - overlap) * nBins + overlap + 1;
        std::get<I>(m_arrays) = Array<I>(lowerBound, upperBound, nPositions);
        std::get<I>(m_overlaps) = overlap;

        const Array<I>& array  = std::get<I>(m_arrays);

        B2INFO("Constructing array for coordinate " << I);
        B2INFO("Lower value " << array.front());
        B2INFO("Upper value " << array.back());
      }

      /// Provide an externally constructed array by coordinate index
      template<size_t I>
      void
      assignArray(Array<I> array, Width<I> overlap)
      {
        // Double move assignment idiom
        std::get<I>(m_arrays) = std::move(array);
        std::get<I>(m_overlaps) = overlap;
      }

      /// Provide an externally constructed array by coordinate type
      template<class T>
      EnableIf< HasType<T>::value, void>
      assignArray(Array<TypeIndex<T>::value > array, Width<TypeIndex<T>::value > overlap)
      {
        // Double move assignment idiom
        // Double move assignment idiom
        std::get<TypeIndex<T>::value >(m_arrays) = std::move(array);
        std::get<TypeIndex<T>::value >(m_overlaps) = overlap;

      }

    public:
      /// Initialise the algorithm by constructing the hough tree from the parameters
      virtual void initialize()
      {
        // Compose the hough space
        HoughBox houghPlane = constructHoughPlane();
        BoxDivision boxDivision(m_overlaps);
        m_houghTree.reset(new HoughTree(houghPlane, boxDivision));
      }

      /// Prepare the leave finding by filling the top node with given hits
      template<class ItemPtrs>
      void seed(ItemPtrs& items)
      {
        if (not m_houghTree) { initialize(); }
        m_houghTree->seed(items);
      }

      /// Terminates the processing by striping all hit information from the tree
      virtual void fell()
      { m_houghTree->fell(); }

      /// Release all memory that the tree aquired during the runs.
      virtual void raze()
      { m_houghTree->raze(); }

    public:
      /// Getter for the tree used in the search in the hough plane.
      HoughTree* getTree() const
      { return m_houghTree.get(); }

      /// Getter for the currently set maximal level
      size_t getMaxLevel() const
      { return m_maxLevel; }

      /// Getter for the array of discrete value for coordinate I.
      template<size_t I>
      const Array<I>& getArray() const
      { return std::get<I>(m_arrays); }

    private:
      /// Construct the box of the top node of the tree. Implementation unroling the indices.
      template<size_t ... Is>
      HoughBox constructHoughPlaneImpl(const IndexSequence<Is...>&)
      { return HoughBox(Type<Is>::getRange(std::get<Is>(m_arrays))...); }

      /// Construct the box of the top node of the tree.
      HoughBox constructHoughPlane()
      { return constructHoughPlaneImpl(GenIndices<sizeof ...(divisions)>()); }

    private:
      /// Number of the maximum tree level.
      size_t m_maxLevel;

      /// Array of the number of divisions at each level
      const std::array<size_t, sizeof ...(divisions)> m_divisions{{divisions ...}};

      /// An tuple of division overlaps in each coordinate.
      typename HoughBox::Delta m_overlaps;

      /// A tuple of value arrays providing the memory for the discrete bin bounds.
      Arrays m_arrays;

      /// Dynamic hough tree structure traversed in the leaf search.
      std::unique_ptr<HoughTree> m_houghTree{nullptr};
    };
  }
}
