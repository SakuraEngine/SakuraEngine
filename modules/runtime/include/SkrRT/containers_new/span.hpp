#pragma once
#include "SkrBase/containers/misc/span.hpp"
#include "SkrRT/containers_new/array.hpp"

namespace skr
{
template <typename T>
using Span = container::Span<T, size_t>;
}

#include "SkrRT/serde/binary/blob_fwd.h"
#include "SkrRT/serde/binary/serde.h"
#include "SkrBase/misc/debug.h" 
#include "SkrRT/serde/binary/reader_fwd.h"
#include "SkrBase/misc/debug.h" 
#include "SkrRT/serde/binary/writer_fwd.h"

namespace skr
{
namespace binary
{
template <class T>
struct BlobBuilderType<Span<T>> {
    using type = vector<typename BlobBuilderType<T>::type>;
};
struct NewSpanSerdeConfig {
    uint32_t maxSize;
};

template <class T>
struct ReadTrait<Span<T>> {
    template <class... Args>
    static int Read(skr_binary_reader_t* archive, Span<T> span, Args&&... args)
    {
        for (auto& v : span)
        {
            if (auto ret = skr::binary::Archive(archive, v, std::forward<Args>(args)...); ret != 0) return ret;
        }
        return 0;
    }

    template <class... Args>
    static int Read(skr_binary_reader_t* archive, skr_blob_arena_t& arena, Span<T>& span, Args&&... args)
    {
        // static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        uint32_t count = 0;
        SKR_ARCHIVE(count);
        if (count == 0)
        {
            span = Span<T>();
            return 0;
        }
        uint32_t offset = 0;
        SKR_ARCHIVE(offset);
        span = Span<T>((T*)((char*)arena.get_buffer() + offset), count);
        for (int i = 0; i < span.size(); ++i)
        {
            auto ret = skr::binary::ArchiveBlob(archive, arena, span[i], std::forward<Args>(args)...);
            if (ret != 0)
            {
                return ret;
            }
        }
        return 0;
    }

    template <class... Args>
    static int Read(skr_binary_reader_t* archive, skr_blob_arena_t& arena, Span<T>& span, NewSpanSerdeConfig cfg, Args&&... args)
    {
        // static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        uint32_t count = 0;
        int      ret   = skr::binary::Archive(archive, count, IntegerPackConfig<uint32_t>{ 0, cfg.maxSize });
        if (ret != 0)
            return ret;
        if (count == 0)
        {
            span = Span<T>();
            return 0;
        }
        uint32_t offset = 0;
        SKR_ARCHIVE(offset);
        span = Span<T>((T*)((char*)arena.get_buffer() + offset), count);
        for (int i = 0; i < span.size(); ++i)
        {
            auto ret = skr::binary::ArchiveBlob(archive, arena, span[i], std::forward<Args>(args)...);
            if (ret != 0)
            {
                return ret;
            }
        }
        return 0;
    }
};

struct NewSpanReader {
    Span<const uint8_t> data;
    size_t              offset = 0;
    int                 read(void* dst, size_t size)
    {
        if (offset + size > data.size())
            return -1;
        memcpy(dst, data.data() + offset, size);
        offset += size;
        return 0;
    }
};

struct NewSpanReaderBitpacked {
    Span<const uint8_t> data;
    size_t              offset    = 0;
    uint8_t             bitOffset = 0;
    int                 read(void* dst, size_t size)
    {
        return read_bits(dst, size * 8);
    }
    int read_bits(void* dst, size_t bitSize)
    {
        if (offset + (bitSize + bitOffset) / 8 > data.size())
            return -1;
        uint8_t* dstPtr = (uint8_t*)dst;
        if (bitOffset == 0)
        {
            memcpy(dst, data.data() + offset, (bitSize + 7) / 8);
            offset += bitSize / 8;
            bitOffset = bitSize % 8;
            if (bitOffset != 0)
                dstPtr[(bitSize + 7) / 8 - 1] &= (1 << bitOffset) - 1;
        }
        else
        {
            int i = 0;
            while (bitSize >= 8)
            {
                dstPtr[i] = data[offset] >> bitOffset | data[offset + 1] << (8 - bitOffset);
                ++offset;
                ++i;
                bitSize -= 8;
            }
            if (bitSize > 0)
            {
                auto newBitOffset = bitOffset + bitSize;
                if (newBitOffset > 8)
                {
                    dstPtr[i] = data[offset] >> bitOffset | data[offset + 1] << (8 - bitOffset);
                    ++offset;
                    newBitOffset -= 8;
                }
                else
                {
                    dstPtr[i] = data[offset] >> bitOffset;
                }
                SKR_ASSERT(newBitOffset <= UINT8_MAX);
                bitOffset = (uint8_t)newBitOffset;
                dstPtr[i] &= (1 << bitSize) - 1;
            }
        }
        return 0;
    }
};

template <class T>
struct WriteTrait<Span<T>> {
    template <class... Args>
    static int Write(skr_binary_writer_t* writer, const Span<T>& span, Args&&... args)
    {
        for (const T& value : span)
        {
            if (auto result = skr::binary::Archive(writer, value, std::forward<Args>(args)...); result != 0)
            {
                return result;
            }
        }
        return 0;
    }
    template <class... Args>
    static int Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const Span<T>& span, Args&&... args)
    {
        auto ptr    = (char*)span.data();
        auto buffer = (char*)arena.get_buffer();
        SKR_ASSERT(ptr >= buffer);
        uint32_t offset = (uint32_t)(ptr - buffer);
        SKR_ASSERT(!arena.get_size() || (offset < arena.get_size()) || span.empty());
        int ret = skr::binary::Archive(writer, (uint32_t)span.size());
        if (ret != 0)
        {
            return ret;
        }
        if (span.empty())
            return 0;
        ret = skr::binary::Archive(writer, offset);
        if (ret != 0)
        {
            return ret;
        }
        for (int i = 0; i < span.size(); ++i)
        {
            ret = skr::binary::ArchiveBlob(writer, arena, span[i], std::forward<Args>(args)...);
            if (ret != 0)
            {
                return ret;
            }
        }
        return 0;
    }
    template <class... Args>
    static int Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const Span<T>& span, NewSpanSerdeConfig cfg, Args&&... args)
    {
        auto ptr    = (char*)span.data();
        auto buffer = (char*)arena.get_buffer();
        SKR_ASSERT(ptr >= buffer);
        uint32_t offset = (uint32_t)(ptr - buffer);
        SKR_ASSERT(!arena.get_size() || (offset < arena.get_size()) || span.empty());
        int ret = skr::binary::Archive(writer, (uint32_t)span.size(), IntegerPackConfig<uint32_t>{ 0, cfg.maxSize });
        if (ret != 0)
        {
            return ret;
        }
        if (span.empty())
            return 0;
        ret = skr::binary::Archive(writer, offset);
        if (ret != 0)
        {
            return ret;
        }
        for (int i = 0; i < span.size(); ++i)
        {
            ret = skr::binary::ArchiveBlob(writer, arena, span[i], std::forward<Args>(args)...);
            if (ret != 0)
            {
                return ret;
            }
        }
        return 0;
    }
};

} // namespace binary

} // namespace skr