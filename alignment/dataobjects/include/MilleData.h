#pragma once

#include <string>
#include <vector>

#include <genfit/MilleBinary.h>
#include <genfit/GblTrajectory.h>
#include <framework/pcore/Mergeable.h>

namespace Belle2 {
  /// Mergeable class holding list of so far opened mille binaries and providing the binaries
  class MilleData : public Mergeable {
  public:
    /// Constructor
    MilleData() : Mergeable() {};
    /// Destructor
    virtual ~MilleData() { close(); }

    /// Implementation of merging
    virtual void merge(const Mergeable* other);
    /// Implementation of clearing
    virtual void clear() { m_files.clear(); }

    /// Open a new file and remember it. Filename should encode also process id!
    void open(std::string filename);
    /// Is some file already open?
    bool isOpen() { return !!m_binary; }
    /// Write a GBL trajectory to the binary file
    void fill(gbl::GblTrajectory& trajectory);
    /// Close current mille binary if opened
    void close();
    /// Get the list of all created files
    const std::vector<std::string>& getFiles() const { return m_files; }
    /// Copy by assignment
    MilleData& operator=(const MilleData& other);
    /// Construct from other object
    MilleData(const MilleData& other) : m_files(other.m_files), m_binary(nullptr) {}
  private:
    std::vector<std::string> m_files = {}; /**< List of already created file names */
    /// Pointer to current binary file
    gbl::MilleBinary* m_binary = nullptr; //! Pointer to opened binary file (not streamed)

    ClassDef(MilleData, 1) /**< Mergeable list of opened mille binaries */
  };
}
