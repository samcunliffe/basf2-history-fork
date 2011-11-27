/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* This is modified version of the genFitterModule to also use reco hits form
 * pxd and svd at a hopfully not so far away point in the future this module
 * will be merged with genFitterModule */


#ifndef GENFITTER2MODULE_H
#define GENFITTER2MODULE_H

#include <framework/core/Module.h>

#include <fstream>


namespace Belle2 {


  class GenFitter2Module : public Module {

  public:

    /** Constructor .
     */
    GenFitter2Module();

    /** Destructor.
     */
    virtual ~GenFitter2Module();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();

  protected:


  private:
    std::ofstream dataOut;
    int m_failedFitCounter;
    int m_fitCounter;
    int m_notPerfectCounter;
    double m_blowUpFactor;
    bool m_filter;
    bool m_useDaf;
    int m_nGFIter;


  };
} // end namespace Belle2


#endif /* GENFITTER2MODULE_H */
