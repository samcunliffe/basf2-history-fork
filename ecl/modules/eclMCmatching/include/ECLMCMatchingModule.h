/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ECLMCMATCHINGMODULE_H_
#define ECLMCMATCHINGMODULE_H_

//basf2 framework headers
#include <framework/core/Module.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLSimHit.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

//ROOT
#include <TRandom3.h>


namespace Belle2 {
  namespace ECL {

    /** Class to represent the hit of one cell */
    class ECLMCMatchingModule : public Module {

    public:
      /** Constructor.*/
      ECLMCMatchingModule();

      /** Destructor.*/
      virtual ~ECLMCMatchingModule();

      /** Initialize variables, print info, and start CPU clock. */
      virtual void initialize();

      /** Nothing so far.*/
      virtual void beginRun();

      /** Actual digitization of all hits in the ECL.
       *
       *  The digitized hits are written into the DataStore.
       */
      virtual void event();

      /** Nothing so far. */
      virtual void endRun();

      /** Stopping of CPU clock.*/
      virtual void terminate();


      typedef std::map< int, int>  PrimaryTrackMap;
      typedef std::multimap< int, int> HitShowerMap;

    protected:
      /** Input array name. */
      std::string m_inColName;
      /** Name of input Hit collection of this module.
        */
      std::string m_eclHitOutColName;
      /** Name of input DigiHit collection of this module.
        */
      std::string m_eclDigiCollectionName;
      /** Name of input collection of this module */
      std::string m_ECLShowerName;
      /**  Name of collection of ECLHitAssignment.*/
      std::string m_eclHitAssignmentName;


    private:

      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;

    };
  } //ECL
} // end of Belle2 namespace

#endif // ECLDIGI_H
