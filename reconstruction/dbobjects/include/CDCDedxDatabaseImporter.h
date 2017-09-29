/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  //! dE/dx database importer.
  /*!
    This module writes data from e.g. a ROOT histogram to database.
  */
  class CDCDedxDatabaseImporter {

  public:

    /**
     * Constructor
     */
    CDCDedxDatabaseImporter(std::string inputFileName, std::string m_name);

    /**
     * Destructor
     */
    virtual ~CDCDedxDatabaseImporter() {};

    /**
     * Import predicted mean parameters to the database
     */
    void importCurveParameters();

    /**
     * Import predicted resolution parameters to the database
     */
    void importSigmaParameters();

  private:

    std::vector<std::string> m_inputFileNames; /**< Name of input ROOT files */
    std::string m_name; /**< Name of database ROOT file */

    ClassDef(CDCDedxDatabaseImporter, 1); /**< ClassDef */
  };

} // Belle2 namespace
