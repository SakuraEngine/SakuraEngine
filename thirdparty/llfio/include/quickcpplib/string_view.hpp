/* string_view support
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Jul 2017


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

#ifndef QUICKCPPLIB_STRING_VIEW_HPP
#define QUICKCPPLIB_STRING_VIEW_HPP

#include "config.hpp"

#if(_HAS_CXX17 || __cplusplus >= 201700) && (!defined(__GLIBCXX__) || __GLIBCXX__ > 20170519)  // libstdc++'s string_view is missing constexpr

#include <string_view>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace string_view
{
  template <typename charT, typename traits = std::char_traits<charT>> using basic_string_view = std::basic_string_view<charT, traits>;
  typedef basic_string_view<char, std::char_traits<char>> string_view;
  typedef basic_string_view<wchar_t, std::char_traits<wchar_t>> wstring_view;
  typedef basic_string_view<char16_t, std::char_traits<char16_t>> u16string_view;
  typedef basic_string_view<char32_t, std::char_traits<char32_t>> u32string_view;
}

QUICKCPPLIB_NAMESPACE_END

#else

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace string_view
{
  template <typename charT, typename traits = std::char_traits<charT>> class basic_string_view;
  typedef basic_string_view<char, std::char_traits<char>> string_view;
  typedef basic_string_view<wchar_t, std::char_traits<wchar_t>> wstring_view;
  typedef basic_string_view<char16_t, std::char_traits<char16_t>> u16string_view;
  typedef basic_string_view<char32_t, std::char_traits<char32_t>> u32string_view;

  // The remainder of this file is a hacked edition of Boost's string_view

  /*
  Copyright (c) Marshall Clow 2012-2015.
  Copyright (c) Beman Dawes 2015

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

  For more information, see http://www.boost.org

  Based on the StringRef implementation in LLVM (http://llvm.org) and
  N3422 by Jeffrey Yasskin
  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3442.html
  Updated July 2015 to reflect the Library Fundamentals TS
  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4480.html
  */

  namespace detail
  {
    //  A helper functor because sometimes we don't have lambdas
    template <typename charT, typename traits> class string_view_traits_eq
    {
    public:
      string_view_traits_eq(charT ch)
          : ch_(ch)
      {
      }
      bool operator()(charT val) const { return traits::eq(ch_, val); }
      charT ch_;
    };
  }

  template <typename charT, typename traits>  // traits defaulted in string_view_fwd.hpp
  class basic_string_view
  {
  public:
    // types
    typedef traits traits_type;
    typedef charT value_type;
    typedef charT *pointer;
    typedef const charT *const_pointer;
    typedef charT &reference;
    typedef const charT &const_reference;
    typedef const_pointer const_iterator;  // impl-defined
    typedef const_iterator iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    static constexpr size_type npos = size_type(-1);

    // construct/copy
    constexpr basic_string_view() noexcept : ptr_(NULL), len_(0) {}

    // by defaulting these functions, basic_string_ref becomes
    //  trivially copy/move constructible.
    constexpr basic_string_view(const basic_string_view &rhs) noexcept = default;

    basic_string_view &operator=(const basic_string_view &rhs) noexcept = default;

    template <typename Allocator> basic_string_view(const std::basic_string<charT, traits, Allocator> &str) noexcept : ptr_(str.data()), len_(str.length()) {}

    // #if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS)
    //       // Constructing a string_view from a temporary string is a bad idea
    //       template<typename Allocator>
    //         basic_string_view(      std::basic_string<charT, traits, Allocator>&&)
    //           = delete;
    // #endif

    constexpr basic_string_view(const charT *str)
        : ptr_(str)
        , len_(traits::length(str))
    {
    }

    constexpr basic_string_view(const charT *str, size_type len)
        : ptr_(str)
        , len_(len)
    {
    }

    // iterators
    constexpr const_iterator begin() const noexcept { return ptr_; }
    constexpr const_iterator cbegin() const noexcept { return ptr_; }
    constexpr const_iterator end() const noexcept { return ptr_ + len_; }
    constexpr const_iterator cend() const noexcept { return ptr_ + len_; }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    // capacity
    constexpr size_type size() const noexcept { return len_; }
    constexpr size_type length() const noexcept { return len_; }
    constexpr size_type max_size() const noexcept { return len_; }
    constexpr bool empty() const noexcept { return len_ == 0; }

    // element access
    constexpr const_reference operator[](size_type pos) const noexcept { return ptr_[pos]; }

    constexpr const_reference at(size_t pos) const { return pos >= len_ ? throw std::out_of_range("boost::string_view::at"), ptr_[0] : ptr_[pos]; }

    constexpr const_reference front() const { return ptr_[0]; }
    constexpr const_reference back() const { return ptr_[len_ - 1]; }
    constexpr const_pointer data() const noexcept { return ptr_; }

    // modifiers
    void clear() noexcept { len_ = 0; }  // Boost extension

    constexpr void remove_prefix(size_type n)
    {
      if(n > len_)
        n = len_;
      ptr_ += n;
      len_ -= n;
    }

    constexpr void remove_suffix(size_type n)
    {
      if(n > len_)
        n = len_;
      len_ -= n;
    }

    constexpr void swap(basic_string_view &s) noexcept
    {
      std::swap(ptr_, s.ptr_);
      std::swap(len_, s.len_);
    }

    // basic_string_view string operations
    template <typename Allocator> explicit operator std::basic_string<charT, traits, Allocator>() const { return std::basic_string<charT, traits, Allocator>(begin(), end()); }

    template <typename Allocator = std::allocator<charT>> std::basic_string<charT, traits, Allocator> to_string(const Allocator &a = Allocator()) const { return std::basic_string<charT, traits, Allocator>(begin(), end(), a); }

    size_type copy(charT *s, size_type n, size_type pos = 0) const
    {
      if(pos > size())
        throw std::out_of_range("string_view::copy");
      size_type rlen = (std::min)(n, len_ - pos);
      traits_type::copy(s, data() + pos, rlen);
      return rlen;
    }

    constexpr basic_string_view substr(size_type pos, size_type n = npos) const
    {
      if(pos > size())
        throw std::out_of_range("string_view::substr");
      return basic_string_view(data() + pos, (std::min)(size() - pos, n));
    }

    constexpr int compare(basic_string_view x) const noexcept
    {
      const int cmp = traits::compare(ptr_, x.ptr_, (std::min)(len_, x.len_));
      return cmp != 0 ? cmp : (len_ == x.len_ ? 0 : len_ < x.len_ ? -1 : 1);
    }

    constexpr int compare(size_type pos1, size_type n1, basic_string_view x) const noexcept { return substr(pos1, n1).compare(x); }

    constexpr int compare(size_type pos1, size_type n1, basic_string_view x, size_type pos2, size_type n2) const { return substr(pos1, n1).compare(x.substr(pos2, n2)); }

    constexpr int compare(const charT *x) const { return compare(basic_string_view(x)); }

    constexpr int compare(size_type pos1, size_type n1, const charT *x) const { return substr(pos1, n1).compare(basic_string_view(x)); }

    constexpr int compare(size_type pos1, size_type n1, const charT *x, size_type n2) const { return substr(pos1, n1).compare(basic_string_view(x, n2)); }

    //  Searches
    constexpr bool starts_with(charT c) const noexcept
    {  // Boost extension
      return !empty() && traits::eq(c, front());
    }

    constexpr bool starts_with(basic_string_view x) const noexcept
    {  // Boost extension
      return len_ >= x.len_ && traits::compare(ptr_, x.ptr_, x.len_) == 0;
    }

    constexpr bool ends_with(charT c) const noexcept
    {  // Boost extension
      return !empty() && traits::eq(c, back());
    }

    constexpr bool ends_with(basic_string_view x) const noexcept
    {  // Boost extension
      return len_ >= x.len_ && traits::compare(ptr_ + len_ - x.len_, x.ptr_, x.len_) == 0;
    }

    //  find
    constexpr size_type find(basic_string_view s, size_type pos = 0) const noexcept
    {
      if(pos > size())
        return npos;
      if(s.empty())
        return pos;
      const_iterator iter = std::search(this->cbegin() + pos, this->cend(), s.cbegin(), s.cend(), traits::eq);
      return iter == this->cend() ? npos : std::distance(this->cbegin(), iter);
    }
    constexpr size_type find(charT c, size_type pos = 0) const noexcept { return find(basic_string_view(&c, 1), pos); }
    constexpr size_type find(const charT *s, size_type pos, size_type n) const noexcept { return find(basic_string_view(s, n), pos); }
    constexpr size_type find(const charT *s, size_type pos = 0) const noexcept { return find(basic_string_view(s), pos); }

    //  rfind
    constexpr size_type rfind(basic_string_view s, size_type pos = npos) const noexcept
    {
      if(len_ < s.len_)
        return npos;
      if(pos > len_ - s.len_)
        pos = len_ - s.len_;
      if(s.len_ == 0u)  // an empty string is always found
        return pos;
      for(const charT *cur = ptr_ + pos;; --cur)
      {
        if(traits::compare(cur, s.ptr_, s.len_) == 0)
          return cur - ptr_;
        if(cur == ptr_)
          return npos;
      };
    }
    constexpr size_type rfind(charT c, size_type pos = npos) const noexcept { return rfind(basic_string_view(&c, 1), pos); }
    constexpr size_type rfind(const charT *s, size_type pos, size_type n) const noexcept { return rfind(basic_string_view(s, n), pos); }
    constexpr size_type rfind(const charT *s, size_type pos = npos) const noexcept { return rfind(basic_string_view(s), pos); }

    //  find_first_of
    constexpr size_type find_first_of(basic_string_view s, size_type pos = 0) const noexcept
    {
      if(pos >= len_ || s.len_ == 0)
        return npos;
      const_iterator iter = std::find_first_of(this->cbegin() + pos, this->cend(), s.cbegin(), s.cend(), traits::eq);
      return iter == this->cend() ? npos : std::distance(this->cbegin(), iter);
    }
    constexpr size_type find_first_of(charT c, size_type pos = 0) const noexcept { return find_first_of(basic_string_view(&c, 1), pos); }
    constexpr size_type find_first_of(const charT *s, size_type pos, size_type n) const noexcept { return find_first_of(basic_string_view(s, n), pos); }
    constexpr size_type find_first_of(const charT *s, size_type pos = 0) const noexcept { return find_first_of(basic_string_view(s), pos); }

    //  find_last_of
    constexpr size_type find_last_of(basic_string_view s, size_type pos = npos) const noexcept
    {
      if(s.len_ == 0u)
        return npos;
      if(pos >= len_)
        pos = 0;
      else
        pos = len_ - (pos + 1);
      const_reverse_iterator iter = std::find_first_of(this->crbegin() + pos, this->crend(), s.cbegin(), s.cend(), traits::eq);
      return iter == this->crend() ? npos : reverse_distance(this->crbegin(), iter);
    }
    constexpr size_type find_last_of(charT c, size_type pos = npos) const noexcept { return find_last_of(basic_string_view(&c, 1), pos); }
    constexpr size_type find_last_of(const charT *s, size_type pos, size_type n) const noexcept { return find_last_of(basic_string_view(s, n), pos); }
    constexpr size_type find_last_of(const charT *s, size_type pos = npos) const noexcept { return find_last_of(basic_string_view(s), pos); }

    //  find_first_not_of
    constexpr size_type find_first_not_of(basic_string_view s, size_type pos = 0) const noexcept
    {
      if(pos >= len_)
        return npos;
      if(s.len_ == 0)
        return pos;
      const_iterator iter = find_not_of(this->cbegin() + pos, this->cend(), s);
      return iter == this->cend() ? npos : std::distance(this->cbegin(), iter);
    }
    constexpr size_type find_first_not_of(charT c, size_type pos = 0) const noexcept { return find_first_not_of(basic_string_view(&c, 1), pos); }
    constexpr size_type find_first_not_of(const charT *s, size_type pos, size_type n) const noexcept { return find_first_not_of(basic_string_view(s, n), pos); }
    constexpr size_type find_first_not_of(const charT *s, size_type pos = 0) const noexcept { return find_first_not_of(basic_string_view(s), pos); }

    //  find_last_not_of
    constexpr size_type find_last_not_of(basic_string_view s, size_type pos = npos) const noexcept
    {
      if(pos >= len_)
        pos = len_ - 1;
      if(s.len_ == 0u)
        return pos;
      pos = len_ - (pos + 1);
      const_reverse_iterator iter = find_not_of(this->crbegin() + pos, this->crend(), s);
      return iter == this->crend() ? npos : reverse_distance(this->crbegin(), iter);
    }
    constexpr size_type find_last_not_of(charT c, size_type pos = npos) const noexcept { return find_last_not_of(basic_string_view(&c, 1), pos); }
    constexpr size_type find_last_not_of(const charT *s, size_type pos, size_type n) const noexcept { return find_last_not_of(basic_string_view(s, n), pos); }
    constexpr size_type find_last_not_of(const charT *s, size_type pos = npos) const noexcept { return find_last_not_of(basic_string_view(s), pos); }

  private:
    template <typename r_iter> size_type reverse_distance(r_iter first, r_iter last) const noexcept
    {
      // Portability note here: std::distance is not NOEXCEPT, but calling it with a string_view::reverse_iterator will not throw.
      return len_ - 1 - std::distance(first, last);
    }

    template <typename Iterator> Iterator find_not_of(Iterator first, Iterator last, basic_string_view s) const noexcept
    {
      for(; first != last; ++first)
        if(0 == traits::find(s.ptr_, s.len_, *first))
          return first;
      return last;
    }

    const charT *ptr_;
    std::size_t len_;
  };


  //  Comparison operators
  //  Equality
  template <typename charT, typename traits> inline constexpr bool operator==(basic_string_view<charT, traits> x, basic_string_view<charT, traits> y) noexcept
  {
    if(x.size() != y.size())
      return false;
    return x.compare(y) == 0;
  }

  //  Inequality
  template <typename charT, typename traits> inline constexpr bool operator!=(basic_string_view<charT, traits> x, basic_string_view<charT, traits> y) noexcept
  {
    if(x.size() != y.size())
      return true;
    return x.compare(y) != 0;
  }

  //  Less than
  template <typename charT, typename traits> inline constexpr bool operator<(basic_string_view<charT, traits> x, basic_string_view<charT, traits> y) noexcept { return x.compare(y) < 0; }

  //  Greater than
  template <typename charT, typename traits> inline constexpr bool operator>(basic_string_view<charT, traits> x, basic_string_view<charT, traits> y) noexcept { return x.compare(y) > 0; }

  //  Less than or equal to
  template <typename charT, typename traits> inline constexpr bool operator<=(basic_string_view<charT, traits> x, basic_string_view<charT, traits> y) noexcept { return x.compare(y) <= 0; }

  //  Greater than or equal to
  template <typename charT, typename traits> inline constexpr bool operator>=(basic_string_view<charT, traits> x, basic_string_view<charT, traits> y) noexcept { return x.compare(y) >= 0; }

  // "sufficient additional overloads of comparison functions"
  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator==(basic_string_view<charT, traits> x, const std::basic_string<charT, traits, Allocator> &y) noexcept { return x == basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator==(const std::basic_string<charT, traits, Allocator> &x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) == y; }

  template <typename charT, typename traits> inline constexpr bool operator==(basic_string_view<charT, traits> x, const charT *y) noexcept { return x == basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits> inline constexpr bool operator==(const charT *x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) == y; }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator!=(basic_string_view<charT, traits> x, const std::basic_string<charT, traits, Allocator> &y) noexcept { return x != basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator!=(const std::basic_string<charT, traits, Allocator> &x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) != y; }

  template <typename charT, typename traits> inline constexpr bool operator!=(basic_string_view<charT, traits> x, const charT *y) noexcept { return x != basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits> inline constexpr bool operator!=(const charT *x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) != y; }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator<(basic_string_view<charT, traits> x, const std::basic_string<charT, traits, Allocator> &y) noexcept { return x < basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator<(const std::basic_string<charT, traits, Allocator> &x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) < y; }

  template <typename charT, typename traits> inline constexpr bool operator<(basic_string_view<charT, traits> x, const charT *y) noexcept { return x < basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits> inline constexpr bool operator<(const charT *x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) < y; }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator>(basic_string_view<charT, traits> x, const std::basic_string<charT, traits, Allocator> &y) noexcept { return x > basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator>(const std::basic_string<charT, traits, Allocator> &x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) > y; }

  template <typename charT, typename traits> inline constexpr bool operator>(basic_string_view<charT, traits> x, const charT *y) noexcept { return x > basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits> inline constexpr bool operator>(const charT *x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) > y; }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator<=(basic_string_view<charT, traits> x, const std::basic_string<charT, traits, Allocator> &y) noexcept { return x <= basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator<=(const std::basic_string<charT, traits, Allocator> &x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) <= y; }

  template <typename charT, typename traits> inline constexpr bool operator<=(basic_string_view<charT, traits> x, const charT *y) noexcept { return x <= basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits> inline constexpr bool operator<=(const charT *x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) <= y; }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator>=(basic_string_view<charT, traits> x, const std::basic_string<charT, traits, Allocator> &y) noexcept { return x >= basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits, typename Allocator> inline constexpr bool operator>=(const std::basic_string<charT, traits, Allocator> &x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) >= y; }

  template <typename charT, typename traits> inline constexpr bool operator>=(basic_string_view<charT, traits> x, const charT *y) noexcept { return x >= basic_string_view<charT, traits>(y); }

  template <typename charT, typename traits> inline constexpr bool operator>=(const charT *x, basic_string_view<charT, traits> y) noexcept { return basic_string_view<charT, traits>(x) >= y; }

  namespace detail
  {

    template <class charT, class traits> inline void sv_insert_fill_chars(std::basic_ostream<charT, traits> &os, std::size_t n)
    {
      enum
      {
        chunk_size = 8
      };
      charT fill_chars[chunk_size];
      std::fill_n(fill_chars, static_cast<std::size_t>(chunk_size), os.fill());
      for(; n >= chunk_size && os.good(); n -= chunk_size)
        os.write(fill_chars, static_cast<std::size_t>(chunk_size));
      if(n > 0 && os.good())
        os.write(fill_chars, n);
    }

    template <class charT, class traits> void sv_insert_aligned(std::basic_ostream<charT, traits> &os, const basic_string_view<charT, traits> &str)
    {
      const std::size_t size = str.size();
      const std::size_t alignment_size = static_cast<std::size_t>(os.width()) - size;
      const bool align_left = (os.flags() & std::basic_ostream<charT, traits>::adjustfield) == std::basic_ostream<charT, traits>::left;
      if(!align_left)
      {
        detail::sv_insert_fill_chars(os, alignment_size);
        if(os.good())
          os.write(str.data(), size);
      }
      else
      {
        os.write(str.data(), size);
        if(os.good())
          detail::sv_insert_fill_chars(os, alignment_size);
      }
    }

  }  // namespace detail

  // Inserter
  template <class charT, class traits> inline std::basic_ostream<charT, traits> &operator<<(std::basic_ostream<charT, traits> &os, const basic_string_view<charT, traits> &str)
  {
    if(os.good())
    {
      const std::size_t size = str.size();
      const std::size_t w = static_cast<std::size_t>(os.width());
      if(w <= size)
        os.write(str.data(), size);
      else
        detail::sv_insert_aligned(os, str);
      os.width(0);
    }
    return os;
  }

#if 0
    // numeric conversions
    //
    //  These are short-term implementations.
    //  In a production environment, I would rather avoid the copying.
    //
    inline int stoi(string_view str, size_t* idx = 0, int base = 10) {
      return std::stoi(std::string(str), idx, base);
    }

    inline long stol(string_view str, size_t* idx = 0, int base = 10) {
      return std::stol(std::string(str), idx, base);
    }

    inline unsigned long stoul(string_view str, size_t* idx = 0, int base = 10) {
      return std::stoul(std::string(str), idx, base);
    }

    inline long long stoll(string_view str, size_t* idx = 0, int base = 10) {
      return std::stoll(std::string(str), idx, base);
    }

    inline unsigned long long stoull(string_view str, size_t* idx = 0, int base = 10) {
      return std::stoull(std::string(str), idx, base);
    }

    inline float stof(string_view str, size_t* idx = 0) {
      return std::stof(std::string(str), idx);
    }

    inline double stod(string_view str, size_t* idx = 0) {
      return std::stod(std::string(str), idx);
    }

    inline long double stold(string_view str, size_t* idx = 0) {
      return std::stold(std::string(str), idx);
    }

    inline int  stoi(wstring_view str, size_t* idx = 0, int base = 10) {
      return std::stoi(std::wstring(str), idx, base);
    }

    inline long stol(wstring_view str, size_t* idx = 0, int base = 10) {
      return std::stol(std::wstring(str), idx, base);
    }

    inline unsigned long stoul(wstring_view str, size_t* idx = 0, int base = 10) {
      return std::stoul(std::wstring(str), idx, base);
    }

    inline long long stoll(wstring_view str, size_t* idx = 0, int base = 10) {
      return std::stoll(std::wstring(str), idx, base);
    }

    inline unsigned long long stoull(wstring_view str, size_t* idx = 0, int base = 10) {
      return std::stoull(std::wstring(str), idx, base);
    }

    inline float  stof(wstring_view str, size_t* idx = 0) {
      return std::stof(std::wstring(str), idx);
    }

    inline double stod(wstring_view str, size_t* idx = 0) {
      return std::stod(std::wstring(str), idx);
    }

    inline long double stold(wstring_view str, size_t* idx = 0) {
      return std::stold(std::wstring(str), idx);
    }
#endif


#if 0
  namespace std {
    // Hashing
    template<> struct hash<boost::string_view>;
    template<> struct hash<boost::u16string_view>;
    template<> struct hash<boost::u32string_view>;
    template<> struct hash<boost::wstring_view>;
  }
#endif
}

QUICKCPPLIB_NAMESPACE_END

#endif

#endif
