#pragma once
#include <type_traits>

// traits def
namespace skr::memory
{
template <typename A, typename B = A>
struct MemoryTraits {
    // need call ctor & dtor & move & copy & assign
    static constexpr bool use_ctor        = true;
    static constexpr bool use_dtor        = true;
    static constexpr bool use_copy        = true;
    static constexpr bool use_move        = true;
    static constexpr bool use_assign      = true;
    static constexpr bool use_move_assign = true;

    // special case for move
    static constexpr bool need_dtor_after_move = true;

    // use realloc for fast alloc
    static constexpr bool use_realloc = false;

    // need call compare operator (otherwise call memcmp)
    static constexpr bool use_compare = true;
};
template <typename T>
struct MemoryTraits<T, T> {
    // need call ctor & dtor & move & copy & assign
    static constexpr bool use_ctor        = !std::is_trivially_constructible_v<T>;
    static constexpr bool use_dtor        = !std::is_trivially_destructible_v<T>;
    static constexpr bool use_copy        = !std::is_trivially_copyable_v<T>;
    static constexpr bool use_move        = !std::is_trivially_move_constructible_v<T>;
    static constexpr bool use_assign      = !std::is_trivially_assignable_v<std::add_lvalue_reference_t<T>, std::add_lvalue_reference_t<T>>;
    static constexpr bool use_move_assign = !std::is_trivially_move_assignable_v<T>;

    // special case for move
    static constexpr bool need_dtor_after_move = use_dtor;

    // use realloc for fast alloc
    static constexpr bool use_realloc = std::is_trivial_v<T> && std::is_trivially_destructible_v<T>;

    // need call compare operator (otherwise call memcmp)
    static constexpr bool use_compare = !std::is_trivial_v<T>;
};
template <typename T>
struct MemoryTraits<T*, T*> {
    // need call ctor & dtor & move & copy & assign
    static constexpr bool use_ctor        = false;
    static constexpr bool use_dtor        = false;
    static constexpr bool use_copy        = false;
    static constexpr bool use_move        = false;
    static constexpr bool use_assign      = false;
    static constexpr bool use_move_assign = false;

    // special case for move
    static constexpr bool need_dtor_after_move = false;

    // use realloc for fast alloc
    static constexpr bool use_realloc = true;

    // need call compare operator (otherwise call memcmp)
    static constexpr bool use_compare = false;
};
template <typename A, typename B>
struct MemoryTraits<const A, B> : public MemoryTraits<A, B> {
};
} // namespace skr::memory

// impl for basic type
namespace skr::memory
{
#define SKR_IMPL_BASIC_MEM_POLICY(__DST, __SRC)         \
    template <>                                         \
    struct MemoryTraits<__DST, __SRC> {                 \
        static constexpr bool call_ctor        = false; \
        static constexpr bool call_dtor        = false; \
        static constexpr bool call_copy        = false; \
        static constexpr bool call_move        = false; \
        static constexpr bool call_assign      = false; \
        static constexpr bool call_move_assign = false; \
                                                        \
        static constexpr bool use_realloc = true;       \
                                                        \
        static constexpr bool call_compare = true;      \
    };

SKR_IMPL_BASIC_MEM_POLICY(uint8_t, int8_t)
SKR_IMPL_BASIC_MEM_POLICY(int8_t, uint8_t)
SKR_IMPL_BASIC_MEM_POLICY(uint16_t, int16_t)
SKR_IMPL_BASIC_MEM_POLICY(int16_t, uint16_t)
SKR_IMPL_BASIC_MEM_POLICY(uint32_t, int32_t)
SKR_IMPL_BASIC_MEM_POLICY(int32_t, uint32_t)
SKR_IMPL_BASIC_MEM_POLICY(uint64_t, int64_t)
SKR_IMPL_BASIC_MEM_POLICY(int64_t, uint64_t)

#undef SKR_IMPL_BASIC_MEM_POLICY
} // namespace skr::memory