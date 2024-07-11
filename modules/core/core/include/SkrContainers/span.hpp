#pragma once
#include "SkrBase/types.h"
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/span.hpp"
#include "SkrContainers/vector.hpp"

namespace skr
{
template <typename T, size_t Extent = skr::container::kDynamicExtent>
using span = container::Span<T, size_t, Extent>;
}

// binary reader
namespace skr::binary
{
template <class T>
struct ReadTrait<skr::span<T>> {
    static bool Read(SBinaryReader* archive, skr::span<T> span)
    {
        for (auto& v : span)
        {
            if (!skr::binary::Read(archive, v))
                return false;
        }
        return true;
    }
};

struct SpanReader {
    skr::span<const uint8_t> data;
    size_t                   offset = 0;
    bool                     read(void* dst, size_t size)
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
    bool                     read(void* dst, size_t size)
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

template <class T>
struct WriteTrait<skr::span<T>> {
    static bool Write(SBinaryWriter* writer, const skr::span<T>& span)
    {
        for (const T& value : span)
        {
            if (!skr::binary::Write(writer, value))
                return false;
        }
        return true;
    }
};

} // namespace skr::binary

#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
namespace skr::json
{
template <class V>
struct WriteTrait<skr::span<V>> {
    static bool Write(skr::archive::JsonWriter* json, const skr::span<V>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<V>(json, v);
        }
        json->EndArray();
        return true;
    }
};
} // namespace skr::json