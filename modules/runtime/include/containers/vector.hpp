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
// std::vector<T>
struct DynArrayMethodTable {
    void (*dtor)(void* self);
    void (*ctor)(void* self, Value* param, size_t nparam);
    void (*copy)(void* self, const void* other);
    void (*move)(void* self, void* other);
    void (*push)(void* self, const void* data);
    void (*insert)(void* self, const void* data, size_t index);
    void (*erase)(void* self, size_t index);
    void (*resize)(void* self, size_t size);
    size_t (*size)(const void* self);
    void* (*get)(const void* self, size_t index);
    void* (*data)(const void* self);
    int (*Serialize)(const void* self, skr_binary_writer_t* writer);
    int (*Deserialize)(void* self, skr_binary_reader_t* reader);
    void (*deleter)(void* self);
    void (*SerializeText)(const void*, skr_json_writer_t* writer);
    json::error_code (*DeserializeText)(void* self, json::value_t&& reader);
};

// std::vector<T>
struct DynArrayType : skr_type_t {
    const struct skr_type_t* elementType;
    DynArrayMethodTable operations;
    skr::string name;
    DynArrayType(const skr_type_t* elementType, DynArrayMethodTable operations)
        : skr_type_t{ SKR_TYPE_CATEGORY_DYNARR }
        , elementType(elementType)
        , operations(operations)
    {
    }
};

template <class V, class T>
struct type_of_vector {
    static const skr_type_t* get()
    {
        static DynArrayType type{
            type_of<T>::get(),
            DynArrayMethodTable{
            GetDtor<V>(),                     // dtor
            GetDefaultCtor<V>(),                                                                           // ctor
            +[](void* self, const void* other) { new (self) V(*((const V*)(other))); },                                                // copy
            +[](void* self, void* other) { new (self) V(std::move(*(V*)(other))); },                                                   // move
            +[](void* self, const void* data) { ((V*)(self))->push_back(*(const T*)data); },                                           // push
            +[](void* self, const void* data, size_t index) { ((V*)(self))->insert(((V*)(self))->begin() + index, *(const T*)data); }, // insert
            +[](void* self, size_t index) { ((V*)(self))->erase(((V*)(self))->begin() + index); },                                     // erase
            +[](void* self, size_t size) { ((V*)(self))->resize(size); },                                                              // resize
            +[](const void* self) { return ((V*)(self))->size(); },                                                                    // size
            +[](const void* self, size_t index) { return (void*)&((V*)(self))[index]; },                                               // get
            +[](const void* self) { return (void*)((V*)(self))->data(); },                                                             // data
            GetSerialize<V>(),                      // serialize
            GetDeserialize<V>(),                     // deserialize
            GetDeleter<V>(),                         // deleter
            GetTextSerialize<V>(),                 // text serializer
            GetTextDeserialize<V>(),               // text deserializer
            }
        };
        return &type;
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