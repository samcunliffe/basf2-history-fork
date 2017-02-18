/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 * Possible runtime improvements:
 * - use a specific allocator for the QuadTrees which can be wiped without calling
 *   each QuadTree constructor once the search is complete.
 */
#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeChildren.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <framework/logging/Logger.h>

#include <array>
#include <vector>
#include <memory>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which holds quadtree structure.
     * @tparam AX type of the X-axis variable
     * @tparam AY type of the Y-axis variable
     * @tparam AItem type of the objects which are filled into QuadTree
     */
    template<typename AX, typename AY, class AItem>
    class QuadTreeTemplate {

    public:
      /// Type of this class
      using This = QuadTreeTemplate<AX, AY, AItem>;

      /// Type for a span in the X direction that is covered by the tree
      using XSpan = std::array<AX, 2>;

      /// Type for a span in the Y direction that is covered by the tree
      using YSpan = std::array<AY, 2>;

      /// Type to store the minimum, center and maximum of the bins in X direction
      using XBinBounds = std::array<AX, 3>;

      /// Type to store the minimum, center and maximum of the bins in Y direction
      using YBinBounds = std::array<AY, 3>;

      /// Type of the child node structure for this node.
      using Children = QuadTreeChildrenTemplate<This>;

      /**
       *  Constructor setting up the potential division points.
       *
       *  Cppcheck may warn to pass the spans by reference here,
       *  however this would come at performance penalty.
       *  If somebody knows the suppression category please apply it here.
       */
      QuadTreeTemplate(XSpan xSpan, YSpan ySpan, int level, This* parent)
        : m_xBinBounds( {xSpan[0], xSpan[0] + (xSpan[1] - xSpan[0]) / 2, xSpan[1]})
      , m_yBinBounds({ySpan[0], ySpan[0] + (ySpan[1] - ySpan[0]) / 2, ySpan[1]})
      , m_level(level)
      , m_parent(level > 0 ? parent : nullptr)
      , m_filled(false)
      {
        // ensure the level value fits into unsigned char
        B2ASSERT("QuadTree datastructure only supports levels < 255", level < 255);
      }

      /** Insert item into node */
      void insertItem(AItem* item)
      {
        m_items.push_back(item);
      };

      /** Reserve memory for holding items */
      void reserveItems(int nItems)
      {
        m_items.reserve(nItems);
      };

      /** Get items from node */
      std::vector<AItem*>& getItems()
      {
        return m_items;
      };

      /** Check if the node passes threshold on number of hits */
      size_t getNItems() const
      {
        return m_items.size();
      };

      /** Clear items which the node holds */
      void clearItems()
      {
        m_items.clear();
      };

      /** Create vector with children of current node */
      void createChildren()
      {
        m_children = makeUnique<Children>();
      }

      /** Returns the children structure of this node */
      Children* getChildren() const
      {
        return m_children.get();
      };

      /**
       *  Clear items which the node holds and destroy all children below this node.
       *  This method must only be called on the root node, for fast QuadTreeTemplate reusage
       */
      void clearChildren()
      {
        // can only be called on root item
        B2ASSERT("ClearTree can only be called on tree of level one", m_level == 0);

        // automatically removes all underlying objects
        m_children.reset(nullptr);
        m_filled = false;
      };

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      unsigned char getLevel() const
      {
        return m_level;
      };

      /** Check whether node has been processed, i.e. children nodes has been filled */
      bool checkFilled() const
      {
        return m_filled;
      };

      /** Set status of node to "filled" (children nodes has been filled) */
      void setFilled()
      {
        m_filled = true;
      };

      /** Return pointer to the parent of the node */
      This* getParent() const
      {
        return m_parent;
      };

      /** Get number of bins in "Theta" direction */
      constexpr int getXNbins() const
      {
        return m_xBinBounds.size() - 1;
      }

      /** Get minimal "Theta" value of the node */
      AX getXMin() const
      {
        return m_xBinBounds[0];
      };

      /** Get mean value of "Theta" */
      AX getXMean() const
      {
        return m_xBinBounds[1];
      };

      /** Get maximal "Theta" value of the node */
      AX getXMax() const
      {
        return m_xBinBounds[2];
      };

      /** Getter for the width of the iBin bin in "Theta" direction */
      AX getXBinWidth(int iBin)
      {
        return std::abs(getXBinBound(iBin + 1) - getXBinBound(iBin));
      }

      /** Get "Theta" value of given bin border */
      AX getXBinBound(int iBin) const
      {
        return m_xBinBounds[iBin];
      };

      /** Get number of bins in "r" direction */
      constexpr int getYNbins() const
      {
        return m_yBinBounds.size() - 1;
      }

      /** Get minimal "r" value of the node */
      AY getYMin() const
      {
        return m_yBinBounds[0];
      };

      /** Get mean value of "r" */
      AY getYMean() const
      {
        return m_yBinBounds[1];
      };

      /** Get maximal "r" value of the node */
      AY getYMax() const
      {
        return m_yBinBounds[2];
      };

      /** Getter for the width of the iBin bin in "r" direction */
      AY getYBinWidth(int iBin)
      {
        return std::abs(getYBinBound(iBin + 1) - getYBinBound(iBin));
      }

      /** Get "r" value of given bin border */
      AY getYBinBound(int iBin) const
      {
        return m_yBinBounds[iBin];
      };

    private:
      /// bins range on r
      XBinBounds m_xBinBounds;

      /// bins range on theta
      YBinBounds m_yBinBounds;

      /// Level of node in the tree
      unsigned char m_level;

      /// Pointer to the parent node
      This* m_parent;

      /// Vector of items which belongs to the node
      std::vector<AItem*> m_items;

      /// Pointers to the children nodes
      std::unique_ptr<Children> m_children;

      /// Is the node has been filled with items
      bool m_filled;
    };
  }
}
