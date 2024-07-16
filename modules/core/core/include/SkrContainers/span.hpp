#pragma once
#include "SkrContainersDef/span.hpp"

// bin serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <class T>
struct BinSerde<skr::span<T>> {
    inline static bool read(SBinaryReader* r, skr::span<T> v)
    {
        for (auto& v : v)
        {
            if (!bin_read(r, v))
                return false;
        }
        return true;
    }
    inline static bool write(SBinaryWriter* w, const skr::span<T>& v)
    {
        for (const T& value : v)
        {
            if (!bin_write(w, value))
                return false;
        }
        return true;
    }
};
} // namespace skr

// bin reader & writer
namespace skr::archive
{
struct BinSpanReader {
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
struct BinSpanReaderBitpacked {
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
} // namespace skr::archive

// json serde
#include "SkrSerde/json_serde.hpp"
namespace skr
{
template <class V>
struct JsonSerde<skr::span<V>> {
    inline static bool write(skr::archive::JsonWriter* w, const skr::span<V>& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        for (auto& value : v)
        {
            json_write<V>(w, value);
        }
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
} // namespace skr