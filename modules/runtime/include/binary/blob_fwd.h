
#pragma once
#include "platform/configure.h"
#include "eastl/vector.h"
#include "containers/span.hpp"
#include "containers/string.hpp"

struct RUNTIME_API skr_blob_arena_t
{
    skr_blob_arena_t();
    skr_blob_arena_t(size_t size, size_t align);
    skr_blob_arena_t(void* buffer, size_t size, size_t align);
    skr_blob_arena_t(skr_blob_arena_t&& other);
    skr_blob_arena_t& operator=(skr_blob_arena_t&& other);

    ~skr_blob_arena_t();
    void* get_buffer() const { return buffer; }
    void* allocate(size_t size, size_t align);
    bool filled() const { return offset == capacity; }
    size_t get_size() const { return offset; }
    size_t get_align() const { return align; }
#ifdef SKR_BLOB_ARENA_CHECK
    void release(size_t);
#endif
private:
    void* buffer;
    size_t align;
    size_t offset;
    size_t capacity;
};

struct RUNTIME_API skr_blob_arena_builder_t
{
    skr_blob_arena_builder_t(size_t align);
    ~skr_blob_arena_builder_t();
    skr_blob_arena_t build();
    void* allocate(size_t size, size_t align);
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
struct BlobOwnedType
{
    using type = T;
};
template<>
struct BlobOwnedType<skr::string_view>
{
    using type = skr::string;
};

template <class T>
struct BlobOwnedType<skr::span<T>>
{
    using type = eastl::vector<typename BlobOwnedType<T>::type>;
};
}
}