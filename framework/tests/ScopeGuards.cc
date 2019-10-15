/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/ScopeGuard.h>
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

namespace {
  /** Helper struct for the C++17 overload pattern */
  template<class... Ts> struct overloads : Ts... { using Ts::operator()...; };
  /** Deduction guide for the C++17 overload pattern */
  template<class... Ts> overloads(Ts...) -> overloads<Ts...>;

  /** Simple functor to set and get the values of an integer */
  struct IntSetterGetterFunctor {
    /** Constructor */
    explicit IntSetterGetterFunctor(int& reference): ref(reference) {}
    /** Call operator to set a new value */
    void operator()(const int& v) { ref = v; }
    /** Call operator to get the value */
    int operator()() const { return ref; }
    /** Reference */
    int& ref;
  };

  /** Check int reference guard */
  TEST(ScopeGuards, IntRef)
  {
    int old{5};
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard = Belle2::ScopeGuard::guardValue(old);
      ASSERT_EQ(old, 5);
      old = 17;
    }
    ASSERT_EQ(old, 5);
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard = Belle2::ScopeGuard::guardValue(old, 17);
      ASSERT_EQ(old, 17);
    }
    ASSERT_EQ(old, 5);
    {
      auto guard = Belle2::ScopeGuard::guardValue(old, 17);
      ASSERT_EQ(old, 17);
      guard.release();
    }
    ASSERT_EQ(old, 17);
  }

  /** Check guard with getter and setter */
  TEST(ScopeGuards, IntSetterGetter)
  {
    int old{5};
    auto setter = [&old](int v) {old = v;};
    auto getter = [&old]() {return old;};
    {
      auto guard = Belle2::ScopeGuard::guardGetterSetter(getter, setter);
      ASSERT_EQ(old, 5);
      old = 17;
    }
    ASSERT_EQ(old, 5);
    {
      auto guard = Belle2::ScopeGuard::guardGetterSetter(getter, setter, 17);
      ASSERT_EQ(old, 17);
    }
    ASSERT_EQ(old, 5);
    {
      auto guard = Belle2::ScopeGuard::guardGetterSetter(getter, setter, 17);
      ASSERT_EQ(old, 17);
      guard.release();
    }
    ASSERT_EQ(old, 17);
  }

  /** Check guard with functor */
  TEST(ScopeGuards, IntSetterGetterFunctor)
  {
    int old{5};
    IntSetterGetterFunctor functor(old);
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard = Belle2::ScopeGuard::guardFunctor(functor);
      ASSERT_EQ(old, 5);
      old = 17;
    }
    ASSERT_EQ(old, 5);
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard = Belle2::ScopeGuard::guardFunctor(functor, 17);
      ASSERT_EQ(old, 17);
    }
    ASSERT_EQ(old, 5);
    {
      auto guard = Belle2::ScopeGuard::guardFunctor(functor, 17);
      ASSERT_EQ(old, 17);
      guard.release();
    }
    ASSERT_EQ(old, 17);
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard = Belle2::ScopeGuard::guardFunctor(overloads{
        [&old](int v) { old = v; },
        [&old]() { return old; }
      }, 21);
      ASSERT_EQ(old, 21);
    }
    ASSERT_EQ(old, 17);
  }

  /** Test guarding a string reference. This is always fun because string
   * literals are not actually strings so it might show some problems with the
   * template arguments */
  TEST(ScopeGuards, StringReference)
  {
    std::string value{"before"};
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard = Belle2::ScopeGuard::guardValue(value, "after");
      ASSERT_EQ(value, "after");
    }
    ASSERT_EQ(value, "before");
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard = Belle2::ScopeGuard::guardFunctor(overloads{
        [&value](const std::string & v) { value = v; },
        [&value]() { return value; }
      }, "after");
      ASSERT_EQ(value, "after");
    };
    ASSERT_EQ(value, "before");
  }

  /** Test guarding a stream state */
  TEST(ScopeGuards, StreamState)
  {
    std::stringstream buf;
    buf << "a:" << std::setprecision(4) << 1.2;
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard1 = Belle2::ScopeGuard::guardStreamState(buf);
      buf << ":b:" << std::fixed << std::setprecision(4) << 2.3;
      {
        // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
        auto guard3 = Belle2::ScopeGuard::guardStreamState(buf);
        buf << ":c:" << std::setprecision(5) << std::setw(10) << std::setfill('-') << 3.4;
      }

      buf << ":d:" << std::scientific << 4.5;
    }
    buf << ":e:" << 5.4;
    ASSERT_EQ(buf.str(), "a:1.2:b:2.3000:c:---3.40000:d:4.5000e+00:e:5.4");
  }

  /** Test guarding the current working directory */
  TEST(ScopeGuards, WorkingDirectory)
  {
    std::string start{boost::filesystem::current_path().c_str()};
    std::string tmpdir("/tmp");
    std::string root("/");
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard1 = Belle2::ScopeGuard::guardWorkingDirectory("/tmp");
      ASSERT_EQ(tmpdir, boost::filesystem::current_path().c_str());
      {
        auto guard2 = Belle2::ScopeGuard::guardWorkingDirectory("/");
        ASSERT_EQ(root, boost::filesystem::current_path().c_str());
        guard2.release();
      }
      ASSERT_EQ(root, boost::filesystem::current_path().c_str());
    }
    ASSERT_EQ(start, boost::filesystem::current_path().c_str());
  }


  /** Test guarding the ROOT batch state */
  TEST(ScopeGuards, Batch)
  {
    const bool start = gROOT->IsBatch();
    {
      // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
      auto guard1 = Belle2::ScopeGuard::guardBatchMode();
      ASSERT_EQ(true, gROOT->IsBatch());
      {
        // cppcheck-suppress unreadVariable ; cppcheck doesn't realize this has side effects.
        auto guard2 = Belle2::ScopeGuard::guardBatchMode(false);
        ASSERT_EQ(false, gROOT->IsBatch());
      }
      ASSERT_EQ(true, gROOT->IsBatch());
    }
    ASSERT_EQ(start, gROOT->IsBatch());
  }
}
