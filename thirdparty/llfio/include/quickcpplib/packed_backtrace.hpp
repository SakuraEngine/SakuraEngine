/* Space packed stack backtrace
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (21 commits)
File Created: Jun 2017


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

#ifndef QUICKCPPLIB_PACKED_BACKTRACE_HPP
#define QUICKCPPLIB_PACKED_BACKTRACE_HPP

#include "config.hpp"
#include "span.hpp"

#include <cstddef>  // for ptrdiff_t etc
#include <cstdint>  // for uint32_t etc
#include <cstdlib>  // for abort()
#include <cstring>  // for memset

QUICKCPPLIB_NAMESPACE_BEGIN

namespace packed_backtrace
{
  namespace detail
  {
    template <class T> struct constify
    {
      using type = const T;
    };
    template <class T> struct constify<T *>
    {
      using type = const T *;
    };
  }
  namespace impl
  {
    template <class FramePtrType, size_t FrameTypeSize> class packed_backtrace
#ifndef DOXYGEN_IS_IN_THE_HOUSE
    {
      using storage_type = span::span<FramePtrType>;
      storage_type _storage;

    protected:
      explicit packed_backtrace(span::span<const char> storage)
          : _storage(reinterpret_cast<FramePtrType *>(const_cast<char *>(storage.data())), storage.size() / sizeof(FramePtrType))
      {
      }
      explicit packed_backtrace(span::span<char> storage, std::nullptr_t)
          : _storage(reinterpret_cast<FramePtrType *>(storage.data()), storage.size() / sizeof(FramePtrType))
      {
      }

    public:
      //! The type stored in the container
      using value_type = typename storage_type::element_type;
      //! The const type stored in the container
      using const_value_type = typename detail::constify<typename storage_type::element_type>::type;
      //! The size type
      using size_type = size_t;
      //! The difference type
      using difference_type = typename storage_type::difference_type;
      //! The reference type
      using reference = typename storage_type::reference;
      //! The const reference type
      using const_reference = typename storage_type::const_reference;
      //! The pointer type
      using pointer = typename storage_type::pointer;
      //! The const pointer type
      using const_pointer = typename storage_type::const_pointer;
      //! The iterator type
      using iterator = typename storage_type::iterator;
      //! The const iterator type
      using const_iterator = decltype(std::cbegin(std::declval<const storage_type>()));
      //! The reverse iterator type
      using reverse_iterator = typename storage_type::reverse_iterator;
      //! The const reverse iterator type
      using const_reverse_iterator = decltype(std::crbegin(std::declval<const storage_type>()));

      //! Returns true if the index is empty
      bool empty() const noexcept { return _storage.empty(); }
      //! Returns the number of items in the backtrace
      size_type size() const noexcept { return _storage.size(); }
      //! Returns the maximum number of items in the backtrace
      size_type max_size() const noexcept { return _storage.size(); }
      //! Returns an iterator to the first item in the backtrace.
      iterator begin() noexcept { return _storage.begin(); }
      //! Returns an iterator to the first item in the backtrace.
      const_iterator begin() const noexcept { return _storage.begin(); }
      //! Returns an iterator to the first item in the backtrace.
      const_iterator cbegin() const noexcept { return begin(); }
      //! Returns an iterator to the item after the last in the backtrace.
      iterator end() noexcept { return _storage.end(); }
      //! Returns an iterator to the item after the last in the backtrace.
      const_iterator end() const noexcept { return _storage.end(); }
      //! Returns an iterator to the item after the last in the backtrace.
      const_iterator cend() const noexcept { return end(); }
      //! Returns the specified element, unchecked.
      value_type operator[](size_type idx) const noexcept { return _storage[idx]; }
      //! Returns the specified element, checked.
      value_type at(size_type idx) const { return _storage.at(idx); }
      //! Swaps with another instance
      void swap(packed_backtrace &o) noexcept { _storage.swap(o._storage); }

      //! Assigns a raw stack backtrace to the packed storage
      void assign(span::span<const_value_type> input) noexcept { memcpy(_storage.data(), input.data(), _storage.size_bytes()); }
    };
    template <class FramePtrType> class packed_backtrace<FramePtrType, 8>
#endif
    {
      using storage_type = span::span<uint8_t>;
      using uintptr_type = std::conditional_t<sizeof(uintptr_t) >= 8, uintptr_t, uint64_t>;
      
      static constexpr uintptr_type bits63_43 = ~((1ULL << 43) - 1);
      static constexpr uintptr_type bit20 = (1ULL << 20);
      static constexpr uintptr_type bits20_0 = ((1ULL << 21) - 1);
      static constexpr uintptr_type bits42_21 = ~(bits63_43 | bits20_0);
      static constexpr uintptr_type bits42_0 = ((1ULL << 43) - 1);

      storage_type _storage;
      size_t _count;  // number of decoded items

      template <size_t signbit> static intptr_t _sign_extend(uintptr_type x) noexcept
      {
#if 1
        const size_t m = (63 - signbit);
        intptr_t v = static_cast<intptr_t>(x << m);
        return v >> m;
#else
        uintptr_type m = 1ULL << signbit;
        uintptr_type y = (x ^ m) - m;
        return static_cast<intptr_t>(y);
#endif
      }
      bool _decode(uintptr_type &out, size_t &idx) const noexcept
      {
        bool first = true;
        for(;;)
        {
          if(idx >= _storage.size())
            return false;
          uint8_t t = _storage[idx++];
          switch(t >> 6)
          {
          case 3:
          {
            if(idx > _storage.size() - 3)  // 22 bit payload
              return false;
            // Replace bits 63-43, keeping bits 42-0
            out &= bits42_0;
            out |= ((uintptr_type)(t & 0x3f) << 59);
            out |= (uintptr_type) _storage[idx++] << 51;
            out |= (uintptr_type) _storage[idx++] << 43;
            break;
          }
          case 2:
          {
            if(idx > _storage.size() - 3)  // 22 bit payload
              return false;
            // Replace bits 42-21, setting bit 20, zeroing 19-0, keeping bits 63-43
            out &= bits63_43;
            out |= bit20;
            out |= ((uintptr_type)(t & 0x3f) << 37);
            out |= (uintptr_type) _storage[idx++] << 29;
            out |= (uintptr_type) _storage[idx++] << 21;
            break;
          }
          case 1:
          {
            if(idx > _storage.size() - 3)  // 22 bit payload
              return false;
            // Offset bits 21-0
            uintptr_type offset = ((uintptr_type)(t & 0x3f) << 16);
            offset |= (uintptr_type) _storage[idx++] << 8;
            offset |= (uintptr_type) _storage[idx++] << 0;
            out += _sign_extend<21>(offset);
            return true;
          }
          case 0:
          {
            if(idx > _storage.size() - 2)  // 14 bit payload
              return false;
            if(first && !t)
            {
              // If he's all bits zero from now until end of storage, we are done
              bool done = true;
              for(size_t _idx = idx; _idx < _storage.size(); ++_idx)
              {
                if(_storage[_idx] != 0)
                {
                  done = false;
                  break;
                }
              }
              if(done)
                return false;
            }
            // Offset bits 13-0
            uintptr_type offset = ((uintptr_type)(t & 0x3f) << 8);
            offset |= (uintptr_type) _storage[idx++] << 0;
            out += _sign_extend<13>(offset);
            return true;
          }
          }
          first = false;
        }
      }
      size_t _decode_count() const noexcept
      {
        uintptr_type out = 0;
        size_t idx = 0, ret = 0;
        while(_decode(out, idx))
        {
          ++ret;
        }
        return ret;
      }

    protected:
      explicit packed_backtrace(span::span<const char> storage)
          : _storage(reinterpret_cast<uint8_t *>(const_cast<char *>(storage.data())), storage.size())
          , _count(_decode_count())
      {
      }
      explicit packed_backtrace(span::span<char> storage, std::nullptr_t)
          : _storage(reinterpret_cast<uint8_t *>(storage.data()), storage.size())
          , _count(0)
      {
      }

    public:
      //! The type stored in the container
      using value_type = FramePtrType;
      //! The const type stored in the container
      using const_value_type = typename detail::constify<FramePtrType>::type;
      //! The size type
      using size_type = size_t;
      //! The difference type
      using difference_type = ptrdiff_t;
      //! The reference type
      using reference = FramePtrType;
      //! The const reference type
      using const_reference = const FramePtrType;
      //! The pointer type
      using pointer = FramePtrType *;
      //! The const pointer type
      using const_pointer = const FramePtrType *;
      //! The iterator type
      class iterator
      {
        friend class packed_backtrace;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename packed_backtrace::value_type;
        using difference_type = typename packed_backtrace::difference_type;
        using pointer = typename packed_backtrace::pointer;
        using reference = typename packed_backtrace::reference;

      private:
        packed_backtrace *_parent;
        size_t _idx;
        value_type _v;

        iterator &_inc() noexcept
        {
          if(_parent)
          {
            uintptr_type v = reinterpret_cast<uintptr_type>(_v);
            if(_parent->_decode(v, _idx))
              _v = reinterpret_cast<value_type>(v);
            else
            {
              _parent = nullptr;
              _idx = (size_t) -1;
              _v = nullptr;
            }
          }
          return *this;
        }

        iterator(packed_backtrace *parent) noexcept : _parent(parent), _idx(0), _v(nullptr) { _inc(); }

      public:
        constexpr iterator() noexcept : _parent(nullptr), _idx((size_t) -1), _v(nullptr) {}
        iterator(const iterator &) = default;
        iterator(iterator &&) noexcept = default;
        iterator &operator=(const iterator &) = default;
        iterator &operator=(iterator &&) noexcept = default;
        void swap(iterator &o) noexcept
        {
          std::swap(_parent, o._parent);
          std::swap(_idx, o._idx);
          std::swap(_v, o._v);
        }

        bool operator==(const iterator &o) const noexcept { return _parent == o._parent && _idx == o._idx; }
        bool operator!=(const iterator &o) const noexcept { return _parent != o._parent || _idx != o._idx; }
        value_type operator*() noexcept
        {
          if(!_parent)
          {
            abort();
          }
          return _v;
        }
        const value_type operator*() const noexcept
        {
          if(!_parent)
          {
            abort();
          }
          return _v;
        }
        iterator &operator++() noexcept { return _inc(); }
        iterator operator++(int) noexcept
        {
          iterator ret(*this);
          ++*this;
          return ret;
        }
        bool operator<(const iterator &o) const noexcept
        {
          if(!_parent && !o._parent)
            return false;
          if(!_parent && o._parent)
            return true;
          if(_parent && !o._parent)
            return false;
          return _idx < o._idx;
        }
        bool operator>(const iterator &o) const noexcept
        {
          if(!_parent && !o._parent)
            return false;
          if(!_parent && o._parent)
            return false;
          if(_parent && !o._parent)
            return true;
          return _idx > o._idx;
        }
        bool operator<=(const iterator &o) const noexcept { return !(o > *this); }
        bool operator>=(const iterator &o) const noexcept { return !(o < *this); }
      };
      friend class iterator;
      //! The const iterator type
      using const_iterator = iterator;
      //! The reverse iterator type
      using reverse_iterator = std::reverse_iterator<iterator>;
      //! The const reverse iterator type
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      //! Returns true if the index is empty
      bool empty() const noexcept { return _storage.empty(); }
      //! Returns the number of items in the backtrace
      size_type size() const noexcept { return _count; }
      //! Returns the maximum number of items in the backtrace
      size_type max_size() const noexcept { return _count; }
      //! Returns an iterator to the first item in the backtrace.
      iterator begin() noexcept { return iterator(this); }
      //! Returns an iterator to the first item in the backtrace.
      const_iterator begin() const noexcept { return iterator(this); }
      //! Returns an iterator to the first item in the backtrace.
      const_iterator cbegin() const noexcept { return iterator(this); }
      //! Returns an iterator to the item after the last in the backtrace.
      iterator end() noexcept { return iterator(); }
      //! Returns an iterator to the item after the last in the backtrace.
      const_iterator end() const noexcept { return iterator(); }
      //! Returns an iterator to the item after the last in the backtrace.
      const_iterator cend() const noexcept { return iterator(); }
      //! Returns the specified element, unchecked.
      value_type operator[](size_type i) const noexcept
      {
        uintptr_type out = 0;
        size_t idx = 0;
        for(size_type n = 0; n <= i; n++)
        {
          if(!_decode(out, idx))
            return nullptr;
        }
        return reinterpret_cast<value_type>(out);
      }
      //! Returns the specified element, checked.
      value_type at(size_type i) const
      {
        uintptr_type out = 0;
        size_t idx = 0;
        for(size_type n = 0; n <= i; n++)
        {
          if(!_decode(out, idx))
          {
            abort();  // out of range
          }
        }
        return reinterpret_cast<value_type>(out);
      }
      //! Swaps with another instance
      void swap(packed_backtrace &o) noexcept
      {
        using std::swap;
        swap(_storage, o._storage);
        swap(_count, o._count);
      }

      //! Assigns a raw stack backtrace to the packed storage
      void assign(span::span<const_value_type> input) noexcept
      {
        uintptr_type out = 0;
        size_t idx = 0;
        memset(_storage.data(), 0, _storage.size());
        _count = 0;
        for(const auto &_i : input)
        {
          size_t startidx = idx;
          uintptr_type i = reinterpret_cast<uintptr_type>(_i);
          uintptr_type delta = i & bits63_43;
          if((out & bits63_43) != delta)
          {
            if(idx > _storage.size() - 3)
              return;
            // std::cout << "For entry " << _i << " encoding bits 63-43: " << (void *) delta << std::endl;
            _storage[idx++] = 0xc0 | ((delta >> 59) & 0x3f);
            _storage[idx++] = (delta >> 51) & 0xff;
            _storage[idx++] = (delta >> 43) & 0xff;
            out &= bits42_0;
            out |= delta;
          }
          delta = i & bits42_21;
          if((out & bits42_21) != delta)
          {
            if(idx > _storage.size() - 3)
            {
              memset(_storage.data() + startidx, 0, idx - startidx);
              return;
            }
            // std::cout << "For entry " << _i << " encoding bits 42-21: " << (void *) delta << std::endl;
            _storage[idx++] = 0x80 | ((delta >> 37) & 0x3f);
            _storage[idx++] = (delta >> 29) & 0xff;
            _storage[idx++] = (delta >> 21) & 0xff;
            out &= bits63_43;
            out |= bit20;
            out |= delta;
          }
          if(i - out >= (1 << 13) && out - i >= (1 << 13))
          {
            if(idx > _storage.size() - 3)
            {
              memset(_storage.data() + startidx, 0, idx - startidx);
              return;
            }
            delta = static_cast<uintptr_type>(i - out);
            // std::cout << "For entry " << _i << " with diff " << (intptr_t) delta << " encoding three byte delta: " << (void *) delta << std::endl;
            _storage[idx++] = 0x40 | ((delta >> 16) & 0x3f);
            _storage[idx++] = (delta >> 8) & 0xff;
            _storage[idx++] = (delta >> 0) & 0xff;
            out = i;
          }
          else
          {
            if(idx > _storage.size() - 2)
            {
              memset(_storage.data() + startidx, 0, idx - startidx);
              return;
            }
            delta = static_cast<uintptr_type>(i - out);
            // std::cout << "For entry " << _i << " with diff " << (intptr_t) delta << " encoding two byte delta: " << (void *) delta << std::endl;
            _storage[idx++] = (delta >> 8) & 0x3f;
            _storage[idx++] = (delta >> 0) & 0xff;
            out = i;
          }
          _count++;
        }
      }
    };
  }
  /*! \class packed_backtrace
  \brief A space packed stack backtrace letting you store twice or more
  stack frames in the same space.
  \tparam FramePtrType The type each stack backtrace frame ought to be represented as.

  \note Use `make_packed_backtrace()` to create one of these from a raw backtrace.
  Construct an instance on a byte span to load in the packed data so you can parse it.

  64 bit address stack backtraces tend to waste a lot of storage which can be a problem
  when storing lengthy backtraces. Most 64 bit architectures only use the first 43 bits
  of address space wasting 2.5 bytes per entry, plus stack backtraces tend to be within
  a few megabytes and even kilobytes from one another. Intelligently packing the bits
  based on this knowledge can double or more the number of items you can store for a
  given number of bytes with virtually no runtime overhead, unlike compression.

  On 32 bit architectures this class simply stores the stack normally, but otherwise
  works the same.

  Performance:
  - GCC 7.1 on x64 3.1Ghz Skylake: Can construct and read 106188139 packed stacktraces/sec
  -  VS2017 on x64 3.1Ghz Skylake: Can construct and read  79755133 packed stacktraces/sec

  The 64-bit coding scheme is quite straightforward:

  - Top bits are 11 when it's bits 63-43 of a 64 bit absolute address (3 bytes)
  - Top bits are 10 when it's bits 42-21 of a 64 bit absolute address (3 bytes)
    - Note that this resets bits 20-0 to 0x100000 (bit 20 set, bits 19-0 cleared)
  - Top bits are 01 when it's a 22 bit offset from previous (3 bytes) (+- 0x40`0000, 4Mb)
  - Top bits are 00 when it's a 14 bit offset from previous (2 bytes) (+- 0x4000, 16Kb)
  - Potential improvement: 12 to 18 items in 40 bytes instead of 5 items

  Sample 1:
  ~~~
  0000`07fe`fd4e`10ac - 6 bytes
  0000`07fe`f48b`ffc7 - 3 bytes
  0000`07fe`f48b`ff70 - 2 bytes
  0000`07fe`f48b`fe23 - 2 bytes
  0000`07fe`f48d`51d8 - 3 bytes
  0000`07fe`f499`5249 - 3 bytes
  0000`07fe`f48a`ef28 - 3 bytes
  0000`07fe`f48a`ecc9 - 2 bytes
  0000`07fe`f071`244c - 6 bytes
  0000`07fe`f071`11b5 - 2 bytes
  0000`07ff`0015`0acf - 6 bytes
  0000`07ff`0015`098c - 2 bytes (40 bytes, 12 items, usually 96 bytes, 58% reduction)
  ~~~

  Sample 2:
  ~~~
  0000003d06e34950 - 6 bytes
  0000000000400bcd - 6 bytes
  0000000000400bf5 - 2 bytes
  0000003d06e1ffe0 - 6 bytes
  00000000004009f9 - 6 bytes (26 bytes, 5 items, usually 40 bytes, 35% reduction)
  ~~~
  */
  template <class FramePtrType = void *> class packed_backtrace : public impl::packed_backtrace<FramePtrType, sizeof(FramePtrType)>
  {
    using base = impl::packed_backtrace<FramePtrType, sizeof(FramePtrType)>;

  public:
    //! \brief Construct a packed backtrace view, parsing the given byte storage
    explicit packed_backtrace(span::span<const char> storage)
        : base(storage)
    {
    }
    //! \brief Construct a packed backtrace view, not parsing the given byte storage
    explicit packed_backtrace(span::span<char> storage, std::nullptr_t)
        : base(storage, nullptr)
    {
    }
    //! \brief Default copy constructor
    packed_backtrace(const packed_backtrace &) = default;
    //! \brief Default move constructor
    packed_backtrace(packed_backtrace &&) = default;
    //! \brief Default copy assignment
    packed_backtrace &operator=(const packed_backtrace &) = default;
    //! \brief Default move assignment
    packed_backtrace &operator=(packed_backtrace &&) = default;
  };

  //! \brief Pack a stack backtrace into byte storage
  inline packed_backtrace<void *> make_packed_backtrace(span::span<char> output, span::span<const void *> input)
  {
    packed_backtrace<void *> ret(output, nullptr);
    ret.assign(input);
    return ret;
  }
}

QUICKCPPLIB_NAMESPACE_END

#endif
