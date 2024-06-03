#pragma once
#include "SkrBase/types.h"
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/span.hpp"
#include "SkrContainers/vector.hpp"

namespace skr
{
template <typename T, size_t Extent = container::kDynamicExtent>
using span = container::Span<T, size_t, Extent>;
}

namespace skr::binary
{
template <class T>
struct BlobBuilderType<skr::span<T>> {
    using type = skr::Vector<typename BlobBuilderType<T>::type>;
};
struct SpanSerdeConfig {
    uint32_t maxSize;
};
} // namespace skr::binary

// binary reader
namespace skr
{
namespace binary
{
template <class T>
struct ReadTrait<skr::span<T>> {
    template <class... Args>
    static bool Read(SBinaryReader* archive, skr::span<T> span, Args&&... args)
    {
        for (auto& v : span)
        {
            if (!skr::binary::Archive(archive, v, std::forward<Args>(args)...))
                return false;
        }
        return true;
    }

    template <class... Args>
    static bool Read(SBinaryReader* archive, skr_blob_arena_t& arena, skr::span<T>& span, Args&&... args)
    {
        // static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        uint32_t count = 0;
        SKR_ARCHIVE(count);
        if (count == 0)
        {
            span = skr::span<T>();
            return true;
        }
        uint32_t offset = 0;
        SKR_ARCHIVE(offset);
        span = skr::span<T>((T*)((char*)arena.get_buffer() + offset), count);
        for (int i = 0; i < span.size(); ++i)
        {
            if (!skr::binary::ArchiveBlob(archive, arena, span[i], std::forward<Args>(args)...))
                return false;
        }
        return true;
    }

    template <class... Args>
    static bool Read(SBinaryReader* archive, skr_blob_arena_t& arena, skr::span<T>& span, SpanSerdeConfig cfg, Args&&... args)
    {
        // static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        uint32_t count = 0;
        if (!skr::binary::Archive(archive, count, IntegerPackConfig<uint32_t>{ 0, cfg.maxSize }))
            return false;
        if (count == 0)
        {
            span = skr::span<T>();
            return true;
        }
        uint32_t offset = 0;
        SKR_ARCHIVE(offset);
        span = skr::span<T>((T*)((char*)arena.get_buffer() + offset), count);
        for (int i = 0; i < span.size(); ++i)
        {
            if (!skr::binary::ArchiveBlob(archive, arena, span[i], std::forward<Args>(args)...))
            {
                return false;
            }
        }
        return true;
    }
};

struct SpanReader {
    skr::span<const uint8_t> data;
    size_t                   offset = 0;
    bool                      read(void* dst, size_t size)
    {
        if (offset + size > data.size())
            return false;
        memcpy(dst, data.data() + offset, size);
        offset += size;
        return true;
    }
};

struct SpanReaderBitpacked {
    skr::span<const uint8_t> data;
    size_t                   offset    = 0;
    uint8_t                  bitOffset = 0;
    bool                      read(void* dst, size_t size)
    {
        return read_bits(dst, size * 8);
    }
    bool read_bits(void* dst, size_t bitSize)
    {
        if (offset + (bitSize + bitOffset) / 8 > data.size())
            return false;
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
        return true;
    }
};
} // namespace binary
} // namespace skr

// binary writer
namespace skr
{
namespace binary
{
template <class T>
struct WriteTrait<skr::span<T>> {
    template <class... Args>
    static bool Write(SBinaryWriter* writer, const skr::span<T>& span, Args&&... args)
    {
        for (const T& value : span)
        {
            if (!skr::binary::Archive(writer, value, std::forward<Args>(args)...))
                return false;
        }
        return true;
    }
    template <class... Args>
    static bool Write(SBinaryWriter* writer, skr_blob_arena_t& arena, const skr::span<T>& span, Args&&... args)
    {
        auto ptr    = (char*)span.data();
        auto buffer = (char*)arena.get_buffer();
        SKR_ASSERT(ptr >= buffer);
        uint32_t offset = (uint32_t)(ptr - buffer);
        SKR_ASSERT(!arena.get_size() || (offset < arena.get_size()) || span.empty());
        if (!skr::binary::Archive(writer, (uint32_t)span.size()))
            return false;
        if (span.empty())
            return 0;
        if (!skr::binary::Archive(writer, offset))
            return false;
        for (int i = 0; i < span.size(); ++i)
        {
            if (!skr::binary::ArchiveBlob(writer, arena, span[i], std::forward<Args>(args)...))
                return false;
        }
        return true;
    }
    template <class... Args>
    static int Write(SBinaryWriter* writer, skr_blob_arena_t& arena, const skr::span<T>& span, SpanSerdeConfig cfg, Args&&... args)
    {
        auto ptr    = (char*)span.data();
        auto buffer = (char*)arena.get_buffer();
        SKR_ASSERT(ptr >= buffer);
        uint32_t offset = (uint32_t)(ptr - buffer);
        SKR_ASSERT(!arena.get_size() || (offset < arena.get_size()) || span.empty());
        if (!skr::binary::Archive(writer, (uint32_t)span.size(), IntegerPackConfig<uint32_t>{ 0, cfg.maxSize }))
        {
            return false;
        }
        if (span.empty())
            return 0;
        if (!skr::binary::Archive(writer, offset))
            return false;
        for (int i = 0; i < span.size(); ++i)
        {
            if (!skr::binary::ArchiveBlob(writer, arena, span[i], std::forward<Args>(args)...))
                return false;
        }
        return 0;
    }
};

} // namespace binary
} // namespace skr