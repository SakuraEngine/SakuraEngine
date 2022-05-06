/* NT kernel error code category
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: July 2017


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

#include "../ntkernel_category.hpp"

#include <cstring>

namespace ntkernel_error_category
{
  namespace detail
  {
    struct field
    {
      int ntstatus;
      int win32;
      int posix;
      const char *message;
    };
    static inline const field *find_ntstatus(int ntstatus) noexcept
    {
      static const field table[] = {
#include "ntkernel-table.ipp"
      };
      for(const field &i : table)
      {
        if(i.ntstatus == ntstatus)
        {
          return &i;
        }
      }
      return nullptr;
    }
  }  // namespace detail
  class ntkernelcategory : public std::error_category
  {
  public:
    virtual const char *name() const noexcept override final { return "ntkernel_category"; }
    // Map where possible to std::errc (generic_category)
    virtual std::error_condition default_error_condition(int code) const noexcept override final
    {
      const detail::field *f = detail::find_ntstatus(code);
      if(f == nullptr || f->posix == 0)
      {
        return {code, *this};
      }
      return {f->posix, std::generic_category()};
    }
    // Test equivalence via mapping my code to their code
    virtual bool equivalent(int mycode, const std::error_condition &theircond) const noexcept override final
    {
      // If I'm comparing to myself, its testing equality
      if(*this == theircond.category() || (strcmp(name(), theircond.category().name()) == 0))
      {
        return theircond.value() == mycode;
      }
#ifdef _WIN32
      // If I'm comparing win32 to me, that's custom
      if(std::system_category() == theircond.category() || (strcmp(std::system_category().name(), theircond.category().name()) == 0))
      {
        const detail::field *f = detail::find_ntstatus(mycode);
        if(f != nullptr && f->win32 == theircond.value())
        {
          return true;
        }
      }
#endif
      // Fall back onto generic_category testing
      if(std::generic_category() == theircond.category() || (strcmp(std::generic_category().name(), theircond.category().name()) == 0))
      {
        const detail::field *f = detail::find_ntstatus(mycode);
        if(f != nullptr && f->posix == theircond.value())
        {
          return true;
        }
      }
      return false;
    }
    // Test equivalence via mapping their code to me
    virtual bool equivalent(const std::error_code &theircode, int mycond) const noexcept override final
    {
      // If I'm comparing to myself, its testing equality
      if(*this == theircode.category() || (strcmp(name(), theircode.category().name()) == 0))
      {
        return theircode.value() == mycond;
      }
#ifdef _WIN32
      // If I'm comparing win32 to me, that's custom
      if(std::system_category() == theircode.category() || (strcmp(std::system_category().name(), theircode.category().name()) == 0))
      {
        const detail::field *f = detail::find_ntstatus(mycond);
        if(f != nullptr && f->win32 == theircode.value())
        {
          return true;
        }
      }
#endif
      // Fall back onto generic_category testing
      if(std::generic_category() == theircode.category() || (strcmp(std::generic_category().name(), theircode.category().name()) == 0))
      {
        const detail::field *f = detail::find_ntstatus(mycond);
        if(f != nullptr && f->posix == theircode.value())
        {
          return true;
        }
      }
      return false;
    }
    virtual std::string message(int condition) const override final
    {
      if(condition == 0)
      {
        return "The operation completed successfully";
      }
      const detail::field *f = detail::find_ntstatus(condition);
      if(f != nullptr)
      {
        return f->message;
      }
      switch(static_cast<unsigned>(condition) >> 30)
      {
      case 0:
        return "Unknown success";
      case 1:
        return "Unknown information";
      case 2:
        return "Unknown warning";
      case 3:
        return "Unknown error";
      }
      return {};
    }
  };
  NTKERNEL_ERROR_CATEGORY_INLINE_API const std::error_category &ntkernel_category() noexcept
  {
    static ntkernelcategory c;
    return c;
  }
}  // namespace ntkernel_error_category
