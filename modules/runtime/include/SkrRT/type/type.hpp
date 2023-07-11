#pragma once
#include "SkrRT/type/type.h"
#include "type_id.hpp"
#include "SkrRT/type/type_helper.hpp"

#include "containers/span.hpp"
#include "containers/vector.hpp"
#include "containers/string.hpp"

#if defined(__cplusplus)
namespace skr
{
namespace type
{
    struct RecordType;
    struct EnumType;
struct STypeRegistry 
{
    virtual const skr_type_t* get_type(skr_guid_t tid) = 0;
    virtual RecordType* register_record(skr_guid_t tid) = 0;
    virtual EnumType* register_enum(skr_guid_t tid) = 0;
    virtual void invalidate_type(skr_guid_t tid) = 0;
    virtual bool is_outdated(const skr_type_t* type) = 0;
};
RUNTIME_API STypeRegistry* GetTypeRegistry();

RUNTIME_API uint64_t Hash(bool value, uint64_t base);
RUNTIME_API uint64_t Hash(int32_t value, uint64_t base);
RUNTIME_API uint64_t Hash(int64_t value, uint64_t base);
RUNTIME_API uint64_t Hash(uint32_t value, uint64_t base);
RUNTIME_API uint64_t Hash(uint64_t value, uint64_t base);
RUNTIME_API uint64_t Hash(float value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_float2_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_float3_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_float4_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_float4x4_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_quaternion_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_rotator_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(double value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_guid_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_md5_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr_resource_handle_t& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr::string& value, uint64_t base);
RUNTIME_API uint64_t Hash(const skr::string_view& value, uint64_t base);

template <class T>
auto GetCopyCtor();
template <class T>
auto GetMoveCtor();
} // namespace type
} // namespace skr
#endif

struct RUNTIME_API skr_type_t {
    skr_type_category_t type SKR_IF_CPP(= SKR_TYPE_CATEGORY_INVALID);
#ifdef __cplusplus
    skr_type_t() = default;
    skr_type_t(skr_type_category_t type);
    virtual ~skr_type_t() SKR_NOEXCEPT;
    // clone type self
    skr_type_t* _Clone() const;

    uint64_t Size() const;
    uint64_t Align() const;
    skr_guid_t Id() const;
    const char8_t* Name() const;
    bool Same(const skr_type_t* srcType) const;
    bool Convertible(const skr_type_t* srcType, bool format = false) const;
    void Convert(void* dst, const void* src, const skr_type_t* srcType, skr::type::ValueSerializePolicy* policy = nullptr) const;
    skr::string ToString(const void* dst, skr::type::ValueSerializePolicy* policy = nullptr) const;
    void FromString(void* dst, skr::string_view str, skr::type::ValueSerializePolicy* policy = nullptr) const;
    uint64_t Hash(const void* dst, uint64_t base) const;
    // lifetime operator
    void Destruct(void* dst) const;
    void Construct(void* dst, skr::type::Value* args, uint64_t nargs) const;
    void* Malloc() const;
    void Free(void*) const;
    // copy construct
    void Copy(void* dst, const void* src) const;
    // move construct
    void Move(void* dst, void* src) const;
    void Delete();
    int Serialize(const void* dst, skr_binary_writer_t* writer) const;
    int Deserialize(void* dst, skr_binary_reader_t* reader) const;
    void SerializeText(const void* dst, skr_json_writer_t* writer) const;
    skr::json::error_code DeserializeText(void* dst, skr::json::value_t&& reader) const;
#endif
};

struct RUNTIME_API skr_field_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    const skr_type_t* type SKR_IF_CPP(= nullptr);
    uint64_t offset SKR_IF_CPP(= 0);
};

struct SKR_ALIGNAS(16) RUNTIME_API skr_value_t {
    const skr_type_t* type SKR_IF_CPP(= nullptr);
    union
    {
        void* _ptr;
        uint8_t _smallObj[24];
    };
#ifdef __cplusplus
    static constexpr uint64_t smallSize = sizeof(_smallObj);

    skr_value_t() = default;
    skr_value_t(const skr_value_ref_t& ref);
    skr_value_t(skr_value_t&& other);
    skr_value_t(const skr_value_t& other);
    skr_value_t& operator=(skr_value_t&& other);
    skr_value_t& operator=(const skr_value_t& other);
    skr_value_t& operator=(const skr_value_ref_t& other);

    ~skr_value_t() { Reset(); }

    explicit operator bool() const { return HasValue(); }
    bool HasValue() const { return type != nullptr; }

    template <class T, class... Args>
    std::enable_if_t<!std::is_reference_v<T> && std::is_constructible_v<T, Args...>, void>
    Emplace(Args&&... args);

    template <class T, class V>
    std::enable_if_t<std::is_reference_v<T>, void>
    Emplace(const V& v);

    template <class T>
    bool Is() const;
    template <class T>
    T& As();

    template <class T>
    bool Convertible() const;
    template <class T>
    T Convert();

    void* Ptr();
    const void* Ptr() const;

    uint64_t Hash() const;
    skr::string ToString() const;

    void Reset();

private:
    void* _Alloc();
    void _Copy(const skr_value_t& other);
    void _Copy(const skr_value_ref_t& other);
    void _Move(skr_value_t&& other);
#endif
};

struct SKR_ALIGNAS(16) RUNTIME_API skr_poly_value_t {
    const skr_type_t* type SKR_IF_CPP(= nullptr);
    union
    {
        void* _ptr;
        uint8_t _smallObj[24];
    };
};

struct RUNTIME_API skr_value_ref_t {
    void* ptr = nullptr;
    const skr_type_t* type = nullptr;
#ifdef __cplusplus
    skr_value_ref_t() = default;
    ~skr_value_ref_t();
    template <class T> skr_value_ref_t(T& t);
    skr_value_ref_t(void* address, const skr_type_t* type);
    skr_value_ref_t(skr_value_t& v);
    skr_value_ref_t(skr_value_ref_t&& other) = default;
    skr_value_ref_t(skr_value_ref_t& other);
    skr_value_ref_t(const skr_value_ref_t& other);
    skr_value_ref_t& operator=(const skr_value_ref_t& other);
    skr_value_ref_t& operator=(skr_value_ref_t& other);
    skr_value_ref_t& operator=(skr_value_ref_t&& other) = default;
    template <class T>
    skr_value_ref_t& operator=(T& t);
    bool operator==(const skr_value_ref_t& other);
    bool operator!=(const skr_value_ref_t& other);
    operator bool() const;
    bool HasValue() const;
    template <class T>
    bool Is() const;
    template <class T>
    T& As();
    template <class T>
    bool Convertible() const;
    template <class T>
    T Convert();

    uint64_t Hash() const;
    skr::string ToString() const;
    void Reset();
#endif
};

struct RUNTIME_API skr_method_t {
    const char* name SKR_IF_CPP(= nullptr);
    const skr_type_t* retType SKR_IF_CPP(= nullptr);
    const skr_field_t* parameters SKR_IF_CPP(= nullptr);
    skr_value_t (*execute)(void* self, skr_value_ref_t* args, uint64_t nargs) SKR_IF_CPP(= nullptr);
};

#ifdef __cplusplus
// base types
namespace skr 
{
namespace type
{
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
// float2
struct Float32x2Type : skr_type_t {
    Float32x2Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_F32_2 }
    {
    }
};
// float3
struct Float32x3Type : skr_type_t {
    Float32x3Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_F32_3 }
    {
    }
};
// float4
struct Float32x4Type : skr_type_t {
    Float32x4Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_F32_4 }
    {
    }
};
// float4x4
struct Float32x4x4Type : skr_type_t {
    Float32x4x4Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_F32_4x4 }
    {
    }
};
// rot
struct RotType : skr_type_t {
    RotType()
        : skr_type_t{ SKR_TYPE_CATEGORY_ROT }
    {
    }
};
// quaternion
struct QuaternionType : skr_type_t {
    QuaternionType()
        : skr_type_t{ SKR_TYPE_CATEGORY_QUAT }
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
// md5
struct MD5Type : skr_type_t {
    MD5Type()
        : skr_type_t{ SKR_TYPE_CATEGORY_MD5 }
    {
    }
};
// handle
struct RUNTIME_API HandleType : skr_type_t {
    const struct skr_type_t* pointee;
    HandleType(const skr_type_t* pointee)
        : skr_type_t{ SKR_TYPE_CATEGORY_HANDLE }
        , pointee(pointee)
    {
    }
};
// skr::string
struct RUNTIME_API StringType : skr_type_t {
    StringType()
        : skr_type_t{ SKR_TYPE_CATEGORY_STR }
    {
    }
};
// skr::string_view
struct RUNTIME_API StringViewType : skr_type_t {
    StringViewType()
        : skr_type_t{ SKR_TYPE_CATEGORY_STRV }
    {
    }
};
// T[]
struct RUNTIME_API ArrayType : skr_type_t {
    const struct skr_type_t* elementType;
    uint64_t num;
    uint64_t size;
    skr::string name;
    ArrayType(const struct skr_type_t* elementType, uint64_t num, uint64_t size)
        : skr_type_t{ SKR_TYPE_CATEGORY_ARR }
        , elementType(elementType)
        , num(num)
        , size(size)
    {
    }
};
// Object
struct ObjectMethodTable {
    void (*dtor)(void* self);
    void (*ctor)(void* self, Value* param, uint64_t nparam);
    void (*copy)(void* self, const void* other);
    void (*move)(void* self, void* other);
    uint64_t (*Hash)(const void* self, uint64_t base);
    int (*Serialize)(const void* self, skr_binary_writer_t* writer);
    int (*Deserialize)(void* self, skr_binary_reader_t* reader);
    void (*SerializeText)(const void*, skr_json_writer_t* writer);
    json::error_code (*DeserializeText)(void* self, json::value_t&& reader);
};
// skr::span<T>
struct RUNTIME_API ArrayViewType : skr_type_t {
    const struct skr_type_t* elementType;
    skr::string name;
    ArrayViewType(const skr_type_t* elementType)
        : skr_type_t{ SKR_TYPE_CATEGORY_ARRV }
        , elementType(elementType)
    {
    }
};
// vector storage
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
    RUNTIME_API void Reset(void* data, uint64_t size) const;
    RUNTIME_API void Reserve(void* data, uint64_t size) const;
    RUNTIME_API void Resize(void* data, uint64_t size) const;
    RUNTIME_API void* Insert(void* data, uint64_t index) const;
    RUNTIME_API void Erase(void* data, uint64_t index, bool bKeepOrder) const;
    DynArrayType(const skr_type_t* elementType)
        : skr_type_t{ SKR_TYPE_CATEGORY_DYNARR }
        , elementType(elementType)
    {
    }
private:
    void Grow(void* data, uint64_t size) const;
};
// struct/class T
struct RUNTIME_API RecordType : skr_type_t {
    uint64_t size = 0;
    uint64_t align = 0;
    skr_guid_t guid = {};
    bool object = false; // true if inherits from SInterface
    const skr::string_view name = u8"";
    const RecordType* base = nullptr;
    ObjectMethodTable nativeMethods = {};
    const skr::span<struct skr_field_t> fields = {};
    const skr::span<struct skr_method_t> methods = {};
    bool IsBaseOf(const RecordType& other) const;
    RecordType() = default;
    RecordType(RecordType&&) = default;
    RecordType(uint64_t size, uint64_t align, skr::string_view name, skr_guid_t guid, bool object, const RecordType* base, ObjectMethodTable nativeMethods,
    const skr::span<struct skr_field_t> fields, const skr::span<struct skr_method_t> methods)
        : skr_type_t{ SKR_TYPE_CATEGORY_OBJ }
        , size(size)
        , align(align)
        , guid(guid)
        , object(object)
        , name(name)
        , base(base)
        , nativeMethods(nativeMethods)
        , fields(fields)
        , methods(methods)
    {
    }
};
// enum T
struct RUNTIME_API EnumType : skr_type_t {
    const skr_type_t* underlyingType;
    const skr::string_view name = {};
    skr_guid_t guid;
    void (*FromString)(void* self, skr::string_view str);
    skr::string (*ToString)(const void* self);
    struct Enumerator {
        const skr::string_view name;
        int64_t value;
    };
    const skr::span<Enumerator> enumerators;
    EnumType() = default;
    EnumType(EnumType&&) = default;
    EnumType(const skr_type_t* underlyingType, const skr::string_view name,
    skr_guid_t guid, void (*FromString)(void* self, skr::string_view str),
    skr::string (*ToString)(const void* self), const skr::span<Enumerator> enumerators)
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
// T*, T&, skr::SPtr<T>
struct RUNTIME_API ReferenceType : skr_type_t {
    enum Ownership
    {
        Observed,
        Shared
    } ownership;
    bool nullable;
    bool object;
    const struct skr_type_t* pointee;
    skr::string name;
    ReferenceType(Ownership ownership, bool nullable, bool object, const skr_type_t* pointee)
        : skr_type_t{ SKR_TYPE_CATEGORY_REF }
        , ownership(ownership)
        , nullable(nullable)
        , object(object)
        , pointee(pointee)
    {
    }
};
}    
}

namespace skr
{
namespace type
{
// void*
template <>
struct type_of<void*> {
    RUNTIME_API static const skr_type_t* get();
};
// SInterface*
template<>
struct type_of<SInterface> {
    RUNTIME_API static const skr_type_t* get();
};
// const wrapper
template <class T>
struct type_of<const T> {
    static const skr_type_t* get()
    {
        return type_of<T>::get();
    }
};
// volatile wrapper
template <class T>
struct type_of<volatile T> {
    static const skr_type_t* get()
    {
        return type_of<T>::get();
    }
};
// ptr wrapper
RUNTIME_API const skr_type_t* make_pointer_type(const skr_type_t* type);
template <class T>
struct type_of<T*> {
    static const skr_type_t* get()
    {
        return make_pointer_type(type_of<T>::get());
    }
};
// ref wrapper
RUNTIME_API const skr_type_t* make_reference_type(const skr_type_t* type);
template <class T>
struct type_of<T&> {
    static const skr_type_t* get()
    {
        return make_reference_type(type_of<T>::get());
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

template <class T, class Allocator>
struct type_of<skr::vector<T, Allocator>> : type_of_vector<skr::vector<T, Allocator>, T> 
{
};

RUNTIME_API const skr_type_t* make_array_type(const skr_type_t* type, uint64_t num, uint64_t size);
template <class T, uint64_t N>
struct type_of<T[N]> {
    static const skr_type_t* get()
    {
        return make_array_type(type_of<T>::get(), N, sizeof(T[N]));
    }
};

RUNTIME_API const skr_type_t* make_array_view_type(const skr_type_t* type);
template <class T, uint64_t size>
struct type_of<skr::span<T, size>> {
    static const skr_type_t* get()
    {
        static_assert(size == -1, "only dynamic extent is supported.");
        return make_array_view_type(type_of<T>::get());
    }
};
} // namespace type
} // namespace skr

// implementations

template <class T>
auto skr::type::GetCopyCtor()
{
    void (*copy)(void* self, const void* other) = nullptr;
    if constexpr (std::is_copy_constructible_v<T>)
        copy = +[](void* self, const void* other) { new (self) T(*(const T*)other); };
    return copy;
}

template <class T>
auto skr::type::GetMoveCtor()
{
    void (*move)(void* self, void* other) = nullptr;
    if constexpr (std::is_move_constructible_v<T>)
        move = +[](void* self, void* other) { new (self) T(std::move(*(T*)other)); };
    return move;
}

// value
template <class T, class... Args>
std::enable_if_t<!std::is_reference_v<T> && std::is_constructible_v<T, Args...>, void>
skr_value_t::Emplace(Args&&... args)
{
    Reset();
    type = skr::type::type_of<T>::get();
    void* ptr = _Alloc();
    new (ptr) T(std::forward<Args>(args)...);
}

template <class T, class V>
std::enable_if_t<std::is_reference_v<T>, void>
skr_value_t::Emplace(const V& v)
{
    Reset();
    type = skr::type::type_of<T>::get();
    void* ptr = _Alloc();
    *(std::remove_reference_t<T>**)ptr = &(V&)v;
}

template <class T>
bool skr_value_t::Is() const
{
    return skr::type::type_of<T>::get() == type;
}

template <class T>
T& skr_value_t::As()
{
    return *(T*)Ptr();
}

template <class T>
bool skr_value_t::Convertible() const
{
    if (!type)
        return false;
    return skr::type::type_of<T>::get()->Convertible(type);
}

template <class T>
T skr_value_t::Convert()
{
    std::aligned_storage_t<sizeof(T), alignof(T)> storage;
    skr::type::type_of<T>::get()->Convert(&storage, Ptr(), type);
    return std::move(*std::launder(reinterpret_cast<T*>(&storage)));
}

// value_ref
template <class T>
skr_value_ref_t::skr_value_ref_t(T& t)
{
    ptr = &t;
    type = skr::type::type_of<std::decay_t<T>>::get();
}

template <class T>
skr_value_ref_t& skr_value_ref_t::operator=(T& t)
{
    ptr = (void*)&t;
    type = skr::type::type_of<T>::get();
    return *this;
}

template <class T>
bool skr_value_ref_t::Is() const
{
    return skr::type::type_of<T>::get() == type;
}

template <class T>
T& skr_value_ref_t::As()
{
    return *(T*)ptr;
}

template <class T>
bool skr_value_ref_t::Convertible() const
{
    if (!type)
        return false;
    return skr::type::type_of<T>::get()->Convertible(type);
}

template <class T>
T skr_value_ref_t::Convert()
{
    using Y = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<std::remove_reference_t<T>>, T>;
    std::aligned_storage_t<sizeof(Y), alignof(Y)> storage;
    skr::type::type_of<T>::get()->Convert(&storage, ptr, type);
    if constexpr (std::is_reference_v<T>)
        return **std::launder(reinterpret_cast<Y*>(&storage));
    else
        return std::move(*std::launder(reinterpret_cast<T*>(&storage)));
}

#endif