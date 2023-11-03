#pragma once
#include "SkrRT/containers_new/skr_allocator.hpp"
#include "SkrBase/containers/array/array.hpp"

namespace skr
{
template <typename T>
using Array = container::Array<T, SkrAllocator>;
}

// serde
#include "SkrRT/serde/binary/serde.h"
#include "SkrRT/serde/binary/reader_fwd.h"
#include "SkrRT/serde/binary/writer_fwd.h"

// binary
namespace skr
{
namespace binary
{
template <class V>
struct ReadTrait<Array<V>> {
    template <class... Args>
    static int Read(skr_binary_reader_t* archive, Array<V>& vec, Args&&... args)
    {
        Array<V> temp;
        uint32_t size;
        SKR_ARCHIVE(size);

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if (auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
            temp.add(std::move(value));
        }
        vec = std::move(temp);
        return 0;
    }

    template <class... Args>
    static int Read(skr_binary_reader_t* archive, Array<V>& vec, ArrayCheckConfig cfg, Args&&... args)
    {
        Array<V> temp;
        uint32_t size;
        SKR_ARCHIVE(size);
        if (size > cfg.max || size < cfg.min)
        {
            // SKR_LOG_ERROR(u8"array size %d is out of range [%d, %d]", size, cfg.min, cfg.max);
            return -2;
        }

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if (auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
            temp.push_back(std::move(value));
        }
        vec = std::move(temp);
        return 0;
    }
};
template <class V>
struct WriteTrait<Array<V>> {
    template <class... Args>
    static int Write(skr_binary_writer_t* archive, const Array<V>& vec, Args&&... args)
    {
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            if (auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
        }
        return 0;
    }
    template <class... Args>
    static int Write(skr_binary_writer_t* archive, const Array<V>& vec, ArrayCheckConfig cfg, Args&&... args)
    {
        if (vec.size() > cfg.max || vec.size() < cfg.min)
        {
            // SKR_LOG_ERROR(u8"array size %d is out of range [%d, %d]", vec.size(), cfg.min, cfg.max);
            return -2;
        }
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            if (auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
        }
        return 0;
    }
};

struct ArrayWriter {
    Array<uint8_t>* buffer;

    int write(const void* data, size_t size)
    {
        buffer->append((uint8_t*)data, size);
        return 0;
    }
};

struct ArrayWriterBitpacked {
    Array<uint8_t>* buffer;
    uint8_t         bitOffset = 0;
    int             write(const void* data, size_t size)
    {
        return write_bits(data, size * 8);
    }
    int write_bits(const void* data, size_t bitSize)
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
                    return 0;
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
        return 0;
    }
};
} // namespace binary
template <typename V>
struct SerdeCompleteChecker<binary::ReadTrait<Array<V>>>
    : std::bool_constant<is_complete_serde_v<binary::ReadTrait<V>>> {
};
} // namespace skr
