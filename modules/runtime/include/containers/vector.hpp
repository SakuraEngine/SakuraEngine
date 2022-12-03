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
#include "utils/traits.hpp"
#include "binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template <class V, class Allocator>
struct ReadTrait<skr::vector<V, Allocator>> {
    static int Read(skr_binary_reader_t* archive, skr::vector<V, Allocator>& vec)
    {
        skr::vector<V, Allocator> temp;
        uint32_t size;
        SKR_ARCHIVE(size);

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            SKR_ARCHIVE(value);
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
#include "binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template <class V, class Allocator>
struct WriteTrait<const skr::vector<V, Allocator>&> {
    static int Write(skr_binary_writer_t* archive, const skr::vector<V, Allocator>& vec)
    {
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            SKR_ARCHIVE(value);
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
} // namespace binary

template <class V, class Allocator>
struct SerdeCompleteChecker<binary::WriteTrait<const skr::vector<V, Allocator>&>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<V>>> {
};
} // namespace skr