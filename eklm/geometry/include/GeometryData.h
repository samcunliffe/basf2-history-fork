/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMGEOMETRYDATA_H
#define EKLMGEOMETRYDATA_H

/* Belle2 headers. */
#include <eklm/geometry/TransformData.h>
#include <framework/core/Environment.h>

/**
 * @file
 * EKLM geometry data.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * Geometry data.
     */
    class GeometryData {

    public:

      /**
       * Create geometry data and write it to file.
       * @param[in] file  Name of file.
       * @return 0    Successful.
       * @return != 0 Error.
       */
      int save(const char* file);

      /**
       * Read geometry data from file.
       * @param[in] file  Name of file.
       * @return 0    Successful.
       * @return != 0 Error.
       */
      int read(
        const char* file =
          std::string(Environment::Instance().getDataSearchPath() +
                      "/eklm/eklm_alignment.dat").c_str());

      /**
       * Get strip length.
       * @param[in] strip Number of strip (numbers start from 1).
       * @return Strip length (cm).
       */
      double getStripLength(int strip);

      /* Transformations. */
      struct TransformData transf;

    private:

      /* Strip lengths. */
      double m_stripLen[75];

      /* Intersection data. */
      bool m_isect[75 * 74 / 2];

    };

  }

}

#endif

