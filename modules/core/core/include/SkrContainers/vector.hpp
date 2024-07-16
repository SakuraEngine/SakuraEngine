#pragma once
#include "SkrContainersDef/vector.hpp"

// bin serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <typename V>
struct BinSerde<Vector<V>> {
    inline static bool read(SBinaryReader* r, Vector<V>& v)
    {
        // read bin
        uint32_t size;
        if (!bin_read(r, (size))) return false;

        // read content
        Vector<V> temp;
        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if (!bin_read(r, value))
                return false;
            temp.add(std::move(value));
        }

        // move to target
        v = std::move(temp);
        return true;
    }
    inline static bool write(SBinaryWriter* r, const Vector<V>& v)
    {
        // write bin
        if (!bin_write(r, ((uint32_t)v.size()))) return false;

        // write content
        for (auto& value : v)
        {
            if (!bin_write(r, value))
                return false;
        }
        return true;
    }
};
} // namespace skr

// vector bin reader writer
namespace skr::archive
{
struct BinVectorWriter {
    Vector<uint8_t>* buffer;

    bool write(const void* data, size_t size)
    {
        buffer->append((uint8_t*)data, size);
        return true;
    }
};
struct BinVectorWriterBitpacked {
    Vector<uint8_t>* buffer;
    uint8_t          bitOffset = 0;
    bool             write(const void* data, size_t size)
    {
        return write_bits(data, size * 8);
    }
    bool write_bits(const void* data, size_t bitSize)
    {
        uint8_t* dataPtr = (uint8_t*)data;
        if (bitOffset == 0)
        {
            buffer->append(dataPtr, (bitSize + 7) / 8);
            bitOffset = bitSize % 8;
            if (bitOffset != 0)
                buffer->last() &= (1 << bitOffset) - 1;
        }
        else
        {
            buffer->last() |= dataPtr[0] << bitOffset;
            int i = 1;
            while (bitSize > 8)
            {
                buffer->add((dataPtr[i - 1] >> (8 - bitOffset)) | (dataPtr[i] << bitOffset));
                ++i;
                bitSize -= 8;
            }
            if (bitSize > 0)
            {
                auto newBitOffset = bitOffset + bitSize;
                if (newBitOffset == 8)
                {
                    bitOffset = 0;
                    return true;
                }
                else if (newBitOffset > 8)
                {
                    buffer->add(dataPtr[i - 1] >> (8 - bitOffset));
                    newBitOffset = newBitOffset - 8;
                }
                buffer->last() &= (1 << newBitOffset) - 1;
                SKR_ASSERT(newBitOffset <= UINT8_MAX);
                bitOffset = (uint8_t)newBitOffset;
            }
        }
        return true;
    }
};

}; // namespace skr::archive

// json serde
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
namespace skr::json
{
template <class V>
struct WriteTrait<skr::Vector<V>> {
    static bool Write(skr::archive::JsonWriter* json, const skr::Vector<V>& vec)
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
template <class V>
struct ReadTrait<skr::Vector<V>> {
    static bool Read(skr::archive::JsonReader* json, skr::Vector<V>& vec)
    {
        size_t count;
        json->StartArray(count);
        vec.reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            V v;
            if (!skr::json::Read<V>(json, v))
                return false;
            vec.emplace(std::move(v));
        }
        json->EndArray();
        return true;
    }
};
} // namespace skr::json