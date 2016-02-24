//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : PeakFinder.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find peaks in Hough Plane
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCPeakFinder_FLAG_
#define TRGCDCPeakFinder_FLAG_

#include <string>
#include <vector>

#ifdef TRGCDC_SHORT_NAMES
#define TCPeakFinder TRGCDCPeakFinder
#endif

namespace Belle2 {

  class TRGCDCCircle;
  class TRGCDCHoughPlaneMulti2;

/// A class to find peaks in Hough Plane
  class TRGCDCPeakFinder {

  public:

    /// Contructor.
    TRGCDCPeakFinder(const std::string& name);

    /// Destructor
    virtual ~TRGCDCPeakFinder();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// do peak finding. This is a copy from "trasan".
    void peaks6(TRGCDCHoughPlaneMulti2& hp,
                const unsigned threshold,
                const bool centerIsPeak,
                std::vector<unsigned>& peakSerialIds) const;

    /// do peak finding. Kaiyu's version.
    void peaks7(TRGCDCHoughPlaneMulti2& hp,
                const unsigned threshold,
                std::vector<std::vector<unsigned>>& peaks) const;

  private:

    /// Makes regions.
    void regions(TRGCDCHoughPlane& hp, const unsigned threshold) const;

    /// Kaiyu's logic.
    void p1p2Methode(TRGCDCHoughPlane& hp,
                     const unsigned threshold,
                     std::vector<std::vector<unsigned>>& peaks) const;

  private:

    /// Name
    const std::string _name;
  };

//-----------------------------------------------------------------------------

  inline
  std::string
  TRGCDCPeakFinder::name(void) const
  {
    return _name;
  }

} // namespace Belle2

#endif
