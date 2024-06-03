// TODO. ！！！！！！！！！！！ MOVE TO SKR_SERDE
#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"
#include <limits>

namespace skr
{

namespace binary
{
template <class T>
struct FloatingPackConfig {
    using type      = T;
    bool asIntegers = false;
    // scale before convert to integer
    T scale       = 1.0f;
    using Integer = std::conditional_t<std::is_same_v<T, float>, int32_t, int64_t>;
    T max         = std::numeric_limits<Integer>::max();
};

template <class T>
struct IntegerPackConfig {
    using type = T;
    T min      = std::numeric_limits<T>::min();
    T max      = std::numeric_limits<T>::max();
};

template <class T>
struct VectorPackConfig {
    using type  = T;
    float scale = 1.0f;
};

template <class T, class E>
struct ContainerConfig {
    using type    = T;
    using element = E;
};

struct VectorCheckConfig {
    uint64_t max = std::numeric_limits<uint64_t>::max();
    uint64_t min = std::numeric_limits<uint64_t>::min();
};
enum class ErrorCode
{
    UnknownError = -1,
    Success      = 0,
    OutOfRange   = -2,
};

} // namespace binary

template <class T>
struct SerdeCompleteChecker : std::true_type {
};

template <class T>
inline constexpr bool is_complete_serde()
{
    if constexpr (skr::is_complete_v<T>)
    {
        return SerdeCompleteChecker<T>::value;
    }
    else
        return false;
}

template <class T>
constexpr bool is_complete_serde_v = is_complete_serde<T>();
} // namespace skr

#ifndef SKR_ARCHIVE
    #define SKR_ARCHIVE(...) \
        if (!skr::binary::Archive(archive, (__VA_ARGS__))) return false
#endif

#pragma region BLOB

struct SKR_STATIC_API skr_blob_arena_t {
    skr_blob_arena_t();
    skr_blob_arena_t(void* buffer, uint64_t base, uint32_t size, uint32_t align);
    skr_blob_arena_t(skr_blob_arena_t&& other);
    skr_blob_arena_t& operator=(skr_blob_arena_t&& other);

    ~skr_blob_arena_t();
    void*    get_buffer() const { return buffer; }
    uint32_t get_size() const { return offset; }
    uint32_t get_align() const { return align; }
#ifdef SKR_BLOB_ARENA_CHECK
    void release(uint32_t);
#endif
    uint64_t base() const { return _base; }

private:
    void*    buffer;
    uint64_t _base;
    uint32_t align;
    uint32_t offset;
    uint32_t capacity;
};

struct SKR_STATIC_API skr_blob_arena_builder_t {
    skr_blob_arena_builder_t(size_t align);
    ~skr_blob_arena_builder_t();
    skr_blob_arena_t build();
    size_t           allocate(size_t size, size_t align);
    void*            get_buffer() const { return buffer; }

private:
    void*  buffer;
    size_t bufferAlign;
    size_t offset;
    size_t capacity;
};

namespace skr
{
namespace binary
{
template <class T, class = void>
struct BlobTrait;
template <class T, class = void>
struct BlobBuilderType {
    using type = T;
    static_assert(!sizeof(T), "BlobBuilderType not implemented for this type");
};
template <class T>
struct BlobBuilderType<T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>> {
    using type = T;
};
template <class T, size_t size>
struct BlobBuilderType<T[size], std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>> {
    using type = T[size];
};
template <class T>
auto make_blob_builder()
{
    return typename binary::BlobBuilderType<T>::type{};
}
template <class T>
skr_blob_arena_t make_arena(T& dst, const typename BlobBuilderType<T>::type& src, size_t align = 32)
{
    skr_blob_arena_builder_t builder(align);
    BlobTrait<T>::BuildArena(builder, dst, src);
    auto arena = builder.build();
    BlobTrait<T>::Remap(arena, dst);
    return arena;
}
} // namespace binary
using binary::make_blob_builder;
} // namespace skr

#define BLOB_CONBINE_GENERATED_NAME(file, type) BLOB_CONBINE_GENERATED_NAME_IMPL(file, type)
#define BLOB_CONBINE_GENERATED_NAME_IMPL(file, type) GENERATED_BLOB_BUILDER_##file##_##type
#ifdef __meta__
    #define GENERATED_BLOB_BUILDER(type) struct type##Builder;
#else
    #define GENERATED_BLOB_BUILDER(type) BLOB_CONBINE_GENERATED_NAME(SKR_FILE_ID, type)
#endif

#define BLOB_POD(t)             \
    template <>                 \
    struct BlobBuilderType<t> { \
        using type = t;         \
    };

#pragma endregion BLOB

#pragma region READER TRAITS

// FUCK MSVC COMPILER
SKR_EXTERN_C SKR_STATIC_API void skr_debug_output(const char* msg);

struct SBinaryReader;

namespace skr
{
namespace binary
{
template <class T, class = void>
struct ReadTrait;

template <class T, class... Args>
bool Archive(SBinaryReader* reader, T&& value, Args&&... args)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, value, std::forward<Args>(args)...);
}

template <class T, class... Args>
bool ArchiveBlob(SBinaryReader* reader, skr_blob_arena_t& arena, T&& value, Args&&... args)
{
    if constexpr (is_complete_v<BlobTrait<std::decay_t<T>>>)
    {
        if (arena.get_buffer() == nullptr)
        {
            SKR_ASSERT(!arena.get_size());
            return true;
        }
        return ReadTrait<std::decay_t<T>>::Read(reader, arena, value, std::forward<Args>(args)...);
    }
    else
        return ReadTrait<std::decay_t<T>>::Read(reader, value, std::forward<Args>(args)...);
}
} // namespace binary
} // namespace skr

#pragma endregion READER TRAITS

#pragma region WRITER TRAITS

struct SBinaryWriter;

namespace skr::binary
{
template <class T, class = void>
struct WriteTrait;

template <class T, class... Args>
bool Archive(SBinaryWriter* writer, const T& value, Args&&... args)
{
    return WriteTrait<T>::Write(writer, value, std::forward<Args>(args)...);
}

template <class T, class... Args>
bool ArchiveBlob(SBinaryWriter* writer, skr_blob_arena_t& arena, const T& value, Args&&... args)
{
    if constexpr (is_complete_v<BlobTrait<T>>)
    {
        if (arena.get_buffer() == nullptr)
        {
            SKR_ASSERT(!arena.get_size());
            return true;
        }
        return WriteTrait<T>::Write(writer, arena, value, std::forward<Args>(args)...);
    }
    else
        return WriteTrait<T>::Write(writer, value, std::forward<Args>(args)...);
}
} // namespace skr::binary

#pragma endregion WRITER TRAITS