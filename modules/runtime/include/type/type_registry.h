#pragma once
#include "utils/types.h"
#include "platform/configure.h"
#include "resource/resource_handle.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct skr_field_t skr_field_t;
typedef struct skr_method_t skr_method_t;
typedef skr_guid_t skr_type_id_t;

typedef enum skr_type_category_t
{
    SKR_TYPE_CATEGORY_BOOL,
    SKR_TYPE_CATEGORY_I32,
    SKR_TYPE_CATEGORY_I64,
    SKR_TYPE_CATEGORY_U32,
    SKR_TYPE_CATEGORY_U64,
    SKR_TYPE_CATEGORY_F32,
    SKR_TYPE_CATEGORY_F64,
    SKR_TYPE_CATEGORY_GUID,
    SKR_TYPE_CATEGORY_HANDLE,
    SKR_TYPE_CATEGORY_STR,
    SKR_TYPE_CATEGORY_STRV,
    SKR_TYPE_CATEGORY_ARR,
    SKR_TYPE_CATEGORY_DYNARR,
    SKR_TYPE_CATEGORY_ARRV,
    SKR_TYPE_CATEGORY_OBJ,
    SKR_TYPE_CATEGORY_ENUM,
    SKR_TYPE_CATEGORY_REF,
} skr_type_category_t;

RUNTIME_API struct skr_type_t* skr_get_type(const skr_type_id_t* id);
RUNTIME_API void skr_get_derived_types(const struct skr_type_t* type, void (*callback)(void* u, struct skr_type_t* type), void* u);
RUNTIME_API void skr_get_type_id(const struct skr_type_t* type, skr_type_id_t* id);
RUNTIME_API uint32_t skr_get_type_size(const struct skr_type_t* type);
RUNTIME_API void skr_get_fields(const struct skr_type_t* type, void (*callback)(void* u, skr_field_t* field), void* u);
RUNTIME_API struct skr_type_t* skr_get_field_type(const skr_field_t* field);
RUNTIME_API const char* skr_get_field_name(const skr_field_t* field);

/*
generated:
skr_type_t* skr_typeof_xxxx();
void skr_typeid_xxxx(skr_type_id_t* id);
*/
#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
    #include "platform/guid.hpp"
    #include "EASTL/string.h"
    #include "gsl/span"
    #include "EASTL/vector.h"
    #include "utils/hashmap.hpp"

namespace skr
{
namespace type
{
struct ValueSerializePolicy;
struct Value;
} // namespace type
} // namespace skr

struct RUNTIME_API skr_type_t {
    skr_type_category_t type;
    size_t Size() const;
    size_t Align() const;
    eastl::string Name() const;
    bool Same(const skr_type_t* srcType) const;
    bool Convertible(const skr_type_t* srcType, bool format = false) const;
    void Convert(void* dst, const void* src, const skr_type_t* srcType, skr::type::ValueSerializePolicy* policy = nullptr) const;
    eastl::string ToString(const void* dst, skr::type::ValueSerializePolicy* policy = nullptr) const;
    void FromString(void* dst, eastl::string_view str, skr::type::ValueSerializePolicy* policy = nullptr) const;
    size_t Hash(const void* dst, size_t base) const;
    // lifetime operator
    void Destruct(void* dst) const;
    void Construct(void* dst, skr::type::Value* args, size_t nargs) const;
    // copy construct
    void Copy(void* dst, const void* src) const;
    // move construct
    void Move(void* dst, void* src) const;
    skr_type_t(skr_type_category_t type)
        : type(type)
    {
    }
    void Delete();
};

namespace skr
{
namespace type
{

struct DynArrayMethodTable {
    void (*dtor)(void* self);
    void (*ctor)(void* self);
    void (*copy)(void* self, const void* other);
    void (*move)(void* self, void* other);
    void (*push)(void* self, const void* data);
    void (*insert)(void* self, const void* data, size_t index);
    void (*erase)(void* self, size_t index);
    void (*resize)(void* self, size_t size);
    size_t (*size)(const void* self);
    void* (*get)(const void* self, size_t index);
    void* (*data)(const void* self);
};
struct ObjectMethodTable {
    void (*dtor)(void* self);
    void (*ctor)(void* self, Value* param, size_t nparam);
    void (*copy)(void* self, const void* other);
    void (*move)(void* self, void* other);
    size_t (*Hash)(const void* self, size_t base);
};

template <class T>
auto GetCopyCtor()
{
    void (*copy)(void* self, const void* other) = nullptr;
    if constexpr (std::is_copy_constructible_v<T>)
        copy = +[](void* self, const void* other) { new (self) T(*(const T*)other); };
    return copy;
}

template <class T>
auto GetMoveCtor()
{
    void (*move)(void* self, void* other) = nullptr;
    if constexpr (std::is_move_constructible_v<T>)
        move = +[](void* self, void* other) { new (self) T(std::move(*(T*)other)); };
    return move;
}

RUNTIME_API size_t Hash(bool value, size_t base);
RUNTIME_API size_t Hash(int32_t value, size_t base);
RUNTIME_API size_t Hash(int64_t value, size_t base);
RUNTIME_API size_t Hash(uint32_t value, size_t base);
RUNTIME_API size_t Hash(uint64_t value, size_t base);
RUNTIME_API size_t Hash(float value, size_t base);
RUNTIME_API size_t Hash(double value, size_t base);
RUNTIME_API size_t Hash(void* value, size_t base);

// bool
struct BoolType : skr_type_t {
    BoolType()
        : skr_type_t{ SKR_TYPE_CATEGORY_BOOL }
    {
    }
};
// int32_t
struct Int32Type : skr_type_t {
    Int32Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_I32 }
    {
    }
};
// int64_t
struct Int64Type : skr_type_t {
    Int64Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_I64 }
    {
    }
};
// uint32_t
struct UInt32Type : skr_type_t {
    UInt32Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_U32 }
    {
    }
};
// uint64_t
struct UInt64Type : skr_type_t {
    UInt64Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_U64 }
    {
    }
};
// float
struct Float32Type : skr_type_t {
    Float32Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_F32 }
    {
    }
};
// double
struct Float64Type : skr_type_t {
    Float64Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_F64 }
    {
    }
};
// guid
struct GUIDType : skr_type_t {
    GUIDType()
        : skr_type_t{ SKR_TYPE_CATEGORY_GUID }
    {
    }
};
// handle
struct HandleType : skr_type_t {
    const struct skr_type_t* pointee;
    HandleType(skr_type_t* pointee)
        : skr_type_t{ SKR_TYPE_CATEGORY_HANDLE }
        , pointee(pointee)
    {
    }
};
// eastl::string
struct StringType : skr_type_t {
    StringType()
        : skr_type_t{ SKR_TYPE_CATEGORY_STR }
    {
    }
};
// eastl::string_view
struct StringViewType : skr_type_t {
    StringViewType()
        : skr_type_t{ SKR_TYPE_CATEGORY_STRV }
    {
    }
};
// T[]
struct ArrayType : skr_type_t {
    const struct skr_type_t* elementType;
    size_t num;
    size_t size;
    ArrayType(const struct skr_type_t* elementType, size_t num, size_t size)
        : skr_type_t{ SKR_TYPE_CATEGORY_ARR }
        , elementType(elementType)
        , num(num)
        , size(size)
    {
    }
};
// std::vector<T>
struct DynArrayType : skr_type_t {
    const struct skr_type_t* elementType;
    DynArrayMethodTable operations;
    DynArrayType(const skr_type_t* elementType, DynArrayMethodTable operations)
        : skr_type_t{ SKR_TYPE_CATEGORY_DYNARR }
        , elementType(elementType)
        , operations(operations)
    {
    }
};
// gsl::span<T>
struct ArrayViewType : skr_type_t {
    const struct skr_type_t* elementType;
    ArrayViewType(const skr_type_t* elementType)
        : skr_type_t{ SKR_TYPE_CATEGORY_ARRV }
        , elementType(elementType)
    {
    }
};
// struct/class T
struct RecordType : skr_type_t {
    size_t size;
    size_t align;
    skr_guid_t guid;
    const eastl::string_view name;
    const RecordType* base;
    ObjectMethodTable nativeMethods;
    const gsl::span<struct skr_field_t> fields;
    const gsl::span<struct skr_method_t> methods;
    bool IsBaseOf(const RecordType& other) const;
    static const RecordType* FromName(eastl::string_view name);
    static void Register(const RecordType* type);
    RecordType(size_t size, size_t align, eastl::string_view name, skr_guid_t guid, const RecordType* base, ObjectMethodTable nativeMethods,
    const gsl::span<struct skr_field_t> fields, const gsl::span<struct skr_method_t> methods)
        : skr_type_t{ SKR_TYPE_CATEGORY_OBJ }
        , size(size)
        , align(align)
        , guid(guid)
        , name(name)
        , base(base)
        , nativeMethods(nativeMethods)
        , fields(fields)
        , methods(methods)
    {
    }
};
// enum T
struct EnumType : skr_type_t {
    const skr_type_t* underlyingType;
    const eastl::string_view name;
    skr_guid_t guid;
    void (*FromString)(void* self, eastl::string_view str);
    eastl::string (*ToString)(const void* self);
    struct Enumerator {
        const eastl::string_view name;
        int64_t value;
    };
    const gsl::span<Enumerator> enumerators;
    static const EnumType* FromName(eastl::string_view name);
    static void Register(const EnumType* type);
    EnumType(const skr_type_t* underlyingType, const eastl::string_view name,
    skr_guid_t guid, void (*FromString)(void* self, eastl::string_view str),
    eastl::string (*ToString)(const void* self), const gsl::span<Enumerator> enumerators)
        : skr_type_t{ SKR_TYPE_CATEGORY_ENUM }
        , underlyingType(underlyingType)
        , name(name)
        , guid(guid)
        , FromString(FromString)
        , ToString(ToString)
        , enumerators(enumerators)
    {
    }
};

struct STypeRegistry {
    skr::flat_hash_map<skr_guid_t, const skr_type_t*, skr::guid::hash> types;
};

RUNTIME_API STypeRegistry* GetTypeRegistry();

// T*, T&, std::unique_ptr<T>, std::shared_ptr<T>
struct ReferenceType : skr_type_t {
    enum Ownership
    {
        Observed,
        Shared
    } ownership;
    bool nullable;
    const struct skr_type_t* pointee;
    ReferenceType(Ownership ownership, bool nullable, const skr_type_t* pointee)
        : skr_type_t{ SKR_TYPE_CATEGORY_REF }
        , ownership(ownership)
        , nullable(nullable)
        , pointee(pointee)
    {
    }
};

template <class T>
struct type_of {
    static const skr_type_t* get();
};

    #define BASE_TYPE(name)                             \
        template <>                                     \
        struct type_of<name> {                          \
            RUNTIME_API static const skr_type_t* get(); \
        }
BASE_TYPE(bool);
BASE_TYPE(uint32_t);
BASE_TYPE(uint64_t);
BASE_TYPE(int32_t);
BASE_TYPE(int64_t);
BASE_TYPE(float);
BASE_TYPE(double);
BASE_TYPE(skr_guid_t);
BASE_TYPE(skr_resource_handle_t);
BASE_TYPE(eastl::string);
BASE_TYPE(eastl::string_view);
    #undef BASE_TYPE

template <>
struct type_of<void*> {
    static const skr_type_t* get()
    {
        static ReferenceType type{
            ReferenceType::Observed,
            true,
            nullptr
        };
        return &type;
    }
};

template <class T>
struct type_of<resource::TResourceHandle<T>> {
    static const skr_type_t* get()
    {
        static HandleType type{
            type_of<T>::get()
        };
        return &type;
    }
};

template <class T>
struct type_of<const T> {
    static const skr_type_t* get()
    {
        return type_of<T>::get();
    }
};

template <class T>
struct type_of<volatile T> {
    static const skr_type_t* get()
    {
        return type_of<T>::get();
    }
};

template <class T>
struct type_of<T*> {
    static const skr_type_t* get()
    {
        static ReferenceType type{
            ReferenceType::Observed,
            true,
            type_of<T>::get()
        };
        return &type;
    }
};

template <class T>
struct type_of<T&> {
    static const skr_type_t* get()
    {
        static ReferenceType type{
            ReferenceType::Observed,
            false,
            type_of<T>::get()
        };
        return &type;
    }
};

template <class T>
struct type_of<std::shared_ptr<T>> {
    static const skr_type_t* get()
    {
        static ReferenceType type{
            ReferenceType::Shared,
            true,
            type_of<T>::get()
        };
        return &type;
    }
};

template <class V, class T>
struct type_of_vector {
    static const skr_type_t* get()
    {
        static DynArrayType type{
            type_of<T>::get(),
            DynArrayMethodTable{
            +[](void* self) { ((V*)(self))->~vector(); },                                                                              // dtor
            +[](void* self) { new (self) V(); },                                                                                       // ctor
            +[](void* self, const void* other) { new (self) V(*((const V*)(other))); },                                                // copy
            +[](void* self, void* other) { new (self) V(std::move(*(V*)(other))); },                                                   // move
            +[](void* self, const void* data) { ((V*)(self))->push_back(*(const T*)data); },                                           // push
            +[](void* self, const void* data, size_t index) { ((V*)(self))->insert(((V*)(self))->begin() + index, *(const T*)data); }, // insert
            +[](void* self, size_t index) { ((V*)(self))->erase(((V*)(self))->begin() + index); },                                     // erase
            +[](void* self, size_t size) { ((V*)(self))->resize(size); },                                                              // resize
            +[](const void* self) { return ((V*)(self))->size(); },                                                                    // size
            +[](const void* self, size_t index) { return (void*)&((V*)(self))[index]; },                                               // get
            +[](const void* self) { return (void*)((V*)(self))->data(); },                                                             // data
            }
        };
        return &type;
    }
};

template <class T, class Allocator>
struct type_of<eastl::vector<T, Allocator>> : type_of_vector<eastl::vector<T, Allocator>, T> {
};

template <class T, size_t num>
struct type_of<T[num]> {
    static const skr_type_t* get()
    {
        static ArrayType type{
            type_of<T>::get(),
            num,
            sizeof(T[num])
        };
        return &type;
    }
};

template <class T, size_t size>
struct type_of<gsl::span<T, size>> {
    static const skr_type_t* get()
    {
        static_assert(size == -1, "only dynamic extent is supported.");
        static ArrayViewType type{
            type_of<T>::get()
        };
        return &type;
    }
};

struct RUNTIME_API alignas(16) Value {
    const skr_type_t* type;
    union
    {
        void* _ptr;
        uint8_t _smallObj[24];
    };

    static constexpr size_t smallSize = sizeof(_smallObj);

    Value();
    Value(Value&& other);
    Value(const Value& other);

    Value& operator=(Value&& other);
    Value& operator=(const Value& other);

    operator bool() const { return HasValue(); }

    bool HasValue() const { return type != nullptr; }

    template <class T, class... Args>
    std::enable_if_t<!std::is_reference_v<T> && std::is_constructible_v<T, Args...>, void>
    Emplace(Args&&... args)
    {
        Reset();
        type = type_of<T>::get();
        void* ptr = _Alloc();
        new (ptr) T(std::forward<Args>(args)...);
    }

    template <class T, class V>
    std::enable_if_t<std::is_reference_v<T>, void>
    Emplace(const V& v)
    {
        Reset();
        type = type_of<T>::get();
        void* ptr = _Alloc();
        *(std::remove_reference_t<T>**)ptr = &(V&)v;
    }

    template <class T>
    bool Is() const
    {
        return type_of<T>::get() == type;
    }

    template <class T>
    T& As()
    {
        return *(T*)Ptr();
    }

    template <class T>
    bool Convertible() const
    {
        if (!type)
            return false;
        return type_of<T>::get()->Convertible(type);
    }

    template <class T>
    T Convert()
    {
        std::aligned_storage_t<sizeof(T), alignof(T)> storage;
        type_of<T>::get()->Convert(&storage, Ptr(), type);
        return std::move(*std::launder(reinterpret_cast<T*>(&storage)));
    }

    void* Ptr();
    const void* Ptr() const;

    size_t Hash() const;
    eastl::string ToString() const;

    void Reset();

    ~Value() { Reset(); }

private:
    void* _Alloc();
    void _Copy(const Value& other);
    void _Move(Value&& other);
};

struct RUNTIME_API ValueRef {
    void* ptr = nullptr;
    const skr_type_t* type = nullptr;
    ValueRef() = default;
    template <class T>
    ValueRef(T& t)
        : ptr(&t)
        , type(type_of<T>::get())
    {
    }
    ValueRef(Value& v)
        : ptr(v.Ptr())
        , type(v.type)
    {
    }
    ValueRef(ValueRef&& other) = default;
    ValueRef(ValueRef& other)
        : ptr(other.ptr)
        , type(other.type)
    {
    }
    ValueRef(const ValueRef& other)
        : ptr(other.ptr)
        , type(other.type)
    {
    }
    ValueRef& operator=(const ValueRef& other)
    {
        ptr = other.ptr;
        type = other.type;
        return *this;
    }
    ValueRef& operator=(ValueRef& other)
    {
        ptr = other.ptr;
        type = other.type;
        return *this;
    }
    ValueRef& operator=(ValueRef&& other) = default;
    template <class T>
    ValueRef& operator=(T& t)
    {
        ptr = (void*)&t;
        type = type_of<T>::get();
        return *this;
    }
    bool operator==(const ValueRef& other)
    {
        return ptr == other.ptr && type == other.type;
    }
    bool operator!=(const ValueRef& other)
    {
        return !((*this) == other);
    }
    operator bool() const { return HasValue(); }
    bool HasValue() const { return type != nullptr; }
    template <class T>
    bool Is() const
    {
        return type_of<T>::get() == type;
    }
    template <class T>
    T& As()
    {
        return *(T*)ptr;
    }
    template <class T>
    bool Convertible() const
    {
        if (!type)
            return false;
        return type_of<T>::get()->Convertible(type);
    }

    template <class T>
    T Convert()
    {
        std::aligned_storage_t<sizeof(T), alignof(T)> storage;
        type_of<T>::get()->Convert(&storage, ptr, type);
        return std::move(*std::launder(reinterpret_cast<T*>(&storage)));
    }

    size_t Hash() const;
    eastl::string ToString() const;
    void Reset();
    ~ValueRef() { Reset(); }
};

} // namespace type
} // namespace skr
struct skr_field_t {
    eastl::string_view name;
    const skr_type_t* type;
    size_t offset;
};

struct skr_method_t {
    eastl::string_view name;
    const skr_type_t* retType;
    const skr_field_t* parameters;
    skr::type::Value (*execute)(void* self, skr::type::ValueRef* args, size_t nargs);
};

namespace skr
{
namespace type
{
struct ValueSerializePolicy {
    eastl::string (*format)(void* self, const void* data, const struct skr_type_t* type);
    void (*parse)(void* self, eastl::string_view str, void* data, const struct skr_type_t* type);
};

/*
struct Serializer
{
    void BeginSerialize();
    eastl::string EndSerialize();
    void BeginDeserialize(eastl::string_view str);
    void EndDeserialize();
    void Raw(eastl::string_view);
    void Bool(bool&);
    void Int32(int32_t&);
    ....
    bool Defined(const RecordType*);
    void Object(void* data, const RecordType*);
    void BeginObject(const RecordType* type);
    void EndObject();
    void BeginField(const skr_field_t* field);
    void EndField();
    void BeginArray(); void EndArray();
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
        parse = +[](void* self, eastl::string_view str, void* data, const struct skr_type_t* type) {
            auto& s = ((TValueSerializePolicy*)self)->s;
            s.BeginDeserialize(str);
            serializeImpl(&s, data, type);
            s.EndSerialize();
        };
    }

    static void serializeImpl(void* self, void* data, const struct skr_type_t* type)
    {
        auto& ctx = *(Serializer*)self;
        switch (type->type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
                ctx.Bool(*(bool*)data);
                break;
            case SKR_TYPE_CATEGORY_I32:
                ctx.Int32(*(int32_t*)data);
                break;
            case SKR_TYPE_CATEGORY_I64:
                ctx.Int64(*(int64_t*)data);
                break;
            case SKR_TYPE_CATEGORY_U32:
                ctx.UInt32(*(uint32_t*)data);
                break;
            case SKR_TYPE_CATEGORY_U64:
                ctx.UInt64(*(uint64_t*)data);
                break;
            case SKR_TYPE_CATEGORY_F32:
                ctx.Float32(*(float*)data);
                break;
            case SKR_TYPE_CATEGORY_F64:
                ctx.Float64(*(double*)data);
                break;
            case SKR_TYPE_CATEGORY_STR:
                ctx.String(*(eastl::string*)data);
                break;
            case SKR_TYPE_CATEGORY_STRV:
                ctx.StringView(*(eastl::string_view*)data);
                break;
            case SKR_TYPE_CATEGORY_ARR: {
                ctx.BeginArray();
                auto& arr = (const ArrayType&)(*type);
                auto element = arr.elementType;
                auto d = (char*)data;
                auto size = element->Size();
                for (int i = 0; i < arr.num; ++i)
                    formatImpl(&ctx, d + i * size, element);
                ctx.EndArray();
                break;
            }
            case SKR_TYPE_CATEGORY_DYNARR: {
                ctx.BeginArray();
                auto& arr = (const DynArrayType&)(*type);
                auto element = arr.elementType;
                auto d = (char*)arr.operations.data(data);
                auto n = arr.operations.size(data);
                auto size = element->Size();
                for (int i = 0; i < n; ++i)
                    formatImpl(&ctx, d + i * size, element);
                ctx.EndArray();
                break;
            }
            case SKR_TYPE_CATEGORY_ARRV: {
                ctx.BeginArray();
                auto& arr = (const ArrayViewType&)(*type);
                auto& element = arr.elementType;
                auto size = element->Size();
                auto v = *(gsl::span<char>*)data;
                auto n = v.size();
                auto d = v.data();
                for (int i = 0; i < n; ++i)
                    formatImpl(&ctx, d + i * size, element);
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
                        ctx.BeginField(field);
                        formatImpl(&ctx, d + field.offset, field.type);
                        ctx.EndField();
                    }
                    ctx.EndObject();
                }
            }
            case SKR_TYPE_CATEGORY_ENUM: {
                auto enm = (const EnumType*)type;
                ctx.Raw(enm->ToString(data));
            }
            case SKR_TYPE_CATEGORY_REF: {
                void* address;
                switch (((const ReferenceType*)type)->ownership)
                {
                    case ReferenceType::Observed:
                        address = *(void**)data;
                        break;
                    case ReferenceType::Shared:
                        address = (*(std::shared_ptr<void>*)data).get();
                        break;
                }
                // TODO: 可选是否递归序列化
                ctx.Reference(address);
                break;
            }
        }
    }
};
} // namespace type
} // namespace skr
#endif