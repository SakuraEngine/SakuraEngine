
#pragma once
#include "platform/configure.h"
#include <type_traits>

struct RUNTIME_API skr_blob_arena_t
{
    skr_blob_arena_t();
    skr_blob_arena_t(void* buffer, uint64_t base, size_t size, size_t align);
    skr_blob_arena_t(skr_blob_arena_t&& other);
    skr_blob_arena_t& operator=(skr_blob_arena_t&& other);

    ~skr_blob_arena_t();
    void* get_buffer() const { return buffer; }
    size_t get_size() const { return offset; }
    size_t get_align() const { return align; }
#ifdef SKR_BLOB_ARENA_CHECK
    void release(size_t);
#endif
    uint64_t base() const { return _base; }
private:
    void* buffer;
    uint64_t _base;
    size_t align;
    size_t offset;
    size_t capacity;
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
struct BlobHelper;
template <class T, class = void>
struct BlobBuilderType
{
    static_assert(sizeof(T), "BlobBuilderType not implemented for this type");
};
template <class T>
struct BlobBuilderType<T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>>
{
    using type = T;
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
    BlobHelper<T>::BuildArena(builder, dst, src);
    auto arena = builder.build();
    BlobHelper<T>::Remap(arena, dst);
    return arena;
}
}
using binary::make_blob_builder;
}

#define CONBINE_GEMERATED_NAME(file, type) CONBINE_GEMERATED_NAME_IMPL(file, type)
#define CONBINE_GEMERATED_NAME_IMPL(file, type) GENERATED_BLOB_BUILDER_##file##_##type
#ifdef __meta__
#define GENERATED_BLOB_BUILDER(type)
#else
#define GENERATED_BLOB_BUILDER(type) CONBINE_GEMERATED_NAME(SKR_FILE_ID, type)
#endif

#define BLOB_POD(t) template<> struct BlobBuilderType<t> {using type = t;};