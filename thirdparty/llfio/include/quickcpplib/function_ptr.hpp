/* Function pointer support
(C) 2019 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: June 2019


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

#ifndef QUICKCPPLIB_FUNCTION_PTR_HPP
#define QUICKCPPLIB_FUNCTION_PTR_HPP

#include "config.hpp"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

QUICKCPPLIB_NAMESPACE_BEGIN

//! \brief The namespace for the function pointer type
namespace function_ptr
{
  static constexpr size_t default_callable_storage_bytes = 32 - sizeof(uintptr_t);

  /*! \brief A move only lightweight `std::function` alternative, with configurable small object optimisation.

  Requirements for small object optimisation:

  1. `U` must be nothrow move constructible.
  2. Default of `sizeof(U) + sizeof(vptr) + sizeof(void *) <= 32`, but is configurable by the make functions.
  */
  template <class F, size_t callable_storage_bytes = default_callable_storage_bytes> class function_ptr;

  template <class R, class... Args, size_t callable_storage_bytes> class function_ptr<R(Args...), callable_storage_bytes>
  {
#if !defined(__clang__) || __clang_major__ >= 5
    template <class ErasedPrototype_, class Callable_, size_t callable_storage_bytes_, class... CallableConstructionArgs_> friend constexpr inline function_ptr<ErasedPrototype_, callable_storage_bytes_> emplace_function_ptr(CallableConstructionArgs_ &&... args);
#endif
    template <class ErasedPrototype_, class Callable_, class... CallableConstructionArgs_> friend constexpr inline function_ptr<ErasedPrototype_, (sizeof(Callable_) + sizeof(void *) + sizeof(void *) - 1) & ~(sizeof(void *) - 1)> emplace_function_ptr_nothrow(CallableConstructionArgs_ &&... args) noexcept;

    struct _function_ptr_storage
    {
      _function_ptr_storage() = default;

      _function_ptr_storage(const _function_ptr_storage &) = delete;

      _function_ptr_storage(_function_ptr_storage &&) = delete;

      _function_ptr_storage &operator=(const _function_ptr_storage &) = delete;

      _function_ptr_storage &operator=(_function_ptr_storage &&) = delete;

      virtual ~_function_ptr_storage() = default;

      virtual R operator()(Args... args) = 0;

      virtual _function_ptr_storage *move(char *v) noexcept = 0;
    };

    // Dynamically allocated callables
    template <class U> struct _function_ptr_storage_nonmoveable final : public _function_ptr_storage
    {
      U c;

      template <class... Args2>
      constexpr explicit _function_ptr_storage_nonmoveable(Args2 &&... args)
          : c(static_cast<Args2 &&>(args)...)
      {
      }

      _function_ptr_storage_nonmoveable(const _function_ptr_storage_nonmoveable &) = delete;

      _function_ptr_storage_nonmoveable(_function_ptr_storage_nonmoveable &&) = delete;

      _function_ptr_storage_nonmoveable &operator=(const _function_ptr_storage_nonmoveable &) = delete;

      _function_ptr_storage_nonmoveable &operator=(_function_ptr_storage_nonmoveable &&) = delete;

      R operator()(Args... args) override { return c(static_cast<Args &&>(args)...); }

      _function_ptr_storage *move(char * /*unused*/) noexcept final { abort(); }
    };

    // In-class stored callables
    template <class U> struct _function_ptr_storage_moveable final : public _function_ptr_storage
    {
      struct standin_t
      {
        template <class... Args2> standin_t(Args2 &&... /*unused*/) {}

        R operator()(Args... /*unused*/) { return {}; }
      };

      using type = std::conditional_t<std::is_move_constructible<U>::value, U, standin_t>;
      type c;

      template <class... Args2>
      constexpr explicit _function_ptr_storage_moveable(Args2 &&... args)
          : c(static_cast<Args2 &&>(args)...)
      {
      }

      _function_ptr_storage_moveable(const _function_ptr_storage_moveable &) = delete;

      _function_ptr_storage_moveable(_function_ptr_storage_moveable &&o) noexcept  // NOLINT
          : c(static_cast<type &&>(o.c))
      {
      }

      _function_ptr_storage_moveable &operator=(const _function_ptr_storage_moveable &) = delete;

      _function_ptr_storage_moveable &operator=(_function_ptr_storage_moveable &&) = delete;

      R operator()(Args... args) override { return c(static_cast<Args &&>(args)...); }

      _function_ptr_storage *move(char *v) noexcept final { return new(v) _function_ptr_storage_moveable(static_cast<_function_ptr_storage_moveable &&>(*this)); }
    };

    // Used for sizing, and nothing else
    struct _empty_callable
    {
      size_t foo;

      R operator()(Args... /*unused*/);
    };

    uintptr_t _ptr_{0};
    char _sso[callable_storage_bytes];

  public:
    //! \brief The type returned by the callable and our call operator
    using result_type = R;
    //! \brief The largest size of callable for which SSO will be used
    static constexpr size_t max_callable_size = sizeof(_sso) - sizeof(_function_ptr_storage_nonmoveable<_empty_callable>) + sizeof(_empty_callable);
    //! \brief True if this callable would be SSOable in this type
    template <class U>
    static constexpr bool is_ssoable = std::is_nothrow_move_constructible<typename std::decay<U>::type>::value  //
                                       && (sizeof(_function_ptr_storage_nonmoveable<typename std::decay<U>::type>) <= sizeof(_sso));
    //! \brief The types of pointer we can store
    enum _ptrtype_t
    {
      none = 0,     //!< We are empty
      owned = 1,    //!< We own our dynamically allocated callable and we will free it upon destruction
      ssoed = 2,    //!< We store our callable internally to ourselves
      external = 3  //!< We use an externally supplied allocation for the callable which we do NOT free upon destruction
    };

#if !defined(__clang__) || __clang_major__ >= 5
  private:
#endif
    // Get the pointer, minus ptrtype_t
    _function_ptr_storage *_ptr() const
    {
      auto *ret = reinterpret_cast<_function_ptr_storage *>(_ptr_ & ~3);
#ifdef __cpp_rtti
      assert(nullptr != dynamic_cast<_function_ptr_storage *>(ret));
#endif
      return ret;
    }

    // Used by the constructors to statically munge in the ptrtype_t bits
    static uintptr_t _ptr(_function_ptr_storage *p, _ptrtype_t type) { return reinterpret_cast<uintptr_t>(p) | static_cast<uintptr_t>(type); }

    template <class U> struct _emplace_t
    {
      void *mem{nullptr};
    };
    template <class U> struct _noallocate_t
    {
    };

    // Non in-class constructor, emplaces callble either into supplied memory or dynamically allocated memory
    template <class U, class... Args2>
    function_ptr(_emplace_t<U> *_, Args2 &&... args)
        : _ptr_((_->mem != nullptr)  //
                ?
                _ptr(new(_->mem) _function_ptr_storage_nonmoveable<typename std::decay<U>::type>(static_cast<Args2 &&>(args)...), external)  //
                :
                _ptr(new _function_ptr_storage_nonmoveable<typename std::decay<U>::type>(static_cast<Args2 &&>(args)...), owned))
    {
    }

    // In-class constructor
    template <class U, class... Args2>
    function_ptr(_noallocate_t<U> /*unused*/, Args2 &&... args)
        : _ptr_(_ptr(new((void *) (is_ssoable<U> ? _sso : nullptr)) _function_ptr_storage_moveable<typename std::decay<U>::type>(static_cast<Args2 &&>(args)...), ssoed))
    {
    }

    // Delegate to in-class or out-of-class constructor
    template <class U, class... Args2>
    explicit function_ptr(_emplace_t<U> _, Args2 &&... args)
        : function_ptr(is_ssoable<U>  //
                       ?
                       function_ptr(_noallocate_t<U>{}, static_cast<Args2 &&>(args)...)  //
                       :
                       function_ptr(&_, static_cast<Args2 &&>(args)...))
    {
    }

  public:
    //! \brief Default constructs to empty
    constexpr function_ptr() {}  // NOLINT
    //! \brief Move constructor
    constexpr function_ptr(function_ptr &&o) noexcept
    {
      if(ssoed == o.ptr_type())
      {
        _ptr_ = _ptr(o._ptr()->move(_sso), ssoed);
      }
      else
      {
        _ptr_ = o._ptr_;
      }
      o._ptr_ = 0;
    }

    //! \brief Move assigment
    function_ptr &operator=(function_ptr &&o) noexcept
    {
      this->~function_ptr();
      new(this) function_ptr(static_cast<function_ptr &&>(o));
      return *this;
    }

    //! \brief Copying prevented
    function_ptr(const function_ptr &) = delete;

    //! \brief Copying prevented
    function_ptr &operator=(const function_ptr &) = delete;

    ~function_ptr() { reset(); }

    //! \brief True if the ptr is not empty
    explicit constexpr operator bool() const noexcept { return _ptr_ != 0; }

    //! \brief Returns whether this ptr's callable is empty, owned, ssoed or externally managed
    _ptrtype_t ptr_type() const { return static_cast<_ptrtype_t>(_ptr_ & 3); }

    //! \brief Calls the callable, returning what the callable returns
    template <class... Args2> constexpr R operator()(Args2 &&... args) const
    {
      auto *r = _ptr();
      return (*r)(static_cast<Args2 &&>(args)...);
    }

    //! \brief Disposes of the callable, resetting ptr to empty
    constexpr void reset() noexcept
    {
      if(_ptr_ != 0)
      {
        switch(ptr_type())
        {
        case none:
        case external:
          break;
        case owned:
          delete _ptr();
          break;
        case ssoed:
          _ptr()->~_function_ptr_storage();
          break;
        }
        _ptr_ = 0;
      }
    }
  };

  /*! \brief Return a `function_ptr<ErasedPrototype>` by emplacing `Callable(CallableConstructionArgs...)`.
  If `Callable` is nothrow move constructible and sufficiently small, avoids
  dynamic memory allocation.
   */
  template <class ErasedPrototype, class Callable, size_t callable_storage_bytes = default_callable_storage_bytes, class... CallableConstructionArgs>  //
  constexpr inline function_ptr<ErasedPrototype, callable_storage_bytes> emplace_function_ptr(CallableConstructionArgs &&... args)
  {
    return function_ptr<ErasedPrototype, callable_storage_bytes>(typename function_ptr<ErasedPrototype, callable_storage_bytes>::template _emplace_t<Callable>(), static_cast<CallableConstructionArgs &&>(args)...);
  }

  /*! \brief Return a `function_ptr<ErasedPrototype>` by emplacing `Callable(CallableConstructionArgs...)`,
  without dynamically allocating memory. Note that the size of function ptr returned will be exactly the
  amount to store the callable, which may not be the default size of `function_ptr<ErasedPrototype>`.
   */
  template <class ErasedPrototype, class Callable, class... CallableConstructionArgs>  //
  constexpr inline function_ptr<ErasedPrototype, (sizeof(Callable) + sizeof(void *) + sizeof(void *) - 1) & ~(sizeof(void *) - 1)> emplace_function_ptr_nothrow(CallableConstructionArgs &&... args) noexcept
  {
    using type = function_ptr<ErasedPrototype, (sizeof(Callable) + sizeof(void *) + sizeof(void *) - 1) & ~(sizeof(void *) - 1)>;
    static_assert(type::template is_ssoable<Callable>, "The specified callable is not SSOable (probably lacks nothrow move construction)");
    return type(typename type::template _emplace_t<Callable>(), static_cast<CallableConstructionArgs &&>(args)...);
  }

  /*! \brief Return a `function_ptr<ErasedPrototype>` by from an input `Callable`.
  If `Callable` is nothrow move constructible and sufficiently small, avoids
  dynamic memory allocation.
   */
  template <class ErasedPrototype, class Callable, size_t callable_storage_bytes = default_callable_storage_bytes>  //
  constexpr inline function_ptr<ErasedPrototype, callable_storage_bytes> make_function_ptr(Callable &&f)
  {
    return emplace_function_ptr<ErasedPrototype, std::decay_t<Callable>, callable_storage_bytes>(static_cast<Callable &&>(f));
  }

  /*! \brief Return a `function_ptr<ErasedPrototype>` by from an input `Callable`,
  without dynamically allocating memory. Note that the size of function ptr returned will be exactly the
  amount to store the callable, which may not be the default size of `function_ptr<ErasedPrototype>`.
   */
  template <class ErasedPrototype, class Callable>  //
  constexpr inline auto make_function_ptr_nothrow(Callable &&f) noexcept
  {
    return emplace_function_ptr_nothrow<ErasedPrototype, std::decay_t<Callable>>(static_cast<Callable &&>(f));
  }

}  // namespace function_ptr

QUICKCPPLIB_NAMESPACE_END

#endif
