/* Provides lightweight Boost.Test macros
(C) 2014-2020 Niall Douglas <http://www.nedproductions.biz/> (26 commits)
File Created: Nov 2014


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
    (See accompanying file Licence.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef QUICKCPPLIB_BOOST_UNIT_TEST_HPP
#define QUICKCPPLIB_BOOST_UNIT_TEST_HPP

#include "../../config.hpp"

/*! \defgroup unittesting Unit test suites

QuickCppLib comes with a minimal emulation of Boost.Test based on one of two
underlying unit test engines:

<dl>
<dt>`QUICKCPPLIB_BOOST_UNIT_TEST_IMPL` = 0 (the default)</dt>
<dd>An internal extremely lightweight engine which works well with exceptions and
RTTI disabled. It works fine with multithreaded test cases, and can write results
to a JUnit XML file for consumption by Jenkins etc. Its implementation is less than
two hundred lines of C++, so it isn't particularly configurable nor flexible. In
particular it maintains no history of checks, no stdio redirection, no trapping of
signals, no interception of failed asserts etc. If a check
fails it simply prints immediately what failed to stderr and increments a fail counter.
Despite its simplicity you'll probably find it works plenty well for most use cases
and it has almost zero overhead either at compile or runtime.

Note that if exceptions are disabled, those checks and requires testing that things
throw are implement as no ops for obvious reasons.

\warning A requirement failure is handled by throwing a magic exception type to
abort the test case and prevent execution of later code, unwinding the stack and
releasing any resources allocated up to that point. If exceptions are disabled,
`longjmp()` is used instead and therefore stack unwinding does not occur, leaking
any stack based resources.

\note Almost all the macros used by this internal engine can be redefined before
including the header file. It's been deliberately made highly customisable.
</dd>

<dt>`QUICKCPPLIB_BOOST_UNIT_TEST_IMPL` = 1</dt>
<dd>Use Phil Nash's CATCH (https://github.com/philsquared/Catch) header only test
engine. The Boost.Test macros simply call CATCH equivalents. The CATCH used is
actually a fork of Phil's with a giant mutex poked in so multiple threads can
do checks concurrently.</dd>
</dl>
*/
//! @{

#ifndef QUICKCPPLIB_BOOST_UNIT_TEST_IMPL
#define QUICKCPPLIB_BOOST_UNIT_TEST_IMPL 0  // default to lightweight
#endif

#if QUICKCPPLIB_BOOST_UNIT_TEST_IMPL == 1  // CATCH
#ifdef __has_include
#if !__has_include("../../CATCH/single_include/catch.hpp")
#error Cannot find the CATCH git submodule. Did you do git submodule update --init --recursive?
#endif
#endif
#ifndef __cpp_exceptions
#error CATCH unit test suite requires C++ exceptions to be enabled
#endif
#endif

// If we are to use our own noexcept capable implementation as the underlying unit test engine
#if QUICKCPPLIB_BOOST_UNIT_TEST_IMPL == 0  // std::terminate
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <vector>

#include "../../console_colours.hpp"
//#ifdef _WIN32
//#include "../../execinfo_win64.h"
//#else
//#include <execinfo.h>
//#endif
#ifndef __cpp_exceptions
#include <setjmp.h>
#endif

QUICKCPPLIB_NAMESPACE_BEGIN
namespace unit_test
{
  using namespace console_colours;
  struct requirement_failed
  {
  };
#ifndef __cpp_exceptions
  extern inline jmp_buf &test_case_failed()
  {
    static jmp_buf b;
    return b;
  }
#endif
  struct test_case
  {
    const char *name, *desc;
    void (*func)();
    std::chrono::steady_clock::duration duration;
    std::atomic<size_t> passes, fails;
    bool skipped, requirement_failed;
    constexpr test_case(const char *_name, const char *_desc, void (*_func)())
        : name(_name)
        , desc(_desc)
        , func(_func)
        , duration()
        , passes(0)
        , fails(0)
        , skipped(false)
        , requirement_failed(false)
    {
    }
    test_case(test_case &&o) noexcept
        : name(o.name)
        , desc(o.desc)
        , func(o.func)
        , duration(static_cast<std::chrono::steady_clock::duration &&>(o.duration))
        , passes(static_cast<size_t>(o.passes))
        , fails(static_cast<size_t>(o.fails))
        , skipped(o.skipped)
        , requirement_failed(o.requirement_failed)
    {
    }
    test_case &operator=(test_case &&o) noexcept
    {
      this->~test_case();
      new(this) test_case(std::move(o));
      return *this;
    }
  };
  struct test_suite
  {
    const char *name;
    std::vector<test_case> test_cases;
    test_suite(const char *_name)
        : name(_name)
    {
    }
  };
  extern inline std::vector<test_suite> &test_suites()
  {
    static std::vector<test_suite> v;
    return v;
  }
  extern inline test_suite *&current_test_suite()
  {
    static test_suite *v;
    return v;
  }
  extern inline test_case *&current_test_case()
  {
    static test_case default_test_case("unnamed", "Default test case for unit test which don't declare test cases", nullptr);
    static test_case *v = &default_test_case;
    return v;
  }
  extern inline int run(int argc, const char *const argv[])
  {
    std::regex enabled(".*"), disabled;
    bool list_tests = false;
    std::string output_xml;
    for(int n = 1; n < argc; n++)
    {
      // Double -- means it's a flag
      if(argv[n][0] == '-' && argv[n][1] == '-')
      {
        if(strstr(argv[n] + 2, "help"))
        {
          std::cout << "\nQuickCppLib minimal unit test framework\n\n"                                                                      //
                    << "Usage: " << argv[0] << " [options] [<regex for tests to run, defaults to .*>] [-<regex for tests to not run>]\n\n"  //
                    << "  --help              : Prints this help\n"                                                                         //
                    << "  --list-tests        : List matching tests\n"                                                                      //
                    << "  --reporter <format> : Reporter to use, only format possible is junit\n"                                           //
                    << "  --out <filename>    : Write JUnit XML test report to filename\n"                                                  //
                    << std::endl;
          return 0;
        }
        else if(strstr(argv[n] + 2, "list-tests"))
        {
          list_tests = true;
        }
        else if(strstr(argv[n] + 2, "reporter"))
        {
          if(n + 1 >= argc || strcmp(argv[n + 1], "junit"))
          {
            std::cerr << "--reporter must be followed by 'junit'" << std::endl;
            return 1;
          }
          n++;
        }
        else if(strstr(argv[n] + 2, "out"))
        {
          if(n + 1 >= argc)
          {
            std::cerr << "--out must be followed by the output filename" << std::endl;
            return 1;
          }
          output_xml = argv[n + 1];
          n++;
        }
      }
      else
      {
        // -regex is disabled, otherwise it's an enabled
        if(argv[n][0] == '-')
          disabled.assign(argv[n] + 1);
        else
          enabled.assign(argv[n]);
      }
    }
    if(list_tests)
    {
      size_t maxname = 0;
      for(const auto &j : test_suites())
      {
        for(const auto &i : j.test_cases)
        {
          if(strlen(i.name) > maxname)
            maxname = strlen(i.name);
        }
      }
      for(const auto &j : test_suites())
      {
        std::cout << "\n" << j.name << ":\n";
        for(const auto &i : j.test_cases)
        {
          if(std::regex_match(i.name, enabled) && !std::regex_match(i.name, disabled))
          {
            std::string padding(maxname - strlen(i.name), ' ');
            std::cout << "  " << i.name << padding << " (" << i.desc << ")\n";
          }
        }
      }
      std::cout << std::endl;
      return 0;
    }
    for(auto &j : test_suites())
    {
      for(auto &i : j.test_cases)
      {
        if(std::regex_match(i.name, enabled) && !std::regex_match(i.name, disabled))
        {
          current_test_case() = &i;
          std::cout << std::endl << bold << blue << i.name << white << " : " << i.desc << normal << std::endl;
          std::chrono::steady_clock::time_point begin, end;
#ifdef __cpp_exceptions
          try
          {
#else
          if(setjmp(test_case_failed()))
          {
            end = std::chrono::steady_clock::now();
            i.requirement_failed = true;
          }
          else
#endif
            {
              begin = std::chrono::steady_clock::now();
              i.func();
              end = std::chrono::steady_clock::now();
            }
#ifdef __cpp_exceptions
          }
          catch(const requirement_failed &)
          {
            end = std::chrono::steady_clock::now();
            i.requirement_failed = true;
          }
          catch(const std::exception &e)
          {
            end = std::chrono::steady_clock::now();
            ++i.fails;
            std::cerr << red << "FAILURE: std::exception '" << e.what() << "' thrown out of test case" << normal << std::endl;
          }
          catch(...)
          {
            end = std::chrono::steady_clock::now();
            ++i.fails;
            std::cerr << red << "FAILURE: Exception thrown out of test case" << normal << std::endl;
          }
#endif
          i.duration = end - begin;
          if(i.passes)
            std::cout << green << i.passes << " checks passed  ";
          if(i.fails)
            std::cout << red << i.fails << " checks failed  ";
          std::cout << normal << "duration " << std::chrono::duration_cast<std::chrono::milliseconds>(i.duration).count() << " ms" << std::endl;
        }
        else
          i.skipped = true;
      }
    }
    current_test_case() = nullptr;
    std::ofstream oh;
    if(!output_xml.empty())
    {
      oh.open(output_xml);
      oh << R"(<?xml version="1.0" encoding="UTF-8"?>
<testsuites>
)";
    }
    size_t totalpassed = 0, totalfailed = 0, totalskipped = 0;
    double totaltime = 0;
    for(const auto &j : test_suites())
    {
      size_t passed = 0, failed = 0, skipped = 0;
      double time = 0;
      for(const auto &i : j.test_cases)
      {
        if(i.skipped)
          ++skipped;
        else if(i.fails)
          ++failed;
        else
          ++passed;
        time += std::chrono::duration_cast<std::chrono::duration<double>>(i.duration).count();
      }
      if(!output_xml.empty())
      {
        oh << "  <testsuite name=\"" << j.name << "\" errors=\"" << 0 << "\" failures=\"" << failed << "\" skipped=\"" << skipped << "\" tests=\"" << j.test_cases.size() << "\" hostname=\"\" time=\"" << time << "\" timestamp=\"\">\n";
        for(const auto &i : j.test_cases)
        {
          oh << "    <testcase classname=\"\" name=\"" << i.name << "\" time=\"" << std::chrono::duration_cast<std::chrono::duration<double>>(i.duration).count() << "\">";
          if(i.skipped)
            oh << "<skipped/>";
          else if(i.fails)
            oh << "<failure/>";
          oh << "</testcase>\n";
        }
        oh << "  </testsuite>\n";
      }
      totalpassed += passed;
      totalfailed += failed;
      totalskipped += skipped;
      totaltime += time;
    }
    if(!output_xml.empty())
    {
      oh << R"(</testsuites>
)";
    }
    std::cout << bold << white << "\n\nTest case summary: " << green << totalpassed << " passed " << red << totalfailed << " failed " << yellow << totalskipped << " skipped" << normal << std::endl;
    return totalfailed > 0;
  }
  struct test_suite_registration
  {
    const char *name;
    test_suite_registration(const char *_name) noexcept : name(_name)
    {
      auto it = std::find_if(test_suites().begin(), test_suites().end(), [this](const test_suite &i) { return !strcmp(i.name, name); });
      if(it == test_suites().end())
      {
        test_suites().push_back(test_suite(name));
        it = --test_suites().end();
      }
      current_test_suite() = &(*it);
    }
  };
  struct test_case_registration
  {
    size_t suite_idx;
    void (*func)();
    test_case_registration(const char *name, const char *desc, void (*_func)()) noexcept : func(_func)
    {
      if(test_suites().empty())
      {
        // No BOOST_AUTO_TEST_SUITE() has been declared yet, so fake one
        test_suite_registration("unset_testsuite");
      }
      suite_idx = current_test_suite() - test_suites().data();
      test_suite *suite = test_suites().data() + suite_idx;
      suite->test_cases.push_back(test_case(name, desc, func));
    }
    ~test_case_registration()
    {
      test_suite *suite = test_suites().data() + suite_idx;
      auto it = std::remove_if(suite->test_cases.begin(), suite->test_cases.end(), [this](const test_case &i) { return i.func == func; });
      suite->test_cases.erase(it);
    }
  };
}
QUICKCPPLIB_NAMESPACE_END

#ifndef QUICKCPPLIB_BOOST_UNIT_TEST_FAIL
#ifdef __cpp_exceptions
#define QUICKCPPLIB_BOOST_UNIT_TEST_FAIL throw QUICKCPPLIB_NAMESPACE::unit_test::requirement_failed()  // NOLINT
#else
#define QUICKCPPLIB_BOOST_UNIT_TEST_FAIL longjmp(QUICKCPPLIB_NAMESPACE::unit_test::test_case_failed(), 1)
#endif
#endif
#ifndef QUICKCPPLIB_BOOST_UNIT_CHECK_FAIL
#define QUICKCPPLIB_BOOST_UNIT_CHECK_FAIL(type, expr)                                                                                                                                                                                                                                                                          \
  ++QUICKCPPLIB_NAMESPACE::unit_test::current_test_case()->fails;                                                                                                                                                                                                                                                              \
  std::cerr << QUICKCPPLIB_NAMESPACE::unit_test::yellow << "CHECK " type "(" #expr ") FAILED" << QUICKCPPLIB_NAMESPACE::unit_test::white << " at " << __FILE__ << ":" << __LINE__ << std::endl
#endif
#ifndef QUICKCPPLIB_BOOST_UNIT_CHECK_PASS
#define QUICKCPPLIB_BOOST_UNIT_CHECK_PASS(type, expr) ++QUICKCPPLIB_NAMESPACE::unit_test::current_test_case()->passes
#endif
#ifndef QUICKCPPLIB_BOOST_UNIT_REQUIRE_FAIL
#define QUICKCPPLIB_BOOST_UNIT_REQUIRE_FAIL(type, expr)                                                                                                                                                                                                                                                                        \
  ++QUICKCPPLIB_NAMESPACE::unit_test::current_test_case()->fails;                                                                                                                                                                                                                                                              \
  std::cerr << QUICKCPPLIB_NAMESPACE::unit_test::red << "REQUIRE " type "(" #expr ") FAILED" << QUICKCPPLIB_NAMESPACE::unit_test::white << " at " << __FILE__ << ":" << __LINE__ << std::endl;                                                                                                                                 \
  QUICKCPPLIB_BOOST_UNIT_TEST_FAIL
#endif
#ifndef QUICKCPPLIB_BOOST_UNIT_REQUIRE_PASS
#define QUICKCPPLIB_BOOST_UNIT_REQUIRE_PASS(type, expr) ++QUICKCPPLIB_NAMESPACE::unit_test::current_test_case()->passes
#endif

#define BOOST_TEST_MESSAGE(msg) std::cout << "INFO: " << msg << std::endl
#define BOOST_WARN_MESSAGE(pred, msg)                                                                                                                                                                                                                                                                                          \
  if(!(pred))                                                                                                                                                                                                                                                                                                                  \
  std::cerr << QUICKCPPLIB_NAMESPACE::unit_test::yellow << "WARNING: " << msg << QUICKCPPLIB_NAMESPACE::unit_test::normal << std::endl
#define BOOST_FAIL(msg)                                                                                                                                                                                                                                                                                                        \
  std::cerr << QUICKCPPLIB_NAMESPACE::unit_test::red << "FAILURE: " << msg << QUICKCPPLIB_NAMESPACE::unit_test::normal << std::endl;                                                                                                                                                                                           \
  QUICKCPPLIB_BOOST_UNIT_TEST_FAIL
#define BOOST_CHECK_MESSAGE(pred, msg)                                                                                                                                                                                                                                                                                         \
  if(!(pred))                                                                                                                                                                                                                                                                                                                  \
  std::cout << "INFO: " << msg << std::endl

#define BOOST_CHECK(expr)                                                                                                                                                                                                                                                                                                      \
  if(!(expr))                                                                                                                                                                                                                                                                                                                  \
  {                                                                                                                                                                                                                                                                                                                            \
    QUICKCPPLIB_BOOST_UNIT_CHECK_FAIL(, expr);                                                                                                                                                                                                                                                                                 \
  }                                                                                                                                                                                                                                                                                                                            \
  \
else                                                                                                                                                                                                                                                                                                                      \
  {                                                                                                                                                                                                                                                                                                                            \
    QUICKCPPLIB_BOOST_UNIT_CHECK_PASS(, expr);                                                                                                                                                                                                                                                                                 \
  }
#define BOOST_CHECK_EQUAL(expr1, expr2) BOOST_CHECK((expr1) == (expr2))
#ifdef __cpp_exceptions
#define BOOST_CHECK_THROWS(expr)                                                                                                                                                                                                                                                                                               \
  try                                                                                                                                                                                                                                                                                                                          \
  \
{                                                                                                                                                                                                                                                                                                                         \
    (expr);                                                                                                                                                                                                                                                                                                                    \
    QUICKCPPLIB_BOOST_UNIT_CHECK_FAIL("THROWS ", expr);                                                                                                                                                                                                                                                                        \
  \
}                                                                                                                                                                                                                                                                                                                         \
  \
catch(const QUICKCPPLIB_NAMESPACE::unit_test::requirement_failed &)                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    throw;                                                                                                                                                                                                                                                                                                                     \
  }                                                                                                                                                                                                                                                                                                                            \
  \
catch(...)                                                                                                                                                                                                                                                                                                                     \
  \
{                                                                                                                                                                                                                                                                                                                         \
    QUICKCPPLIB_BOOST_UNIT_CHECK_PASS("THROWS ", expr);                                                                                                                                                                                                                                                                        \
  \
}
#define BOOST_CHECK_THROW(expr, type)                                                                                                                                                                                                                                                                                          \
  try                                                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    (expr);                                                                                                                                                                                                                                                                                                                    \
    QUICKCPPLIB_BOOST_UNIT_CHECK_FAIL("THROW " #type " ", expr);                                                                                                                                                                                                                                                               \
  }                                                                                                                                                                                                                                                                                                                            \
  \
catch(const QUICKCPPLIB_NAMESPACE::unit_test::requirement_failed &)                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    throw;                                                                                                                                                                                                                                                                                                                     \
  }                                                                                                                                                                                                                                                                                                                            \
  \
catch(const type &)                                                                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    QUICKCPPLIB_BOOST_UNIT_CHECK_PASS("THROW " #type " ", expr);                                                                                                                                                                                                                                                               \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(...) { QUICKCPPLIB_BOOST_UNIT_CHECK_FAIL("THROW " #type " ", expr); }
#define BOOST_CHECK_NO_THROW(expr)                                                                                                                                                                                                                                                                                             \
  try                                                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    (expr);                                                                                                                                                                                                                                                                                                                    \
    QUICKCPPLIB_BOOST_UNIT_CHECK_PASS("NO THROW ", expr);                                                                                                                                                                                                                                                                      \
  }                                                                                                                                                                                                                                                                                                                            \
  \
catch(const QUICKCPPLIB_NAMESPACE::unit_test::requirement_failed &)                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    throw;                                                                                                                                                                                                                                                                                                                     \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(...) { QUICKCPPLIB_BOOST_UNIT_CHECK_FAIL("NO THROW ", expr); }
#else
#define BOOST_CHECK_THROWS(expr)
#define BOOST_CHECK_THROW(expr, type)
#define BOOST_CHECK_NO_THROW(expr) expr
#endif

#define BOOST_REQUIRE(expr)                                                                                                                                                                                                                                                                                                    \
  if(!(expr))                                                                                                                                                                                                                                                                                                                  \
  {                                                                                                                                                                                                                                                                                                                            \
    QUICKCPPLIB_BOOST_UNIT_REQUIRE_FAIL(, expr);                                                                                                                                                                                                                                                                               \
  }                                                                                                                                                                                                                                                                                                                            \
  \
{                                                                                                                                                                                                                                                                                                                         \
    QUICKCPPLIB_BOOST_UNIT_REQUIRE_PASS(, expr);                                                                                                                                                                                                                                                                               \
  \
}
#ifdef __cpp_exceptions
#define BOOST_REQUIRE_THROWS(expr)                                                                                                                                                                                                                                                                                             \
  try                                                                                                                                                                                                                                                                                                                          \
  \
{                                                                                                                                                                                                                                                                                                                         \
    (expr);                                                                                                                                                                                                                                                                                                                    \
    QUICKCPPLIB_BOOST_UNIT_REQUIRE_FAIL("THROWS ", expr);                                                                                                                                                                                                                                                                      \
  \
}                                                                                                                                                                                                                                                                                                                         \
  \
catch(const QUICKCPPLIB_NAMESPACE::unit_test::requirement_failed &)                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    throw;                                                                                                                                                                                                                                                                                                                     \
  }                                                                                                                                                                                                                                                                                                                            \
  \
catch(...)                                                                                                                                                                                                                                                                                                                     \
  \
{                                                                                                                                                                                                                                                                                                                         \
    QUICKCPPLIB_BOOST_UNIT_REQUIRE_PASS("THROWS ", expr);                                                                                                                                                                                                                                                                      \
  \
}
#define BOOST_CHECK_REQUIRE(expr, type)                                                                                                                                                                                                                                                                                        \
  try                                                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    (expr);                                                                                                                                                                                                                                                                                                                    \
    QUICKCPPLIB_BOOST_UNIT_REQUIRE_FAIL("THROW " #type " ", expr);                                                                                                                                                                                                                                                             \
  }                                                                                                                                                                                                                                                                                                                            \
  \
catch(const QUICKCPPLIB_NAMESPACE::unit_test::requirement_failed &)                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    throw;                                                                                                                                                                                                                                                                                                                     \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(const type &) { QUICKCPPLIB_BOOST_UNIT_REQUIRE_PASS("THROW " #type " ", expr); }                                                                                                                                                                                                                                       \
  catch(...) { QUICKCPPLIB_BOOST_UNIT_REQUIRE_FAIL("THROW " #type " ", expr); }
#define BOOST_REQUIRE_NO_THROW(expr)                                                                                                                                                                                                                                                                                           \
  try                                                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    (expr);                                                                                                                                                                                                                                                                                                                    \
    QUICKCPPLIB_BOOST_UNIT_REQUIRE_PASS("NO THROW ", expr);                                                                                                                                                                                                                                                                    \
  }                                                                                                                                                                                                                                                                                                                            \
  \
catch(const QUICKCPPLIB_NAMESPACE::unit_test::requirement_failed &)                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    throw;                                                                                                                                                                                                                                                                                                                     \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(...) { QUICKCPPLIB_BOOST_UNIT_REQUIRE_FAIL("NO THROW ", expr); }
#else
#define BOOST_REQUIRE_THROWS(expr)
#define BOOST_CHECK_REQUIRE(expr, type)
#define BOOST_REQUIRE_NO_THROW(expr) expr
#endif

#define BOOST_AUTO_TEST_SUITE3(a, b) a##b
#define BOOST_AUTO_TEST_SUITE2(a, b) BOOST_AUTO_TEST_SUITE3(a, b)
#define BOOST_AUTO_TEST_SUITE(name)                                                                                                                                                                                                                                                                                            \
  namespace BOOST_AUTO_TEST_SUITE2(boostlite_auto_test_suite, __COUNTER__)                                                                                                                                                                                                                                                     \
  {                                                                                                                                                                                                                                                                                                                            \
    static QUICKCPPLIB_NAMESPACE::unit_test::test_suite_registration boostlite_auto_test_suite_registration(#name);
//
#define BOOST_AUTO_TEST_SUITE_END() }

#ifndef QUICKCPPLIB_BOOST_UNIT_TEST_CASE_NAME
#define QUICKCPPLIB_BOOST_UNIT_TEST_CASE_NAME(name) #name
#endif
#define QUICKCPPLIB_BOOST_UNIT_TEST_CASE_UNIQUE(prefix) BOOST_AUTO_TEST_SUITE2(prefix, __COUNTER__)
#define BOOST_AUTO_TEST_CASE2(test_name, desc, func_name)                                                                                                                                                                                                                                                                      \
  \
static void                                                                                                                                                                                                                                                                                                                    \
  func_name();                                                                                                                                                                                                                                                                                                                 \
  \
static QUICKCPPLIB_NAMESPACE::unit_test::test_case_registration BOOST_AUTO_TEST_SUITE2(func_name, _registration)(static_cast<const char *>(test_name), static_cast<const char *>(desc), func_name);                                                                                                                            \
  \
static void                                                                                                                                                                                                                                                                                                                    \
  func_name()
#define BOOST_AUTO_TEST_CASE(test_name, desc) BOOST_AUTO_TEST_CASE2(QUICKCPPLIB_BOOST_UNIT_TEST_CASE_NAME(test_name), desc, QUICKCPPLIB_BOOST_UNIT_TEST_CASE_UNIQUE(boostlite_auto_test_case))

#define QUICKCPPLIB_BOOST_UNIT_TEST_RUN_TESTS(argc, argv) QUICKCPPLIB_NAMESPACE::unit_test::run(argc, argv)
#endif


// If we are to use threadsafe CATCH as the underlying unit test engine
#if QUICKCPPLIB_BOOST_UNIT_TEST_IMPL == 1
#include <atomic>
#include <mutex>
#define CATCH_CONFIG_PREFIX_ALL
#define CATCH_CONFIG_RUNNER
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4702)
#endif
#include "../../CATCH/single_include/catch.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define BOOST_TEST_MESSAGE(msg) CATCH_INFO(msg)
#define BOOST_WARN_MESSAGE(pred, msg)                                                                                                                                                                                                                                                                                          \
  if(!(pred))                                                                                                                                                                                                                                                                                                                  \
  CATCH_WARN(msg)
#define BOOST_FAIL(msg) CATCH_FAIL(msg)
#define BOOST_CHECK_MESSAGE(pred, msg)                                                                                                                                                                                                                                                                                         \
  if(!(pred))                                                                                                                                                                                                                                                                                                                  \
  CATCH_INFO(msg)

#define BOOST_CHECK(expr) CATCH_CHECK(expr)
#define BOOST_CHECK_EQUAL(expr1, expr2) BOOST_CHECK((expr1) == (expr2))
#define BOOST_CHECK_THROWS(expr) CATCH_CHECK_THROWS(expr)
#define BOOST_CHECK_THROW(expr, type) CATCH_CHECK_THROWS_AS(expr, const type &)
#define BOOST_CHECK_NO_THROW(expr) CATCH_CHECK_NOTHROW(expr)

#define BOOST_REQUIRE(expr) CATCH_REQUIRE(expr)
#define BOOST_REQUIRE_THROWS(expr) CATCH_REQUIRE_THROWS(expr)
#define BOOST_CHECK_REQUIRE(expr, type) CATCH_REQUIRE_THROWS_AS(expr, const type &)
#define BOOST_REQUIRE_NO_THROW(expr) CATCH_REQUIRE_NOTHROW(expr)

#define BOOST_AUTO_TEST_SUITE3(a, b) a##b
#define BOOST_AUTO_TEST_SUITE2(a, b) BOOST_AUTO_TEST_SUITE3(a, b)
#define BOOST_AUTO_TEST_SUITE(name)                                                                                                                                                                                                                                                                                            \
  namespace BOOST_AUTO_TEST_SUITE2(boostlite_auto_test_suite, __COUNTER__)                                                                                                                                                                                                                                                     \
  {
//
#define BOOST_AUTO_TEST_SUITE_END() }
#ifndef QUICKCPPLIB_BOOST_UNIT_TEST_CASE_NAME
#define QUICKCPPLIB_BOOST_UNIT_TEST_CASE_NAME(name) #name
#endif
#define BOOST_AUTO_TEST_CASE(test_name, desc) CATCH_TEST_CASE(QUICKCPPLIB_BOOST_UNIT_TEST_CASE_NAME(test_name), desc)

#define QUICKCPPLIB_BOOST_UNIT_TEST_RUN_TESTS(argc, argv) Catch::Session().run(argc, argv)

#endif

#if defined _MSC_VER
#define BOOST_BINDLIB_ENABLE_MULTIPLE_DEFINITIONS
#elif defined __MINGW32__
#define BOOST_BINDLIB_ENABLE_MULTIPLE_DEFINITIONS
#elif defined __GNUC__
#define BOOST_BINDLIB_ENABLE_MULTIPLE_DEFINITIONS __attribute__((weak))
#else
#define BOOST_BINDLIB_ENABLE_MULTIPLE_DEFINITIONS inline
#endif

#ifndef QUICKCPPLIB_BOOST_UNIT_TEST_CUSTOM_MAIN_DEFINED
#ifdef _MSC_VER
extern inline int quickcpplib_unit_testing_main(int argc, const char* const argv[]) {
  int result = QUICKCPPLIB_BOOST_UNIT_TEST_RUN_TESTS(argc, argv);
  return result;
}
#if(defined(__x86_64__) || defined(_M_X64)) || (defined(__aarch64__) || defined(_M_ARM64))
#pragma comment(linker, "/alternatename:main=?quickcpplib_unit_testing_main@@YAHHQEBQEBD@Z")
#elif defined(__x86__) || defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/alternatename:_main=?quickcpplib_unit_testing_main@@YAHHQBQBD@Z")
#elif defined(__arm__) || defined(_M_ARM)
#pragma comment(linker, "/alternatename:main=?quickcpplib_unit_testing_main@@YAHHQBQBD@Z")
#else
#error Unknown architecture
#endif
#else
BOOST_BINDLIB_ENABLE_MULTIPLE_DEFINITIONS int main(int argc, const char *const argv[])
{
  int result = QUICKCPPLIB_BOOST_UNIT_TEST_RUN_TESTS(argc, argv);
  return result;
}
#endif
#endif

//! @}

#endif
