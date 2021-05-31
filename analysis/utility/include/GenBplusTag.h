/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {

  /**
   * Class to determine generated decay modes of B+ and B-
   */
  class GenBplusTag {

  public:

    /**
     * Constructor
     */
    GenBplusTag();

    int Mode_B_plus(std::vector<int> genDAU); /**< returns B+ mode identifier */
    int Mode_B_minus(std::vector<int> genDAU); /**< returns B- mode identifier */

  private:
    int m_nPhotos; /**< number of photons */
    /** check decay with two daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2);
    /** check decay with three daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3);
    /** check decay with four daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4);
    /** check decay with five daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5);
    /** check decay with six daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6);
    /** check decay with seven daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7);
    /** check decay with eight daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7, int da8);
  };

} //End of Belle2 namespace