/* Portable colourful console printing
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Apr 2016


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

#ifndef QUICKCPPLIB_CONSOLE_COLOURS_HPP
#define QUICKCPPLIB_CONSOLE_COLOURS_HPP

#include "config.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

QUICKCPPLIB_NAMESPACE_BEGIN

namespace console_colours
{
#ifdef _WIN32
  namespace detail
  {
    inline bool &am_in_bold()
    {
      static bool v;
      return v;
    }
    inline void set(WORD v)
    {
      if(am_in_bold())
        v |= FOREGROUND_INTENSITY;
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), v);
    }
  }
  //! Makes the text on the console red
  inline std::ostream &red(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED);
    return s;
  }
  //! Makes the text on the console green
  inline std::ostream &green(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_GREEN);
    return s;
  }
  //! Makes the text on the console blue
  inline std::ostream &blue(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_BLUE);
    return s;
  }
  //! Makes the text on the console yellow
  inline std::ostream &yellow(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED | FOREGROUND_GREEN);
    return s;
  }
  //! Makes the text on the console magenta
  inline std::ostream &magenta(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED | FOREGROUND_BLUE);
    return s;
  }
  //! Makes the text on the console cyan
  inline std::ostream &cyan(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_GREEN | FOREGROUND_BLUE);
    return s;
  }
  //! Makes the text on the console white
  inline std::ostream &white(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return s;
  }
  //! Makes the text on the console bold
  inline std::ostream &bold(std::ostream &s)
  {
    detail::am_in_bold() = true;
    return s;
  }
  //! Makes the text on the console non-bold and white
  inline std::ostream &normal(std::ostream &s)
  {
    detail::am_in_bold() = false;
    return white(s);
  }
#else
  namespace detail
  {
    inline std::ostream &color_if_term(std::ostream &s, const char seq[])
    {
      if((&s == &std::cout && isatty(1 /*STDOUT_FILENO*/)) || (&s == &std::cerr && isatty(2 /*STDERR_FILENO*/)))
        s << seq;
      return s;
    }
    constexpr const char red[] = {0x1b, '[', '3', '1', 'm', 0};
    constexpr const char green[] = {0x1b, '[', '3', '2', 'm', 0};
    constexpr const char blue[] = {0x1b, '[', '3', '4', 'm', 0};
    constexpr const char yellow[] = {0x1b, '[', '3', '3', 'm', 0};
    constexpr const char magenta[] = {0x1b, '[', '3', '5', 'm', 0};
    constexpr const char cyan[] = {0x1b, '[', '3', '6', 'm', 0};
    constexpr const char white[] = {0x1b, '[', '3', '7', 'm', 0};
    constexpr const char bold[] = {0x1b, '[', '1', 'm', 0};
    constexpr const char normal[] = {0x1b, '[', '0', 'm', 0};
  }
  //! Makes the text on the console red
  inline std::ostream &red(std::ostream &s) { return detail::color_if_term(s, detail::red); }
  //! Makes the text on the console green
  inline std::ostream &green(std::ostream &s) { return detail::color_if_term(s, detail::green); }
  //! Makes the text on the console blue
  inline std::ostream &blue(std::ostream &s) { return detail::color_if_term(s, detail::blue); }
  //! Makes the text on the console yellow
  inline std::ostream &yellow(std::ostream &s) { return detail::color_if_term(s, detail::yellow); }
  //! Makes the text on the console magenta
  inline std::ostream &magenta(std::ostream &s) { return detail::color_if_term(s, detail::magenta); }
  //! Makes the text on the console cyan
  inline std::ostream &cyan(std::ostream &s) { return detail::color_if_term(s, detail::cyan); }
  //! Makes the text on the console white
  inline std::ostream &white(std::ostream &s) { return detail::color_if_term(s, detail::white); }
  //! Makes the text on the console bold
  inline std::ostream &bold(std::ostream &s) { return detail::color_if_term(s, detail::bold); }
  //! Makes the text on the console non-bold and white
  inline std::ostream &normal(std::ostream &s) { return detail::color_if_term(s, detail::normal); }
#endif
}

QUICKCPPLIB_NAMESPACE_END

#endif
