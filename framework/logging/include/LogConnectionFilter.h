/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGCONNECTIONFILTER_H_
#define LOGCONNECTIONFILTER_H_

#include <framework/logging/LogConnectionBase.h>


namespace Belle2 {

  /**
   * Implements a log connection that filters repeated messages.
   */
  class LogConnectionFilter : public LogConnectionBase {

  public:

    /**
     * The constructor.
     *
     * @param logConnection The log connection that actually prints the messages.
     */
    LogConnectionFilter(LogConnectionBase* logConnection);

    /**
     * The LogConnectionFilter destructor.
     */
    virtual ~LogConnectionFilter();

    /**
     * Sends a log message.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    virtual bool sendMessage(LogMessage message);

    /**
     * Returns true if the log connection could be established.
     *
     * @return True if the log connection could be established.
     */
    virtual bool isConnected();

  protected:

  private:

    /**
     * If there are repeated messages print the number of repetitions.
     */
    void printRepetitions();

    LogConnectionBase* m_logConnection; /**< The actual log connection.*/
    LogMessage m_previousMessage;       /**< The previous log message. */
    int m_repeatCounter;                /**< Counter for repeated messages. */

  };

} // end namespace Belle2

#endif /* LOGCONNECTIONFILTER_H_ */
