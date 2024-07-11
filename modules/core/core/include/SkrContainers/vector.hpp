#pragma once
#include "SkrBase/types.h"
#include "SkrBase/containers/vector/vector_memory.hpp"
#include "SkrBase/containers/vector/vector.hpp"
#include "SkrContainers/skr_allocator.hpp"

namespace skr
{
template <typename T, typename Allocator = SkrAllocator>
using Vector = container::Vector<container::VectorMemory<
T,        /*type*/
uint64_t, /*size type*/
Allocator /*allocator*/
>>;

template <typename T, uint64_t kCount>
using FixedVector = container::Vector<container::FixedVectorMemory<
T,        /*type*/
uint64_t, /*size type*/
kCount    /*allocator*/
>>;

template <typename T, uint64_t kCount, typename Allocator = SkrAllocator>
using InlineVector = container::Vector<container::InlineVectorMemory<
T,        /*type*/
uint64_t, /*size type*/
kCount,   /*allocator*/
Allocator /*allocator*/
>>;

template <typename T>
using SerializeConstVector = Vector<T>;
} // namespace skr

// binary
namespace skr
{
namespace binary
{
template <class V>
struct ReadTrait<Vector<V>> {
    template <class... Args>
    static bool Read(SBinaryReader* archive, Vector<V>& vec, Args&&... args)
    {
        Vector<V> temp;
        uint32_t  size;
        SKR_ARCHIVE(size);

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if (!skr::binary::Archive(archive, value, std::forward<Args>(args)...))
                return false;
            temp.add(std::move(value));
        }
        vec = std::move(temp);
        return true;
    }

    template <class... Args>
    static bool Read(SBinaryReader* archive, Vector<V>& vec, VectorCheckConfig cfg, Args&&... args)
    {
        Vector<V> temp;
        uint32_t  size;
        SKR_ARCHIVE(size);
        if (size > cfg.max || size < cfg.min)
        {
            // SKR_LOG_ERROR(u8"Vector size %d is out of range [%d, %d]", size, cfg.min, cfg.max);
            return false;
        }

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if (!skr::binary::Archive(archive, value, std::forward<Args>(args)...))
                return false;
            temp.push_back(std::move(value));
        }
        vec = std::move(temp);
        return true;
    }
};
template <class V>
struct WriteTrait<Vector<V>> {
    template <class... Args>
    static bool Write(SBinaryWriter* archive, const Vector<V>& vec, Args&&... args)
    {
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            if (!skr::binary::Archive(archive, value, std::forward<Args>(args)...))
                return false;
        }
        return true;
    }
    template <class... Args>
    static bool Write(SBinaryWriter* archive, const Vector<V>& vec, VectorCheckConfig cfg, Args&&... args)
    {
        if (vec.size() > cfg.max || vec.size() < cfg.min)
        {
            // SKR_LOG_ERROR(u8"Vector size %d is out of range [%d, %d]", vec.size(), cfg.min, cfg.max);
            return false;
        }
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            if (!skr::binary::Archive(archive, value, std::forward<Args>(args)...))
                return false;
        }
        return true;
    }
};

struct VectorWriter {
    Vector<uint8_t>* buffer;

    bool write(const void* data, size_t size)
    {
        buffer->append((uint8_t*)data, size);
        return true;
    }
};

struct VectorWriterBitpacked {
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
} // namespace binary
template <typename V>
struct SerdeCompleteChecker<binary::ReadTrait<Vector<V>>>
    : std::bool_constant<is_complete_serde_v<binary::ReadTrait<V>>> {
};
} // namespace skr
