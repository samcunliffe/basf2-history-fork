#pragma once

#include <framework/logging/LogMethod.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackHit;

    /**
     * This class serves as a wrapper around all things that should go into a QuadTree.
     * For usage in the QuadTree we need to provide a used flag.
     * If your typeData-class does provide a used flag itself, just spezialize this template as done for the TrackHit below.
     */
    template<class typeData>
    class QuadTreeItem {
    public:
      QuadTreeItem(typeData* data) : m_usedFlag(false), m_pointer(data) {};

      /**
       * Do not copy!
       */
      QuadTreeItem(const QuadTreeItem& copy) = delete;

      /**
       * Do not copy!
       */
      QuadTreeItem& operator=(QuadTreeItem const& copy) = delete;

      virtual ~QuadTreeItem() {}

      /**
       * Flag is set if the item was used as a result of the quad tree search and
       * should not be used in the next quad tree search round.
       * @return
       */
      virtual bool isUsed() const
      {
        return m_usedFlag;
      }

      /**
       * Set the used flag if you do not want that item to go into the next search round again.
       * @param usedFlag
       */
      virtual void setUsedFlag(bool usedFlag = true)
      {
        m_usedFlag = usedFlag;
      }

      /**
       * Unset the used flag again if you have noticed that a previous setting was wrong.
       */
      virtual void unsetUsedFlag() final {
        setUsedFlag(false);
      }

      /**
       * Returns a pointer to the underlying item. This function is not called in the quad tree item itself but only in the filling and postprocessing stage.
       */
      virtual typeData* getPointer() final {
        return m_pointer;
      }

      /**
       * Const version of getPointer() above.
       */
      virtual const typeData* const getPointer() const final
      {
        return m_pointer;
      }

    private:
      bool m_usedFlag;        /**< This flag can be set to not use the item in the next quad tree search round */
      typeData* m_pointer;    /**< A pointer to the underlying event */
    };

    /** As special case: the TrackHit has a used flag itself */
    template<>
    bool QuadTreeItem<TrackHit>::isUsed() const;

    template<>
    void QuadTreeItem<TrackHit>::setUsedFlag(bool usedFlag);

  }

}
