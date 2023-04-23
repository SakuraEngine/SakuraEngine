
#pragma once
#include "platform/configure.h"
#include <type_traits>

struct RUNTIME_API skr_blob_arena_t
{
    skr_blob_arena_t();
    skr_blob_arena_t(void* buffer, uint64_t base, uint32_t size, uint32_t align);
    skr_blob_arena_t(skr_blob_arena_t&& other);
    skr_blob_arena_t& operator=(skr_blob_arena_t&& other);

    ~skr_blob_arena_t();
    void* get_buffer() const { return buffer; }
    uint32_t get_size() const { return offset; }
    uint32_t get_align() const { return align; }
#ifdef SKR_BLOB_ARENA_CHECK
    void release(uint32_t);
#endif
    uint64_t base() const { return _base; }
private:
    void* buffer;
    uint64_t _base;
    uint32_t align;
    uint32_t offset;
    uint32_t capacity;
};

struct RUNTIME_API skr_blob_arena_builder_t
{
    skr_blob_arena_builder_t(size_t align);
    ~skr_blob_arena_builder_t();
    skr_blob_arena_t build();
    size_t allocate(size_t size, size_t align);
    void* get_buffer() const { return buffer; }
private:
    void* buffer;
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
struct BlobBuilderType
{
    using type = T;
    static_assert(!sizeof(T), "BlobBuilderType not implemented for this type");
};
template <class T>
struct BlobBuilderType<T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>>
{
    using type = T;
};
template <class T, size_t size>
struct BlobBuilderType<T[size], std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>>
{
    using type = T[size];
};
template<class T>
auto make_blob_builder()
{
    return typename binary::BlobBuilderType<T>::type{};
}
template<class T>
skr_blob_arena_t make_arena(T& dst, const typename BlobBuilderType<T>::type& src, size_t align = 32)
{
    skr_blob_arena_builder_t builder(align);
    BlobTrait<T>::BuildArena(builder, dst, src);
    auto arena = builder.build();
    BlobTrait<T>::Remap(arena, dst);
    return arena;
}
}
using binary::make_blob_builder;
}

#define BLOB_CONBINE_GENERATED_NAME(file, type) BLOB_CONBINE_GENERATED_NAME_IMPL(file, type)
#define BLOB_CONBINE_GENERATED_NAME_IMPL(file, type) GENERATED_BLOB_BUILDER_##file##_##type
#ifdef __meta__
#define GENERATED_BLOB_BUILDER(type) struct type##Builder;
#else
#define GENERATED_BLOB_BUILDER(type) BLOB_CONBINE_GENERATED_NAME(SKR_FILE_ID, type)
#endif

#define BLOB_POD(t) template<> struct BlobBuilderType<t> {using type = t;};