#include "fmt/core.h"
#include "containers/sptr.hpp"
#include "containers/vector.hpp"
#include "utils/format.hpp"
#include "platform/memory.h"
#include "platform/guid.hpp"
#include "resource/resource_handle.h"
#include "containers/hashmap.hpp"
#include "binary/reader.h"
#include "binary/writer.h"
#include "json/reader.h"
#include "json/writer.h"
#include "type/type.hpp"
#include "type/type_serde.h"

static auto& skr_get_type_name_map()
{
    static skr::flat_hash_map<skr_guid_t, const char*, skr::guid::hash> type_name_map;
    return type_name_map;
}

const char* skr_get_type_name(const skr_guid_t* typeId)
{
    auto& type_name_map = skr_get_type_name_map();
    auto it = type_name_map.find(*typeId);
    if (it != type_name_map.end())
    {
        return it->second;
    }
    else
    {
        if (auto type = skr_get_type(typeId))
            return type->Name();
    }
    return nullptr;
}

void skr_register_type_name(const skr_guid_t* type, const char* name)
{
    auto& type_name_map = skr_get_type_name_map();
    type_name_map[*type] = name;
}

namespace skr::type
{
struct STypeRegistryImpl final : public STypeRegistry {
    const skr_type_t* get_type(skr_guid_t guid) final
    {
        auto it = types.find(guid);
        if (it != types.end())
        {
            return it->second;
        }
        return nullptr;
    }

    const void register_type(skr_guid_t tid, const skr_type_t* type) final
    {
        types.insert({ tid, type });
    }

    skr::flat_hash_map<skr_guid_t, const skr_type_t*, skr::guid::hash> types;
};

RUNTIME_API STypeRegistry* GetTypeRegistry()
{
    static STypeRegistryImpl registry = {};
    return &registry;
}
} // namespace skr::type

skr_type_t::skr_type_t(skr_type_category_t type)
    : type(type)
{
}

#define SKR_TYPE_TRIVAL(fun)                             \
    fun(SKR_TYPE_CATEGORY_BOOL, bool)                    \
    fun(SKR_TYPE_CATEGORY_I32, int32_t)                  \
    fun(SKR_TYPE_CATEGORY_I64, int64_t)                  \
    fun(SKR_TYPE_CATEGORY_U32, uint32_t)                 \
    fun(SKR_TYPE_CATEGORY_U64, uint64_t)                 \
    fun(SKR_TYPE_CATEGORY_F32, float)                    \
    fun(SKR_TYPE_CATEGORY_F32_2, skr_float2_t)           \
    fun(SKR_TYPE_CATEGORY_F32_3, skr_float3_t)           \
    fun(SKR_TYPE_CATEGORY_F32_4, skr_float4_t)           \
    fun(SKR_TYPE_CATEGORY_F32_4x4, skr_float4x4_t)       \
    fun(SKR_TYPE_CATEGORY_ROT, skr_rotator_t)            \
    fun(SKR_TYPE_CATEGORY_QUAT, skr_quaternion_t)        \
    fun(SKR_TYPE_CATEGORY_F64, double)                   \
    fun(SKR_TYPE_CATEGORY_GUID, skr_guid_t)              \
    fun(SKR_TYPE_CATEGORY_MD5, skr_md5_t)              \
    fun(SKR_TYPE_CATEGORY_HANDLE, skr_resource_handle_t) \
    fun(SKR_TYPE_CATEGORY_STR, skr::string)              \
    fun(SKR_TYPE_CATEGORY_STRV, skr::string_view)

size_t skr_type_t::Size() const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return sizeof(type);
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR:
            return ((ArrayType*)this)->size;
        case SKR_TYPE_CATEGORY_DYNARR:
            return sizeof(eastl::vector<char>);
        case SKR_TYPE_CATEGORY_ARRV:
            return sizeof(skr::span<char>);
        case SKR_TYPE_CATEGORY_OBJ:
            return ((RecordType*)this)->size;
        case SKR_TYPE_CATEGORY_ENUM:
            return ((EnumType*)this)->underlyingType->Size();
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    return sizeof(void*);
                case ReferenceType::Shared:
                    if (((ReferenceType*)this)->object)
                        return sizeof(skr::SObjectPtr<skr::SInterface>);
                    else
                        return sizeof(skr::SPtr<void>);
            }
        }
        case SKR_TYPE_CATEGORY_VARIANT:
            return ((VariantType*)this)->size;
    }
    return 0;
}

size_t skr_type_t::Align() const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return alignof(type);
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR:
            return ((ArrayType*)this)->elementType->Align();
        case SKR_TYPE_CATEGORY_DYNARR:
            return alignof(eastl::vector<char>);
        case SKR_TYPE_CATEGORY_ARRV:
            return alignof(skr::span<char>);
        case SKR_TYPE_CATEGORY_OBJ:
            return ((RecordType*)this)->align;
        case SKR_TYPE_CATEGORY_ENUM:
            return ((EnumType*)this)->underlyingType->Align();
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    return alignof(void*);
                case ReferenceType::Shared:
                    if (((ReferenceType*)this)->object)
                        return alignof(skr::SObjectPtr<skr::SInterface>);
                    else
                        return alignof(skr::SPtr<void>);
            }
        }
        case SKR_TYPE_CATEGORY_VARIANT:
            return ((VariantType*)this)->align;
    }
    return 0;
}

skr_guid_t skr_type_t::Id() const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return type_id<type>::get();
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_OBJ:
            return ((RecordType*)this)->guid;
        default:
            SKR_UNREACHABLE_CODE();
            break;
    }
    return {};
}

const char* skr_type_t::Name() const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return #type;
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (ArrayType&)(*this);
            if (arr.name.empty())
                arr.name = skr::string(arr.elementType->Name()) + "[" + skr::to_string(arr.size) + "]";
            return arr.name.c_str();
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (DynArrayType&)(*this);
            if (arr.name.empty())
                arr.name = skr::format("eastl::vector<{}>", arr.elementType->Name());
            return arr.name.c_str();
        }
        case SKR_TYPE_CATEGORY_ARRV: {
            auto& arr = (ArrayViewType&)(*this);
            if (arr.name.empty())
                arr.name = skr::format("skr::span<{}>", arr.elementType->Name());
            return arr.name.c_str();
        }
        case SKR_TYPE_CATEGORY_OBJ:
            return ((RecordType*)this)->name.data();
        case SKR_TYPE_CATEGORY_ENUM:
            return ((EnumType*)this)->name.data();
        case SKR_TYPE_CATEGORY_REF: {
            auto& ref = (ReferenceType&)(*this);
            if (!ref.name.empty())
                return ref.name.c_str();
            switch (ref.ownership)
            {
                case ReferenceType::Shared:
                    if (ref.object)
                        ref.name = skr::format("skr::SObjectPtr<{}>", ref.pointee ? ref.pointee->Name() : "skr::SInterface");
                    else
                        ref.name = skr::format("skr::SPtr<{}>", ref.pointee ? ref.pointee->Name() : "void");
                    break;
                case ReferenceType::Observed:
                    ref.name = ref.pointee ? (skr::string(ref.pointee->Name()) + " *") : "void*";
                    break;
            }
            return ref.name.c_str();
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto& variant = (VariantType&)(*this);
            if (!variant.name.empty())
                return variant.name.c_str();
            variant.name = "skr::variant<";
            bool first = true;
            for (auto& type : variant.types)
            {
                if (!first)
                    variant.name += ", ";
                first = false;
                variant.name += type->Name();
            }
            variant.name += ">";
            return variant.name.c_str();
        }
    }
    return "";
}

bool skr_type_t::Same(const skr_type_t* srcType) const
{
    using namespace skr::type;
    if (type != srcType->type)
        return false;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        return true;
        case SKR_TYPE_CATEGORY_ARR:
            return ((ArrayType*)this)->elementType->Same(((ArrayType*)srcType)->elementType) && ((ArrayType*)this)->num == ((ArrayType*)srcType)->num;
        case SKR_TYPE_CATEGORY_DYNARR:
            return ((DynArrayType*)this)->elementType->Same(((DynArrayType*)srcType)->elementType);
        case SKR_TYPE_CATEGORY_ARRV:
            return ((ArrayViewType*)this)->elementType->Same(((ArrayViewType*)srcType)->elementType);
        case SKR_TYPE_CATEGORY_OBJ:
        case SKR_TYPE_CATEGORY_ENUM:
            return this == srcType; // 对象类型直接比较地址
        case SKR_TYPE_CATEGORY_REF: {
            {
                auto ptr = ((ReferenceType*)this);
                auto sptr = ((ReferenceType*)srcType);
                if ((ptr->pointee == nullptr || sptr->pointee == nullptr) && ptr->pointee != sptr->pointee)
                    return false;
                return ptr->ownership == sptr->ownership && ptr->nullable == sptr->nullable && ptr->pointee->Same(sptr->pointee);
            }
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto ptr = ((VariantType*)this);
            auto sptr = ((VariantType*)srcType);
            if (ptr->types.size() != sptr->types.size())
                return false;
            for (size_t i = 0; i < ptr->types.size(); i++)
                if (!ptr->types[i]->Same(sptr->types[i]))
                    return false;
            return true;
        }
        default:
            return false;
    }
    return false;
}
bool skr_type_t::Convertible(const skr_type_t* srcType, bool format) const
{
    using namespace skr::type;
    auto stype = srcType->type;
    skr::span<size_t> acceptIndices;
    if (Same(srcType))
        return true;
    if (srcType->type == SKR_TYPE_CATEGORY_REF)
    {
        auto& sptr = (const ReferenceType&)(*srcType);
        if (!sptr.nullable && sptr.pointee != nullptr && Convertible(sptr.pointee))
        {
            return true;
        }
    }
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL: {
            if (format)
            {
                static size_t accept[] = { SKR_TYPE_CATEGORY_BOOL, SKR_TYPE_CATEGORY_I32, SKR_TYPE_CATEGORY_I64, SKR_TYPE_CATEGORY_U32, SKR_TYPE_CATEGORY_U64, SKR_TYPE_CATEGORY_F32, SKR_TYPE_CATEGORY_F64, SKR_TYPE_CATEGORY_STR, SKR_TYPE_CATEGORY_STRV, SKR_TYPE_CATEGORY_REF };
                acceptIndices = accept;
            }
            else
            {
                static size_t accept[] = { SKR_TYPE_CATEGORY_BOOL, SKR_TYPE_CATEGORY_I32, SKR_TYPE_CATEGORY_I64, SKR_TYPE_CATEGORY_U32, SKR_TYPE_CATEGORY_U64, SKR_TYPE_CATEGORY_F32, SKR_TYPE_CATEGORY_F64, SKR_TYPE_CATEGORY_REF };
                acceptIndices = accept;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_I32:
        case SKR_TYPE_CATEGORY_I64:
        case SKR_TYPE_CATEGORY_U32:
        case SKR_TYPE_CATEGORY_U64:
            if (srcType->type == SKR_TYPE_CATEGORY_ENUM)
            {
                if (srcType->Size() > Size())
                    return false;
                else
                    return true;
            }
        case SKR_TYPE_CATEGORY_F32:
        case SKR_TYPE_CATEGORY_F64:
            if (format)
            {
                static size_t accept[] = { SKR_TYPE_CATEGORY_BOOL, SKR_TYPE_CATEGORY_I32, SKR_TYPE_CATEGORY_I64, SKR_TYPE_CATEGORY_U32, SKR_TYPE_CATEGORY_U64, SKR_TYPE_CATEGORY_F32, SKR_TYPE_CATEGORY_F64, SKR_TYPE_CATEGORY_STR, SKR_TYPE_CATEGORY_STRV };
                acceptIndices = accept;
            }
            else
            {
                static size_t accept[] = { SKR_TYPE_CATEGORY_BOOL, SKR_TYPE_CATEGORY_I32, SKR_TYPE_CATEGORY_I64, SKR_TYPE_CATEGORY_U32, SKR_TYPE_CATEGORY_U64, SKR_TYPE_CATEGORY_F32, SKR_TYPE_CATEGORY_F64 };
                acceptIndices = accept;
            }
            break;
        case SKR_TYPE_CATEGORY_HANDLE: {
            if (stype == SKR_TYPE_CATEGORY_REF)
            {
                auto sptr = (const ReferenceType*)srcType;
                // TODO: check if this is asset?
                if (sptr->pointee->type == SKR_TYPE_CATEGORY_OBJ)
                    return true;
            }
        }
        case SKR_TYPE_CATEGORY_GUID: {
            if (format)
            {
                static size_t accept[] = { SKR_TYPE_CATEGORY_GUID, SKR_TYPE_CATEGORY_HANDLE, SKR_TYPE_CATEGORY_STR, SKR_TYPE_CATEGORY_STRV };
                acceptIndices = accept;
                break;
            }
            else
                return false;
        }
        case SKR_TYPE_CATEGORY_MD5: {
            if (format)
            {
                static size_t accept[] = { SKR_TYPE_CATEGORY_MD5, SKR_TYPE_CATEGORY_HANDLE, SKR_TYPE_CATEGORY_STR, SKR_TYPE_CATEGORY_STRV };
                acceptIndices = accept;
                break;
            }
            else
                return false;
        }
        case SKR_TYPE_CATEGORY_STRV: {
            static size_t accept[] = { SKR_TYPE_CATEGORY_STR, SKR_TYPE_CATEGORY_STRV };
            acceptIndices = accept;
            break;
        }
        case SKR_TYPE_CATEGORY_STR: {
            if (format)
                return true;
            static size_t accept[] = { SKR_TYPE_CATEGORY_BOOL, SKR_TYPE_CATEGORY_I32, SKR_TYPE_CATEGORY_I64, SKR_TYPE_CATEGORY_U32, SKR_TYPE_CATEGORY_U64, SKR_TYPE_CATEGORY_F32, SKR_TYPE_CATEGORY_F64, SKR_TYPE_CATEGORY_ENUM, SKR_TYPE_CATEGORY_STR, SKR_TYPE_CATEGORY_STRV };
            acceptIndices = accept;
            break;
        }
        case SKR_TYPE_CATEGORY_ARR: {
            if (stype == SKR_TYPE_CATEGORY_ARR)
            {
                auto& sarray = (const ArrayType&)(*srcType);
                auto& array = (const ArrayType&)(*this);
                if (array.num != sarray.num)
                    return false;
                if (array.elementType->Convertible(sarray.elementType, format))
                    return true;
            }
            else
                return false;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& array = (const DynArrayType&)(*this);
            if (stype == SKR_TYPE_CATEGORY_ARR)
            {
                auto& sarray = (const ArrayType&)(*srcType);
                if (array.elementType->Convertible(sarray.elementType, format))
                    return true;
            }
            else if (stype == SKR_TYPE_CATEGORY_DYNARR)
            {
                auto& sarray = (const DynArrayType&)(*srcType);
                if (array.elementType->Convertible(sarray.elementType, format))
                    return true;
            }
            else if (stype == SKR_TYPE_CATEGORY_ARRV)
            {
                auto& sarray = (const ArrayViewType&)(*srcType);
                if (array.elementType->Convertible(sarray.elementType, format))
                    return true;
            }
            else
                return false;
        }
        case SKR_TYPE_CATEGORY_ARRV: {
            static size_t accept[] = { SKR_TYPE_CATEGORY_ARR, SKR_TYPE_CATEGORY_DYNARR, SKR_TYPE_CATEGORY_ARRV };
            acceptIndices = accept;
            break;
        }
        case SKR_TYPE_CATEGORY_OBJ: {
            return false;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            switch (srcType->type)
            {
                case SKR_TYPE_CATEGORY_I32:
                case SKR_TYPE_CATEGORY_I64:
                case SKR_TYPE_CATEGORY_U32:
                case SKR_TYPE_CATEGORY_U64:
                    return srcType->Size() >= Size();
                case SKR_TYPE_CATEGORY_STR:
                case SKR_TYPE_CATEGORY_STRV:
                    return true;
                default:
                    return false;
            }
        }
        case SKR_TYPE_CATEGORY_REF: {
            auto& ptr = (const ReferenceType&)(*this);
            if (stype == SKR_TYPE_CATEGORY_REF)
            {
                auto& sptr = (const ReferenceType&)(*srcType);
                if (sptr.nullable > ptr.nullable)
                    return false;
                if (sptr.ownership != ptr.ownership && ptr.ownership != ReferenceType::Observed)
                    return false;
                if (sptr.object != ptr.object && ptr.ownership != ReferenceType::Observed)
                    return false;
                if (ptr.pointee == nullptr || sptr.pointee == nullptr)
                    return true;
                if (ptr.pointee->Same(sptr.pointee))
                    return true;
                else if (ptr.pointee->type == SKR_TYPE_CATEGORY_ENUM || sptr.pointee->type == SKR_TYPE_CATEGORY_ENUM)
                {
                    auto type1 = ptr.pointee->type == SKR_TYPE_CATEGORY_ENUM ? ((EnumType*)ptr.pointee)->underlyingType : ptr.pointee;
                    auto type2 = sptr.pointee->type == SKR_TYPE_CATEGORY_ENUM ? ((EnumType*)sptr.pointee)->underlyingType : sptr.pointee;
                    return type1->type == type2->type;
                }
                else if (ptr.pointee->type == SKR_TYPE_CATEGORY_OBJ && sptr.pointee->type == SKR_TYPE_CATEGORY_OBJ)
                {
                    auto& sobj = (const RecordType&)(*sptr.pointee);
                    auto& obj = (const RecordType&)(*ptr.pointee);
                    if (obj.IsBaseOf(sobj))
                        return true;
                    else
                        return false;
                }
                else
                    return false;
            }
            if (stype == SKR_TYPE_CATEGORY_HANDLE) // TODO: handle's ownership?
            {
                bool obs = ptr.ownership == ReferenceType::Observed;
                auto& sptr = (const HandleType&)(*srcType);
                if (obs && (ptr.pointee == nullptr || sptr.pointee == nullptr))
                    return true;
                if (ptr.pointee->type == SKR_TYPE_CATEGORY_OBJ)
                {
                    if (ptr.pointee->Same(sptr.pointee))
                        return true;
                    auto& sobj = (const RecordType&)(*sptr.pointee);
                    if (!(!obs && ptr.object && sobj.object))
                        return false;
                    auto& obj = (const RecordType&)(*ptr.pointee);
                    if (obj.IsBaseOf(sobj))
                        return true;
                    else
                        return false;
                }
                return false;
            }
            else if (ptr.ownership == ReferenceType::Observed && ptr.pointee && ptr.pointee->Same(srcType))
                return true;
            else
                return false;
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto& variant = (const VariantType&)(*this);
            if (stype == SKR_TYPE_CATEGORY_VARIANT)
                return false;
            else
            {
                for (auto& type : variant.types)
                {
                    if (type->Same(srcType))
                        return true;
                }
                return false;
            }
        }
    };
    if (std::find(acceptIndices.begin(), acceptIndices.end(), stype) != acceptIndices.end())
        return true;
    return false;
}

void skr_type_t::Convert(void* dst, const void* src, const skr_type_t* srcType, skr::type::ValueSerializePolicy* policy) const
{
    using namespace skr::type;
    if (Same(srcType))
    {
        Copy(dst, src);
        return;
    }
    if (!Convertible(srcType, policy != nullptr))
        return;

    if (srcType->type == SKR_TYPE_CATEGORY_REF)
    {
        auto& sptr = (const ReferenceType&)(*srcType);
        if (!sptr.nullable && sptr.pointee != nullptr && Convertible(sptr.pointee))
            Convert(dst, *(void**)src, srcType, policy); // dereference and convert
    }

#define BASE_CONVERT                 \
    case SKR_TYPE_CATEGORY_BOOL:     \
        dstV = (T) * (bool*)src;     \
        break;                       \
    case SKR_TYPE_CATEGORY_I32:      \
        dstV = (T) * (int32_t*)src;  \
        break;                       \
    case SKR_TYPE_CATEGORY_I64:      \
        dstV = (T) * (int64_t*)src;  \
        break;                       \
    case SKR_TYPE_CATEGORY_U32:      \
        dstV = (T) * (uint32_t*)src; \
        break;                       \
    case SKR_TYPE_CATEGORY_U64:      \
        dstV = (T) * (uint64_t*)src; \
        break;                       \
    case SKR_TYPE_CATEGORY_F32:      \
        dstV = (T) * (float*)src;    \
        break;                       \
    case SKR_TYPE_CATEGORY_F64:      \
        dstV = (T) * (double*)src;   \
        break;
#define STR_CONVERT                                                    \
    case SKR_TYPE_CATEGORY_STR:                                        \
        FromString(dst, skr::string_view(*(skr::string*)src), policy); \
    case SKR_TYPE_CATEGORY_STRV:                                       \
        FromString(dst, skr::string_view(*(skr::string_view*)src), policy);
#define ENUM_CONVERT                                   \
    case SKR_TYPE_CATEGORY_ENUM: {                     \
        auto& enm = (const EnumType&)(*srcType);       \
        Convert(dst, src, enm.underlyingType, policy); \
    }

    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL: {
            using T = bool;
            auto& dstV = *(T*)dst;
            switch (srcType->type)
            {
                BASE_CONVERT
                case SKR_TYPE_CATEGORY_STR:
                    policy->parse(policy, skr::string_view(*(skr::string*)src), dst, this);
                case SKR_TYPE_CATEGORY_STRV:
                    policy->parse(policy, *(skr::string_view*)src, dst, this);
                case SKR_TYPE_CATEGORY_REF: {
                    auto& ref = (const ReferenceType&)(*srcType);
                    switch (ref.ownership)
                    {
                        case ReferenceType::Observed: {
                            auto& srcV = *(void**)src;
                            dstV = (bool)srcV;
                            break;
                        }
                        case ReferenceType::Shared:
                            if (ref.object)
                            {
                                auto& srcV = *(skr::SObjectPtr<skr::SInterface>*)src;
                                dstV = (bool)srcV;
                            }
                            else
                            {
                                auto& srcV = *(skr::SPtr<void>*)src;
                                dstV = (bool)srcV;
                            }
                            break;
                    }
                }
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_I32: {
            using T = int32_t;
            auto& dstV = *(T*)dst;
            switch (srcType->type)
            {
                BASE_CONVERT
                STR_CONVERT
                ENUM_CONVERT
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_I64: {
            using T = int64_t;
            auto& dstV = *(T*)dst;
            switch (srcType->type)
            {
                BASE_CONVERT
                STR_CONVERT
                ENUM_CONVERT
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_U32: {
            using T = uint32_t;
            auto& dstV = *(T*)dst;
            switch (srcType->type)
            {
                BASE_CONVERT
                STR_CONVERT
                ENUM_CONVERT
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_U64: {
            using T = uint64_t;
            auto& dstV = *(T*)dst;
            switch (srcType->type)
            {
                BASE_CONVERT
                STR_CONVERT
                ENUM_CONVERT
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_F32: {
            using T = float;
            auto& dstV = *(T*)dst;
            switch (srcType->type)
            {
                BASE_CONVERT
                STR_CONVERT
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_F64: {
            using T = double;
            auto& dstV = *(T*)dst;
            switch (srcType->type)
            {
                BASE_CONVERT
                STR_CONVERT
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_GUID: {
            switch (srcType->type)
            {
                STR_CONVERT
                case SKR_TYPE_CATEGORY_HANDLE:
                    *(skr_guid_t*)dst = (*(skr_resource_handle_t*)src).get_serialized();
                    break;
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_MD5: {
            switch (srcType->type)
            {
                STR_CONVERT
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_HANDLE: {
            switch (srcType->type)
            {
                STR_CONVERT
                case SKR_TYPE_CATEGORY_GUID:
                    (*(skr_resource_handle_t*)dst).set_guid(*(skr_guid_t*)src);
                    break;
                case SKR_TYPE_CATEGORY_REF:
                    (*(skr_resource_handle_t*)dst).set_ptr(*(void**)src);
                    break;
                default:
                    break;
            }
        }
        case SKR_TYPE_CATEGORY_STR: {
            auto& dstV = *(skr::string*)dst;
            switch (srcType->type)
            {
                case SKR_TYPE_CATEGORY_STRV:
                    dstV = *(skr::string_view*)src;
                    break;
                default:
                    dstV = srcType->ToString(src);
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_STRV: {
            auto& dstV = *(skr::string_view*)dst;
            switch (srcType->type)
            {
                case SKR_TYPE_CATEGORY_STR:
                    dstV = *(skr::string*)src;
                    break;
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)dst;
            auto& sarr = (const ArrayType&)(*srcType);
            auto& selement = sarr.elementType;
            auto sdata = (char*)src;
            auto size = element->Size(), ssize = selement->Size();
            for (int i = 0; i < arr.num; ++i)
                element->Convert(data + i * size, sdata + i * ssize, selement);
            break;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            auto element = arr.elementType;
            auto size = element->Size();
            auto num = arr.operations.size(dst);
            switch (srcType->type)
            {
                case SKR_TYPE_CATEGORY_DYNARR: {
                    auto& sarr = (const DynArrayType&)(*srcType);
                    auto& selement = sarr.elementType;
                    auto ssize = selement->Size();
                    auto snum = sarr.operations.size((void*)src);
                    if (num != snum)
                        arr.operations.resize(dst, snum);
                    auto data = (char*)arr.operations.data(dst);
                    auto sdata = (char*)sarr.operations.data((void*)src);
                    for (int i = 0; i < snum; ++i)
                        element->Convert(data + i * size, sdata + i * ssize, selement);
                    break;
                }
                case SKR_TYPE_CATEGORY_ARR: {
                    auto& sarr = (const ArrayType&)(*srcType);
                    auto& selement = sarr.elementType;
                    auto ssize = selement->Size();
                    if (num != sarr.num)
                        arr.operations.resize(dst, sarr.num);
                    auto data = (char*)arr.operations.data(dst);
                    auto sdata = (char*)src;
                    for (int i = 0; i < sarr.num; ++i)
                        element->Convert(data + i * size, sdata + i * ssize, selement);
                    break;
                }
                case SKR_TYPE_CATEGORY_ARRV: {
                    auto& sarr = (const ArrayViewType&)(*srcType);
                    auto& selement = sarr.elementType;
                    auto ssize = selement->Size();
                    auto sv = *(skr::span<char>*)src;
                    auto snum = sv.size();
                    if (num != snum)
                        arr.operations.resize(dst, snum);
                    auto data = (char*)arr.operations.data(dst);
                    auto sdata = sv.data();
                    for (int i = 0; i < snum; ++i)
                        element->Convert(data + i * size, sdata + i * ssize, selement);
                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SKR_TYPE_CATEGORY_ARRV: {
            auto& dstV = *(skr::span<char>*)dst;
            switch (srcType->type)
            {
                case SKR_TYPE_CATEGORY_ARR:
                    dstV = { (char*)src, ((const ArrayType&)(*srcType)).num };
                    break;
                case SKR_TYPE_CATEGORY_DYNARR: {
                    auto& sarr = (const DynArrayType&)(*srcType);
                    auto snum = sarr.operations.size((void*)src);
                    auto sdata = (char*)sarr.operations.data((void*)src);
                    dstV = { sdata, snum };
                    break;
                }
                case SKR_TYPE_CATEGORY_ARRV:
                    dstV = *(skr::span<char>*)src;
                    break;
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            if (srcType->type == SKR_TYPE_CATEGORY_STR)
                enm.FromString(dst, *(skr::string*)src);
            else if (srcType->type == SKR_TYPE_CATEGORY_STRV)
                enm.FromString(dst, *(skr::string_view*)src);
            else
                enm.underlyingType->Convert(dst, src, srcType);
            break;
        }
        case SKR_TYPE_CATEGORY_REF: {
            auto& ptr = (const ReferenceType&)(*this);
            if (srcType->type == SKR_TYPE_CATEGORY_REF)
            {
                auto& sptr = (const ReferenceType&)(*srcType);
                if (sptr.ownership == ReferenceType::Observed)
                {
                    auto& srcV = *(void**)src;
                    auto& dstV = *(void**)dst;
                    dstV = srcV;
                }
                else
                {
                    if (sptr.object)
                    {
                        auto& srcV = *(skr::SObjectPtr<skr::SInterface>*)src;
                        if (ptr.ownership == ReferenceType::Observed)
                        {
                            auto& dstV = *(void**)dst;
                            dstV = srcV.get();
                        }
                        else if (ptr.ownership == ReferenceType::Shared)
                        {
                            SKR_ASSERT(ptr.object);
                            auto& dstV = *(skr::SObjectPtr<skr::SInterface>*)dst;
                            dstV = srcV;
                        }
                    }
                    else
                    {
                        auto& srcV = *(skr::SPtr<void>*)src;
                        if (ptr.ownership == ReferenceType::Observed)
                        {
                            auto& dstV = *(void**)dst;
                            dstV = srcV.get();
                        }
                        else if (ptr.ownership == ReferenceType::Shared)
                        {
                            SKR_ASSERT(!ptr.object);
                            auto& dstV = *(skr::SPtr<void>*)dst;
                            dstV = srcV;
                        }
                    }
                }
            }
            else if (srcType->type == SKR_TYPE_CATEGORY_HANDLE)
            {
                auto& handle = *(skr_resource_handle_t*)src;
                if (ptr.object)
                    ((skr::SObjectPtr<skr::SInterface>*)dst)->reset((skr::SInterface*)handle.get_resolved());
                else
                    *(void**)dst = handle.get_resolved();
            }
            else if (ptr.ownership == ReferenceType::Observed)
            {
                auto& dstV = *(void**)dst;
                dstV = (void*)src;
            }
        }
        break;
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto& variant = (const VariantType&)(*this);
            uint32_t i = 0;
            for (auto& type : variant.types)
            {
                if (type->Same(srcType))
                {
                    variant.operations.setters[i](dst, src);
                    break;
                }
                ++i;
            }
        }
        break;
        default:
            SKR_ASSERT(false);
            break;
    }
}

template <class T>
skr::string ToStringImpl(const void* dst)
{
    return skr::format("{}", *(T*)dst);
}

template <>
skr::string ToStringImpl<skr_resource_handle_t>(const void* dst)
{
    auto guid = (*(skr_resource_handle_t*)dst).get_serialized();
    return ToStringImpl<skr_guid_t>(&guid);
}

skr::string skr_type_t::ToString(const void* dst, skr::type::ValueSerializePolicy* policy) const
{
    using namespace skr::type;
    if (policy)
        return policy->format(policy, dst, this);
    else
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return "";
        // #define TRIVAL_TYPE_IMPL(name, type) \
//     case name:                       \
//         return ToStringImpl<type>(dst);
        //         switch (type)
        //         {
        //             SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
        // #undef TRIVAL_TYPE_IMPL
        //             case SKR_TYPE_CATEGORY_ENUM:
        //                 return ((const EnumType*)this)->ToString(dst);
        //             case SKR_TYPE_CATEGORY_VARIANT: {
        //                 auto variant = (const VariantType*)this;
        //                 auto index = variant->operations.indexer(dst);
        //                 return variant->types[index]->ToString(variant->operations.getters[index]((void*)dst), policy);
        //             }
        //             default:
        //                 SKR_UNIMPLEMENTED_FUNCTION();
        //                 break;
        //         }
        //         return "";
    }
}

template <class T>
void (*DeleterImpl())(void*)
{
    return skr::GetDeleter<T>();
}

void (*skr_type_t::Deleter() const)(void*)
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return DeleterImpl<type>();
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ENUM:
            return ((const EnumType*)this)->underlyingType->Deleter();
        case SKR_TYPE_CATEGORY_REF:
            return ((const ReferenceType*)this)->object ? DeleterImpl<skr::SObjectPtr<skr::SInterface>>() : DeleterImpl<skr::SPtr<void>>();
        case SKR_TYPE_CATEGORY_VARIANT:
            return ((const VariantType*)this)->operations.deleter;
        case SKR_TYPE_CATEGORY_OBJ:
            return ((const RecordType*)this)->nativeMethods.deleter;
        case SKR_TYPE_CATEGORY_DYNARR:
            return ((const DynArrayType*)this)->operations.deleter;
        default:
            SKR_UNREACHABLE_CODE();
    }
    return nullptr;
}

void skr_type_t::Construct(void* dst, skr::type::Value* args, size_t nargs) const
{
    SKR_ASSERT(args == nullptr && nargs == 0);
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_STR:
            new (dst) skr::string();
            break;
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto d = (char*)dst;
            auto size = element->Size();
            for (int i = 0; i < arr.num; ++i)
                element->Construct(d + i * size, args, nargs);
        }
        break;
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            arr.Construct(dst, args, nargs);
        }
        break;
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            obj.nativeMethods.ctor(dst, args, nargs);
        }
        break;
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto& variant = (const VariantType&)(*this);
            variant.operations.ctor(dst, args, nargs);
        }
        break;
        default:
            SKR_UNREACHABLE_CODE();
            break;
    }
}

template <class T>
size_t HashImpl(const void* dst, size_t base)
{
    return skr::type::Hash(*(const T*)dst, base);
}

size_t skr_type_t::Hash(const void* dst, size_t base) const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return HashImpl<type>(dst, base);
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            enm.underlyingType->Hash(dst, base);
            return 0;
        }
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto d = (char*)dst;
            auto size = element->Size();
            for (int i = 0; i < arr.num; ++i)
                base = element->Hash(d + i * size, base);
            return base;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            auto element = arr.elementType;
            auto d = (char*)arr.operations.data(dst);
            auto n = arr.operations.size(dst);
            auto size = element->Size();
            for (int i = 0; i < n; ++i)
                base = element->Hash(d + i * size, base);
            return base;
        }
        case SKR_TYPE_CATEGORY_ARRV: {
            auto& arr = (const ArrayViewType&)(*this);
            auto& element = arr.elementType;
            auto size = element->Size();
            auto v = *(skr::span<char>*)dst;
            auto n = v.size();
            auto d = v.data();
            for (int i = 0; i < n; ++i)
                base = element->Hash(d + i * size, base);
            return base;
        }
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            if (obj.nativeMethods.Hash)
                return obj.nativeMethods.Hash(dst, base);
            else
                return 0;
        }
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    return HashImpl<void*>(*(void**)dst, base);
                    break;
                case ReferenceType::Shared:
                    if (((ReferenceType*)this)->object)
                        return HashImpl<void*>((*(skr::SObjectPtr<skr::SInterface>*)dst).get(), base);
                    else
                        return HashImpl<void*>((*(skr::SPtr<void>*)dst).get(), base);
                    break;
            }
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto variant = (const VariantType*)this;
            auto index = variant->operations.indexer(dst);
            return variant->types[index]->Hash(variant->operations.getters[index]((void*)dst), base);
        }
    }
    return 0;
}

void skr_type_t::Destruct(void* address) const
{
    using namespace skr::type;
    // destruct owned data
    switch (type)
    {
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = ((const RecordType&)(*this));
            if (obj.nativeMethods.dtor)
                obj.nativeMethods.dtor(address);
        }
        case SKR_TYPE_CATEGORY_STR: {
            ((skr::string*)address)->~basic_string();
            break;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            arr.operations.dtor(address);
            break;
        }
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)address;
            auto size = element->Size();
            for (int i = 0; i < arr.num; ++i)
                element->Destruct(data + i * size);
            break;
        }
        case SKR_TYPE_CATEGORY_REF: {
            auto& ptr = (const ReferenceType&)(*this);
            if (ptr.ownership == ReferenceType::Shared)
            {
                if (ptr.object)
                    ((skr::SObjectPtr<skr::SInterface>*)address)->~SPtrHelper();
                else
                    ((skr::SPtr<void>*)address)->~SPtrHelper();
            }
            break;
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto& variant = (const VariantType&)(*this);
            variant.operations.dtor(address);
            break;
        }
        default:
            break;
    }
}

void* skr_type_t::Malloc() const
{
    using namespace skr::type;
    return sakura_calloc_aligned(1, Size(), Align());
}

void skr_type_t::Free(void* address) const
{
    using namespace skr::type;
    sakura_free_aligned(address, Align());
}

template <class T>
void CopyImpl(void* dst, const void* src)
{
    new (dst) T(*(const T*)src);
}

void skr_type_t::Copy(void* dst, const void* src) const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return CopyImpl<type>(dst, src);
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)dst;
            auto sdata = (char*)src;
            auto size = element->Size();
            for (int i = 0; i < arr.num; ++i)
                element->Copy(data + i * size, sdata + i * size);
            break;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            arr.operations.copy(dst, src);
        }
        case SKR_TYPE_CATEGORY_ARRV:
            CopyImpl<skr::span<char>>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            obj.nativeMethods.copy(dst, src);
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            enm.underlyingType->Copy(dst, src);
            break;
        }
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    CopyImpl<void*>(dst, src);
                    break;
                case ReferenceType::Shared:
                    if (((ReferenceType*)this)->object)
                        CopyImpl<skr::SObjectPtr<skr::SInterface>>(dst, src);
                    else
                        CopyImpl<skr::SPtr<void>>(dst, src);
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto& variant = (const VariantType&)(*this);
            variant.operations.copy(dst, src);
            break;
        }
    }
}

template <class T>
void MoveImpl(void* dst, void* src)
{
    new (dst) T(std::move(*(T*)src));
}

void skr_type_t::Move(void* dst, void* src) const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return MoveImpl<type>(dst, src);
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)dst;
            auto sdata = (char*)src;
            auto size = element->Size();
            for (int i = 0; i < arr.num; ++i)
                element->Move(data + i * size, sdata + i * size);
            break;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            arr.operations.move(dst, src);
            break;
        }
        case SKR_TYPE_CATEGORY_ARRV:
            MoveImpl<skr::span<char>>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            obj.nativeMethods.move(dst, src);
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            switch (enm.underlyingType->type)
            {
                case SKR_TYPE_CATEGORY_I32:
                    MoveImpl<int32_t>(dst, src);
                    break;
                case SKR_TYPE_CATEGORY_I64:
                    MoveImpl<int64_t>(dst, src);
                    break;
                case SKR_TYPE_CATEGORY_U32:
                    MoveImpl<uint32_t>(dst, src);
                    break;
                case SKR_TYPE_CATEGORY_U64:
                    MoveImpl<uint64_t>(dst, src);
                    break;
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    MoveImpl<void*>(dst, src);
                    break;
                case ReferenceType::Shared:
                    if (((ReferenceType*)this)->object)
                        MoveImpl<skr::SObjectPtr<skr::SInterface>>(dst, src);
                    else
                        MoveImpl<skr::SPtr<void>>(dst, src);
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            auto& variant = (const VariantType&)(*this);
            variant.operations.move(dst, src);
            break;
        }
    }
}

void skr_type_t::Delete()
{
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_STR:
            SkrDelete((StringType*)this);
            break;
        case SKR_TYPE_CATEGORY_STRV:
            SkrDelete((StringViewType*)this);
            break;
        case SKR_TYPE_CATEGORY_ARR:
            SkrDelete((ArrayType*)this);
            break;
        case SKR_TYPE_CATEGORY_DYNARR:
            SkrDelete((DynArrayType*)this);
            break;
        case SKR_TYPE_CATEGORY_ARRV:
            SkrDelete((ArrayViewType*)this);
            break;
        case SKR_TYPE_CATEGORY_OBJ:
            SkrDelete((RecordType*)this);
            break;
        case SKR_TYPE_CATEGORY_ENUM:
            SkrDelete((EnumType*)this);
            break;
        case SKR_TYPE_CATEGORY_REF:
            SkrDelete((ReferenceType*)this);
            break;
        case SKR_TYPE_CATEGORY_VARIANT:
            SkrDelete((VariantType*)this);
            break;
        case SKR_TYPE_CATEGORY_F32_2:
        case SKR_TYPE_CATEGORY_F32_3:
        case SKR_TYPE_CATEGORY_F32_4:
        case SKR_TYPE_CATEGORY_F32_4x4:
        case SKR_TYPE_CATEGORY_ROT:
        case SKR_TYPE_CATEGORY_QUAT:
        case SKR_TYPE_CATEGORY_BOOL:
        case SKR_TYPE_CATEGORY_I32:
        case SKR_TYPE_CATEGORY_I64:
        case SKR_TYPE_CATEGORY_U32:
        case SKR_TYPE_CATEGORY_U64:
        case SKR_TYPE_CATEGORY_F32:
        case SKR_TYPE_CATEGORY_F64:
        case SKR_TYPE_CATEGORY_GUID:
        case SKR_TYPE_CATEGORY_MD5:
        case SKR_TYPE_CATEGORY_HANDLE:
            SkrDelete((skr_type_t*)this);
            break;
        default:
            SKR_UNREACHABLE_CODE()
            break;
    }
}

template <class T>
int SerializeImpl(const void* dst, skr_binary_writer_t* writer)
{
    return skr::binary::Write(writer, *(T*)dst);
}

int skr_type_t::Serialize(const void* dst, skr_binary_writer_t* writer) const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return SerializeImpl<type>(dst, writer);
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)dst;
            auto size = element->Size();
            for (int i = 0; i < arr.num; ++i)
                if (auto ret = element->Serialize(data + i * size, writer); ret != 0)
                    return ret;
            break;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            return arr.operations.Serialize(dst, writer);
            break;
        }
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            return obj.nativeMethods.Serialize(dst, writer);
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            return enm.underlyingType->Serialize(dst, writer);
            break;
        }
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    // SerializeImpl<void*>(dst, writer);
                    break;
                case ReferenceType::Shared: {
                    void* ptr = nullptr;
                    if (((ReferenceType*)this)->object)
                        ptr = ((skr::SObjectPtr<skr::SInterface>*)dst)->get();
                    else
                        ptr = ((skr::SObjectPtr<void>*)dst)->get();
                    return ((ReferenceType*)this)->pointee->Serialize(ptr, writer);
                }
                break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            return ((VariantType*)this)->operations.Serialize(dst, writer);
            // auto& variant = (const VariantType&)(*this);
            // uint32_t index = (uint32_t)variant.operations.indexer(dst);
            // if (auto ret = skr::binary::Write(writer, index); ret != 0)
            //     return ret;
            // return variant.types[index]->Serialize(variant.operations.getters[index]((void*)dst), writer);
            break;
        }
    }
    SKR_UNREACHABLE_CODE()
    return 1;
}

template <class T>
void SerializeTextImpl(const void* dst, skr_json_writer_t* writer)
{
    if constexpr (skr::is_complete_v<skr::json::WriteHelper<T>>)
        return skr::json::Write(writer, *(T*)dst);
    SKR_UNIMPLEMENTED_FUNCTION();
}

void skr_type_t::SerializeText(const void* dst, skr_json_writer_t* writer) const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type)          \
    case name:                                \
        SerializeTextImpl<type>(dst, writer); \
        break;
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)dst;
            auto size = element->Size();
            writer->StartArray();
            for (int i = 0; i < arr.num; ++i)
                element->SerializeText(data + i * size, writer);
            writer->EndArray();
            break;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            arr.operations.SerializeText(dst, writer);
            break;
        }
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            obj.nativeMethods.SerializeText(dst, writer);
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            enm.underlyingType->SerializeText(dst, writer);
            break;
        }
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    // SerializeImpl<void*>(dst, writer);
                    break;
                case ReferenceType::Shared: {
                    void* ptr = nullptr;
                    if (((ReferenceType*)this)->object)
                        ptr = ((skr::SObjectPtr<skr::SInterface>*)dst)->get();
                    else
                        ptr = ((skr::SObjectPtr<void>*)dst)->get();
                    ((ReferenceType*)this)->pointee->SerializeText(ptr, writer);
                }
                break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            return ((VariantType*)this)->operations.SerializeText(dst, writer);
            // auto& variant = (const VariantType&)(*this);
            // uint32_t index = (uint32_t)variant.operations.indexer(dst);
            // writer->StartObject();
            // writer->Key("type");
            // skr::json::Write(writer, variant.types[index]->Id());
            // writer->Key("value");
            // variant.types[index]->SerializeText(variant.operations.getters[index]((void*)dst), writer);
            // writer->EndObject();
            break;
        }
    }
}

template <class T>
int DeserializeImpl(void* dst, skr_binary_reader_t* reader)
{
    return skr::binary::Read(reader, *(T*)dst);
}

template <>
int DeserializeImpl<skr::string_view>(void* dst, skr_binary_reader_t* reader)
{
    SKR_UNREACHABLE_CODE();
    return 1;
}

int skr_type_t::Deserialize(void* dst, skr_binary_reader_t* reader) const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return DeserializeImpl<type>(dst, reader);
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)dst;
            auto size = element->Size();
            for (int i = 0; i < arr.num; ++i)
                if (auto ret = element->Deserialize(data + i * size, reader); ret != 0)
                    return ret;
            break;
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            return arr.operations.Deserialize(dst, reader);
            break;
        }
        case SKR_TYPE_CATEGORY_ARRV:
            // DeserializeImpl<skr::span<char>>(dst, reader);
            break;
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            return obj.nativeMethods.Deserialize(dst, reader);
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            return enm.underlyingType->Deserialize(dst, reader);
            break;
        }
        case SKR_TYPE_CATEGORY_REF: {
            auto& ref = (*(ReferenceType*)this);
            switch (ref.ownership)
            {
                case ReferenceType::Observed:
                    // DeserializeImpl<void*>(dst, reader);
                    break;
                case ReferenceType::Shared: {
                    void* ptr = ref.pointee->Malloc();
                    ref.pointee->Construct(ptr, nullptr, 0);
                    if (auto ret = ref.pointee->Deserialize(ptr, reader); ret != 0)
                    {
                        ref.pointee->Destruct(ptr);
                        ref.pointee->Free(ptr);
                        return ret;
                    }
                    if (((ReferenceType*)this)->object)
                        ((skr::SObjectPtr<skr::SInterface>*)dst)->reset((skr::SInterface*)ptr);
                    else
                        ((skr::SPtr<void>*)dst)->reset(ptr, ref.pointee->Deleter());
                    return 0;
                }
                break;
            }
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            return ((VariantType*)this)->operations.Deserialize(dst, reader);
            // auto& variant = (const VariantType&)(*this);
            // uint32_t index = 0;
            // if (auto ret = skr::binary::Read(reader, index); ret != 0)
            //     return ret;
            // variant.operations.setters[index](dst, nullptr);
            // return variant.types[index]->Deserialize(variant.operations.getters[index](dst), reader);
            break;
        }
    }
    SKR_UNREACHABLE_CODE()
    return 1;
}

template <class T>
skr::json::error_code DeserializeTextImpl(void* dst, skr::json::value_t&& reader)
{
    if constexpr (skr::is_complete_v<skr::json::ReadHelper<T>>)
        return skr::json::Read(std::move(reader), *(T*)dst);
    SKR_UNIMPLEMENTED_FUNCTION();
    return skr::json::error_code::INCORRECT_TYPE;
}

template <>
skr::json::error_code DeserializeTextImpl<skr::string_view>(void* dst, skr::json::value_t&& reader)
{
    SKR_UNREACHABLE_CODE();
    return skr::json::error_code::INCORRECT_TYPE;
}

skr::json::error_code skr_type_t::DeserializeText(void* dst, skr::json::value_t&& reader) const
{
    using namespace skr::type;
#define TRIVAL_TYPE_IMPL(name, type) \
    case name:                       \
        return DeserializeTextImpl<type>(dst, std::move(reader));
    switch (type)
    {
        SKR_TYPE_TRIVAL(TRIVAL_TYPE_IMPL)
#undef TRIVAL_TYPE_IMPL
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (const ArrayType&)(*this);
            auto element = arr.elementType;
            auto data = (char*)dst;
            const auto size = element->Size();
            auto jarray = reader.get_array();
            const auto jarray_size = static_cast<uint32_t>(jarray.count_elements().value_unsafe());
            auto len = std::min<uint32_t>(jarray_size, (uint32_t)arr.num);
            for (uint32_t i = 0u; i < len; i++)
            {
                if (auto ret = element->DeserializeText(data + i * size, jarray.at(i).value_unsafe()); ret != 0)
                {
                    return ret;
                }
            }
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (const DynArrayType&)(*this);
            return arr.operations.DeserializeText(dst, std::move(reader));
        }
        case SKR_TYPE_CATEGORY_ARRV:
            // DeserializeTextImpl<skr::span<char>>(dst, std::move(reader));
            break;
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            return obj.nativeMethods.DeserializeText(dst, std::move(reader));
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            return enm.underlyingType->DeserializeText(dst, std::move(reader));
        }
        case SKR_TYPE_CATEGORY_REF: {
            auto& ref = (*(ReferenceType*)this);
            switch (ref.ownership)
            {
                case ReferenceType::Observed:
                    // DeserializeTextImpl<void*>(dst, std::move(reader));
                    break;
                case ReferenceType::Shared: {
                    void* ptr = ref.pointee->Malloc();
                    ref.pointee->Construct(ptr, nullptr, 0);
                    if (auto ret = ref.pointee->DeserializeText(ptr, std::move(reader)); ret != skr::json::SUCCESS)
                    {
                        ref.pointee->Destruct(ptr);
                        ref.pointee->Free(ptr);
                        return ret;
                    }
                    if (((ReferenceType*)this)->object)
                        ((skr::SObjectPtr<skr::SInterface>*)dst)->reset((skr::SInterface*)ptr);
                    else
                        ((skr::SPtr<void>*)dst)->reset(ptr, ref.pointee->Deleter());
                    return skr::json::SUCCESS;
                }
            }
        }
        case SKR_TYPE_CATEGORY_VARIANT: {
            return ((VariantType*)this)->operations.DeserializeText(dst, std::move(reader));
            // auto& variant = (const VariantType&)(*this);
            // skr_guid_t tid;
            // auto _tid = reader["type"];
            // if(auto error = _tid.error(); error != simdjson::SUCCESS)
            //     return (skr::json::error_code)error;
            // if (auto ret = skr::json::Read(std::move(reader), _tid.value_unsafe()); ret != skr::json::SUCCESS)
            //     return ret;
            // auto _value = reader["value"];
            // if(auto error = _value.error(); error != simdjson::SUCCESS)
            //     return (skr::json::error_code)error;
            // uint32_t index;
            // for(index=0; index<variant.types.size(); ++index)
            //     if(variant.types[index]->Id() == tid)
            //         break;
            // variant.operations.setters[index](dst, nullptr);
            // return variant.types[index]->DeserializeText(variant.operations.getters[index](dst), std::move(reader));
        }
        case SKR_TYPE_CATEGORY_INVALID:
            SKR_UNREACHABLE_CODE();
            break;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            break;
    }
    return skr::json::error_code::INCORRECT_TYPE;
}

bool skr::type::RecordType::IsBaseOf(const RecordType& other) const
{
    for (auto ptr = other.base; ptr; ptr = base->base)
        if (ptr == this)
            return true;
    return false;
}

const skr_type_t* skr_get_type(const skr_type_id_t* id)
{
    auto registry = skr::type::GetTypeRegistry();
    return registry->get_type(*id);
}