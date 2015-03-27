/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "NamedFloatTuple.h"

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Const expression that can compare two string at compile time
     *
     *  @param s1    One string
     *  @param s2    Other string
     *  @return      True for equal strings.
     */
    constexpr
    bool strequal(char const* s1, char const* s2)
    {
      return (not * s1 and not * s2) or (*s1 and * s2 and * s1 == *s2 and strequal(s1 + 1, s2 + 1));
    }

    /** Const expression that can search through an array of string at compile time and find the
     *  index of the string.
     *
     *  @param names   An array of cstrings
     *  @param name    The sought string
     *  @param iName   Optional index at which the search should begin.
     *  @return        The index at which the string was found.
     *                 nNames if not found, which points to one after the array.
     */
    template<size_t nNames>
    constexpr
    int index(const char* const(&names)[nNames],
              const char* const name,
              const size_t iName = 0)
    {
      return ((nNames == iName) ?
              iName :
              strequal(names[iName], name) ? iName : index(names, name, iName + 1));
    }

    /** Generic class that contains a fixed number of named float values.
     *  This object template provides the memory and the names of the float values.
     *
     *  As a template parameter it takes class with two containing parameters.
     *  nNames - Number of parts that will be peeled from the complex object.
     *  names - Array of names which contain the nNames names of the float values.
     **/
    template<class Names>
    class FixedSizeNamedFloatTuple : public NamedFloatTuple {

    private:
      /// Number of floating point values represented by this class.
      static const size_t nNames = Names::nNames;

    protected:
      /** Static getter for the index from the name.
       *  Looks through the associated names and returns the right index if found
       *  Returns nNames (one after the last element) if not found.
       *
       *  Short hand named spells nice in implementation code.
       *
       *  @param name   The sough name.
       *  @return       Index of the name, nNames if not found.
       */
      IF_NOT_CINT(constexpr)
      static int named(const char* const name)
      {
        return index(Names::names, name);
      }

    public:
      /// Constructure taking a optional prefix that can be attached to the names if request.
      FixedSizeNamedFloatTuple(const std::string& prefix = "") :
        NamedFloatTuple(prefix)
      {;}

      /// Getter for number of floating point values represented by this class.
      virtual size_t size() const override final
      {
        return Names::nNames;
      }

      /** Getter for the index from a name.
       *  Looks through the associated names and returns the right index if found.
       *  Returns size() (one after the last element) if not found.
       *
       *  @param partName   Name of the sought part
       *  @return           Index of the name, nParts if not found.
       */
      virtual int getNameIndex(const char* const name) const override final
      {
        return named(name);
      }

      /// Getter for the ith name.
      virtual std::string getName(int iValue) const override final
      {
        assert(iValue < (int)nNames);
        assert(iValue >= 0);
        return Names::names[iValue];
      }

    public:
      /// Setter for the ith values. Static index version.
      template<int iValue>
      void set(Float_t value)
      {
        IF_NOT_CINT(static_assert(nNames != iValue, "Requested name not found in names.");)
        assert(iValue < (int)nNames);
        assert(iValue >= 0);
        m_values[iValue] = value;
      }

      /// Setter for the ith value.
      virtual void set(int iValue, Float_t value) override final
      {
        assert(iValue < (int)nNames);
        assert(iValue >= 0);
        m_values[iValue] = value;
      }

      /// Setter for the value with the given name.
      void set(const char* const name, Float_t value)
      {
        set(named(name), value);
      }

      /// Getter for the ith value. Static index version.
      template<int iValue>
      Float_t get() const
      {
        IF_NOT_CINT(static_assert(nNames != iValue,
                                  "Requested name not found in names.");)
        assert(iValue < (int)nNames);
        assert(iValue >= 0);
        return m_values[iValue];
      }

      /// Getter for the ith value.
      virtual Float_t get(int iValue) const override final
      {
        assert(iValue < (int)nNames);
        assert(iValue >= 0);
        return m_values[iValue];
      }

      /// Getter for the value with the given name.
      Float_t get(const char* const name) const
      {
        return get(named(name));
      }

      /// Reference getter for the ith value. Static index version.
      template<int iValue>
      Float_t& value()
      {
        IF_NOT_CINT(static_assert(nNames != iValue,
                                  "Requested name not found in names.");)
        assert(iValue < (int)nNames);
        assert(iValue >= 0);
        return m_values[iValue];
      }

      /// Reference getter for the ith value.
      virtual Float_t& operator[](int iValue) override final
      {
        assert(iValue < (int)nNames);
        assert(iValue >= 0);
        return m_values[iValue];
      }

      /// Reference getter for the value with the given name.
      Float_t& operator[](const char* const name)
      {
        return value(named(name));
      }

    public:
      /// Memory for nNames floating point values.
      Float_t m_values[nNames];

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
