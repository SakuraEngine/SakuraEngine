#pragma once
#include <EASTL/vector.h>

namespace skr
{
using eastl::vector;
}

#include "type/type.hpp"

namespace skr
{
namespace type
{
struct DynArrayStorage
{
    uint8_t* begin;
    uint8_t* end;
    uint8_t* capacity;
};
// skr::vector<T>
struct DynArrayType : skr_type_t {
    const struct skr_type_t* elementType;
    skr::string name;
    RUNTIME_API uint64_t Num(void* data) const;
    RUNTIME_API void* Get(void* data, uint64_t index) const;
    RUNTIME_API void Resize(void* data, uint64_t size) const;
    //RUNTIME_API void* Get(void* data, uint64_t index);
    //RUNTIME_API void* Add(void* data);
    DynArrayType(const skr_type_t* elementType)
        : skr_type_t{ SKR_TYPE_CATEGORY_DYNARR }
        , elementType(elementType)
    {
    }
};
RUNTIME_API const skr_type_t* make_dynarray_type(const skr_type_t* type);
template <class V, class T>
struct type_of_vector {
    static const skr_type_t* get()
    {
        return make_dynarray_type(type_of<T>::get());
    }
};

template <class T, class Allocator>\
struct type_of<skr::vector<T, Allocator>> : type_of_vector<skr::vector<T, Allocator>, T> 
{
};

}
}

// binary reader
#include "misc/traits.hpp"
#include "serde/binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template <class V, class Allocator>
struct ReadTrait<skr::vector<V, Allocator>> {
    template<class... Args>
    static int Read(skr_binary_reader_t* archive, skr::vector<V, Allocator>& vec, Args&&... args)
    {
        skr::vector<V, Allocator> temp;
        uint32_t size;
        SKR_ARCHIVE(size);

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if(auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
            temp.push_back(std::move(value));
        }
        vec = std::move(temp);
        return 0;
    }

    template<class... Args>
    static int Read(skr_binary_reader_t* archive, skr::vector<V, Allocator>& vec, ArrayCheckConfig cfg, Args&&... args)
    {
        skr::vector<V, Allocator> temp;
        uint32_t size;
        SKR_ARCHIVE(size);
        if(size > cfg.max || size < cfg.min) 
        {
            //SKR_LOG_ERROR("array size %d is out of range [%d, %d]", size, cfg.min, cfg.max);
            return -2;
        }

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if(auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
            temp.push_back(std::move(value));
        }
        vec = std::move(temp);
        return 0;
    }
};
}
template <typename V, typename Allocator>
struct SerdeCompleteChecker<binary::ReadTrait<skr::vector<V, Allocator>>>
    : std::bool_constant<is_complete_serde_v<binary::ReadTrait<V>>> {
};
}

// binary writer
#include "serde/binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template <class V, class Allocator>
struct WriteTrait<const skr::vector<V, Allocator>&> {
    template<class... Args>
    static int Write(skr_binary_writer_t* archive, const skr::vector<V, Allocator>& vec, Args&&... args)
    {
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            if(auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
        }
        return 0;
    }
    template<class... Args>
    static int Write(skr_binary_writer_t* archive, const skr::vector<V, Allocator>& vec, ArrayCheckConfig cfg, Args&&... args)
    {
        if(vec.size() > cfg.max || vec.size() < cfg.min) 
        {
            //SKR_LOG_ERROR("array size %d is out of range [%d, %d]", vec.size(), cfg.min, cfg.max);
            return -2;
        }
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            if(auto ret = skr::binary::Archive(archive, value, std::forward<Args>(args)...); ret != 0) return ret;
        }
        return 0;
    }
};

struct VectorWriter
{
    eastl::vector<uint8_t>* buffer;
    int write(const void* data, size_t size)
    {
        buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
        return 0;
    }
};

struct VectorWriterBitpacked
{
    eastl::vector<uint8_t>* buffer;
    uint8_t bitOffset = 0;
    int write(const void* data, size_t size)
    {
        return write_bits(data, size * 8);
    }
    int write_bits(const void* data, size_t bitSize)
    {
        uint8_t* dataPtr = (uint8_t*)data;
        if(bitOffset == 0)
        {
            buffer->insert(buffer->end(), dataPtr, dataPtr + (bitSize + 7) / 8);
            bitOffset = bitSize % 8;
            if(bitOffset != 0)
                buffer->back() &= (1 << bitOffset) - 1;
        }
        else
        {
            buffer->back() |= dataPtr[0] << bitOffset;
            int i = 1;
            while(bitSize > 8)
            {
                buffer->push_back((dataPtr[i - 1] >> (8 - bitOffset)) | (dataPtr[i] << bitOffset));
                ++i;
                bitSize -= 8;
            }
            if(bitSize > 0)
            {
                auto newBitOffset = bitOffset + bitSize;
                if(newBitOffset == 8)
                {
                    bitOffset = 0;
                    return 0;
                }
                else if(newBitOffset > 8)
                {
                    buffer->push_back(dataPtr[i - 1] >> (8 - bitOffset));
                    newBitOffset = newBitOffset - 8;
                }
                buffer->back() &= (1 << newBitOffset) - 1;
                SKR_ASSERT(newBitOffset <= UINT8_MAX);
                bitOffset = (uint8_t)newBitOffset;
            }
        }
        return 0;
    }
};
} // namespace binary

template <class V, class Allocator>
struct SerdeCompleteChecker<binary::WriteTrait<const skr::vector<V, Allocator>&>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<V>>> {
};
} // namespace skr