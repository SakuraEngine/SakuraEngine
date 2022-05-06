/* Bitwise trie algorithm
(C) 2010-2021 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
File Created: Jun 2016


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

/* This is basically a C++-ification of https://github.com/ned14/nedtries,
and some of the implementation code is lifted from that library.
*/

/* On my Windows 10 laptop with VS2019:

There are 1.99201 TSCs in 1 nanosecond and it takes 26 ticks per nanoclock().
For bitwise_trie:
   1: 700 ns per item insert
   2: 405 ns per item insert
   4: 221.25 ns per item insert
   8: 124.5 ns per item insert
   16: 86.1875 ns per item insert
   32: 54.5938 ns per item insert
   64: 40 ns per item insert
   128: 67.8438 ns per item insert
   256: 56.4453 ns per item insert
   512: 50.1895 ns per item insert
   1024: 49.541 ns per item insert
   2048: 52.668 ns per item insert
   4096: 54.4043 ns per item insert
   8192: 58.7235 ns per item insert
   16384: 65.7745 ns per item insert
   32768: 64.5418 ns per item insert
   65536: 77.3544 ns per item insert
   131072: 136.162 ns per item insert
   262144: 235.861 ns per item insert
   524288: 327.583 ns per item insert
   1048576: 487.093 ns per item insert
   2097152: 618.338 ns per item insert
   4194304: 775.045 ns per item insert
   8388608: 960.853 ns per item insert
   16777216: 1193.55 ns per item insert
   33554432: 1449.82 ns per item insert

Allocating 1879048192 bytes, 28 bytes per item seems acceptable.
For set:
   1: 360 ns per item insert
   2: 256 ns per item insert
   4: 167.75 ns per item insert
   8: 138.5 ns per item insert
   16: 116.812 ns per item insert
   32: 96.7188 ns per item insert
   64: 348.109 ns per item insert
   128: 257.531 ns per item insert
   256: 258.66 ns per item insert
   512: 179.943 ns per item insert
   1024: 143.746 ns per item insert
   2048: 135.893 ns per item insert
   4096: 127.59 ns per item insert
   8192: 127.266 ns per item insert
   16384: 137.855 ns per item insert
   32768: 158.687 ns per item insert
   65536: 177.508 ns per item insert
   131072: 268.773 ns per item insert
   262144: 321.283 ns per item insert
   524288: 438.037 ns per item insert
   1048576: 678.942 ns per item insert
   2097152: 954.575 ns per item insert
   4194304: 1229.11 ns per item insert
   8388608: 1503.02 ns per item insert
   16777216: 1830.15 ns per item insert
   33554432: 2183.64 ns per item insert

Allocating 1879048192 bytes, 28 bytes per item seems acceptable.
For unordered_set:
   1: 903 ns per item insert
   2: 580.5 ns per item insert
   4: 361.75 ns per item insert
   8: 257.625 ns per item insert
   16: 204.188 ns per item insert
   32: 168.438 ns per item insert
   64: 139.391 ns per item insert
   128: 125.227 ns per item insert
   256: 113.055 ns per item insert
   512: 107.533 ns per item insert
   1024: 104.273 ns per item insert
   2048: 104.322 ns per item insert
   4096: 101.578 ns per item insert
   8192: 101.251 ns per item insert
   16384: 98.8997 ns per item insert
   32768: 99.8262 ns per item insert
   65536: 96.0937 ns per item insert
   131072: 93.246 ns per item insert
   262144: 97.9117 ns per item insert
   524288: 107.957 ns per item insert
   1048576: 121.08 ns per item insert
   2097152: 124.938 ns per item insert
   4194304: 132.597 ns per item insert
   8388608: 141.761 ns per item insert
   16777216: 158.779 ns per item insert
   33554432: 194.535 ns per item insert
duration 134773 ms
*/

#ifndef QUICKCPPLIB_ALGORITHM_BITWISE_TRIE_HPP
#define QUICKCPPLIB_ALGORITHM_BITWISE_TRIE_HPP

#define QUICKCPPLIB_ALGORITHM_BITWISE_TRIE_DEBUG 1

#include "../declval.hpp"

#include <cassert>
#include <cstring>  // for memset
#include <iterator>
#include <type_traits>

#if __cpp_exceptions && !QUICKCPPLIB_ALGORITHM_BITWISE_TRIE_DISABLE_EXCEPTION_THROWS
#include <stdexcept>
#endif

QUICKCPPLIB_NAMESPACE_BEGIN

namespace algorithm
{
  namespace bitwise_trie
  {
    namespace detail
    {
      inline unsigned bitscanr(size_t value)
      {
        if(!value)
          return 0;
#if defined(_MSC_VER) && !defined(__cplusplus_cli)
        {
          unsigned long bitpos;
#if defined(_M_IA64) || defined(_M_X64) || defined(WIN64)
          assert(8 == sizeof(size_t));
          _BitScanReverse64(&bitpos, value);
#else
          assert(4 == sizeof(size_t));
          _BitScanReverse(&bitpos, value);
#endif
          return (unsigned) bitpos;
        }
#elif defined(__GNUC__)
        return sizeof(value) * 8 /*CHAR_BIT*/ - 1 - (unsigned) __builtin_clzl(value);
#else
          /* The following code is illegal C, but it almost certainly will work.
          If not use the legal implementation below */
#if !defined(__cplusplus_cli)
        union
        {
          unsigned asInt[2];
          double asDouble;
        };
        int n;

        asDouble = (double) value + 0.5;
        n = (asInt[0 /*Use 1 if your CPU is big endian!*/] >> 20) - 1023;
#ifdef _MSC_VER
#pragma message(__FILE__ ": WARNING: Make sure you change the line above me if your CPU is big endian!")
#else
#warning Make sure you change the line above me if your CPU is big endian!
#endif
        return (unsigned) n;
#else
#if CHAR_BIT != 8
#error CHAR_BIT is not eight, and therefore this generic bitscan routine will need adjusting!
#endif
        /* This is a generic 32 and 64 bit compatible branch free bitscan right */
        size_t x = value;
        x = x | (x >> 1);
        x = x | (x >> 2);
        x = x | (x >> 4);
        x = x | (x >> 8);
        if(16 < sizeof(x) * CHAR_BIT)
          x = x | (x >> 16);
        if(32 < sizeof(x) * CHAR_BIT)
          x = x | (x >> 32);
        x = ~x;
        x = x - ((x >> 1) & (SIZE_MAX / 3));
        x = (x & (SIZE_MAX / 15 * 3)) + ((x >> 2) & (SIZE_MAX / 15 * 3));
        x = ((x + (x >> 4)) & (SIZE_MAX / UCHAR_MAX * 15)) * (SIZE_MAX / UCHAR_MAX);
        x = (CHAR_BIT * sizeof(x) - 1) - (x >> (CHAR_BIT * (sizeof(x) - 1)));
        return (unsigned) x;
#endif
#endif
      }
      template <int Dir> struct nobble_function_implementation
      {
        template <class T> constexpr bool operator()(T &&accessors) const noexcept { return accessors.flip_nobbledir(); }
      };
      template <> struct nobble_function_implementation<-1>
      {
        template <class T> constexpr bool operator()(T && /*unused*/) const noexcept { return false; }
      };
      template <> struct nobble_function_implementation<1>
      {
        template <class T> constexpr bool operator()(T && /*unused*/) const noexcept { return true; }
      };
      template <class T, class ItemType, class = int> struct trie_sibling
      {
        static constexpr ItemType *get(const T * /*unused*/, bool /*unused*/, ItemType *r) noexcept { return r; }
        static constexpr bool set(T * /*unused*/, bool /*unused*/, ItemType * /*unused*/) noexcept { return false; }
      };
      template <class T, class ItemType> struct trie_sibling<T, ItemType, decltype((void) ItemType::trie_sibling, 0)>
      {
        static constexpr ItemType *get(const T *inst, bool right, const ItemType * /*unused*/) noexcept { return inst->trie_sibling[right]; }
        static constexpr bool set(T *inst, bool right, ItemType *x) noexcept
        {
          inst->trie_sibling[right] = x;
          return true;
        }
      };
    }  // namespace detail

    /*! \class bitwise_trie_item_accessors
    \brief Default accessor for a bitwise trie item.
    \tparam ItemType The type of item indexed.

    This default accessor requires the following member variables in the trie item type:

    - `ItemType *trie_parent`
    - `ItemType *trie_child[2]`
    - `ItemType *trie_sibling[2]`
    - `KeyType trie_key`
     */
    template <class ItemType> class bitwise_trie_item_accessors
    {
      ItemType *_v;

    public:
      constexpr bitwise_trie_item_accessors(ItemType *v)
          : _v(v)
      {
      }
      constexpr explicit operator bool() const noexcept { return _v != nullptr; }

      constexpr const ItemType *parent() const noexcept
      {
        assert(!parent_is_index());
        return _v->trie_parent;
      }
      constexpr ItemType *parent() noexcept
      {
        assert(!parent_is_index());
        return _v->trie_parent;
      }
      constexpr void set_parent(ItemType *x) noexcept { _v->trie_parent = x; }

      constexpr bool parent_is_index() const noexcept { return ((uintptr_t) _v->trie_parent & 3) == 3; }
      constexpr unsigned bit_index() const noexcept
      {
        assert(parent_is_index());
        return ((unsigned) (uintptr_t) _v->trie_parent) >> 2;
      }
      constexpr void set_parent_is_index(unsigned bit_index) noexcept { _v->trie_parent = (ItemType *) (((uintptr_t) bit_index << 2) | 3); }

      constexpr const ItemType *child(bool right) const noexcept { return _v->trie_child[right]; }
      constexpr ItemType *child(bool right) noexcept { return _v->trie_child[right]; }
      constexpr void set_child(bool right, ItemType *x) noexcept { _v->trie_child[right] = x; }

      constexpr const ItemType *sibling(bool right) const noexcept { return detail::trie_sibling<ItemType, ItemType>::get(_v, right, _v); }
      constexpr ItemType *sibling(bool right) noexcept { return detail::trie_sibling<ItemType, ItemType>::get(_v, right, _v); }
      constexpr bool set_sibling(bool right, ItemType *x) noexcept { return detail::trie_sibling<ItemType, ItemType>::set(_v, right, x); }

      constexpr auto key() const noexcept { return _v->trie_key; }

      constexpr bool is_primary_sibling() const noexcept { return _v->trie_parent != nullptr; }  // there is exactly one of these ever per key value
      constexpr void set_is_primary_sibling() noexcept { assert(_v->trie_parent != nullptr); }

      constexpr bool is_secondary_sibling() const noexcept { return _v->trie_parent == nullptr; }  // i.e. has same key as primary sibling
      constexpr void set_is_secondary_sibling() noexcept { _v->trie_parent = nullptr; }
    };

    /*! \class bitwise_trie_head_accessors
    \brief Default accessor for a bitwise trie index head.
    \tparam HeadBaseType The type from which `bitwise_trie` inherits
    \tparam ItemType The type of item indexed.

    This default accessor requires the following member variables in the trie index head type:

    - `<unsigned type> trie_count`
    - `ItemType *trie_children[8 * sizeof(<unsigned type>)]`
    - `bool trie_nobbledir` (if you use equal nobbling only)
     */
    template <class HeadBaseType, class ItemType> class bitwise_trie_head_accessors
    {
      HeadBaseType *_v;
      using _index_type = decltype(_v->trie_count);
      static_assert(std::is_unsigned<_index_type>::value, "count type must be unsigned");
      static constexpr size_t _index_type_bits = 8 * sizeof(_index_type);
      using _child_array_type = decltype(_v->trie_children);
      static_assert(sizeof(_child_array_type) / sizeof(void *) >= _index_type_bits, "children array is not big enough");
      using _key_type = decltype(declval<bitwise_trie_item_accessors<ItemType> *>()->key());

    public:
      constexpr bitwise_trie_head_accessors(HeadBaseType *v)
          : _v(v)
      {
      }
      constexpr explicit operator bool() const noexcept { return _v != nullptr; }

      constexpr _index_type size() const noexcept { return _v->trie_count; }
      constexpr void incr_size() noexcept { ++_v->trie_count; }
      constexpr void decr_size() noexcept { --_v->trie_count; }
      constexpr void set_size(_index_type x) noexcept { _v->trie_count = x; }

      constexpr _index_type max_size() const noexcept { return (_index_type) -1; }

      constexpr const ItemType *child(_index_type idx) const noexcept { return _v->trie_children[idx]; }
      constexpr ItemType *child(_index_type idx) noexcept { return _v->trie_children[idx]; }
      constexpr void set_child(_index_type idx, ItemType *x) noexcept { _v->trie_children[idx] = x; }

      constexpr _index_type lock_branch(_key_type key, bool exclusive, unsigned bitidxhint = (unsigned) -1) const noexcept  // note return type can be ANY type
      {
        (void) key;
        (void) exclusive;
        (void) bitidxhint;
        return 0;
      }
      constexpr void unlock_branch(_index_type /*unused*/, bool /*unused*/) const noexcept {}

      constexpr bool flip_nobbledir() noexcept { return (_v->trie_nobbledir = !_v->trie_nobbledir); }
    };

    /*! \class bitwise_trie
    \brief Never-allocating in-place bitwise Fredkin trie index head type.
    \tparam Base The base type from which to inherit (and thus overlay the index member functions).
    \tparam ItemType The type of item indexed.
    \tparam NobbleDir -1 to nobble zeros, +1 to nobble ones, 0 to nobble both equally (see below).

    This uses the bitwise Fredkin trie algorithm to index a collection of items by an unsigned
    integral key (e.g. a `size_t` from `std::hash`), providing identical O(log2 N) time insertion,
    removal, and finds (i.e. insert, remove and find all take identical time). It is thus most like a
    red-black tree. However it has a number of very useful characteristics which make
    it invaluable in certain use cases.

    Firstly, unlike a hash table, this algorithm requires no additional memory allocation
    whatsoever. It wholly and exclusively uses only the items added to the index, and the
    index head, for storage. This makes it invaluable for bootstrap type scenarios, such as
    in memory allocators or first boot of a kernel. It also works well with C++ exceptions
    globally disabled.

    Secondly, unlike a hash table it provides a _bounded time_ close fit find,
    which is particularly useful for where you need an item closely matching what you need,
    but you don't care if it's the *closest* matching item. An example of where this is
    super useful to have is in memory allocators, where you need a free block bigger
    than or equal to the size you are allocating.

    There is also a guaranteed closest rather than closely matching item find, however
    it is more expensive than a red-black tree whose guaranteed sortedness makes finding
    exact upper bounds easy.

    Thirdly, unlike any other algorithm, this one is much faster for low key values than
    large key values, so if you can keep most of your keys small, you will see more benefit.

    As you can see, bitwise tries have most of the same benefits of red-black trees,
    but they approximate hash tables for performance of insert-find-remove-findclosefit on
    most CPUs. This makes them very compelling where red-black trees are too slow, or
    where some concurrency is desirable (concurrent modify is easy to implement for all
    keys whose topmost set bit differs).

    The order of the items during iteration is *somewhat* sorted by key incrementing. Note the
    somewhat, it is fully ordered for each top bit set increment, but within each of
    those there is an interleave of insertion order and key increment. A bubble sort
    can be a good sort algorithm depending on relationship of key increment to insertion
    order, but you shouldn't assume it to be so without empirically checking.

    Items inserted with the same key preserve order of insertion. Performance is great
    on all CPUs which have a single cycle opcode for finding the first set bit in a
    key. If your CPU has a slow bitscan opcode (e.g. older Intel Atom), performance is
    merely good rather than great.

    The index is intrusive, as in, your types must provide the storage needed by the
    index for housekeeping. You can very tightly pack or compress or calculate those
    numbers by defining a specialised `bitwise_trie_head_accessors<Base, ItemType>`
    if you wish to customise storage of housekeeping for the trie index head; and/or
    specialise `bitwise_trie_item_accessors<ItemType>` if you wish to customise
    storage of housekeeping for each item indexed by the trie. The default
    implementations of those accessor types require various member variables prefixed
    with `trie_` in your types:

    - The default `bitwise_trie_head_accessors<Base, ItemType>` requires the following
    member variables in the trie index head type:

      - `<unsigned type> trie_count`
      - `ItemType *trie_children[8 * sizeof(<unsigned type>)]`
      - `bool trie_nobbledir` (if you use equal nobbling only)

    - The default `bitwise_trie_item_accessors<ItemType>` requires the following member
    variables in the trie item type:

      - `ItemType *trie_parent`
      - `ItemType *trie_child[2]`
      - `ItemType *trie_sibling[2]` (if you allow multiple items with the same key value only)
      - `KeyType trie_key`

    Again, I stress that the above can be completely customised and packed tighter with
    custom accessor type specialisations for your type. The tigher you can pack your
    structures, the more fits into L3 cache, and the faster everything goes. You can
    also store these in a file, and store offset pointers which are safe when a
    memory map relocates in memory.

    Close and closest fit finds always find an item whose key is larger or equal to
    the key sought. Lower bound is not implemented yet.

    Most of this implementation is lifted from https://github.com/ned14/nedtries, but
    it has been modernised for current C++ idomatic practice.

    ### Differences from a C++ container

    As this is an index, not a container, the value type is always a pointer. I chose
    pointers instead of references to aid readability i.e. it is very
    clear from reading code using this index that it is an index not a container.

    Because custom accessors may not store pointers as pointers, `reference` is not
    a reference, but also a pointer. Iteration thus yields pointers, not references.

    ### Nobble direction

    During item removal **only**, to keep the tree balanced one needs to choose which
    node to nobble. If for all the keys you use there is a surplus of zeros after the
    first set bit (this would be common for pointers), you should nobble zeros by
    setting the template parameter `NobbleDir` to `-1`. If for all the keys you use
    there is a surplus of ones after the first set bit, you should nobble ones by
    setting the template parameter `NobbleDir` to `1`.

    If for all the keys you use there is an equal balance between zeros and ones after
    the first set bit (this would be common for hashes), you will get the best results
    if you add state storage to keep a nobble direction boolean which flips between false
    and true such that nobbling is equally distributed. In this case, set `NobbleDir` to 0.

    tl;dr; If your key results from a hash function, choose `NobbleDir = 0`. If your key
    results from a pointer, or is some number which clusters on regular even boundaries,
    choose `NobbleDir = -1`.

    \todo Implement `lower_bound()`.
    */
    template <class Base, class ItemType, int NobbleDir = 0> class bitwise_trie : public Base
    {
      constexpr bitwise_trie_head_accessors<const Base, const ItemType> _head_accessors() const noexcept
      {
        return bitwise_trie_head_accessors<const Base, const ItemType>(this);
      }
      constexpr bitwise_trie_head_accessors<Base, ItemType> _head_accessors() noexcept { return bitwise_trie_head_accessors<Base, ItemType>(this); }

      static constexpr bitwise_trie_item_accessors<const ItemType> _item_accessors(const ItemType *item) noexcept
      {
        return bitwise_trie_item_accessors<const ItemType>(item);
      }
      static constexpr bitwise_trie_item_accessors<ItemType> _item_accessors(ItemType *item) noexcept { return bitwise_trie_item_accessors<ItemType>(item); }
      template <class T> static constexpr bitwise_trie_item_accessors<T> _null_item_accessors(T * /*unused*/) noexcept
      {
        return bitwise_trie_item_accessors<T>(nullptr);
      }

    public:
      //! Key type indexing the items
      using key_type = decltype(_item_accessors(static_cast<ItemType *>(nullptr)).key());
      //! The type of item indexed
      using mapped_type = ItemType *;
      //! The value type
      using value_type = ItemType *;
      //! The size type
      using size_type = decltype(bitwise_trie_head_accessors<const Base, const ItemType>(nullptr).size());
      //! The type of a difference between pointers to the type of item indexed
      using difference_type = ptrdiff_t;
      //! A reference to the type of item indexed
      using reference = value_type;
      //! A const reference to the type of item indexed
      using const_reference = const ItemType;
      //! A pointer to the type of item indexed
      using pointer = ItemType *;
      //! A const pointer to the type of item indexed
      using const_pointer = const ItemType *;
      //! The direction of nobble configured.
      static constexpr int nobble_direction = (NobbleDir < 0) ? -1 : ((NobbleDir > 0) ? 1 : 0);

    private:
      static constexpr unsigned _key_type_bits = (unsigned) (8 * sizeof(key_type));
      static_assert(std::is_unsigned<key_type>::value, "key type must be unsigned");
      static_assert(std::is_unsigned<size_type>::value, "head_accessor size type must be unsigned");

      bool _to_nobble() noexcept { return detail::nobble_function_implementation<nobble_direction>()(_head_accessors()); }

      struct _lock_unlock_branch
      {
        const bitwise_trie *_parent{nullptr};
        bool _exclusive{false};
        decltype(bitwise_trie_head_accessors<const Base, const ItemType>(nullptr).lock_branch((key_type) 0, 0)) _v;
        _lock_unlock_branch(const bitwise_trie *parent, key_type key, bool exclusive, unsigned bitidxhint = (unsigned) -1) noexcept
            : _parent(parent)
            , _exclusive(exclusive)
            , _v(parent->_head_accessors().lock_branch(key, bitidxhint))
        {
        }
        ~_lock_unlock_branch()
        {
          if(_parent != nullptr)
          {
            _parent->_head_accessors().unlock_branch(_v, _exclusive);
          }
        }
      };

      const_pointer _triemin() const noexcept
      {
        auto head = _head_accessors();
        const_pointer node = nullptr;
        if(0 == head.size())
        {
          return nullptr;
        }
        for(unsigned bitidx = 0; bitidx < _key_type_bits && nullptr == (node = head.child(bitidx)); bitidx++)
          ;
        assert(node != nullptr);
        return node;
      }
      pointer _triemin() noexcept { return const_cast<pointer>(static_cast<const bitwise_trie *>(this)->_triemin()); }
      const_pointer _triemax() const noexcept
      {
        auto head = _head_accessors();
        const_pointer node = 0, child;
        if(0 == head.size())
        {
          return nullptr;
        }
        unsigned bitidx = _key_type_bits - 1;
        for(; bitidx < _key_type_bits && nullptr == (node = head.child(bitidx)); bitidx--)
          ;
        assert(node != nullptr);
        auto nodelink = _item_accessors(node);
        _lock_unlock_branch lock_unlock(this, nodelink.key(), false, bitidx);
        while(nullptr != (child = (nodelink.child(true) != nullptr) ? nodelink.child(true) : nodelink.child(false)))
        {
          node = child;
          nodelink = _item_accessors(node);
        }
        /* Now go to end leaf */
        if(nodelink.sibling(false) != node)
        {
          return nodelink.sibling(false);
        }
        return node;
      }
      pointer _triemax() noexcept { return const_cast<pointer>(static_cast<const bitwise_trie *>(this)->_triemax()); }

      pointer _trieinsert(pointer r) noexcept
      {
        auto head = _head_accessors();
        if(head.size() >= head.max_size() - 1)
        {
          return nullptr;
        }

        pointer node = nullptr;
        auto nodelink = _item_accessors(node);
        auto rlink = _item_accessors(r);
        key_type rkey = rlink.key();

        rlink.set_parent(nullptr);
        rlink.set_child(false, nullptr);
        rlink.set_child(true, nullptr);
        rlink.set_sibling(false, r);
        rlink.set_sibling(true, r);
        unsigned bitidx = detail::bitscanr(rkey);
        /* Avoid unknown bit shifts where possible, their performance can suck */
        key_type keybit = (key_type) 1 << bitidx;
        assert(bitidx < _key_type_bits);
        if(nullptr == (node = head.child(bitidx)))
        { /* Set parent is index flag */
          rlink.set_parent_is_index(bitidx);
          head.set_child(bitidx, r);
          head.incr_size();
          return r;
        }
        _lock_unlock_branch lock_unlock(this, rkey, true, bitidx);
        for(pointer childnode = nullptr;; node = childnode)
        {
          nodelink = _item_accessors(node);
          key_type nodekey = nodelink.key();
          if(nodekey == rkey)
          { /* Insert into end of ring list */
#if 0
            {
              auto *left = nodelink.sibling(false), *right = nodelink.sibling(true);
              assert(left->trie_key == node->trie_key);
              assert(right->trie_key == node->trie_key);
              {
                auto link = _item_accessors(right);
                while(link.is_secondary_sibling())
                {
                  assert(_item_accessors(link.sibling(true)).sibling(false) == right);
                  right = link.sibling(true);
                  assert(right->trie_key == node->trie_key);
                  link = _item_accessors(right);
                }
                assert(_item_accessors(link.sibling(true)).sibling(false) == right);
              }
              {
                auto link = _item_accessors(left);
                while(link.is_secondary_sibling())
                {
                  assert(_item_accessors(link.sibling(false)).sibling(true) == left);
                  left = link.sibling(false);
                  assert(left->trie_key == node->trie_key);
                  link = _item_accessors(left);
                }
                assert(_item_accessors(link.sibling(false)).sibling(true) == left);
              }
              assert(left == right);
            }
#endif
            rlink.set_is_secondary_sibling();
            if(!rlink.set_sibling(true, node))
            {
              return node;
            }
            auto *newest_sibling = nodelink.sibling(false);
            _item_accessors(newest_sibling).set_sibling(true, r);
            rlink.set_sibling(false, newest_sibling);
            nodelink.set_sibling(false, r);
#if 0
            {
              auto *left = rlink.sibling(false), *right = rlink.sibling(true);
              assert(left->trie_key == r->trie_key);
              assert(right->trie_key == r->trie_key);
              {
                auto link = _item_accessors(right);
                while(link.is_secondary_sibling())
                {
                  assert(_item_accessors(link.sibling(true)).sibling(false) == right);
                  right = link.sibling(true);
                  assert(right->trie_key == r->trie_key);
                  link = _item_accessors(right);
                }
                assert(_item_accessors(link.sibling(true)).sibling(false) == right);
              }
              {
                auto link = _item_accessors(left);
                while(link.is_secondary_sibling())
                {
                  assert(_item_accessors(link.sibling(false)).sibling(true) == left);
                  left = link.sibling(false);
                  assert(left->trie_key == r->trie_key);
                  link = _item_accessors(left);
                }
                assert(_item_accessors(link.sibling(false)).sibling(true) == left);
              }
              assert(left == right);
            }
            {
              auto *left = nodelink.sibling(false), *right = nodelink.sibling(true);
              assert(left->trie_key == node->trie_key);
              assert(right->trie_key == node->trie_key);
              {
                auto link = _item_accessors(right);
                while(link.is_secondary_sibling())
                {
                  assert(_item_accessors(link.sibling(true)).sibling(false) == right);
                  right = link.sibling(true);
                  assert(right->trie_key == node->trie_key);
                  link = _item_accessors(right);
                }
                assert(_item_accessors(link.sibling(true)).sibling(false) == right);
              }
              {
                auto link = _item_accessors(left);
                while(link.is_secondary_sibling())
                {
                  assert(_item_accessors(link.sibling(false)).sibling(true) == left);
                  left = link.sibling(false);
                  assert(left->trie_key == node->trie_key);
                  link = _item_accessors(left);
                }
                assert(_item_accessors(link.sibling(false)).sibling(true) == left);
              }
              assert(left == right);
            }
#endif
            break;
          }
          keybit >>= 1;
          const bool keybitset = !!(rkey & keybit);
          childnode = nodelink.child(keybitset);
          if(nullptr == childnode)
          { /* Insert here */
            rlink.set_parent(node);
            rlink.set_is_primary_sibling();
            nodelink.set_child(keybitset, r);
            break;
          }
        }
        head.incr_size();
        return r;
      }

      void _trieremove(pointer r) noexcept
      {
        auto head = _head_accessors();

        pointer node = nullptr;
        auto nodelink = _item_accessors(node);
        auto rlink = _item_accessors(r);
        _lock_unlock_branch lock_unlock(this, rlink.key(), true);

        /* Am I a leaf off the tree? */
        if(rlink.is_secondary_sibling())
        { /* Remove from linked list */
          assert(rlink.parent() == nullptr);
#if 0
          {
            auto *left = rlink.sibling(false), *right = rlink.sibling(true);
            assert(left->trie_key == r->trie_key);
            assert(right->trie_key == r->trie_key);
            {
              auto link = _item_accessors(right);
              while(link.is_secondary_sibling())
              {
                assert(_item_accessors(link.sibling(true)).sibling(false) == right);
                right = link.sibling(true);
                assert(right->trie_key == r->trie_key);
                link = _item_accessors(right);
              }
              assert(_item_accessors(link.sibling(true)).sibling(false) == right);
            }
            {
              auto link = _item_accessors(left);
              while(link.is_secondary_sibling())
              {
                assert(_item_accessors(link.sibling(false)).sibling(true) == left);
                left = link.sibling(false);
                assert(left->trie_key == r->trie_key);
                link = _item_accessors(left);
              }
              assert(_item_accessors(link.sibling(false)).sibling(true) == left);
            }
            assert(left == right);
          }
#endif
          auto *left = rlink.sibling(false), *right = rlink.sibling(true);
          nodelink = _item_accessors(left);
          nodelink.set_sibling(true, right);
          nodelink = _item_accessors(right);
          nodelink.set_sibling(false, left);
#if 0
          {
            auto link = _item_accessors(right);
            while(link.is_secondary_sibling())
            {
              assert(_item_accessors(link.sibling(true)).sibling(false) == right);
              right = link.sibling(true);
              assert(right->trie_key == r->trie_key);
              link = _item_accessors(right);
            }
            assert(_item_accessors(link.sibling(true)).sibling(false) == right);
          }
          {
            auto link = _item_accessors(left);
            while(link.is_secondary_sibling())
            {
              assert(_item_accessors(link.sibling(false)).sibling(true) == left);
              left = link.sibling(false);
              assert(left->trie_key == r->trie_key);
              link = _item_accessors(left);
            }
            assert(_item_accessors(link.sibling(false)).sibling(true) == left);
          }
          assert(left == right);
#endif
          head.decr_size();
#ifndef NDEBUG
          rlink.set_parent(nullptr);
          rlink.set_child(false, nullptr);
          rlink.set_child(true, nullptr);
          rlink.set_sibling(false, r);
          rlink.set_sibling(true, r);
#endif
          return;
        }
        /* I must therefore be part of the tree */
        assert(rlink.parent_is_index() || rlink.parent() != nullptr);
        assert(rlink.is_primary_sibling());
        auto set_parent = [&]() {  // sets rlink's parent to node, and node's parent to rlink's parent
          // Set the replacement node to my children, and their parent to the new node
          pointer childnode;
          if(nullptr != (childnode = rlink.child(false)))
          {
            auto childnodelink = _item_accessors(childnode);
            nodelink.set_child(false, childnode);
            childnodelink.set_parent(node);
          }
          if(nullptr != (childnode = rlink.child(true)))
          {
            auto childnodelink = _item_accessors(childnode);
            nodelink.set_child(true, childnode);
            childnodelink.set_parent(node);
          }
          // Set the replacement node parent to my parent, and its child
          if(rlink.parent_is_index())
          {
            /* Extract my bitidx */
            unsigned bitidx = rlink.bit_index();
            assert(head.child(bitidx) == r);
            head.set_child(bitidx, node);
            nodelink.set_parent_is_index(bitidx);
          }
          else
          {
            auto parentlink = _item_accessors(rlink.parent());
            if(parentlink.child(false) == r)
            {
              parentlink.set_child(false, node);
            }
            else
            {
              assert(parentlink.child(true) == r);
              parentlink.set_child(true, node);
            }
            nodelink.set_parent(rlink.parent());
          }
          nodelink.set_is_primary_sibling();
#ifndef NDEBUG
          assert(node->trie_key == nodelink.sibling(false)->trie_key);
          rlink.set_parent(nullptr);
          rlink.set_child(false, nullptr);
          rlink.set_child(true, nullptr);
          rlink.set_sibling(false, r);
          rlink.set_sibling(true, r);
#endif
        };
        /* Can I replace me with a sibling? */
        if(rlink.sibling(true) != r)
        {
#if 0
          {
            auto *left = rlink.sibling(false), *right = rlink.sibling(true);
            assert(left->trie_key == r->trie_key);
            assert(right->trie_key == r->trie_key);
            {
              auto link = _item_accessors(right);
              while(link.is_secondary_sibling())
              {
                assert(_item_accessors(link.sibling(true)).sibling(false) == right);
                right = link.sibling(true);
                assert(right->trie_key == r->trie_key);
                link = _item_accessors(right);
              }
              assert(_item_accessors(link.sibling(true)).sibling(false) == right);
            }
            {
              auto link = _item_accessors(left);
              while(link.is_secondary_sibling())
              {
                assert(_item_accessors(link.sibling(false)).sibling(true) == left);
                left = link.sibling(false);
                assert(left->trie_key == r->trie_key);
                link = _item_accessors(left);
              }
              assert(_item_accessors(link.sibling(false)).sibling(true) == left);
            }
            assert(left == right);
          }
#endif
          auto *left = rlink.sibling(false), *right = rlink.sibling(true);
          nodelink = _item_accessors(left);
          nodelink.set_sibling(true, right);
          nodelink = _item_accessors(right);
          nodelink.set_sibling(false, left);
#if 0
          nodelink.set_parent_is_index(1);
          {
            auto link = _item_accessors(right);
            while(link.is_secondary_sibling())
            {
              assert(_item_accessors(link.sibling(true)).sibling(false) == right);
              right = link.sibling(true);
              assert(right->trie_key == r->trie_key);
              link = _item_accessors(right);
            }
            assert(_item_accessors(link.sibling(true)).sibling(false) == right);
          }
          {
            auto link = _item_accessors(left);
            while(link.is_secondary_sibling())
            {
              assert(_item_accessors(link.sibling(false)).sibling(true) == left);
              left = link.sibling(false);
              assert(left->trie_key == r->trie_key);
              link = _item_accessors(left);
            }
            assert(_item_accessors(link.sibling(false)).sibling(true) == left);
          }
          assert(left == right);
#endif
          node = right;
          set_parent();
          head.decr_size();
          return;
        }
        /* Can I simply remove myself from my parent? */
        if(nullptr == rlink.child(false) && nullptr == rlink.child(true))
        {
          if(rlink.parent_is_index())
          {
            /* Extract my bitidx */
            unsigned bitidx = rlink.bit_index();
            assert(head.child(bitidx) == r);
            head.set_child(bitidx, nullptr);
          }
          else
          {
            auto parentlink = _item_accessors(rlink.parent());
            if(parentlink.child(false) == r)
            {
              parentlink.set_child(false, nullptr);
            }
            else
            {
              assert(parentlink.child(true) == r);
              parentlink.set_child(true, nullptr);
            }
          }
          head.decr_size();
#ifndef NDEBUG
          rlink.set_parent(nullptr);
          rlink.set_child(false, nullptr);
          rlink.set_child(true, nullptr);
          rlink.set_sibling(false, r);
          rlink.set_sibling(true, r);
#endif
          return;
        }
        /* I need someone to replace me in the trie, so simply find any
           grandchild of mine (who has the right bits to be here) which has no children.
        */
        const bool nobbledir = _to_nobble();
        bool parentchildidx;
        pointer childnode;
        if(rlink.child(nobbledir) != nullptr)
        {
          childnode = rlink.child(nobbledir);
          parentchildidx = nobbledir;
        }
        else
        {
          childnode = rlink.child(!nobbledir);
          parentchildidx = !nobbledir;
        }
        auto childnodelink = _item_accessors(childnode);
        for(;;)
        {
          if(nullptr != childnodelink.child(nobbledir))
          {
            childnode = childnodelink.child(nobbledir);
            parentchildidx = nobbledir;
            childnodelink = _item_accessors(childnode);
            continue;
          }
          if(nullptr != childnodelink.child(!nobbledir))
          {
            childnode = childnodelink.child(!nobbledir);
            parentchildidx = !nobbledir;
            childnodelink = _item_accessors(childnode);
            continue;
          }
          break;
        }
        // Detach this grandchild from its parent
        _item_accessors(childnodelink.parent()).set_child(parentchildidx, nullptr);
        node = childnode;
        nodelink = childnodelink;
        // Set my parent to point at the replacement node
        set_parent();
        head.decr_size();
      }

      static const_pointer _triebranchprev(const_pointer r, bitwise_trie_item_accessors<const ItemType> *rlinkaddr = nullptr) noexcept
      {
        const_pointer node = nullptr, child = nullptr;
        auto nodelink = _item_accessors(node);
        auto rlink = _item_accessors(r);

        /* Am I a leaf off the tree? */
        if(rlink.is_secondary_sibling())
        {
          return rlink.sibling(false);
        }
        /* Trace up my parents to prev branch */
        while(!rlink.parent_is_index())
        {
          node = rlink.parent();
          nodelink = _item_accessors(node);
          /* If I was on child[1] and there is a child[0], go to bottom of child[0] */
          if(nodelink.child(true) == r && nodelink.child(false) != nullptr)
          {
            node = nodelink.child(false);
            nodelink = _item_accessors(node);
            /* Follow child[1] preferentially downwards */
            while(nullptr != (child = (nodelink.child(true) != nullptr) ? nodelink.child(1) : nodelink.child(0)))
            {
              node = child;
              nodelink = _item_accessors(node);
            }
          }
          /* If I was already on child[0] or there are no more children, return this node */
          /* Now go to end leaf */
          if(nodelink.sibling(false) != node)
          {
            return nodelink.sibling(false);
          }
          return node;
        }
        /* I have reached the top of my trie, no more on this branch */
        if(rlinkaddr != nullptr)
        {
          *rlinkaddr = rlink;
        }
        return nullptr;
      }
      const_pointer _trieprev(const_pointer r) const noexcept
      {
        auto head = _head_accessors();
        const_pointer node = nullptr, child = nullptr;
        auto nodelink = _item_accessors(node);
        auto rlink = _item_accessors(r);
        _lock_unlock_branch lock_unlock(this, rlink.key(), false);

        rlink = _null_item_accessors(r);
        if((node = _triebranchprev(r, &rlink)) != nullptr || !rlink)
        {
          return node;
        }
        /* I have reached the top of my trie, so on to prev bin */
        unsigned bitidx = rlink.bit_index();
        assert(head.child(bitidx) == r);
        for(bitidx--; bitidx < _key_type_bits && nullptr == (node = head.child(bitidx)); bitidx--)
          ;
        if(bitidx >= _key_type_bits)
        {
          return nullptr;
        }
        nodelink = _item_accessors(node);
        /* Follow child[1] preferentially downwards */
        while(nullptr != (child = (nodelink.child(1) != nullptr) ? nodelink.child(1) : nodelink.child(0)))
        {
          node = child;
          nodelink = _item_accessors(node);
        }
        /* Now go to end leaf */
        if(nodelink.sibling(false) != node)
        {
          return nodelink.sibling(false);
        }
        return node;
      }
      pointer _trieprev(const_pointer r) noexcept { return const_cast<pointer>(static_cast<const bitwise_trie *>(this)->_trieprev(r)); }

      static const_pointer _triebranchnext(const_pointer r, bitwise_trie_item_accessors<const ItemType> *rlinkaddr = nullptr) noexcept
      {
        const_pointer node = nullptr;
        auto nodelink = _item_accessors(node);
        auto rlink = _item_accessors(r);

        /* Am I a leaf off the tree? */
        if(rlink.sibling(true) != node)
        {
          r = rlink.sibling(true);
          rlink = _item_accessors(r);
          if(!rlink.is_primary_sibling())
          {
            return r;
          }
        }
        /* Follow my children, preferring child[0] */
        if(nullptr != (node = (rlink.child(false) != nullptr) ? rlink.child(false) : rlink.child(true)))
        {
          nodelink = _item_accessors(node);
          assert(nodelink.parent() == r);
          return node;
        }
        /* Trace up my parents to next branch */
        while(!rlink.parent_is_index())
        {
          node = rlink.parent();
          nodelink = _item_accessors(node);
          if(nodelink.child(false) == r && nodelink.child(true) != nullptr)
          {
            return nodelink.child(true);
          }
          r = node;
          rlink = nodelink;
        }
        /* I have reached the top of my trie, no more on this branch */
        if(rlinkaddr != nullptr)
        {
          *rlinkaddr = rlink;
        }
        return nullptr;
      }
      const_pointer _trienext(const_pointer r) const noexcept
      {
        auto head = _head_accessors();
        const_pointer node = nullptr;
        auto rlink = _item_accessors(r);
        _lock_unlock_branch lock_unlock(this, rlink.key(), false);

        rlink = _null_item_accessors(r);
        if((node = _triebranchnext(r, &rlink)) != nullptr)
        {
          return node;
        }
        /* I have reached the top of my trie, so on to next bin */
        unsigned bitidx = rlink.bit_index();
        for(bitidx++; bitidx < _key_type_bits && nullptr == (node = head.child(bitidx)); bitidx++)
          ;
        if(bitidx >= _key_type_bits)
        {
          return nullptr;
        }
        return node;
      }
      pointer _trienext(const_pointer r) noexcept { return const_cast<pointer>(static_cast<const bitwise_trie *>(this)->_trienext(r)); }

      pointer _triefind(key_type rkey) const noexcept
      {
        auto head = _head_accessors();
        if(0 == head.size())
        {
          return nullptr;
        }

        const_pointer node = nullptr;

        unsigned bitidx = detail::bitscanr(rkey);
        /* Avoid unknown bit shifts where possible, their performance can suck */
        key_type keybit = (key_type) 1 << bitidx;
        assert(bitidx < _key_type_bits);
        if(nullptr == (node = head.child(bitidx)))
        {
          return nullptr;
        }
        _lock_unlock_branch lock_unlock(this, rkey, false, bitidx);
        for(const_pointer childnode = nullptr;; node = childnode)
        {
          auto nodelink = _item_accessors(node);
          key_type nodekey = nodelink.key();
          if(nodekey == rkey)
          {
            return const_cast<pointer>(node);
          }
          keybit >>= 1;
          const bool keybitset = !!(rkey & keybit);
          childnode = nodelink.child(keybitset);
          if(childnode == nullptr)
          {
            return nullptr;
          }
        }
        return nullptr;
      }
      /* The algorithm for this is the most complex in here, so it's worth documenting
      as it also documents the others.

      Firstly to describe the trie, it is a binary tree switching on each bit in the
      key: if the bit is zero, we go left, if one, we go right. In parallel to this,
      each node also stores a value, so during traversal down the tree we may well
      encounter the value we seek on our way. We thus get two orderings during a
      traversal, the first is somewhat randomly ordered, but we are guaranteed that
      each node is ordered with respect to the bit switches which occurred previous
      to encountering it i.e. the bit switches from the top of each branch will match
      that of the value encountered. The second ordering is by key increasing from
      left in the binary tree towards the right.

      If you think this through, for an equidistributed key, approximately half
      the time keys will be found during traversal downwards, the other half of
      the time keys will found at the end of traversal downwards in the leaf node.

      To create a close find for some given key value, we traverse down the trie,
      same as we would for exact find. If we exact find, we return immediately.
      Otherwise we reach the point at which the key would be inserted as a new leaf.

      From the perspective of the second ordering, every leaf node to the right
      of this point will contain values greater than our origin point, so by
      traversing right one will find all the values greater than our search key,
      by the second ordering. One therefore just traverses right until one finds
      the next rightmost leaf key.

      But, in the path between our origin point and the first rightmost leaf key,
      closer values to the search key may be encountered. We still need to find
      the first rightmost leaf key, as that is our maximum bound, but in so doing
      we should encounter all possible values between our search value and the
      value of that first rightmost leaf key.

      It's a bit hard to estimate the complexities of this operation with rounds
      approximating infinity. For an equidistributed key, the average should not
      exceed O(log2 N) where N is the number of items with the same top bit set.
      That's a worst case however. To estimate the average case, I still can't
      do better than the original nedtries documentation from 2010: it depends
      somewhat upon consecutive distance between keys. So, if the origin point
      is at X, and we need to find the first rightmost leaf key and there are
      say a run of six consecutive difference bits between the search key and
      the next rightmost leaf key, then we would have to traverse three parents
      upwards and three parents downwards to find the first rightmost leaf key.
      However the probability of there being six consecutive difference bits is
      identical to the probability of there being any other sequence of six bits,
      so the probability of there being two consecutive same bits is 0.25, of
      three consecutive same bits 0.125, of six consecutive same bits 0.015625.
      Therefore, most of the time, there will be very little parent traversal.

      It was a long time ago I did pure maths, but I vaguely remember that
      summing a series of 0.25 + 0.125 + 0.0625 ... approaches 0.5. That's
      a linear multiplier of bit difference which is one, so I'm going to
      suggest the average complexity of nearest find is O(1) for an
      equidistributed key. Email or other communications proving I am wrong is
      welcomed.

      Obviously, in the real world, this algorithm will mostly stall on
      main memory just like any binary tree if the tree is not in CPU cache.
      This is why bitwise Fredkin tries almost exactly approximate a 2x
      performance increase over red-black trees, because they are half as
      deep for the same item count, and therefore there are half as many main
      memory stalls, for large item counts. For smaller item counts,
      branch predictors since Haswell are basically statistical bitfield
      predictors, so you see O(1) performance similar to a hash table. Put
      simply, if all your binary tree fits inside L2 cache, the CPU can do
      four chained indirections in a similar time to a single indirection.
      */
      pointer _trieCfind(key_type rkey, int64_t rounds) const noexcept
      {
        auto head = _head_accessors();
        if(0 == head.size())
        {
          return 0;
        }

        const_pointer node = nullptr, ret = nullptr;

        unsigned bitidx = detail::bitscanr(rkey);
        assert(bitidx < _key_type_bits);
        do
        {
          /* Keeping raising the bin until we find a larger key */
          while(bitidx < _key_type_bits && nullptr == (node = head.child(bitidx)))
          {
            bitidx++;
            rkey = (key_type) 1 << bitidx;
          }
          if(bitidx >= _key_type_bits)
          {
            return nullptr;
          }
          /* Avoid unknown bit shifts where possible, their performance can suck */
          key_type keybit = (key_type) 1 << bitidx;
          _lock_unlock_branch lock_unlock(this, keybit, false, bitidx);
          key_type retkey = (key_type) -1;
          /* Find where we would insert this key */
          auto nodelink = _item_accessors(node);
          bool keybitset;
          for(const_pointer childnode = nullptr;; node = childnode, nodelink = _item_accessors(node))
          {
            auto nodekey = nodelink.key();
            /* If nodekey is a closer fit to search key, mark as best result so far */
            if(nodekey >= rkey && nodekey - rkey < retkey)
            {
              ret = node;
              retkey = nodekey - rkey;
            }
            if(ret != nullptr)
            {
              --rounds;
            }
            if((retkey == 0 || rounds <= 0) && ret != nullptr)
            {
              return const_cast<pointer>(ret);
            }
            /* Which child branch should we check? */
            keybit >>= 1;
            keybitset = !!(rkey & keybit);
            childnode = nodelink.child(keybitset);
            if(childnode == nullptr)
            {
              break;
            }
          }
          /* node now points at where we would insert the key.
          Find the rightmost leaf.
          */
          if(keybitset == false && nodelink.child(true) != nullptr)
          {
            node = nodelink.child(true);
            do
            {
              nodelink = _item_accessors(node);
              auto nodekey = nodelink.key();
              if(nodekey >= rkey && nodekey - rkey < retkey)
              {
                ret = node;
                retkey = nodekey - rkey;
              }
              if(ret != nullptr)
              {
                --rounds;
              }
              if(rounds <= 0 && ret != nullptr)
              {
                return const_cast<pointer>(ret);
              }
              node = (nodelink.child(false) != nullptr) ? nodelink.child(false) : nodelink.child(true);
            } while(node != nullptr);
            // We are at the rightmost leaf, so we have by now encountered the closest possible value
            return const_cast<pointer>(ret);
          }
          auto *origin = node;
          auto originlink = nodelink;
          while(!originlink.parent_is_index())
          {
            node = originlink.parent();
            nodelink = _item_accessors(node);
            auto nodekey = nodelink.key();
            if(nodekey >= rkey && nodekey - rkey < retkey)
            {
              ret = node;
              retkey = nodekey - rkey;
            }
            if(ret != nullptr)
            {
              --rounds;
            }
            if(rounds <= 0 && ret != nullptr)
            {
              return const_cast<pointer>(ret);
            }
            if(nodelink.child(false) == origin && nodelink.child(true) != nullptr)
            {
              node = nodelink.child(true);
              do
              {
                nodelink = _item_accessors(node);
                nodekey = nodelink.key();
                if(nodekey >= rkey && nodekey - rkey < retkey)
                {
                  ret = node;
                  retkey = nodekey - rkey;
                }
                if(ret != nullptr)
                {
                  --rounds;
                }
                node = (nodelink.child(false) != nullptr) ? nodelink.child(false) : nodelink.child(true);
              } while(node != nullptr);
              // We are at the rightmost leaf, so we have by now encountered the closest possible value
              return const_cast<pointer>(ret);
            }
            else
            {
              origin = node;
              originlink = nodelink;
            }
          }
          /* Move up a branch, resetting key sought to the smallest
          possible for that branch */
          bitidx++;
          rkey = (key_type) 1 << bitidx;
        } while(nullptr == ret);
        return const_cast<pointer>(ret);
      }

#ifndef NDEBUG
      struct _trie_validity_state
      {
        size_type count, tops, lefts, rights, leafs;
        key_type smallestkey, largestkey;
      };

      static void _triecheckvaliditybranch(const_pointer node, key_type bitidx, _trie_validity_state &state) noexcept
      {
        auto nodelink = _item_accessors(node);
        key_type nodekey = nodelink.key();

        if(nodekey < state.smallestkey)
        {
          state.smallestkey = nodekey;
        }
        if(nodekey > state.largestkey)
        {
          state.largestkey = nodekey;
        }
        assert(nodelink.parent() != nullptr);
        auto *child = nodelink.parent();
        auto childlink = _item_accessors(child);
        assert(childlink.child(0) == node || childlink.child(1) == node);
        assert(node == childlink.child(!!(nodekey & ((key_type) 1 << bitidx))));
        while(_item_accessors(child = nodelink.sibling(true)).is_secondary_sibling())
        {
          state.leafs++;
          childlink = _item_accessors(child);
          assert(nullptr == childlink.parent());
          assert(nullptr == childlink.child(0));
          assert(nullptr == childlink.child(1));
          assert(child == _item_accessors(childlink.sibling(false)).sibling(true));
          assert(child == _item_accessors(childlink.sibling(true)).sibling(false));
          nodelink = childlink;
          state.count++;
        }
        nodelink = _item_accessors(node);
        state.count++;
        if(nodelink.child(0) != nullptr)
        {
          state.lefts++;
          _triecheckvaliditybranch(nodelink.child(0), bitidx - 1, state);
        }
        if(nodelink.child(1) != nullptr)
        {
          state.rights++;
          _triecheckvaliditybranch(nodelink.child(1), bitidx - 1, state);
        }
      }
#endif
    public:
      void triecheckvalidity(const key_type *tocheck = nullptr) const noexcept
      {
        (void) tocheck;
#ifndef NDEBUG
        auto head = _head_accessors();
        const_pointer node = nullptr, child = nullptr;
        _trie_validity_state state;
        memset(&state, 0, sizeof(state));
        for(unsigned n = 0; n < _key_type_bits; n++)
        {
          if(tocheck != nullptr)
          {
            if((((key_type) -1 << n) & *tocheck) != ((key_type) 1 << n))
            {
              continue;
            }
          }
          if((node = head.child(n)) != nullptr)
          {
            auto nodelink = _item_accessors(node);
            key_type nodekey = nodelink.key();
            _lock_unlock_branch lock_unlock(this, nodekey, false, n);
            state.tops++;
            auto bitidx = nodelink.bit_index();
            assert(bitidx == n);
            assert(head.child(bitidx) == node);
            assert(0 == nodekey || ((((size_t) -1) << bitidx) & nodekey) == ((size_t) 1 << bitidx));
            while(_item_accessors(child = nodelink.sibling(true)).is_secondary_sibling())
            {
              state.leafs++;
              auto childlink = _item_accessors(child);
              assert(nullptr == childlink.parent());
              assert(nullptr == childlink.child(false));
              assert(nullptr == childlink.child(true));
              assert(child == _item_accessors(childlink.sibling(false)).sibling(true));
              assert(child == _item_accessors(childlink.sibling(true)).sibling(false));
              nodelink = childlink;
              state.count++;
            }
            nodelink = _item_accessors(node);
            state.count++;
            if(nodelink.child(0) != nullptr)
            {
              state.lefts++;
              state.smallestkey = (key_type) -1;
              state.largestkey = 0;
              _triecheckvaliditybranch(nodelink.child(0), bitidx - 1, state);
              assert(0 == state.smallestkey || state.smallestkey >= (key_type) 1 << bitidx);
              assert(bitidx + 1 >= _key_type_bits || state.largestkey < (key_type) 1 << (bitidx + 1));
            }
            if(nodelink.child(1) != nullptr)
            {
              state.rights++;
              state.smallestkey = (key_type) -1;
              state.largestkey = 0;
              _triecheckvaliditybranch(nodelink.child(1), bitidx - 1, state);
              assert(state.smallestkey >= (key_type) 1 << bitidx);
              assert(bitidx + 1 >= _key_type_bits || state.largestkey < (key_type) 1 << (bitidx + 1));
            }
          }
        }
        if(tocheck == nullptr)
        {
          assert(state.count == head.size());
        }
#if 1
        for(state.count = 0, node = _triemin(); node != nullptr; (node = _trienext(node)), state.count++)
#if 0
      printf("%p\n", node)
#endif
          ;
        if(state.count != head.size())
        {
          assert(state.count == head.size());
        }
#if 0
    printf("\n");
#endif
        for(state.count = 0, node = _triemax(); node != nullptr; (node = _trieprev(node)), state.count++)
#if 0
      printf("%p\n", node)
#endif
          ;
        if(state.count != head.size())
        {
          assert(state.count == head.size());
        }
#if 0
    printf("\n");
#endif
#if !defined(NDEBUG) && 0
        if(count > 50)
          printf("Of count %u, tops %.2lf%%, lefts %.2lf%%, rights %.2lf%%, leafs %.2lf%%\n", count, 100.0 * tops / count, 100.0 * lefts / count,
                 100.0 * rights / count, 100.0 * leafs / count);
#endif
#endif /* !NDEBUG */
#endif
      }

    private:
      template <bool is_const, class Parent, class Pointer, class Reference> class iterator_
      {
        friend class bitwise_trie;
        template <bool _is_const, class _Parent, class _Pointer, class _Reference> friend class iterator_;
        Parent *_parent{nullptr};
        Pointer _p{nullptr};

        iterator_ &_inc() noexcept
        {
          if(_parent != nullptr && _p != nullptr)
          {
            _p = _parent->_trienext(_p);
          }
          return *this;
        }
        iterator_ &_dec() noexcept
        {
          if(_parent != nullptr)
          {
            if(_p == nullptr)
            {
              _p = _parent->_triemax();
            }
            else
            {
              _p = _parent->_trieprev(_p);
            }
          }
          return *this;
        }

        constexpr iterator_(const Parent *parent, Pointer p) noexcept
            : _parent(const_cast<Parent *>(parent))
            , _p(p)
        {
        }
        constexpr iterator_(const Parent *parent) noexcept
            : _parent(const_cast<Parent *>(parent))
            , _p(nullptr)
        {
        }

        struct _implicit_nonconst_to_const_conversion
        {
        };
        struct _explicit_const_to_nonconst_conversion
        {
        };

      public:
        using difference_type = typename Parent::difference_type;
        using value_type = typename Parent::value_type;
        using pointer = Pointer;
        using reference = Reference;
        using iterator_category = std::bidirectional_iterator_tag;
        constexpr iterator_() noexcept
            : _parent(nullptr)
            , _p(nullptr)
        {
        }
        constexpr iterator_(const iterator_ &) = default;
        constexpr iterator_(iterator_ &&) noexcept = default;
        constexpr iterator_ &operator=(const iterator_ &) = default;
        constexpr iterator_ &operator=(iterator_ &&) noexcept = default;
        // Implicit non-const to const iterator
        QUICKCPPLIB_TEMPLATE(class _Parent, class _Pointer, class _Reference)
        QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(is_const &&std::is_same<typename std::remove_const<Parent>::type, _Parent>::value))
        constexpr iterator_(const iterator_<false, _Parent, _Pointer, _Reference> &o, _implicit_nonconst_to_const_conversion = {}) noexcept
            : _parent(o._parent)
            , _p(o._p)
        {
        }
        // Explicit const to non-const iterator
        QUICKCPPLIB_TEMPLATE(class _Parent, class _Pointer, class _Reference)
        QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(!is_const && std::is_same<typename std::remove_const<_Parent>::type, Parent>::value))
        explicit constexpr iterator_(const iterator_<true, _Parent, _Pointer, _Reference> &o, _explicit_const_to_nonconst_conversion = {}) noexcept
            : _parent(const_cast<Parent *>(o._parent))
            , _p(const_cast<Pointer>(o._p))
        {
        }
        void swap(iterator_ &o) noexcept
        {
          std::swap(_parent, o._parent);
          std::swap(_p, o._p);
        }

        explicit operator bool() const noexcept { return _parent != nullptr && _p != nullptr; }
        bool operator!() const noexcept { return _parent == nullptr || _p == nullptr; }
        Pointer operator->() noexcept { return _p; }
        const Pointer operator->() const noexcept { return _p; }
        bool operator==(const iterator_ &o) const noexcept { return _parent == o._parent && _p == o._p; }
        bool operator!=(const iterator_ &o) const noexcept { return _parent != o._parent || _p != o._p; }
        Reference operator*() noexcept
        {
          if(_parent == nullptr || _p == nullptr)
          {
            abort();
          }
          return _p;
        }
        const Reference operator*() const noexcept
        {
          if(_parent == nullptr || _p == nullptr)
          {
            abort();
          }
          return _p;
        }
        iterator_ &operator++() noexcept { return _inc(); }
        iterator_ operator++(int) noexcept
        {
          iterator_ ret(*this);
          ++*this;
          return ret;
        }
        iterator_ &operator--() noexcept { return _dec(); }
        iterator_ operator--(int) noexcept
        {
          iterator_ ret(*this);
          --*this;
          return ret;
        }
        bool operator<(const iterator_ &o) const noexcept
        {
          if(_parent != o._parent)
          {
            abort();
          }
          return _p != nullptr && (o._p == nullptr || _p < o._p);
        }
        bool operator>(const iterator_ &o) const noexcept
        {
          if(_parent != o._parent)
          {
            abort();
          }
          return o._p != nullptr && (_p == nullptr || _p > o._p);
        }
        bool operator<=(const iterator_ &o) const noexcept
        {
          if(_parent != o._parent)
          {
            abort();
          }
          return !(o > *this);
        }
        bool operator>=(const iterator_ &o) const noexcept
        {
          if(_parent != o._parent)
          {
            abort();
          }
          return !(o < *this);
        }
      };

    public:
      //! The iterator type
      using iterator = iterator_<false, bitwise_trie, pointer, reference>;
      //! The const iterator type
      using const_iterator = iterator_<true, const bitwise_trie, const_pointer, const_reference>;
      //! The reverse iterator type
      using reverse_iterator = std::reverse_iterator<iterator>;
      //! The const reverse iterator type
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      static_assert(std::is_convertible<iterator, const_iterator>::value, "iterator is not implicitly convertible to const_iterator");
      static_assert(!std::is_convertible<const_iterator, iterator>::value, "iterator is implicitly convertible to const_iterator");
      static_assert(std::is_constructible<iterator, const_iterator>::value, "iterator is not explicitly constructible from const_iterator");

      constexpr bitwise_trie() { clear(); }
      bitwise_trie(const bitwise_trie &o) noexcept
      {
        auto myhead = _head_accessors();
        auto ohead = o._head_accessors();
        for(unsigned n = 0; n < _key_type_bits; n++)
        {
          myhead.set_child(n, ohead.child(n));
        }
        myhead.set_size(ohead.size());
      }
      bitwise_trie &operator=(const bitwise_trie &o) noexcept
      {
        auto myhead = _head_accessors();
        auto ohead = o._head_accessors();
        for(unsigned n = 0; n < _key_type_bits; n++)
        {
          myhead.set_child(n, ohead.child(n));
        }
        myhead.set_size(ohead.size());
        return *this;
      }

      //! Swaps the contents of the index
      void swap(bitwise_trie &o) noexcept
      {
        auto myhead = _head_accessors();
        auto ohead = o._head_accessors();
        for(unsigned n = 0; n < _key_type_bits; n++)
        {
          auto t = myhead.child(n);
          myhead.set_child(n, ohead.child(n));
          ohead.set_child(n, t);
        }
        auto t = myhead.size();
        myhead.set_size(ohead.size());
        ohead.set_size(t);
      }

      //! True if the bitwise trie is empty
      QUICKCPPLIB_NODISCARD constexpr bool empty() const noexcept { return size() == 0; }
      //! Returns the number of items in the bitwise trie
      constexpr size_type size() const noexcept { return _head_accessors().size(); }
      //! Returns the maximum number of items in the index
      constexpr size_type max_size() const noexcept { return _head_accessors().max_size(); }

      //! Returns the front of the index.
      reference front() noexcept
      {
        if(auto p = _triemin())
        {
          return *p;
        }
        abort();
      }
      //! Returns the front of the index.
      const_reference front() const noexcept
      {
        if(auto p = _triemin())
        {
          return *p;
        }
        abort();
      }
      //! Returns the back of the index.
      reference back() noexcept
      {
        if(auto p = _triemax())
        {
          return *p;
        }
        abort();
      }
      //! Returns the back of the index.
      const_reference back() const noexcept
      {
        if(auto p = _triemax())
        {
          return *p;
        }
        abort();
      }

      //! Returns an iterator to the first item in the index.
      iterator begin() noexcept
      {
        if(auto p = _triemin())
        {
          return iterator(this, p);
        }
        return iterator(this);
      }
      //! Returns an iterator to the first item in the index.
      const_iterator begin() const noexcept
      {
        if(auto p = _triemin())
        {
          return const_iterator(this, p);
        }
        return const_iterator(this);
      }
      //! Returns an iterator to the last item in the index.
      reverse_iterator rbegin() noexcept
      {
        if(auto p = _triemax())
        {
          return reverse_iterator(iterator(this, p));
        }
        return reverse_iterator(iterator(this));
      }
      //! Returns an iterator to the last item in the index.
      const_reverse_iterator rbegin() const noexcept
      {
        if(auto p = _triemax())
        {
          return const_reverse_iterator(const_iterator(this, p));
        }
        return const_reverse_iterator(const_iterator(this));
      }
      //! Returns an iterator to the first item in the index.
      const_iterator cbegin() const noexcept { return begin(); }
      //! Returns an iterator to the last item in the index.
      const_reverse_iterator crbegin() const noexcept { return rbegin(); }
      //! Returns an iterator to the item after the last in the index.
      iterator end() noexcept { return iterator(this); }
      //! Returns an iterator to the item before the first in the index.
      const_iterator end() const noexcept { return const_iterator(this); }
      //! Returns an iterator to the item before the first in the index.
      reverse_iterator rend() noexcept { return reverse_iterator(iterator(this)); }
      //! Returns an iterator to the item before the first in the index.
      const_reverse_iterator rend() const noexcept { return const_reverse_iterator(const_iterator(this)); }
      //! Returns an iterator to the item after the last in the index.
      const_iterator cend() const noexcept { return const_iterator(this); }
      //! Returns an iterator to the item before the first in the index.
      const_reverse_iterator crend() const noexcept { return const_reverse_iterator(const_iterator(this)); }

      //! Clears the index.
      constexpr void clear() noexcept
      {
        auto head = _head_accessors();
        for(unsigned n = 0; n < _key_type_bits; n++)
        {
          head.set_child(n, nullptr);
        }
        head.set_size(0);
      }
      //! Return how many items with key there are.
      size_type count(key_type k) const noexcept { return count(find(k)); }
      //! Return how many items with the same key as iterator there are.
      size_type count(const_iterator it) const noexcept
      {
        if(it != end())
        {
          size_type ret = 1;
          auto plink = _item_accessors(it._p);
          for(auto *i = plink.sibling(true); i != it._p; i = _item_accessors(i).sibling(true))
          {
            ret++;
          }
          return ret;
        }
        return 0;
      }
      /*! Inserts a new item, returning an iterator to the new item if the key is new. If there
      is an item with that key already, if sibling storage is enabled, insert a new item with
      the same key. If sibling storage is disabled, return an iterator to the existing item.

      If the maximum number of items has been inserted, if C++ exceptions are disabled
      or `QUICKCPPLIB_ALGORITHM_BITWISE_TRIE_DISABLE_EXCEPTION_THROWS != 0`, the returned iterator
      is invalid if there is no more space. Otherwise it throws `std::length_error`.
      */
      iterator insert(pointer p)
      {
        if(size() == max_size())
        {
#if __cpp_exceptions && !QUICKCPPLIB_ALGORITHM_BITWISE_TRIE_DISABLE_EXCEPTION_THROWS
          throw std::length_error("too many items");
#else
          return end();
#endif
        }
        p = _trieinsert(p);
        if(p != nullptr)
        {
          return iterator(this, p);
        }
        return end();
      }
      //! Erases an item.
      iterator erase(const_iterator it) noexcept
      {
        if(it == end())
        {
          assert(it != end());
          return end();
        }
        iterator ret(this, const_cast<pointer>(it._p));
        ++ret;
        _trieremove(const_cast<pointer>(it._p));
        return ret;
      }
      //! Erases an item.
      iterator erase(key_type k) noexcept { return erase(find(k)); }
      //! Finds an item
      iterator find(key_type k) const noexcept
      {
        if(auto p = _triefind(k))
        {
          return iterator(this, p);
        }
        return iterator(this);
      }
      /*! Finds either an item with identical key, or an item with a larger key. The higher the value in `rounds`,
      the less average distance between the larger key and the key requested. The complexity of this function
      is bound by `rounds`.

      Some useful statistics about `rounds` values for five million ideally distributed keys:

      - `rounds = 17` returns an item with key 99.99% close to the ideal key.
      - `rounds = 12` returns an item with key 99.9% close to the ideal key.
      - `rounds = 7` returns an item with key 99.1% close to the ideal key.
      - `rounds = 3` returns an item with key 95.1% close to the ideal key.
      - `rounds = 2` returns an item with key 92% close to the ideal key.
      */
      iterator find_equal_or_larger(key_type k, int64_t rounds) const noexcept
      {
        if(auto p = _trieCfind(k, rounds))
        {
          return iterator(this, p);
        }
        return iterator(this);
      }
      //! Finds either an item with identical key, or an item with the guaranteed next largest key. This is
      //! identical to `close_find(k, INT64_MAX)` and its average case complexity is `O(log N)` where `N` is
      //! the number of items in the index with the same top bit set. This is equivalent to `upper_bound(k - 1)`.
      iterator find_equal_or_next_largest(key_type k) const noexcept
      {
        if(auto p = _trieCfind(k, INT64_MAX))
        {
          return iterator(this, p);
        }
        return iterator(this);
      }
      //! Finds the item next larger than the key. This is equivalent to `find_equal_or_next_largest(k + 1)`.
      iterator upper_bound(key_type k) const noexcept
      {
        if(auto p = _trieCfind(k + 1, INT64_MAX))
        {
          return iterator(this, p);
        }
        return iterator(this);
      }
      //! Estimate the item next larger than the key. This is equivalent to `find_equal_or_larger(k + 1, rounds)`.
      iterator upper_bound_estimate(key_type k, int64_t rounds) const noexcept
      {
        if(auto p = _trieCfind(k + 1, rounds))
        {
          return iterator(this, p);
        }
        return iterator(this);
      }
      //! True if the index contains the key
      bool contains(key_type k) const noexcept { return nullptr != _triefind(k); }
      //! Returns a reference to the specified element, aborting if key not found.
      reference operator[](key_type k) noexcept
      {
        if(auto p = _triefind(k))
        {
          return *p;
        }
        abort();
      }
      //! Returns a reference to the specified element, aborting if key not found.
      const_reference operator[](const key_type &k) const noexcept
      {
        if(auto p = _triefind(k))
        {
          return *p;
        }
        abort();
      }
    };
  }  // namespace bitwise_trie
}  // namespace algorithm

QUICKCPPLIB_NAMESPACE_END

#endif
