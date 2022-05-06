/* Very fast threadsafe unordered lookup
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Aug 2016


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

#ifndef QUICKCPPLIB_ALGORITHM_OPEN_HASH_INDEX_HPP
#define QUICKCPPLIB_ALGORITHM_OPEN_HASH_INDEX_HPP

#include "../spinlock.hpp"

#include <cstddef>  // for ptrdiff_t etc
#include <cstdint>  // for uint32_t etc
#include <cstdlib>  // for abort()

QUICKCPPLIB_NAMESPACE_BEGIN

namespace algorithm
{

  namespace open_hash_index
  {
    namespace detail
    {
      template <size_t limit, class Container, class U> auto linear_find(Container &c, size_t hint, U &&pred) -> decltype(&c[0])
      {
        if(pred(c[hint]))
          return &c[hint];
        size_t idx1 = hint, idx2 = hint;
        for(size_t n = 0; n < limit; n++)
        {
          --idx1;
          ++idx2;
          if(idx2 < c.size() && pred(c[idx2]))
            return &c[idx2];
          if(idx1 < c.size() && pred(c[idx1]))
            return &c[idx1];
        }
        return nullptr;
      }
      template <class T> auto look_for_lock_shared(int) -> typename std::enable_if<std::is_same<decltype(std::declval<T>().lock_shared()), decltype(std::declval<T>().lock_shared())>::value, std::true_type>::type;
      template <class T> std::false_type look_for_lock_shared(...);
      template <class T> struct is_shared_mutex : decltype(look_for_lock_shared<T>(0))
      {
      };
      template <bool lock_is_shared> struct lock_shared_if
      {
        template <class T> lock_shared_if(T &lock) { lock.lock(); }
      };
      template <> struct lock_shared_if<true>
      {
        template <class T> lock_shared_if(T &lock) { lock.lock_shared(); }
      };
      template <bool lock_is_shared> struct unlock_shared_if
      {
        template <class T> unlock_shared_if(T &lock) { lock.unlock(); }
      };
      template <> struct unlock_shared_if<true>
      {
        template <class T> unlock_shared_if(T &lock) { lock.unlock_shared(); }
      };
    }

    //! Performs k % divisor which is up to 40 CPU cycles depending on architecture
    template <class KeyType> struct arithmetic_modulus
    {
      size_t operator()(const KeyType &k, size_t divisor) const noexcept { return divisor ? static_cast<size_t>(k % divisor) : 0; }
    };
    //! Performs k & (divisor-1) which is typically one or two CPU cycles. Suitable if divisor is always going to be a two's power.
    template <class KeyType> struct twos_power_modulus
    {
      size_t operator()(const KeyType &k, size_t divisor) const noexcept { return divisor ? static_cast<size_t>(k & (divisor - 1)) : 0; }
    };

    /*! \struct linear_memory_policy
    \brief The simplest possible open_hash_index memory policy, with just the key, value and a boolean marking if the item is in use.
    A linear scan is performed up to LinearSearchLimit either side of any collision.
    \tparam KeyType The type of key to use. Must be a trivial type which acts as if an unsigned integer type.
    \tparam T The type of mapped value to use.
    \tparam LinearSearchLimit How far to search either side of a collison for an entry or an empty slot.
    \tparam KeyModulus A callable which performs modulus of the KeyType with the storage container's size. If you know the container's
    size will always be a power of two, use `twos_power_modulus` instead of `arithmetic_modulus`.
    \tparam KeyCompare A callable which compares two KeyTypes.
    */
    template <class KeyType, class T, size_t LinearSearchLimit = 2, class KeyModulus = arithmetic_modulus<KeyType>, class KeyCompare = std::equal_to<KeyType>> struct linear_memory_policy
    {
      using key_type = typename std::add_const<KeyType>::type;
      using mapped_type = T;
      struct value_type
      {
        using first_type = key_type;
        using second_type = mapped_type;
        key_type first;
        bool _inuse;
        mapped_type second;
        constexpr value_type()
            : first()
            , _inuse(false)
            , second()
        {
        }
        template <class U, class V>
        constexpr value_type(U &&f, V &&s)
            : first(std::forward<U>(f))
            , _inuse(false)
            , second(std::forward<V>(s))
        {
        }
        template <class U, class V>
        constexpr value_type(const std::pair<U, V> &v)
            : first(v.first)
            , _inuse(false)
            , second(v.second)
        {
        }
        template <class U, class V>
        constexpr value_type(std::pair<U, V> &&v)
            : first(std::move(v.first))
            , _inuse(false)
            , second(std::move(v.second))
        {
        }
        value_type(value_type &&o) noexcept : first(std::move(o.first)), _inuse(o._inuse), second(std::move(o.second)) {}
        value_type &operator=(value_type &&o) noexcept
        {
          this->~value_type();
          new(this) value_type(std::move(o));
          return *this;
        }
        constexpr bool operator==(const value_type &o) const noexcept { return first == o.first && second == o.second; }
        constexpr bool operator!=(const value_type &o) const noexcept { return first != o.first || second != o.second; }
        constexpr bool operator<(const value_type &o) const noexcept { return first < o.first || (first == o.first && second < o.second); }
        constexpr bool operator<=(const value_type &o) const noexcept { return first < o.first || (first == o.first && second <= o.second); }
        constexpr bool operator>(const value_type &o) const noexcept { return o < *this; }
        constexpr bool operator>=(const value_type &o) const noexcept { return !(*this < o); }
      };
      using size_type = size_t;
      using difference_type = ptrdiff_t;
      using reference = value_type &;
      using const_reference = const value_type &;
      using pointer = value_type *;
      using const_pointer = const value_type *;
      using items_count_type = size_t;

      // True if item is in use
      static bool is_inuse(const value_type *p) noexcept { return p->_inuse; }
      // Returns a pointer to an item if it is in use
      static pointer if_inuse_to_pointer(value_type *p) noexcept { return pointer(p->_inuse ? p : nullptr); }
      // Returns a pointer to an item if it is in use
      static const_pointer if_inuse_to_pointer(const value_type *p) noexcept { return const_pointer(p->_inuse ? p : nullptr); }
      // Looks up a key in the contiguous container returning a value_type_ptr to any value_type found
      template <class Container> static pointer find_exclusive(Container &c, const key_type &k) noexcept
      {
        auto count = c.size();
        if(!count)
          return nullptr;
        return detail::linear_find<LinearSearchLimit>(c, KeyModulus()(k, count), [&k](const_reference v) { return v._inuse && KeyCompare()(k, v.first); });
      }
      // Looks up a key in the contiguous container returning a value_type_ptr to any value_type found
      template <class Container> static const_pointer find_shared(const Container &c, const key_type &k) noexcept
      {
        auto count = c.size();
        if(!count)
          return nullptr;
        return detail::linear_find<LinearSearchLimit>(c, KeyModulus()(k, count), [&k](const_reference v) { return v._inuse && KeyCompare()(k, v.first); });
      }
      // Inserts an item into the contiguous container returning a value_type_ptr to any value_type found
      template <class Container> static pointer insert(Container &c, value_type &&newv) noexcept
      {
        return detail::linear_find<LinearSearchLimit>(c, KeyModulus()(newv.first, c.size()), [&newv](reference v) {
          if(v._inuse)
            return false;
          v = std::move(newv);
          v._inuse = true;
          return true;
        });
      }
      // Removes an item, returning by value the value_type removed. If returned item doesn't have its bool set, key was not found.
      static std::pair<value_type, bool> erase(pointer v) noexcept
      {
        value_type ret;
        if(v)
        {
          ret = std::move(*v);
          v->_inuse = false;
          return std::make_pair(std::move(ret), true);
        }
        return std::make_pair(std::move(ret), false);
      }
      // Removes an item, returning by value the value_type removed. If returned item doesn't have its bool set, key was not found.
      template <class Container> static std::pair<value_type, bool> erase(Container &c, const key_type &k) noexcept { return erase(find_exclusive(c, k)); }
    };
    /*! \struct atomic_linear_memory_policy
    \brief Like `linear_memory_policy`, but threadsafe for key finding, insertion and removal.

    \warning Anything returning an iterator is threadsafe because any iterator holds the lock on the item until it is destructed, but
    anything returning a reference is NOT THREADSAFE. Additionally, all finds which traverse a locked item if `LinearSearchLimit` is not
    zero must necessarily spin on that lock even if that item isn't actually the right one, so if you set `LinearSearchLimit` to
    anything but zero, then you can only ever safely take one exclusive lock at a time ever, otherwise there is a chance of deadlock.

    \note If you use a `SharedMutex` instead of a `Mutex` as the LockType (detected via Expression SFINAE), const operations returning
    a `const_iterator` will take a shared lock instead of an exclusive lock. That makes concurrent read-only usage non-blocking.

    \note Because iterators hold a lock to their pointed to item, iterators are move only and cannot be copied. As basic_open_hash_index
    is a hash table, simply look up a new iterator, though obviously looking up the same key twice means instant deadlock unless you are using
    a shared or recursive mutex as the lock.

    \tparam KeyType The type of key to use. Must be a trivial type which acts as if an unsigned integer type.
    \tparam T The type of mapped value to use.
    \tparam LinearSearchLimit How far to search either side of a collison for an entry or an empty slot.
    \tparam LockType The type of `Mutex` or `SharedMutex` to use as the lock type.
    \tparam KeyModulus A callable which performs modulus of the KeyType with the storage container's size. If you know the container's
    size will always be a power of two, use `twos_power_modulus` instead of `arithmetic_modulus`.
    \tparam KeyCompare A callable which compares two KeyTypes.
    */
    template <class KeyType, class T, size_t LinearSearchLimit = 0, class LockType = configurable_spinlock::spinlock<uint32_t>, class KeyModulus = arithmetic_modulus<KeyType>, class KeyCompare = std::equal_to<KeyType>> struct atomic_linear_memory_policy
    {
      using key_type = typename std::add_const<KeyType>::type;
      using mapped_type = T;
      using lock_type = LockType;
      static constexpr bool lock_type_is_shared = detail::is_shared_mutex<lock_type>::value;
      struct value_type
      {
        using first_type = key_type;
        using second_type = mapped_type;
        mutable lock_type lock;
        std::atomic<uint32_t> _inuse;  // 0 = not in use, 1 = in use, 2 = being constructed or destroyed right now
        key_type first;
        mapped_type second;
        constexpr value_type()
            : _inuse(0)
            , first()
            , second()
        {
        }
        template <class U, class V>
        constexpr value_type(U &&f, V &&s)
            : _inuse(0)
            , first(std::forward<U>(f))
            , second(std::forward<V>(s))
        {
        }
        template <class U, class V>
        constexpr value_type(const std::pair<U, V> &v)
            : _inuse(false)
            , first(v.first)
            , second(v.second)
        {
        }
        template <class U, class V>
        constexpr value_type(std::pair<U, V> &&v)
            : _inuse(false)
            , first(std::move(v.first))
            , second(std::move(v.second))
        {
        }
        value_type(value_type &&o) noexcept : _inuse(o._inuse.exchange(0)), first(std::move(o.first)), second(std::move(o.second)) {}
        value_type(const value_type &) = delete;
        value_type &operator=(value_type &&o) noexcept
        {
          this->~value_type();
          new(this) value_type(std::move(o));
          return *this;
        }
        constexpr bool operator==(const value_type &o) const noexcept { return first == o.first && second == o.second; }
        constexpr bool operator!=(const value_type &o) const noexcept { return first != o.first || second != o.second; }
        constexpr bool operator<(const value_type &o) const noexcept { return first < o.first || (first == o.first && second < o.second); }
        constexpr bool operator<=(const value_type &o) const noexcept { return first < o.first || (first == o.first && second <= o.second); }
        constexpr bool operator>(const value_type &o) const noexcept { return o < *this; }
        constexpr bool operator>=(const value_type &o) const noexcept { return !(*this < o); }
      };
      using size_type = size_t;
      using difference_type = ptrdiff_t;
      using reference = value_type &;
      using const_reference = const value_type &;
      struct const_pointer
      {
        const value_type *_v;
        int _locked_type;
        explicit const_pointer(const value_type *v = nullptr, bool exclusive_locked = false) noexcept : _v(v), _locked_type(exclusive_locked ? 2 : 1) { /*assert(!v || is_lockable_locked(v->lock));*/}
        constexpr const_pointer(const_pointer &&o) noexcept : _v(o._v), _locked_type(o._locked_type) { o._v = nullptr; }
        const_pointer(const const_pointer &) = delete;
        void reset() noexcept
        {
          if(_v)
          {
            if(2 == _locked_type)
              _v->lock.unlock();
            else if(1 == _locked_type)
              detail::unlock_shared_if<lock_type_is_shared>(_v->lock);
            // IMPORTANT: DO NOT RESET _v TO NULLPTR!
            _locked_type = 0;
          }
        }
        ~const_pointer() { reset(); }
        const_pointer &operator=(const_pointer &&o) noexcept
        {
          this->~const_pointer();
          new(this) const_pointer(std::move(o));
          return *this;
        }
        constexpr explicit operator bool() const noexcept { return !!_v; }
        constexpr bool operator!() const noexcept { return !_v; }
        constexpr const value_type *operator->() const noexcept { return _v; }
        constexpr const value_type &operator*() const noexcept { return *_v; }
        constexpr explicit operator const value_type *() const noexcept { return _v; }

        constexpr bool operator==(const const_pointer &o) const noexcept { return _v == o._v; }
        constexpr bool operator!=(const const_pointer &o) const noexcept { return _v != o._v; }
        constexpr bool operator<(const const_pointer &o) const noexcept { return _v < o._v; }
        constexpr bool operator<=(const const_pointer &o) const noexcept { return _v <= o._v; }
        constexpr bool operator>(const const_pointer &o) const noexcept { return _v > o._v; }
        constexpr bool operator>=(const const_pointer &o) const noexcept { return _v >= o._v; }
        constexpr bool operator==(const value_type *o) const noexcept { return _v == o; }
        constexpr bool operator!=(const value_type *o) const noexcept { return _v != o; }
        constexpr bool operator<(const value_type *o) const noexcept { return _v < o; }
        constexpr bool operator<=(const value_type *o) const noexcept { return _v <= o; }
        constexpr bool operator>(const value_type *o) const noexcept { return _v > o; }
        constexpr bool operator>=(const value_type *o) const noexcept { return _v >= o; }

        constexpr size_t operator-(const value_type *o) const noexcept { return _v - o; }
      };
      struct pointer : public const_pointer
      {
        explicit pointer(value_type *v = nullptr) noexcept : const_pointer(v, true) {}
        constexpr pointer(pointer &&o) noexcept : const_pointer(std::move(o)) {}
        pointer &operator=(pointer &&o) noexcept
        {
          this->~pointer();
          new(this) pointer(std::move(o));
          return *this;
        }
        constexpr value_type *operator->() noexcept { return const_cast<value_type *>(this->_v); }
        constexpr const value_type *operator->() const noexcept { return this->_v; }
        constexpr value_type &operator*() noexcept { return *const_cast<value_type *>(this->_v); }
        constexpr const value_type &operator*() const noexcept { return *this->_v; }
        constexpr explicit operator value_type *() noexcept { return const_cast<value_type *>(this->_v); }
        constexpr explicit operator const value_type *() const noexcept { return this->_v; }
      };
      struct items_count_type
      {
        std::atomic<size_t> _v;
        constexpr items_count_type(size_t v) noexcept : _v(v) {}
        items_count_type &operator--() noexcept
        {
          _v.fetch_sub(1, std::memory_order_relaxed);
          return *this;
        }
        items_count_type &operator++() noexcept
        {
          _v.fetch_add(1, std::memory_order_relaxed);
          return *this;
        }
        explicit operator size_t() const noexcept { return _v.load(std::memory_order_relaxed); }
      };

      // True if item is in use
      static bool is_inuse(const value_type *p) noexcept { return p->_inuse.load(std::memory_order_acquire) == 1; }
      // Returns a LOCKED pointer to an item if it is in use
      static pointer if_inuse_to_pointer(value_type *p) noexcept
      {
        if(p->_inuse.load(std::memory_order_acquire) != 1)
          return pointer();
        p->lock.lock();
        if(p->_inuse.load(std::memory_order_acquire) == 1)
          return pointer(p);
        p->lock.unlock();
        return pointer();
      }
      // Returns a LOCKED pointer to an item if it is in use
      static const_pointer if_inuse_to_pointer(const value_type *p) noexcept
      {
        if(p->_inuse.load(std::memory_order_acquire) != 1)
          return const_pointer();
        detail::lock_shared_if<lock_type_is_shared>(p->lock);
        if(p->_inuse.load(std::memory_order_acquire) == 1)
          return const_pointer(p);
        detail::unlock_shared_if<lock_type_is_shared>(p->lock);
        return const_pointer();
      }
      // Looks up a key in the contiguous container returning the LOCKED address of any value_type found
      template <class Container> static pointer find_exclusive(Container &c, const key_type &k) noexcept
      {
        return pointer(detail::linear_find<LinearSearchLimit>(c, KeyModulus()(k, c.size()), [&k](const value_type &v) {
          if(v._inuse.load(std::memory_order_acquire) != 1)
            return false;
          v.lock.lock();
          if(v._inuse.load(std::memory_order_acquire) == 1 && KeyCompare()(k, v.first))
            return true;
          v.lock.unlock();
          return false;
        }));
      }
      // Looks up a key in the contiguous container returning the LOCKED address of any value_type found
      template <class Container> static const_pointer find_shared(const Container &c, const key_type &k) noexcept
      {
        return const_pointer(detail::linear_find<LinearSearchLimit>(c, KeyModulus()(k, c.size()), [&k](const value_type &v) {
          if(v._inuse.load(std::memory_order_acquire) != 1)
            return false;
          detail::lock_shared_if<lock_type_is_shared>(v.lock);
          if(v._inuse.load(std::memory_order_acquire) == 1 && KeyCompare()(k, v.first))
            return true;
          detail::unlock_shared_if<lock_type_is_shared>(v.lock);
          return false;
        }));
      }
      // Inserts an item into the contiguous container returning a LOCKED value_type_ptr to any value_type inserted
      template <class Container> static pointer insert(Container &c, value_type &&newv) noexcept
      {
        return pointer(detail::linear_find<LinearSearchLimit>(c, KeyModulus()(newv.first, c.size()), [&newv](value_type &v) {
          uint32_t expected = 0;
          if(v._inuse.compare_exchange_strong(expected, 2, std::memory_order_acquire, std::memory_order_relaxed))
          {
            newv._inuse.store(2, std::memory_order_release);
            v = std::move(newv);
            v.lock.lock();
            v._inuse.store(1, std::memory_order_release);
            return true;
          }
          return false;
        }));
      }
      // Removes an item, returning by value the value_type removed. If returned item doesn't have its bool set, key was not found.
      static std::pair<value_type, bool> erase(pointer &v) noexcept
      {
        value_type ret;
        if(v)
        {
          uint32_t expected = 1;
          if(v->_inuse.compare_exchange_strong(expected, 2, std::memory_order_acquire, std::memory_order_relaxed))
          {
            ret = value_type(std::move(v->first), std::move(v->second));
            // assert(is_lockable_locked(v->lock));
            v.reset();                                      // unlocks the item and makes the pointer no longer unlock on destruct
            v->_inuse.store(0, std::memory_order_release);  // mark as unused
            return std::make_pair(std::move(ret), true);
          }
        }
        return std::make_pair(std::move(ret), false);
      }
      // Removes an item, returning by value the value_type removed. If returned item doesn't have its bool set, key was not found.
      template <class Container> static std::pair<value_type, bool> erase(Container &c, const key_type &k) noexcept
      {
        pointer p = find_exclusive(c, k);
        return erase(p);
      }
    };

    /* \class basic_open_hash_index
    \brief Policy driven open addressed hash index
    \tparam Policy See list of standard policies below.
    \tparam ContiguousContainerType Some STL container meeting the `ContiguousContainer` concept. `std::array<>` and `std::vector` are
    two of the most commonly used.
    \tparam disable_existing_key_check Don't check for existing keys during insertion which is faster.

    This is the most fundamental building block of a hash table possible - it simply lets you look up a key and get back
    a value. Indexing is based on open addressing whereby the key value is modulused by the underlying container's size
    into an index. We then compare keys at the index and either side of the index until a match or empty slot as appropriate
    is found.

    This type of hash table is therefore highly dependent on a high quality hashing function. Use FNV1 quality or better,
    not the usually poor quality `std::hash` implementation (see `fnv1a_hash`). Note this particular implementation does NOT hash keys for
    you, 'key' here means some type which behaves as if an unsigned integer integral type.

    Because it does so much less work than an `unordered_map<>`, insert/erase performance is typically ~4x faster and lookup
    performance is typically <= ~2x faster depending on how your STL implements `std::hash<>`.

    A particularly interesting use of this class is as a shared memory hash index. Simply use `std::array<>` as the container
    and placement new this class directly into shared memory - `sizeof(basic_open_hash_index) == sizeof(ContiguousContainerType) + sizeof(size_t)`.
    If you'd like to safely concurrently read and modify the shared memory hash index, see the `atomic_linear_memory_policy<...>`
    below.

    Some standard policies available:

    * `linear_memory_policy<...>`: Keeps a simple boolean for whether the slot is in use. Collision is handled
    via a linear scan either side of the arrival point.
    * `atomic_linear_memory_policy<...>`: Keeps a tristate atomic for whether the slot is in use or being constructed or
    destructed. Also keeps a mutex held locked <b>whilst any iterator is held open to that item</b>.

    You can of course supply your own policies to implement any semantics you like e.g. storage is a huge sparse file on disc
    and every key lookup is a `read()` and every key update is a `write()`.

    */
    template <class Policy, template <class> class ContiguousContainerType, bool disable_existing_key_check = false> class basic_open_hash_index
    {
    public:
      //! The key type used to look up a mapped type
      using key_type = typename Policy::key_type;
      //! The type looked up by a key
      using mapped_type = typename Policy::mapped_type;
      //! The type stored in the container
      using value_type = typename Policy::value_type;
      //! The size type
      using size_type = typename Policy::size_type;
      //! The difference type
      using difference_type = typename Policy::difference_type;
      //! The reference type
      using reference = typename Policy::reference;
      //! The const reference type
      using const_reference = typename Policy::const_reference;
      //! The pointer type. May be a smart pointer which unlocks things on destruction.
      using pointer = typename Policy::pointer;
      //! The const pointer type. May be a smart pointer which unlocks things on destruction.
      using const_pointer = typename Policy::const_pointer;

      /*! The container used to store the hash entries defined by
      Policy::value_type. Must be a ContiguousContainer.
      */
      using container_type = ContiguousContainerType<value_type>;

    protected:
      template <bool is_const, class Parent, class Pointer, class Reference> class iterator_;
      template <bool is_const, class Parent, class Pointer, class Reference> class iterator_ : public std::iterator<std::bidirectional_iterator_tag, value_type, difference_type, pointer, reference>
      {
        friend class basic_open_hash_index;
        template <bool is_const_, class Parent_, class Pointer_, class Reference_> friend class iterator_;
        Parent *_parent;
        Pointer _p;

        using underlying_pointer_type = typename std::add_pointer<decltype(*_p)>::type;
        using const_underlying_pointer_type = typename std::add_const<underlying_pointer_type>::type;
        static_assert(is_const == std::is_const<typename std::remove_reference<decltype(*_p)>::type>::value, "");

        iterator_ &_inc() noexcept
        {
          if(_parent && _p >= _parent->_store.data() && _p < (_parent->_store.data() + _parent->_store.size()))
          {
            underlying_pointer_type p(_p);
            do
            {
              ++p;
              if(p < (_parent->_store.data() + _parent->_store.size()))
                _p = Policy::if_inuse_to_pointer(p);
              else
                break;
            } while(!_p);
            if(p >= (_parent->_store.data() + _parent->_store.size()))
              _p = Pointer(nullptr);
          }
          return *this;
        }
        iterator_ &_dec() noexcept
        {
          if(_parent)
          {
            underlying_pointer_type p(_p);
            if(p < _parent->_store.data())
              p = (_parent->_store.data() + _parent->_store.size());
            do
            {
              --p;
              if(p >= _parent->_store.data())
                _p = Policy::if_inuse_to_pointer(p);
              else
                break;
            } while(!_p);
            if(p < _parent->_store.data())
              _p = Pointer(nullptr);
          }
          return *this;
        }

        constexpr iterator_(Parent *parent, Pointer &&p) noexcept : _parent(parent), _p(std::move(p)) {}
        constexpr iterator_(Parent *parent) noexcept : _parent(parent), _p(nullptr) {}

      public:
        constexpr iterator_() noexcept : _parent(nullptr), _p(nullptr) {}
        constexpr iterator_(const iterator_ &) = default;
        constexpr iterator_(iterator_ &&) noexcept = default;
        constexpr iterator_ &operator=(const iterator_ &) = default;
        constexpr iterator_ &operator=(iterator_ &&) noexcept = default;
        // Non-const to const iterator
        template <class _Parent, class _Pointer, class _Reference, typename = typename std::enable_if<std::is_same<_Parent, _Parent>::value && is_const, _Parent>::type> constexpr iterator_(const iterator_<false, _Parent, _Pointer, _Reference> &o) noexcept : _parent(o._parent), _p(o._p) {}
        template <class _Parent, class _Pointer, class _Reference, typename = typename std::enable_if<std::is_same<_Parent, _Parent>::value && is_const, _Parent>::type> constexpr iterator_(iterator_<false, _Parent, _Pointer, _Reference> &&o) noexcept : _parent(std::move(o._parent)), _p(std::move(o._p)) {}
        void swap(iterator_ &o) noexcept
        {
          std::swap(_parent, o._parent);
          std::swap(_p, o._p);
        }

        explicit operator bool() const noexcept { return _parent && _p; }
        bool operator!() const noexcept { return !_parent || !_p; }
        underlying_pointer_type operator->() noexcept
        {
          if(!_parent || !_p || _p < _parent->_store.data() || _p >= (_parent->_store.data() + _parent->_store.size()))
            return nullptr;
          return underlying_pointer_type(_p);
        }
        const_underlying_pointer_type operator->() const noexcept
        {
          if(!_parent || !_p || _p < _parent->_store.data() || _p >= (_parent->_store.data() + _parent->_store.size()))
            return nullptr;
          return const_underlying_pointer_type(_p);
        }
        bool operator==(const iterator_ &o) const noexcept { return _parent == o._parent && _p == o._p; }
        bool operator!=(const iterator_ &o) const noexcept { return _parent != o._parent || _p != o._p; }
        Reference operator*() noexcept
        {
          if(!_parent || !_p || _p < _parent->_store.data() || _p >= (_parent->_store.data() + _parent->_store.size()))
          {
            abort();
          }
          return *_p;
        }
        const Reference operator*() const noexcept
        {
          if(!_parent || !_p || _p < _parent->_store.data() || _p >= (_parent->_store.data() + _parent->_store.size()))
          {
            abort();
          }
          return *_p;
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
            abort();
          return !!_p && (!o._p || _p < o._p);
        }
        bool operator>(const iterator_ &o) const noexcept
        {
          if(_parent != o._parent)
            abort();
          return !!o._p && (!_p || _p > o._p);
        }
        bool operator<=(const iterator_ &o) const noexcept
        {
          if(_parent != o._parent)
            abort();
          return !(o > *this);
        }
        bool operator>=(const iterator_ &o) const noexcept
        {
          if(_parent != o._parent)
            abort();
          return !(o < *this);
        }
      };
      template <bool is_const, class Parent, class Pointer, class Reference> friend class iterator_;

    public:
      //! The iterator type
      using iterator = iterator_<false, basic_open_hash_index, pointer, reference>;
      //! The const iterator type
      using const_iterator = iterator_<true, const basic_open_hash_index, const_pointer, const_reference>;
      //! The reverse iterator type
      using reverse_iterator = std::reverse_iterator<iterator>;
      //! The const reverse iterator type
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    protected:
      container_type _store;
      typename Policy::items_count_type _count;

    public:
      //! Default construction, passes through args to container_type
      template <class... Args>
      basic_open_hash_index(Args &&... args)
          : _store(std::forward<Args>(args)...)
          , _count(0)
      {
      }
      //! Swaps with another instance
      void swap(basic_open_hash_index &o) noexcept
      {
        std::swap(_store, o._store);
        std::swap(_count, o._count);
      }

      //! Returns true if the index is empty
      bool empty() const noexcept { return size_type(_count) == 0; }
      //! Returns the number of items in the index
      size_type size() const noexcept { return size_type(_count); }
      //! Returns the maximum number of items in the index
      size_type max_size() const noexcept { return _store.size(); }
      //! Returns the STL container backing the store of this index
      const container_type &container() const noexcept { return _store; }
      //! Returns the STL container backing the store of this index
      container_type &container() noexcept { return _store; }

      //! Returns the front of the index.
      reference front() noexcept
      {
        for(size_t idx = 0; idx < _store.size(); idx++)
        {
          if(Policy::is_inuse(&_store[idx]))
            return _store[idx];
        }
        abort();
      }
      //! Returns the front of the index.
      const_reference front() const noexcept
      {
        for(size_t idx = 0; idx < _store.size(); idx++)
        {
          if(Policy::is_inuse(&_store[idx]))
            return _store[idx];
        }
        abort();
      }
      //! Returns the back of the index.
      reference back() noexcept
      {
        for(size_t idx = _store.size() - 1; idx < _store.size(); idx--)
        {
          if(Policy::is_inuse(&_store[idx]))
            return _store[idx];
        }
        abort();
      }
      //! Returns the back of the index.
      const_reference back() const noexcept
      {
        for(size_t idx = _store.size() - 1; idx < _store.size(); idx--)
        {
          if(Policy::is_inuse(&_store[idx]))
            return _store[idx];
        }
        abort();
      }

      //! Returns an iterator to the first item in the index.
      iterator begin() noexcept
      {
        pointer p;
        for(size_t idx = 0; idx < _store.size(); idx++)
        {
          p = Policy::if_inuse_to_pointer(&_store[idx]);
          if(p)
            return iterator(this, std::move(p));
        }
        return iterator(this);
      }
      //! Returns an iterator to the first item in the index.
      const_iterator begin() const noexcept
      {
        const_pointer p;
        for(size_t idx = 0; idx < _store.size(); idx++)
        {
          p = Policy::if_inuse_to_pointer(&_store[idx]);
          if(p)
            return const_iterator(this, std::move(p));
        }
        return const_iterator(this);
      }
      //! Returns an iterator to the first item in the index.
      const_iterator cbegin() const noexcept { return begin(); }
      //! Returns an iterator to the item after the last in the index.
      iterator end() noexcept { return iterator(this); }
      //! Returns an iterator to the item after the last in the index.
      const_iterator end() const noexcept { return const_iterator(this); }
      //! Returns an iterator to the item after the last in the index.
      const_iterator cend() const noexcept { return const_iterator(this); }

      //! Clears the log
      void clear() noexcept
      {
        _count = typename Policy::items_count_type(0);
        auto oldsize = _store.size();
        _store.clear();
        _store.resize(oldsize);
      }
      //! Inserts a new item, returning an iterator to the new item if the bool is true, else an iterator to existing item. If the returned iterator is invalid, there is no more space.
      std::pair<iterator, bool> insert(value_type &&v) noexcept
      {
        pointer p(!disable_existing_key_check ? Policy::find_exclusive(_store, v.first) : pointer(nullptr));
        if(p)
          return std::make_pair(iterator(this, std::move(p)), false);
        p = Policy::insert(_store, std::move(v));
        if(!p)
          return std::make_pair(end(), false);
        ++_count;
        return std::make_pair(iterator(this, std::move(p)), true);
      }
      //! Inserts a new item or assigns to an existing one, returning an iterator to the new or existing item. If the returned iterator is invalid, there is no more space.
      template <class T> std::pair<iterator, bool> insert_or_assign(const key_type &k, T &&t) noexcept
      {
        auto p = Policy::find_exclusive(_store, k);
        if(p)
        {
          p->second = std::forward<T>(t);
          return std::make_pair(iterator(this, std::move(p)), false);
        }
        p = Policy::insert(_store, value_type(k, std::forward<T>(t)));
        if(!p)
          return std::make_pair(end(), false);
        ++_count;
        return std::make_pair(iterator(this, std::move(p)), true);
      }
      //! Emplaces a new item, returning an iterator to the new item
      template <class... Args> std::pair<iterator, bool> emplace(Args &&... args) noexcept { return insert(value_type(std::forward<Args>(args)...)); }
      //! Emplaces a new item, returning an iterator to the new item
      template <class... Args> std::pair<iterator, bool> try_emplace(const key_type &k, Args &&... args) noexcept
      {
        pointer p(!disable_existing_key_check ? Policy::find_exclusive(_store, k) : pointer(nullptr));
        if(p)
          return std::make_pair(iterator(this, std::move(p)), false);
        p = Policy::insert(_store, value_type(k, mapped_type(std::forward<Args>(args)...)));
        if(!p)
          return std::make_pair(end(), false);
        ++_count;
        return std::make_pair(iterator(this, std::move(p)), true);
      }
      //! Erases an item. Note takes an iterator not a const_iterator as the STL would.
      iterator erase(iterator it) noexcept
      {
        if(it._parent != this)
          abort();
        auto v = Policy::erase(it._p);
        if(v.second)
          --_count;
        // it._p is still valid but now unlocked
        ++it;
        return it;
      }
      //! Erases an item. Note this implementation does not return an iterator as per the STL.
      void erase(const key_type &k) noexcept
      {
        auto v = Policy::erase(_store, k);
        if(v.second)
          --_count;
      }
      //! Finds an item
      iterator find_exclusive(const key_type &k) noexcept
      {
        auto p = Policy::find_exclusive(_store, k);
        if(p)
          return iterator(this, std::move(p));
        return end();
      }
      //! Finds an item
      const_iterator find_shared(const key_type &k) const noexcept
      {
        auto p = Policy::find_shared(_store, k);
        if(p)
          return const_iterator(this, std::move(p));
        return cend();
      }
      //! Finds an item (exclusive)
      iterator find(const key_type &k) noexcept { return find_exclusive(k); }
      //! Finds an item (shared)
      const_iterator find(const key_type &k) const noexcept { return find_shared(k); }
      //! Returns a reference to the specified element, inserting an empty mapped type if necessary
      reference operator[](const key_type &k) noexcept
      {
        auto p = Policy::find_exclusive(_store, k);
        if(p)
          return p->second;
        p = Policy::insert(_store, value_type(k, mapped_type()));
        if(p)
        {
          ++_count;
          return p->second;
        }
        abort();
      }
      //! Returns a reference to the specified element.
      const_reference operator[](const key_type &k) const noexcept
      {
        auto p = Policy::find_exclusive(_store, k);
        if(p)
        {
          ++_count;
          return p->second;
        }
        abort();
      }
    };

    //! std::ostream writer for an index
    template <class Policy, template <class> class ContiguousContainerType> inline std::ostream &operator<<(std::ostream &s, const basic_open_hash_index<Policy, ContiguousContainerType> &l)
    {
      for(const auto &i : l)
      {
        s << i;
      }
      return s;
    }
  }
}

QUICKCPPLIB_NAMESPACE_END


#endif
