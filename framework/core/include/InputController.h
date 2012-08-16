#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H

namespace Belle2 {
  /** A static class to control supported input modules.
   *
   *  You can use setNextEntry() to request loading of any event in 0..numEntries()-1,
   *  which will be done the next time the input module's event() function is called.
   *
   *  Use canControlInput() to check wether control is actually possible.
   */
  class InputController {
  public:
    /** Is there an input module to be controlled. */
    static bool canControlInput() { return s_canControlInput; }

    /** Call this function from supported input modules. */
    static void setCanControlInput(bool on) { s_canControlInput = on; }

    /** Set the file entry to be loaded the next time event() is called.
     *
     * This is mainly useful for interactive applications (e.g. event display).
     *
     * The input module should call eventLoaded() after the entry was loaded.
     */
    static void setNextEntry(long entry) { s_nextEntry = entry; }

    /** Return entry number set via setNextEntry(). */
    static long getNextEntry() { return s_nextEntry; }

    /** Set the file entry to be loaded the next time event() is called, by evt/run/exp number.
     *
     * The input module should call eventLoaded() after the entry was loaded.
     */
    static void setNextEntry(long exp, long run, long event) { s_nextExperiment = exp; s_nextRun = run; s_nextEvent = event; }

    /** Return experiment number set via setNextEntry(). */
    static long getNextExperiment() { return s_nextExperiment; }

    /** Return run number set via setNextEntry(). */
    static long getNextRun() { return s_nextRun; }

    /** Return event number set via setNextEntry(). */
    static long getNextEvent() { return s_nextEvent; }


    /** returns the entry number currently loaded. */
    static long getCurrentEntry() { return s_currentEntry; }
    /** Returns total number of entries in the event tree.
     *
     * If no file is opened, zero is returned.
     */
    static long numEntries() { return s_numEntries; }

    /** set total number of entries in the opened file. */
    static void setNumEntries(long n) { s_numEntries = n; }

    /** Indicate that an event (in the given entry) was loaded and reset all members related to the next entry. */
    static void eventLoaded(long entry) {
      s_nextEntry = -1;
      s_nextExperiment = -1;
      s_nextRun = -1;
      s_nextEvent = -1;
      s_currentEntry = entry;
    }

  private:
    InputController() { }
    ~InputController() { }

    /** Is there an input module to be controlled? */
    static bool s_canControlInput;

    /** entry to be loaded the next time event() is called in an input module.
     *
     *  -1 indicates that execution should continue normally.
     */
    static long s_nextEntry;

    /** Experiment number to load next.
     *
     * -1 by default.
     */
    static long s_nextExperiment;

    /** Run number to load next. */
    static long s_nextRun;

    /** Event (not entry!) to load next. */
    static long s_nextEvent;


    /** current entry in file. */
    static long s_currentEntry;

    /** total number of entries in event tree, or zero if none exists. */
    static long s_numEntries;
  };
}
#endif
