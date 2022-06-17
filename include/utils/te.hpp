//
// Copyright (c) 2018-2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#if not defined(__cpp_variadic_templates) or                               \
not defined(__cpp_rvalue_references) or not defined(__cpp_decltype) or     \
not defined(__cpp_alias_templates) or                                      \
not defined(__cpp_generic_lambdas) or not defined(__cpp_constexpr) or      \
not defined(__cpp_return_type_deduction) or                                \
not defined(__cpp_fold_expressions) or not defined(__cpp_static_assert) or \
not defined(__cpp_delegating_constructors)
    #error "[Boost].TE requires C++17 support"
#else
    #include "platform/memory.h"
    #include <type_traits>
    #include <utility>
    #include <stdexcept>
    #include <memory>

namespace boost
{
inline namespace ext
{
namespace te
{
inline namespace v1
{
namespace detail
{
template <class...>
struct type_list {
};

template <class, std::size_t>
struct mappings final {
    friend auto get(mappings);
    template <class T>
    struct set {
        friend auto get(mappings) { return T{}; }
    };
};

template <std::size_t, class...>
constexpr std::size_t mappings_size_impl(...)
{
    return {};
}

template <std::size_t N, class T, class... Ts>
constexpr auto mappings_size_impl(bool dummy)
-> decltype(get(mappings<T, N>{}), std::size_t{})
{
    return 1 + mappings_size_impl<N + 1, T, Ts...>(dummy);
}

template <class... Ts>
constexpr auto mappings_size()
{
    return mappings_size_impl<1, Ts...>(bool{});
}

template <class T, class = decltype(sizeof(T))>
std::true_type is_complete_impl(bool);
template <class>
std::false_type is_complete_impl(...);
template <class T>
struct is_complete : decltype(is_complete_impl<T>(bool{})) {
};

template <class T>
constexpr auto requires__(bool)
-> decltype(std::declval<T>().template requires__<T>());
template <class>
constexpr auto requires__(...) -> void;

template <class TExpr>
class expr_wrapper final
{
    static_assert(std::is_empty<TExpr>{});

public:
    template <class... Ts>
    decltype(auto) operator()(Ts&&... args) const
    {
        return reinterpret_cast<const TExpr&>(*this)(std::forward<Ts>(args)...);
    }
};

/*! Same as std::exchange() but is guaranteed constexpr !*/
template <class T, class U>
constexpr inline T exchange(T& obj, U&& new_value) noexcept(
std::is_nothrow_move_constructible<T>::value&&
std::is_nothrow_assignable<T&, U>::value)
{
    T old_value = std::move(obj);
    obj = std::forward<U>(new_value);
    return old_value;
}
} // namespace detail

struct non_owning_storage {
    template <
    class T,
    class T_ = std::decay_t<T>,
    std::enable_if_t<!std::is_same_v<T_, non_owning_storage>, bool> = true>
    constexpr explicit non_owning_storage(T&& t) noexcept
        : ptr{ &t }
    {
    }

    void* ptr = nullptr;
};

struct shared_storage {
    template <
    class T,
    class T_ = std::decay_t<T>,
    std::enable_if_t<!std::is_same_v<T_, shared_storage>, bool> = true>
    constexpr explicit shared_storage(T&& t) noexcept(noexcept(std::make_shared<T_>(std::forward<T>(t))))
        : ptr{ std::make_shared<T_>(std::forward<T>(t)) }
    {
    }

    std::shared_ptr<void> ptr;
};

struct dynamic_storage {
    template <
    class T,
    class T_ = std::decay_t<T>,
    std::enable_if_t<!std::is_same_v<T_, dynamic_storage>, bool> = true>
    constexpr explicit dynamic_storage(T&& t) noexcept(noexcept(SkrNew<T_>(std::forward<T>(t))))
        : ptr{ SkrNew<T_>(std::forward<T>(t)) }
        , del{ [](void* self) {
            delete reinterpret_cast<T_*>(self);
        } }
        , copy{ [](const void* self) -> void* {
            if constexpr (std::is_copy_constructible_v<T_>)
                return SkrNew<T_>(*reinterpret_cast<const T_*>(self));
            else
                throw std::runtime_error("dynamic_storage : erased type is not copy constructible");
        } }
    {
    }

    constexpr dynamic_storage(const dynamic_storage& other)
        : ptr{ other.ptr ? other.copy(other.ptr) : nullptr }
        , del{ other.del }
        , copy{ other.copy }
    {
    }

    constexpr dynamic_storage& operator=(const dynamic_storage& other)
    {
        if (other.ptr != ptr)
        {
            reset();
            ptr = other.ptr ? other.copy(other.ptr) : nullptr;
            del = other.del;
            copy = other.copy;
        }
        return *this;
    }

    constexpr dynamic_storage(dynamic_storage&& other) noexcept
        : ptr{ detail::exchange(other.ptr, nullptr) }
        , del{ detail::exchange(other.del, nullptr) }
        , copy{ detail::exchange(other.copy, nullptr) }
    {
    }

    constexpr dynamic_storage& operator=(dynamic_storage&& other) noexcept
    {
        if (other.ptr != ptr)
        {
            reset();
            ptr = detail::exchange(other.ptr, nullptr);
            del = detail::exchange(other.del, nullptr);
            copy = detail::exchange(other.copy, nullptr);
        }
        return *this;
    }

    ~dynamic_storage()
    {
        reset();
    }

    constexpr void reset() noexcept
    {
        if (ptr)
            del(ptr);
        ptr = nullptr;
    }

    void* ptr = nullptr;
    void (*del)(void*) = nullptr;
    void* (*copy)(const void*) = nullptr;
};

template <std::size_t Size, std::size_t Alignment = 8>
struct local_storage {
    template <
    class T,
    class T_ = std::decay_t<T>,
    std::enable_if_t<!std::is_same_v<T_, local_storage>, bool> = true>
    constexpr explicit local_storage(T&& t) noexcept(noexcept(new (&data) T_{ std::forward<T>(t) }))
        : ptr{ new (&data) T_{ std::forward<T>(t) } }
        , del{ [](void* mem) {
            reinterpret_cast<T_*>(mem)->~T_();
        } }
        , copy{ [](const void* self, void* mem) -> void* {
            if constexpr (std::is_copy_constructible_v<T_>)
                return new (mem) T_{ *reinterpret_cast<const T_*>(self) };
            else
                throw std::runtime_error("local_storage : erased type is not copy constructible");
        } }
        , move{ [](void* self, void* mem) -> void* {
            if constexpr (std::is_move_constructible_v<T_>)
                return new (mem) T_{ std::move(*reinterpret_cast<T_*>(self)) };
            else
                throw std::runtime_error("local_storage : erased type is not move constructible");
        } }
    {
        static_assert(sizeof(T_) <= Size, "insufficient size");
        static_assert(Alignment % alignof(T_) == 0, "bad alignment");
    }

    constexpr local_storage(const local_storage& other)
        : ptr{ other.ptr ? other.copy(other.ptr, &data) : nullptr }
        , del{ other.del }
        , copy{ other.copy }
        , move{ other.move }
    {
    }

    constexpr local_storage& operator=(const local_storage& other)
    {
        if (other.ptr != ptr)
        {
            reset();
            ptr = other.ptr ? other.copy(other.ptr, &data) : nullptr;
            del = other.del;
            copy = other.copy;
            move = other.move;
        }
        return *this;
    }

    constexpr local_storage(local_storage&& other)
        : ptr{ other.ptr ? other.move(other.ptr, &data) : nullptr }
        , del{ other.del }
        , copy{ other.copy }
        , move{ other.move }
    {
    }

    constexpr local_storage& operator=(local_storage&& other)
    {
        if (other.ptr != ptr)
        {
            reset();
            ptr = other.ptr ? other.move(other.ptr, &data) : nullptr;
            del = other.del;
            copy = other.copy;
            move = other.move;
        }
        return *this;
    }

    ~local_storage()
    {
        reset();
    }

    constexpr void reset() noexcept
    {
        if (ptr)
            del(&data);
        ptr = nullptr;
    }

    std::aligned_storage_t<Size, Alignment> data;
    void* ptr = nullptr;
    void (*del)(void*) = nullptr;
    void* (*copy)(const void*, void* mem) = nullptr;
    void* (*move)(void*, void* mem) = nullptr;
};

template <std::size_t Size, std::size_t Alignment = 8>
struct sbo_storage {
    template <typename T_>
    struct type_fits : std::integral_constant<bool, sizeof(T_) <= Size && Alignment % alignof(T_) == 0> {
    };

    template <
    class T,
    class T_ = std::decay_t<T>,
    std::enable_if_t<!std::is_same_v<T_, sbo_storage>, bool> = true,
    std::enable_if_t<type_fits<T_>::value, bool> = true>
    constexpr explicit sbo_storage(T&& t) noexcept(noexcept(new (&data) T_{ std::forward<T>(t) }))
        : ptr{ new (&data) T_{ std::forward<T>(t) } }
        , del{ [](void*, void* mem) {
            reinterpret_cast<T_*>(mem)->~T_();
        } }
        , copy{ [](const void* self, void* mem) -> void* {
            if constexpr (std::is_copy_constructible_v<T_>)
                return new (mem) T_{ *reinterpret_cast<const T_*>(self) };
            else
                throw std::runtime_error("sbo_storage : erased type is not copy constructible");
        } }
        , move{ [](void*& self, void* mem) -> void* {
            if constexpr (std::is_move_constructible_v<T_>)
                return new (mem) T_{ std::move(*reinterpret_cast<T_*>(self)) };
            else
                throw std::runtime_error("sbo_storage : erased type is not move constructible");
        } }
    {
    }

    template <
    class T,
    class T_ = std::decay_t<T>,
    std::enable_if_t<!std::is_same_v<T_, sbo_storage>, bool> = true,
    std::enable_if_t<!type_fits<T_>::value, bool> = true>
    constexpr explicit sbo_storage(T&& t) noexcept(noexcept(SkrNew<T_>(std::forward<T>(t))))
        : ptr{ SkrNew<T_>(std::forward<T>(t)) }
        , del{ [](void* self, void*) {
            delete reinterpret_cast<T_*>(self);
        } }
        , copy{ [](const void* self, void*) -> void* {
            if constexpr (std::is_copy_constructible_v<T_>)
                return SkrNew<T_>(*reinterpret_cast<const T_*>(self));
            else
                throw std::runtime_error("dynamic_storage : erased type is not copy constructible");
        } }
        , move{ [](void*& self, void*) {
            return detail::exchange(self, nullptr);
        } }
    {
    }

    constexpr sbo_storage(const sbo_storage& other)
        : ptr{ other.ptr ? other.copy(other.ptr, &data) : nullptr }
        , del{ other.del }
        , copy{ other.copy }
        , move{ other.move }
    {
    }

    constexpr sbo_storage& operator=(const sbo_storage& other)
    {
        if (other.ptr != ptr)
        {
            reset();
            ptr = other.ptr ? other.copy(other.ptr, &data) : nullptr;
            del = other.del;
            copy = other.copy;
            move = other.move;
        }
        return *this;
    }

    constexpr sbo_storage(sbo_storage&& other)
        : ptr{ other.ptr ? other.move(other.ptr, &data) : nullptr }
        , del{ other.del }
        , copy{ other.copy }
        , move{ other.move }
    {
    }

    constexpr sbo_storage& operator=(sbo_storage&& other)
    {
        if (other.ptr != ptr)
        {
            reset();
            ptr = other.ptr ? other.move(other.ptr, &data) : nullptr;
            del = other.del;
            copy = other.copy;
            move = other.move;
        }
        return *this;
    }

    ~sbo_storage()
    {
        reset();
    }

    constexpr void reset() noexcept
    {
        if (ptr)
            del(ptr, &data);
        ptr = nullptr;
    }

    std::aligned_storage_t<Size, Alignment> data;
    void* ptr = nullptr;
    void (*del)(void*, void*) = nullptr;
    void* (*copy)(const void*, void*) = nullptr;
    void* (*move)(void*&, void*) = nullptr;
};

class static_vtable
{
    using ptr_t = void*;

public:
    template <class T, std::size_t Size>
    static_vtable(T&&, ptr_t*& vtable,
    std::integral_constant<std::size_t, Size>) noexcept
    {
        static ptr_t vt[Size]{};
        vtable = vt;
    }
};

namespace detail
{
struct poly_base {
    void** vptr = nullptr;
    virtual void* ptr() const noexcept = 0;
};
} // namespace detail

template <
class I,
class TStorage = dynamic_storage,
class TVtable = static_vtable>
class poly : detail::poly_base,
             public std::conditional_t<detail::is_complete<I>{}, I,
             detail::type_list<I>>
{
public:
    template <
    class T,
    class T_ = std::decay_t<T>,
    class = std::enable_if_t<not std::is_convertible<T_, poly>::value>>
    constexpr poly(T&& t) noexcept(std::is_nothrow_constructible_v<T_, T&&>)
        : poly{ std::forward<T>(t),
            detail::type_list<decltype(detail::requires__<I>(bool{}))>{} }
    {
    }

    constexpr poly(poly const&) noexcept(std::is_nothrow_copy_constructible_v<TStorage>) = default;
    constexpr poly& operator=(poly const&) noexcept(std::is_nothrow_copy_constructible_v<TStorage>) = default;
    constexpr poly(poly&&) noexcept(std::is_nothrow_move_constructible_v<TStorage>) = default;
    constexpr poly& operator=(poly&&) noexcept(std::is_nothrow_move_constructible_v<TStorage>) = default;

private:
    template <
    class T,
    class T_ = std::decay_t<T>,
    class TRequires>
    constexpr poly(T&& t, const TRequires) noexcept(std::is_nothrow_constructible_v<T_, T&&>)
        : poly{ std::forward<T>(t),
            std::make_index_sequence<detail::mappings_size<I>()>{} }
    {
    }

    template <
    class T,
    class T_ = std::decay_t<T>,
    std::size_t... Ns>
    constexpr poly(T&& t, std::index_sequence<Ns...>) noexcept(std::is_nothrow_constructible_v<T_, T&&>)
        : detail::poly_base{}
        , vtable{ std::forward<T>(t), vptr,
            std::integral_constant<std::size_t, sizeof...(Ns)>{} }
        , storage{ std::forward<T>(t) }
    {
        static_assert(sizeof...(Ns) > 0);
        static_assert(std::is_destructible<T_>{});
        static_assert(std::is_copy_constructible<T>{} or
                      std::is_move_constructible<T>{});
        (init<Ns + 1, std::decay_t<T>>(
         decltype(get(detail::mappings<I, Ns + 1>{})){}),
        ...);
    }

    template <std::size_t N, class T, class TExpr, class... TArgs>
    constexpr void init(detail::type_list<TExpr, TArgs...>) noexcept
    {
        vptr[N - 1] = reinterpret_cast<void*>(+[](void* self, TArgs... args) {
            return detail::expr_wrapper<TExpr>{}(*static_cast<T*>(self), args...);
        });
    }

    void* ptr() const noexcept
    {
        if constexpr (std::is_same_v<TStorage, shared_storage>)
            return storage.ptr.get();
        else
            return storage.ptr;
    }

    TStorage storage;
    TVtable vtable;
};

namespace detail
{
template <
class I,
std::size_t N,
class R,
class TExpr,
class... Ts>
constexpr auto call_impl(
const poly_base& self,
std::integral_constant<std::size_t, N>,
type_list<R>,
const TExpr,
Ts&&... args)
{
    void(typename mappings<I, N>::template set<type_list<TExpr, Ts...>>{});
    return reinterpret_cast<R (*)(void*, Ts...)>(self.vptr[N - 1])(
    self.ptr(), std::forward<Ts>(args)...);
}

template <class I, class T, std::size_t... Ns>
constexpr auto extends_impl(std::index_sequence<Ns...>) noexcept
{
    (void(typename mappings<T, Ns + 1>::template set<decltype(get(mappings<I, Ns + 1>{}))>{}),
    ...);
}

template <class T, class TExpr, class... Ts>
constexpr auto requires_impl(type_list<TExpr, Ts...>)
-> decltype(&TExpr::template operator()<T, Ts...>);

template <class I, class T, std::size_t... Ns>
constexpr auto requires_impl(std::index_sequence<Ns...>) -> type_list<
decltype(requires_impl<I>(decltype(get(mappings<T, Ns + 1>{})){}))...>;
} // namespace detail

template <
class R = void,
std::size_t N = 0,
class TExpr,
class I,
class... Ts>
constexpr auto call(
const TExpr expr,
const I& itf,
Ts&&... args)
{
    static_assert(std::is_empty<TExpr>{});
    return detail::call_impl<I>(
    reinterpret_cast<const detail::poly_base&>(itf),
    std::integral_constant<std::size_t, detail::mappings_size<I, class call>() + 1>{},
    detail::type_list<R>{},
    expr,
    std::forward<Ts>(args)...);
}

template <class I, class T>
constexpr auto extends(const T&) noexcept
{
    detail::extends_impl<I, T>(
    std::make_index_sequence<detail::mappings_size<I, T>()>{});
}

    #if defined(__cpp_concepts)
template <class I, class T>
concept bool var = requires
{
    detail::requires_impl<I, T>(
    std::make_index_sequence<detail::mappings_size<T, I>()>{});
};

template <class I, class T>
concept bool conceptify = requires
{
    detail::requires_impl<I, T>(
    std::make_index_sequence<detail::mappings_size<T, I>()>{});
};
    #endif

} // namespace v1
} // namespace te
} // namespace ext
} // namespace boost

    #if not defined(REQUIRES)
        #define REQUIRES(R, name, ...)                                                                          \
            R                                                                                                   \
            {                                                                                                   \
                return ::te::call<R>(                                                                           \
                [](auto&& self, auto&&... args) -> decltype(self.name(std::forward<decltype(args)>(args)...)) { \
                    return self.name(std::forward<decltype(args)>(args)...);                                    \
                },                                                                                              \
                *this, ##__VA_ARGS__);                                                                          \
            }
    #endif

#endif