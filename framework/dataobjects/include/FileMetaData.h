/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef FILEMETADATA_H
#define FILEMETADATA_H

#include <TObject.h>

#include <iosfwd>
#include <vector>
#include <string>

namespace Belle2 {

  /** Metadata information about a file
   */
  class FileMetaData : public TObject {
  public:


    /** Constructor.
     */
    FileMetaData();

    /** Logical file name getter.
     */
    std::string getLfn() const {return m_lfn;}

    /** Number of events getter.
     */
    int getNEvents() const {return m_nEvents;}

    /** Lowest experiment number getter.
     */
    int getExperimentLow() const {return m_experimentLow;}

    /** Lowest run number getter.
     */
    int getRunLow() const {return m_runLow;}

    /** Lowest event number in lowest run getter.
     */
    unsigned int getEventLow() const {return m_eventLow;}

    /** Highest experiment number getter.
     */
    int getExperimentHigh() const {return m_experimentHigh;}

    /** Highest run number getter.
     */
    int getRunHigh() const {return m_runHigh;}

    /** Highest event number in highest run getter.
     */
    unsigned int getEventHigh() const {return m_eventHigh;}

    /** Check whether the given event is in the covered range of events.
     *
     *  @param experiment The experiment number of the event.
     *  @param run The run number of the event.
     *  @param event The event number of the event.
     */
    bool containsEvent(int experiment, int run, unsigned int event) const;

    /** Get number of parent files.
     */
    int getNParents() const {return m_parentLfns.size();}

    /** Get LFN of parent file.
     *
     *  @param iParent The number of the parent file.
     */
    std::string getParent(int iParent) const {return m_parentLfns[iParent];}

    /** File creation date and time getter.
     */
    std::string getDate() const {return m_date;}

    /** Site where the file was created getter.
     */
    std::string getSite() const {return m_site;}

    /** User who created the file getter.
     */
    std::string getUser() const {return m_user;}

    /** Random seed getter.
     */
    std::string getRandomSeed() const {return m_randomSeed;}

    /** Software release version getter.
     */
    std::string getRelease() const {return m_release;}

    /** Steering file content getter.
     */
    std::string getSteering() const {return m_steering;}

    /** Number of geerated events getter.
     */
    int getMcEvents() const {return m_mcEvents;}

    /** Setter for LFN.
      *
      *  @param lfn The logical file name.
      */
    void setLfn(const std::string& lfn) {m_lfn = lfn;}

    /** Number of events setter.
     *
     *  @param nEvents The number of events.
     */
    void setNEvents(int nEvents) {m_nEvents = nEvents;}

    /** Lowest experiment, run and event number setter.
     *
     *  @param experiment The lowest experiment number.
     *  @param run The lowest run number.
     *  @param event The lowest event number of the lowest run.
     */
    void setLow(int experiment, int run, unsigned int event) {m_experimentLow = experiment; m_runLow = run; m_eventLow = event;}

    /** Highest experiment, run and event number setter.
     *
     *  @param experiment The highest experiment number.
     *  @param run The highest run number.
     *  @param event The highest event number of the highest run.
     */
    void setHigh(int experiment, int run, unsigned int event) {m_experimentHigh = experiment; m_runHigh = run; m_eventHigh = event;}

    /** Parents setter.
     *
     *  @param parents The vector of parent IDs.
     */
    void setParents(const std::vector<std::string>& parents) {m_parentLfns = parents;}

    /** Creation data setter.
     *
     *  @param date The creation date and time.
     *  @param site The site where the file was created.
     *  @param user The user who created the file.
     *  @param release The software release.
     */
    void setCreationData(const std::string& date, const std::string& site, const std::string& user, const std::string& release)
    {m_date = date; m_site = site; m_user = user; m_release = release;}

    /** Random seed setter.
     *
     *  @param seed The random seed.
     */
    void setRandomSeed(const std::string& seed) {m_randomSeed = seed;}

    /** Steering file content setter.
     *
     *  @param steering The content of the steering file.
     */
    void setSteering(const std::string& steering) {m_steering = steering;}

    /** Number of generated events setter.
     *
     *  @param nEvents The number of generated events.
     */
    void setMcEvents(int nEvents) {m_mcEvents = nEvents;}

    /**
     * Exposes methods of the FileMateData class to Python.
     */
    static void exposePythonAPI();

    /** Print the content of the meta data object.
     *
     *  @param option Use "all" to print everything, except steering file. Use "steering" for printing steering file.
     */
    virtual void Print(Option_t* option = "") const;

    /** Read file meta data in xml format from the input stream.
     *
     *  @param input The input stream.
     *  @param physicalFileName The physical file name. Will be set on return.
     *  @return True if metadata could be read
     */
    bool read(std::istream& input, std::string& physicalFileName);

    /** Write file meta data in xml format to the output stream.
     *
     *  @param output The output stream.
     *  @param physicalFileName The physical file name.
     *  @return True if metadata could be written
     */
    bool write(std::ostream& output, std::string physicalFileName);

  private:

    std::string m_lfn; /**< Logical file name.  */

    int m_nEvents; /**< Number of events.  */

    int m_experimentLow; /**< Lowest experiment number.  */

    int m_runLow; /**< Lowest run number.  */

    unsigned int m_eventLow; /**< Lowest event number in lowest run.  */

    int m_experimentHigh; /**< Highest experiment number.  */

    int m_runHigh; /**< Highest run number.  */

    unsigned int m_eventHigh; /**< Highest event number in highest run.  */

    std::vector<std::string> m_parentLfns; /**< LFNs of parent files.  */

    std::string m_date; /**< File creation date and time.  */

    std::string m_site; /**< Site where the file was created.  */

    std::string m_user; /**< User who created the file.  */

    std::string m_randomSeed; /**< The random seed used when producing the file */

    std::string m_release; /**< Software release version.  */

    std::string m_steering; /**< The steering file content.  */

    int m_mcEvents; /**< Number of generated events, 0 for real data.  */

    ClassDef(FileMetaData, 6); /**< Metadata information about a file. */

  }; //class

} // namespace Belle2
#endif // FILEMETADATA
