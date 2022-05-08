#include "type/type_registry.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "platform/guid.h"
#include "platform/memory.h"
#include "resource/resource_handle.h"
#include "utils/hash.h"
#include "utils/format.hpp"
#include "utils/fast_float.h"
#include <charconv>

namespace skr::type
{
RUNTIME_API STypeRegistry* GetTypeRegistry()
{
    static STypeRegistry registry;
    return &registry;
}
} // namespace skr::type

namespace skr
{
namespace type
{
const skr_type_t* type_of<bool>::get()
{
    static BoolType type;
    return &type;
}

const skr_type_t* type_of<int32_t>::get()
{
    static Int32Type type;
    return &type;
}

const skr_type_t* type_of<int64_t>::get()
{
    static Int64Type type;
    return &type;
}

const skr_type_t* type_of<uint32_t>::get()
{
    static UInt32Type type;
    return &type;
}

const skr_type_t* type_of<uint64_t>::get()
{
    static UInt64Type type;
    return &type;
}

const skr_type_t* type_of<float>::get()
{
    static Float32Type type;
    return &type;
}

const skr_type_t* type_of<double>::get()
{
    static Float64Type type;
    return &type;
}

const skr_type_t* type_of<skr_guid_t>::get()
{
    static GUIDType type;
    return &type;
}

const skr_type_t* type_of<skr_resource_handle_t>::get()
{
    static HandleType type{nullptr};
    return &type;
}

const skr_type_t* type_of<eastl::string>::get()
{
    static StringType type;
    return &type;
}

const skr_type_t* type_of<eastl::string_view>::get()
{
    static StringViewType type;
    return &type;
}

} // namespace type
} // namespace skr

size_t skr_type_t::Size() const
{
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
            return sizeof(bool);
        case SKR_TYPE_CATEGORY_I32:
            return sizeof(int32_t);
        case SKR_TYPE_CATEGORY_I64:
            return sizeof(int64_t);
        case SKR_TYPE_CATEGORY_U32:
            return sizeof(uint32_t);
        case SKR_TYPE_CATEGORY_U64:
            return sizeof(uint64_t);
        case SKR_TYPE_CATEGORY_F32:
            return sizeof(float);
        case SKR_TYPE_CATEGORY_F64:
            return sizeof(double);
        case SKR_TYPE_CATEGORY_GUID:
            return sizeof(skr_guid_t);
        case SKR_TYPE_CATEGORY_HANDLE:
            return sizeof(skr_resource_handle_t);
        case SKR_TYPE_CATEGORY_STR:
            return sizeof(eastl::string);
        case SKR_TYPE_CATEGORY_STRV:
            return sizeof(eastl::string_view);
        case SKR_TYPE_CATEGORY_ARR:
            return ((ArrayType*)this)->size;
        case SKR_TYPE_CATEGORY_DYNARR:
            return sizeof(eastl::vector<char>);
        case SKR_TYPE_CATEGORY_ARRV:
            return sizeof(gsl::span<char>);
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
                    return sizeof(std::shared_ptr<void>);
            }
        }
    }
    return 0;
}

size_t skr_type_t::Align() const
{
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
            return alignof(bool);
        case SKR_TYPE_CATEGORY_I32:
            return alignof(int32_t);
        case SKR_TYPE_CATEGORY_I64:
            return alignof(int64_t);
        case SKR_TYPE_CATEGORY_U32:
            return alignof(uint32_t);
        case SKR_TYPE_CATEGORY_U64:
            return alignof(uint64_t);
        case SKR_TYPE_CATEGORY_F32:
            return alignof(float);
        case SKR_TYPE_CATEGORY_F64:
            return alignof(double);
        case SKR_TYPE_CATEGORY_GUID:
            return alignof(skr_guid_t);
        case SKR_TYPE_CATEGORY_HANDLE:
            return alignof(skr_resource_handle_t);
        case SKR_TYPE_CATEGORY_STR:
            return alignof(eastl::string);
        case SKR_TYPE_CATEGORY_STRV:
            return alignof(eastl::string_view);
        case SKR_TYPE_CATEGORY_ARR:
            return ((ArrayType*)this)->size;
        case SKR_TYPE_CATEGORY_DYNARR:
            return alignof(eastl::vector<char>);
        case SKR_TYPE_CATEGORY_ARRV:
            return alignof(gsl::span<char>);
        case SKR_TYPE_CATEGORY_OBJ:
            return ((RecordType*)this)->size;
        case SKR_TYPE_CATEGORY_ENUM:
            return ((EnumType*)this)->underlyingType->Align();
        case SKR_TYPE_CATEGORY_REF: {
            switch (((ReferenceType*)this)->ownership)
            {
                case ReferenceType::Observed:
                    return alignof(void*);
                case ReferenceType::Shared:
                    return alignof(std::shared_ptr<void>);
            }
        }
    }
    return 0;
}
eastl::string skr_type_t::Name() const
{
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
            return "bool";
        case SKR_TYPE_CATEGORY_I32:
            return "int32_t";
        case SKR_TYPE_CATEGORY_I64:
            return "int64_t";
        case SKR_TYPE_CATEGORY_U32:
            return "uint32_t";
        case SKR_TYPE_CATEGORY_U64:
            return "uint64_t";
        case SKR_TYPE_CATEGORY_F32:
            return "float";
        case SKR_TYPE_CATEGORY_F64:
            return "double";
        case SKR_TYPE_CATEGORY_GUID:
            return "guid";
        case SKR_TYPE_CATEGORY_HANDLE:
            return "handle";
        case SKR_TYPE_CATEGORY_STR:
            return "eastl::string";
        case SKR_TYPE_CATEGORY_STRV:
            return "eastl::string_view";
        case SKR_TYPE_CATEGORY_ARR: {
            auto& arr = (ArrayType&)(*this);
            return format("{}[{}]", arr.elementType->Name(), (int)arr.num);
        }
        case SKR_TYPE_CATEGORY_DYNARR: {
            auto& arr = (DynArrayType&)(*this);
            return format("eastl::vector<{}>", arr.elementType->Name());
        }
        case SKR_TYPE_CATEGORY_ARRV: {
            auto& arr = (ArrayViewType&)(*this);
            return format("gsl::span<{}>", arr.elementType->Name());
        }
        case SKR_TYPE_CATEGORY_OBJ:
            return eastl::string(((RecordType*)this)->name);
        case SKR_TYPE_CATEGORY_ENUM:
            return eastl::string(((EnumType*)this)->name);
        case SKR_TYPE_CATEGORY_REF: {
            auto& ref = (ReferenceType&)(*this);
            switch (ref.ownership)
            {
                case ReferenceType::Shared:
                    return format("std::shared_ptr<{}>", ref.pointee ? ref.pointee->Name() : "void");
                case ReferenceType::Observed:
                    return ref.pointee ? (ref.pointee->Name() + " *") : "void*";
            }
        }
    }
    return "";
}

bool skr_type_t::Same(const skr_type_t* srcType) const
{
    using namespace skr::type;
    if (type != srcType->type)
        return false;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
        case SKR_TYPE_CATEGORY_I32:
        case SKR_TYPE_CATEGORY_I64:
        case SKR_TYPE_CATEGORY_U32:
        case SKR_TYPE_CATEGORY_U64:
        case SKR_TYPE_CATEGORY_F32:
        case SKR_TYPE_CATEGORY_F64:
        case SKR_TYPE_CATEGORY_GUID:
        case SKR_TYPE_CATEGORY_HANDLE:
        case SKR_TYPE_CATEGORY_STR:
        case SKR_TYPE_CATEGORY_STRV:
            return true;
        case SKR_TYPE_CATEGORY_ARR:
            return ((ArrayType*)this)->elementType->Same(((ArrayType*)srcType)->elementType) && ((ArrayType*)this)->num == ((ArrayType*)srcType)->num;
        case SKR_TYPE_CATEGORY_DYNARR:
            return ((DynArrayType*)this)->elementType->Same(((DynArrayType*)srcType)->elementType);
        case SKR_TYPE_CATEGORY_ARRV:
            return ((ArrayViewType*)this)->elementType->Same(((ArrayViewType*)srcType)->elementType);
        case SKR_TYPE_CATEGORY_OBJ:
        case SKR_TYPE_CATEGORY_ENUM:
            return this == srcType; //对象类型直接比较地址
        case SKR_TYPE_CATEGORY_REF: {
            {
                auto ptr = ((ReferenceType*)this);
                auto sptr = ((ReferenceType*)srcType);
                if ((ptr->pointee == nullptr || sptr->pointee == nullptr) && ptr->pointee != sptr->pointee)
                    return false;
                return ptr->ownership == sptr->ownership && ptr->nullable == sptr->nullable && ptr->pointee->Same(sptr->pointee);
            }
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
    gsl::span<size_t> acceptIndices;
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
            if(stype == SKR_TYPE_CATEGORY_REF)
            {
                auto sptr = (const ReferenceType*)srcType;
                //TODO: check if this is asset?
                if(sptr->pointee->type == SKR_TYPE_CATEGORY_OBJ)
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
            if(ptr.ownership == ReferenceType::Observed && stype == SKR_TYPE_CATEGORY_HANDLE) //TODO: handle's ownership?
            {
                auto& sptr = (const HandleType&)(*srcType);
                if (ptr.pointee == nullptr || sptr.pointee == nullptr)
                    return true;
                if (ptr.pointee->type == SKR_TYPE_CATEGORY_OBJ)
                {
                    if (ptr.pointee->Same(sptr.pointee))
                        return true;
                    auto& sobj = (const RecordType&)(*sptr.pointee);
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
#define STR_CONVERT                                                        \
    case SKR_TYPE_CATEGORY_STR:                                            \
        FromString(dst, eastl::string_view(*(eastl::string*)src), policy); \
    case SKR_TYPE_CATEGORY_STRV:                                           \
        FromString(dst, eastl::string_view(*(eastl::string_view*)src), policy);
#define ENUM_CONVERT                          \
    case SKR_TYPE_CATEGORY_ENUM: {            \
        auto& enm = (const EnumType&)(*this); \
        switch (enm.underlyingType->type)     \
        {                                     \
            BASE_CONVERT                      \
            default:                          \
                break;                        \
        }                                     \
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
                    policy->parse(policy, eastl::string_view(*(eastl::string*)src), dst, this);
                case SKR_TYPE_CATEGORY_STRV:
                    policy->parse(policy, *(eastl::string_view*)src, dst, this);
                case SKR_TYPE_CATEGORY_REF: {
                    auto& ref = (const ReferenceType&)(*srcType);
                    switch (ref.ownership)
                    {
                        case ReferenceType::Observed: {
                            auto& srcV = *(void**)src;
                            dstV = (bool)srcV;
                            break;
                        }
                        case ReferenceType::Shared: {
                            auto& srcV = *(std::shared_ptr<void>*)src;
                            dstV = (bool)srcV;
                            break;
                        }
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
            auto& dstV = *(eastl::string*)dst;
            switch (srcType->type)
            {
                case SKR_TYPE_CATEGORY_STRV:
                    dstV = *(eastl::string_view*)src;
                    break;
                default:
                    dstV = srcType->ToString(src);
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_STRV: {
            auto& dstV = *(eastl::string_view*)dst;
            switch (srcType->type)
            {
                case SKR_TYPE_CATEGORY_STR:
                    dstV = *(eastl::string*)src;
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
                    auto sv = *(gsl::span<char>*)src;
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
            auto& dstV = *(gsl::span<char>*)dst;
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
                    dstV = *(gsl::span<char>*)src;
                    break;
                default:
                    break;
            }
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            switch (enm.underlyingType->type)
            {
                case SKR_TYPE_CATEGORY_I32: {
                    using T = int32_t;
                    auto& dstV = *(T*)dst;
                    switch (srcType->type)
                    {
                        BASE_CONVERT
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
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            }
            if (srcType->type == SKR_TYPE_CATEGORY_STR)
                enm.FromString(dst, *(eastl::string*)src);
            else if (srcType->type == SKR_TYPE_CATEGORY_STRV)
                enm.FromString(dst, *(eastl::string_view*)src);
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
                    auto& srcV = *(std::shared_ptr<void>*)src;
                    if (ptr.ownership == ReferenceType::Observed)
                    {
                        auto& dstV = *(void**)dst;
                        dstV = srcV.get();
                    }
                    else if (ptr.ownership == ReferenceType::Shared)
                    {
                        auto& dstV = *(std::shared_ptr<void>*)dst;
                        dstV = srcV;
                    }
                }
            }
            else if (srcType->type == SKR_TYPE_CATEGORY_HANDLE)
            {
                auto& handle = *(skr_resource_handle_t*)src;
                *(void**)dst = handle.get_resolved();
            }
            else if (ptr.ownership == ReferenceType::Observed)
            {
                auto& dstV = *(void**)dst;
                dstV = (void*)src;
            }
        }
        default:
            break;
    }
}

eastl::string skr_type_t::ToString(const void* dst, skr::type::ValueSerializePolicy* policy) const
{
    using namespace skr::type;
    if (policy)
        return policy->format(policy, dst, this);
    else
    {
        switch (type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
                return *(bool*)dst == true ? "true" : "false";
            case SKR_TYPE_CATEGORY_I32:
                return format("{}", *(int32_t*)dst);
            case SKR_TYPE_CATEGORY_I64:
                return format("{}", *(int64_t*)dst);
            case SKR_TYPE_CATEGORY_U32:
                return format("{}", *(uint32_t*)dst);
            case SKR_TYPE_CATEGORY_U64:
                return format("{}", *(uint64_t*)dst);
            case SKR_TYPE_CATEGORY_F32:
                return format("{}", *(float*)dst);
            case SKR_TYPE_CATEGORY_F64:
                return format("{}", *(double*)dst);
            case SKR_TYPE_CATEGORY_GUID:
                return format("{}", *(skr_guid_t*)dst);
            case SKR_TYPE_CATEGORY_HANDLE:
                return format("{}", (*(skr_resource_handle_t*)dst).get_serialized());
            case SKR_TYPE_CATEGORY_ENUM:
                return ((const EnumType*)this)->ToString(dst);
            default:
                break;
        }
        return "";
    }
}

void skr_type_t::FromString(void* dst, eastl::string_view str, skr::type::ValueSerializePolicy* policy) const
{
    using namespace skr::type;
    if (policy)
        policy->parse(policy, str, dst, this);
    else
    {
        switch (type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
                if (str.compare("true") == 0)
                    *(bool*)dst = true;
                else
                    *(bool*)dst = false;
                break;
            case SKR_TYPE_CATEGORY_I32:
                std::from_chars(str.begin(), str.end(), *(int32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_I64:
                std::from_chars(str.begin(), str.end(), *(int64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U32:
                std::from_chars(str.begin(), str.end(), *(uint32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U64:
                std::from_chars(str.begin(), str.end(), *(uint64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_F32:
                fast_float::from_chars(str.begin(), str.end(), *(float*)dst);
                break;
            case SKR_TYPE_CATEGORY_F64:
                fast_float::from_chars(str.begin(), str.end(), *(double*)dst);
                break;
            case SKR_TYPE_CATEGORY_GUID:
                *(skr_guid_t*)dst = skr::guid::make_guid({ str.data(), str.size() });
                break;
            case SKR_TYPE_CATEGORY_HANDLE:
                (*(skr_resource_handle_t*)dst).set_guid(skr::guid::make_guid({ str.data(), str.size() }));
            case SKR_TYPE_CATEGORY_ENUM:
                ((const EnumType*)this)->FromString(dst, str);
                break;
            default:
                break;
        }
    }
}

size_t Hash(bool value, size_t base)
{
    return skr_hash(&value, 1, base);
}
size_t Hash(int32_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(int64_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(uint32_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(uint64_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(float value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(double value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_guid_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_resource_handle_t& value, size_t base)
{
    auto guid = value.get_guid();
    return skr_hash(&guid, sizeof(guid), base);
}
size_t Hash(void* value, size_t base)
{
    return skr_hash((void*)&value, sizeof(value), base);
}
size_t Hash(const eastl::string& value, size_t base)
{
    return skr_hash(value.data(), value.size(), base);
}
size_t Hash(const eastl::string_view& value, size_t base)
{
    return skr_hash(value.data(), value.size(), base);
}

template <class T>
size_t HashImpl(const void* dst, size_t base)
{
    return Hash(*(T*)dst, base);
}

size_t skr_type_t::Hash(const void* dst, size_t base) const
{
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
            return HashImpl<bool>(dst, base);
        case SKR_TYPE_CATEGORY_I32:
            return HashImpl<int32_t>(dst, base);
        case SKR_TYPE_CATEGORY_I64:
            return HashImpl<int64_t>(dst, base);
        case SKR_TYPE_CATEGORY_U32:
            return HashImpl<uint32_t>(dst, base);
        case SKR_TYPE_CATEGORY_U64:
            return HashImpl<uint64_t>(dst, base);
        case SKR_TYPE_CATEGORY_F32:
            return HashImpl<float>(dst, base);
        case SKR_TYPE_CATEGORY_F64:
            return HashImpl<double>(dst, base);
        case SKR_TYPE_CATEGORY_GUID:
            return HashImpl<skr_guid_t>(dst, base);
        case SKR_TYPE_CATEGORY_HANDLE:
            return HashImpl<skr_resource_handle_t>(dst, base);
        case SKR_TYPE_CATEGORY_STR:
            return HashImpl<eastl::string>(dst, base);
        case SKR_TYPE_CATEGORY_STRV:
            return HashImpl<eastl::string_view>(dst, base);
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            switch (enm.underlyingType->type)
            {
                case SKR_TYPE_CATEGORY_I32:
                    return HashImpl<int32_t>(dst, base);
                case SKR_TYPE_CATEGORY_I64:
                    return HashImpl<int64_t>(dst, base);
                case SKR_TYPE_CATEGORY_U32:
                    return HashImpl<uint32_t>(dst, base);
                case SKR_TYPE_CATEGORY_U64:
                    return HashImpl<uint64_t>(dst, base);
                default:
                    return 0;
            }
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
            auto v = *(gsl::span<char>*)dst;
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
                    return HashImpl<void*>((*(std::shared_ptr<void>*)dst).get(), base);
                    break;
            }
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
            ((eastl::string*)address)->~basic_string();
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
                ((std::shared_ptr<void>*)address)->~shared_ptr();
            }
        }
        default:
            break;
    }
}

template <class T>
void CopyImpl(void* dst, const void* src)
{
    new (dst) T(*(const T*)src);
}

void skr_type_t::Copy(void* dst, const void* src) const
{
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
            CopyImpl<bool>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_I32:
            CopyImpl<int32_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_I64:
            CopyImpl<int64_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_U32:
            CopyImpl<uint32_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_U64:
            CopyImpl<uint64_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_F32:
            CopyImpl<float>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_F64:
            CopyImpl<double>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_GUID:
            CopyImpl<skr_guid_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_HANDLE:
            CopyImpl<skr_resource_handle_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_STR:
            CopyImpl<eastl::string>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_STRV:
            CopyImpl<eastl::string_view>(dst, src);
            break;
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
            CopyImpl<gsl::span<char>>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_OBJ: {
            auto& obj = (const RecordType&)(*this);
            obj.nativeMethods.copy(dst, src);
            break;
        }
        case SKR_TYPE_CATEGORY_ENUM: {
            auto& enm = (const EnumType&)(*this);
            switch (enm.underlyingType->type)
            {
                case SKR_TYPE_CATEGORY_I32:
                    CopyImpl<int32_t>(dst, src);
                    break;
                case SKR_TYPE_CATEGORY_I64:
                    CopyImpl<int64_t>(dst, src);
                    break;
                case SKR_TYPE_CATEGORY_U32:
                    CopyImpl<uint32_t>(dst, src);
                    break;
                case SKR_TYPE_CATEGORY_U64:
                    CopyImpl<uint64_t>(dst, src);
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
                    CopyImpl<void*>(dst, src);
                    break;
                case ReferenceType::Shared:
                    CopyImpl<std::shared_ptr<void>>(dst, src);
                    break;
            }
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
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
            MoveImpl<bool>(dst, src);
            break;
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
        case SKR_TYPE_CATEGORY_F32:
            MoveImpl<float>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_F64:
            MoveImpl<double>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_GUID:
            MoveImpl<skr_guid_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_HANDLE:
            MoveImpl<skr_resource_handle_t>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_STR:
            MoveImpl<eastl::string>(dst, src);
            break;
        case SKR_TYPE_CATEGORY_STRV:
            MoveImpl<eastl::string_view>(dst, src);
            break;
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
            MoveImpl<gsl::span<char>>(dst, src);
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
                    MoveImpl<std::shared_ptr<void>>(dst, src);
                    break;
            }
            break;
        }
    }
}

void skr_type_t::Delete()
{
    using namespace skr::type;
    switch (type)
    {
        case SKR_TYPE_CATEGORY_BOOL:
            SkrDelete((BoolType*)this);
        case SKR_TYPE_CATEGORY_I32:
            SkrDelete((Int32Type*)this);
        case SKR_TYPE_CATEGORY_I64:
            SkrDelete((Int64Type*)this);
        case SKR_TYPE_CATEGORY_U32:
            SkrDelete((UInt32Type*)this);
        case SKR_TYPE_CATEGORY_U64:
            SkrDelete((UInt64Type*)this);
        case SKR_TYPE_CATEGORY_F32:
            SkrDelete((Float32Type*)this);
        case SKR_TYPE_CATEGORY_F64:
            SkrDelete((Float64Type*)this);
        case SKR_TYPE_CATEGORY_GUID:
            SkrDelete((GUIDType*)this);
        case SKR_TYPE_CATEGORY_HANDLE:
            SkrDelete((HandleType*)this);
        case SKR_TYPE_CATEGORY_STR:
            SkrDelete((StringType*)this);
        case SKR_TYPE_CATEGORY_STRV:
            SkrDelete((StringViewType*)this);
        case SKR_TYPE_CATEGORY_ARR:
            SkrDelete((ArrayType*)this);
        case SKR_TYPE_CATEGORY_DYNARR:
            SkrDelete((DynArrayType*)this);
        case SKR_TYPE_CATEGORY_ARRV:
            SkrDelete((ArrayViewType*)this);
        case SKR_TYPE_CATEGORY_OBJ:
            SkrDelete((RecordType*)this);
        case SKR_TYPE_CATEGORY_ENUM:
            SkrDelete((EnumType*)this);
        case SKR_TYPE_CATEGORY_REF:
            SkrDelete((ReferenceType*)this);
    }
}

bool skr::type::RecordType::IsBaseOf(const RecordType& other) const
{

    for (auto ptr = other.base; ptr; ptr = base->base)
        if (ptr == this)
            return true;
    return false;
}

skr_type_t* skr_get_type(const skr_type_id_t* id)
{
    auto& types = skr::type::GetTypeRegistry()->types;
    auto iter = types.find(*id);
    if (iter == types.end())
        return nullptr;
    return (skr_type_t*)iter->second;
}

namespace skr::type
{
Value::Value()
    : type(nullptr)
{
}

Value::Value(const Value& other)
{
    _Copy(other);
}

Value::Value(Value&& other)
{
    _Move(std::move(other));
}

Value& Value::operator=(const Value& other)
{
    Reset();
    _Copy(other);
    return *this;
}

Value& Value::operator=(Value&& other)
{
    Reset();
    _Move(std::move(other));
    return *this;
}

void* Value::Ptr()
{
    if (!type)
        return nullptr;
    if (type->Size() < smallSize)
        return &_smallObj[0];
    else
        return _ptr;
}

const void* Value::Ptr() const
{
    if (!type)
        return nullptr;
    if (type->Size() < smallSize)
        return &_smallObj[0];
    else
        return _ptr;
}

void Value::Reset()
{
    if (!type)
        return;
    if (type->Size() < smallSize)
        type->Destruct(&_smallObj[0]);
    else
    {
        type->Destruct(_ptr);
        free(_ptr);
    }
    type = nullptr;
}

size_t Value::Hash() const
{
    if (!type)
        return 0;
    return type->Hash(Ptr(), 0);
}

eastl::string Value::ToString() const
{
    if (!type)
        return {};
    return type->ToString(Ptr());
}

void* Value::_Alloc()
{
    if (!type)
        return nullptr;
    auto size = type->Size();
    if (size < smallSize)
        return &_smallObj[0];
    else
        return _ptr = malloc(size);
}

void Value::_Copy(const Value& other)
{
    type = other.type;
    if (!type)
        return;
    auto ptr = _Alloc();
    type->Copy(ptr, other.Ptr());
}

void Value::_Move(Value&& other)
{
    type = other.type;
    if (!type)
        return;
    auto ptr = _Alloc();
    type->Move(ptr, other.Ptr());
    other.Reset();
}

void ValueRef::Reset()
{
    type = nullptr;
    ptr = nullptr;
}

size_t ValueRef::Hash() const
{
    if (!type)
        return 0;
    return type->Hash(ptr, 0);
}

eastl::string ValueRef::ToString() const
{
    if (!type)
        return {};
    return type->ToString(ptr);
}
} // namespace skr::type