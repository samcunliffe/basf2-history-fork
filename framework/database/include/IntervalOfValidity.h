/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>


namespace Belle2 {
  class EventMetaData;

  /**
   * A class that describes the interval of experiments/runs
   * for which an object in the database is valid.
   */
  class IntervalOfValidity {
  public:

    /**
     * Constructor. By default an empty validity interval is created where low and high experiment numbers are -1.
     * @param experimentLow   lowest experiment number of the validity range, -1 means no bound
     * @param runLow          lowest run number in the experiment with number experimentLow of the validity range, -1 means no bound
     * @param experimentHigh  highest experiment number of the validity range, -1 means no bound
     * @param runHigh         highest run number in the experiment with number experimentHigh of the validity range, -1 means no bound
     */
    IntervalOfValidity(int experimentLow = -1, int runLow = -1, int experimentHigh = -1, int runHigh = -1);

    /**
     * Destructor.
     */
    virtual ~IntervalOfValidity() {};

    /**
     * Function that checks whether the event is inside the validity interval.
     * @param event   event meta data to be checked, if the run number 0 it's checked whether the whole experiment is inside the validity interval
     * @return             true if the given event is inside the validity interval.
     */
    bool contains(const EventMetaData& event) const;

    /**
     * Function that checks whether the validity interval is empty.
     * @return             true if the validity interval is empty.
     */
    bool empty() const {return ((m_experimentLow < 0) && (m_experimentHigh < 0));};

    /**
     * Check whether two intervals of validity are identical.
     */
    virtual bool operator==(const IntervalOfValidity& other)
    {
      return (m_experimentLow == other.m_experimentLow) && (m_runLow == other.m_runLow) &&
             (m_experimentHigh == other.m_experimentHigh) && (m_runHigh == other.m_runHigh);
    }

    /**
     * Check whether two intervals of validity are different.
     */
    virtual bool operator!=(const IntervalOfValidity& other)
    {
      return !(*this == other);
    }

    /** Input stream operator for reading IoV data from a text file.
     *
     *  @param input The input stream.
     *  @param metaData The InervalOfValidity object.
     */
    friend std::istream& operator>> (std::istream& input, IntervalOfValidity& iov);

    /** Output stream operator for writing IoV data to a text file.
     *
     *  @param output The output stream.
     *  @param metaData The IntervalOfValidity object.
     */
    friend std::ostream& operator<< (std::ostream& output, const IntervalOfValidity& iov);

  private:

    /** Lowest experiment number. 0 means no bound.
     */
    int m_experimentLow;

    /** Lowest run number. 0 means no bound.
     */
    int m_runLow;

    /** Highest experiment number. 0 means no bound.
     */
    int m_experimentHigh;

    /** Highest run number. 0 means no bound.
     */
    int m_runHigh;

    ClassDef(IntervalOfValidity, 2);  /**< describes the interval of experiments/runs for which an object in the database is valid. */
  };
}
