#pragma once
#include "type.hpp"
#include <containers/vector.hpp>
#include <containers/sptr.hpp>
#include <containers/span.hpp>
#include <containers/variant.hpp>
#include "resource/resource_handle.h"

#if defined(__cplusplus)
namespace skr
{
namespace type
{
struct ValueSerializePolicy {
    skr::string (*format)(void* self, const void* data, const struct skr_type_t* type);
    void (*parse)(void* self, skr::string_view str, void* data, const struct skr_type_t* type);
};

/*
struct Serializer
{
    void BeginSerialize();
    skr::string EndSerialize();
    void BeginDeserialize(skr::string_view str);
    void EndDeserialize();
    void Value(bool&);
    void Value(int32_t&);
    ....
    bool Defined(const RecordType*);
    void Enum(void* data, const EnumType*);
    void Object(void* data, const RecordType*);
    void BeginObject(const RecordType* type);
    void EndObject();
    bool BeginField(const skr_field_t* field);
    void EndField();
    void BeginArray(size_t size); void EndArray();
};
*/
template <class Serializer>
struct TValueSerializePolicy : ValueSerializePolicy {
    Serializer s;
    TValueSerializePolicy(Serializer s)
        : s(s)
    {
        format = +[](void* self, const void* data, const struct skr_type_t* type) {
            auto& s = ((TValueSerializePolicy*)self)->s;
            s.BeginSerialize();
            serializeImpl(&s, data, type);
            return s.EndSerialize();
        };
        parse = +[](void* self, skr::string_view str, void* data, const struct skr_type_t* type) {
            auto& s = ((TValueSerializePolicy*)self)->s;
            s.BeginDeserialize(str);
            serializeImpl(&s, data, type);
            s.EndDeserialize();
        };
    }

    static void serializeImpl(void* self, void* data, const struct skr_type_t* type)
    {
        auto& ctx = *(Serializer*)self;
        switch (type->type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
                ctx.Value(*(bool*)data);
                break;
            case SKR_TYPE_CATEGORY_I32:
                ctx.Value(*(int32_t*)data);
                break;
            case SKR_TYPE_CATEGORY_I64:
                ctx.Value(*(int64_t*)data);
                break;
            case SKR_TYPE_CATEGORY_U32:
                ctx.Value(*(uint32_t*)data);
                break;
            case SKR_TYPE_CATEGORY_U64:
                ctx.Value(*(uint64_t*)data);
                break;
            case SKR_TYPE_CATEGORY_F32:
                ctx.Value(*(float*)data);
                break;
            case SKR_TYPE_CATEGORY_F64:
                ctx.Value(*(double*)data);
                break;
            case SKR_TYPE_CATEGORY_STR:
                ctx.Value(*(skr::string*)data);
                break;
            case SKR_TYPE_CATEGORY_STRV:
                ctx.Value(*(skr::string_view*)data);
                break;
            case SKR_TYPE_CATEGORY_GUID:
                ctx.Value(*(skr_guid_t*)data);
                break;
            case SKR_TYPE_CATEGORY_MD5:
                ctx.Value(*(skr_md5_t*)data);
                break;
            case SKR_TYPE_CATEGORY_HANDLE:
                ctx.Value(((skr_resource_handle_t*)data)->get_serialized());
                break;
            case SKR_TYPE_CATEGORY_ARR: {
                ctx.BeginArray();
                auto& arr = (const ArrayType&)(*type);
                auto element = arr.elementType;
                auto d = (char*)data;
                auto size = element->Size();
                for (int i = 0; i < arr.num; ++i)
                    serializeImpl(&ctx, d + i * size, element);
                ctx.EndArray();
                break;
            }
            case SKR_TYPE_CATEGORY_DYNARR: {
                auto& arr = (const DynArrayType&)(*type);
                auto element = arr.elementType;
                auto d = (char*)arr.operations.data(data);
                auto n = arr.operations.size(data);
                auto size = element->Size();
                ctx.BeginArray(n);
                for (int i = 0; i < n; ++i)
                    serializeImpl(&ctx, d + i * size, element);
                ctx.EndArray();
                break;
            }
            case SKR_TYPE_CATEGORY_ARRV: {
                auto& arr = (const ArrayViewType&)(*type);
                auto& element = arr.elementType;
                auto size = element->Size();
                auto v = *(skr::span<char>*)data;
                auto n = v.size();
                auto d = v.data();
                ctx.BeginArray(n);
                for (int i = 0; i < n; ++i)
                    serializeImpl(&ctx, d + i * size, element);
                ctx.EndArray();
                break;
            }
            case SKR_TYPE_CATEGORY_OBJ: {
                auto obj = (const RecordType*)type;
                if (ctx.Defined(obj)) // object can be predefined
                {
                    ctx.Object((const RecordType*)type, data);
                }
                else
                {
                    ctx.BeginObject(obj);
                    auto d = (char*)data;
                    for (const auto& field : obj->fields)
                    {
                        if (ctx.BeginField(field))
                        {
                            serializeImpl(&ctx, d + field.offset, field.type);
                            ctx.EndField();
                        }
                    }
                    ctx.EndObject();
                }
            }
            case SKR_TYPE_CATEGORY_ENUM: {
                auto enm = (const EnumType*)type;
                ctx.Enum(data, enm);
            }
            case SKR_TYPE_CATEGORY_REF: {
                void* address;
                auto ref = ((const ReferenceType*)type);
                switch (ref->ownership)
                {
                    case ReferenceType::Observed:
                        address = *(void**)data;
                        break;
                    case ReferenceType::Shared:
                        address = (*(skr::SPtr<void>*)data).get();
                        break;
                }
                serializeImpl(&ctx, address, ref->pointee);
                break;
            }
            default:
                SKR_UNREACHABLE_CODE();
        }
    }
};
} // namespace type
} // namespace skr

#endif