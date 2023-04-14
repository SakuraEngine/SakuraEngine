#pragma once
#include <EASTL/span.h>

namespace skr
{
using eastl::span;
}

#include "EASTL/vector.h"
#include "binary/blob_fwd.h"

namespace skr::binary
{
template <class T>
struct BlobBuilderType<skr::span<T>>
{
    using type = eastl::vector<typename BlobBuilderType<T>::type>;
};
}

// binary reader
#include "utils/traits.hpp"
#include "binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template<class T>
struct ReadTrait<skr::span<T>> {
    static int Read(skr_binary_reader_t* archive, skr::span<T> span)
    {
        for(auto& v : span)
        {
            SKR_ARCHIVE(v);
        }
        return 0;
    }

    static int Read(skr_binary_reader_t* archive, skr_blob_arena_t& arena, skr::span<T>& span)
    {
        //static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        uint32_t count = 0;
        SKR_ARCHIVE(count);
        if(count == 0)
        {
            span = skr::span<T>();
            return 0;
        }
        uint32_t offset = 0;
        SKR_ARCHIVE(offset);
        span = skr::span<T>((T*)((char*)arena.get_buffer() + offset), count);
        for(int i = 0; i < span.size(); ++i)
        {
            auto ret = skr::binary::Archive(archive, arena, span[i]);
            if (ret != 0) {
                return ret;
            }
        }
        return 0;
    }
};

struct SpanReader {
    skr::span<const uint8_t> data;
    size_t offset = 0;
    int read(void* dst, size_t size)
    {
        if (offset + size > data.size())
            return -1;
        memcpy(dst, data.data() + offset, size);
        offset += size;
        return 0;
    }
};
} // namespace binary
} // namespace skr

// binary writer
#include "binary/writer_fwd.h"
#include "platform/debug.h"

namespace skr
{
namespace binary
{
template <class T>
struct WriteTrait<const skr::span<T>&> {
    static int Write(skr_binary_writer_t* writer, const skr::span<T>& span)
    {
        for (const T& value : span) {
            if(auto result = skr::binary::Archive(writer, value); result != 0) {
                return result;
            }
        }
        return 0;
    }
    static int Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const skr::span<T>& span)
    {
        auto ptr = (char*)span.data();
        auto buffer = (char*)arena.get_buffer();
        SKR_ASSERT(ptr >= buffer);
        uint32_t offset = (uint32_t)(ptr - buffer);
        SKR_ASSERT(!arena.get_size() || (offset < arena.get_size()) || span.empty());
        int ret = skr::binary::Archive(writer, (uint32_t)span.size());
        if (ret != 0) {
            return ret;
        }
        if(span.empty())
            return 0;
        ret = skr::binary::Archive(writer, offset);
        if (ret != 0) {
            return ret;
        }
        for(int i = 0; i < span.size(); ++i)
        {
            ret = skr::binary::Archive(writer, arena, span[i]);
            if (ret != 0) {
                return ret;
            }
        }
        return 0;
    }
};

} // namespace binary
} // namespace skr