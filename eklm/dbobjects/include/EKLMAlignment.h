/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENT_H
#define EKLMALIGNMENT_H

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TObject.h>

/* Belle2 Headers. */
#include <eklm/dbobjects/EKLMAlignmentData.h>

namespace Belle2 {

  /**
   * Class to store EKLM alignment data in the database.
   */
  class EKLMAlignment : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMAlignment();

    /**
     * Destructor.
     */
    ~EKLMAlignment();

    /**
     * Set alignment data.
     * @param[in] segment Segment number.
     * @param[in] dat     Alignment data.
     */
    void setAlignmentData(uint16_t segment, EKLMAlignmentData* dat);

    /**
     * Get alignment data.
     * @param[in] segment Segment number.
     */
    EKLMAlignmentData* getAlignmentData(uint16_t segment);

    /**
     * Clean alignment data.
     */
    void cleanAlignmentData();

    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 40;}
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short param)
    {

      auto adata = getAlignmentData(element);
      if (!adata)
        return 0.;
      if (param == 1) return adata->getDx();
      if (param == 2) return adata->getDy();
      if (param == 6) return adata->getDalpha();

      return 0.;
    }
    /// Set global parameter
    void setGlobalParam(double value, unsigned short element, unsigned short param)
    {

      auto adata = getAlignmentData(element);
      if (!adata)
        return;
      if (param == 1) adata->setDx(value);
      if (param == 2) adata->setDy(value);
      if (param == 6) adata->setDalpha(value);

    }
    /// TODO: list stored global parameters
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams()
    {
      std::vector<std::pair<unsigned short, unsigned short>> result;
      for (auto data : m_Data) {
        result.push_back({data.first, 1});
        result.push_back({data.first, 2});
        result.push_back({data.first, 6});
      }
      return result;
    }
    /// Not used
    void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>&) {}
    // ------------------------------------------------------------------------

  private:

    /** Alignment data. */
    std::map<uint16_t, EKLMAlignmentData> m_Data;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMAlignment, 1);

  };

}

#endif

