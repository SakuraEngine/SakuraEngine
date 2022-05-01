#pragma once
#include "platform/guid.h"
#include "platform/configure.h"
#include <type_traits>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct skr_type_t skr_type_t;
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
    SKR_TYPE_CATEGORY_STR,
    SKR_TYPE_CATEGORY_STRV,
    SKR_TYPE_CATEGORY_ARR,
    SKR_TYPE_CATEGORY_DYNARR,
    SKR_TYPE_CATEGORY_ARRV,
    SKR_TYPE_CATEGORY_OBJ,
    SKR_TYPE_CATEGORY_ENUM,
    SKR_TYPE_CATEGORY_REF,
} skr_type_category_t;

typedef struct skr_any_t {
    skr_type_t* type;
    union
    {
        void* _ptr;
        uint8_t _smallObj[24];
    };
} skr_any_t;

typedef struct skr_any_ref_t {
    skr_type_t* type;
    void* _ptr;
} skr_any_ref_t;

skr_type_t* skr_get_type(const skr_type_id_t* id);
void skr_get_derived_types(const skr_type_t* type, void (*callback)(void* u, skr_type_t* type), void* u);
void skr_get_type_id(const skr_type_t* type, skr_type_id_t* id);
uint32_t skr_get_type_size(const skr_type_t* type);
void skr_get_fields(const skr_type_t* type, void (*callback)(void* u, skr_field_t* field), void* u);
skr_type_t* skr_get_field_type(const skr_field_t* field);
const char* skr_get_field_name(const skr_field_t* field);

/*
generated:
skr_type_t* skr_typeof_xxxx();
void skr_typeid_xxxx(skr_type_id_t* id);
*/

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)

    #include "EASTL/string.h"
    #include "gsl/span"
    #include "EASTL/vector.h"
    #include <memory>

extern "C" struct RUNTIME_API skr_type_t {
    skr_type_category_t type;
    size_t Size() const;
    size_t Align() const;
    eastl::string Name() const;
    bool Same(const skr_type_t* srcType) const;
    bool Convertible(const skr_type_t* srcType, bool format = false) const;
    void Convert(void* dst, const void* src, const skr_type_t* srcType, struct ValueSerializePolicy* policy = nullptr) const;
    eastl::string ToString(const void* dst, struct ValueSerializePolicy* policy = nullptr) const;
    void FromString(void* dst, eastl::string_view str, struct ValueSerializePolicy* policy = nullptr) const;
    size_t Hash(const void* dst, size_t base) const;
    // lifetime operator
    void Destruct(void* dst) const;
    void Construct(void* dst, struct Value* args, size_t nargs) const;
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
    void (*ctor)(void* self, struct Value* param, size_t nparam);
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
    const eastl::string_view name;
    const RecordType* base;
    ObjectMethodTable nativeMethods;
    const gsl::span<struct Field> fields;
    const gsl::span<struct Method> methods;
    bool IsBaseOf(const RecordType& other) const;
    static const RecordType* FromName(eastl::string_view name);
    static void Register(const RecordType* type);
    RecordType(size_t size, size_t align, eastl::string_view name, const RecordType* base, ObjectMethodTable nativeMethods,
    const gsl::span<struct Field> fields, const gsl::span<struct Method> methods)
        : skr_type_t{ SKR_TYPE_CATEGORY_OBJ }
        , size(size)
        , align(align)
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
    void (*FromString)(void* self, eastl::string_view str);
    eastl::string (*ToString)(const void* self);
    struct Enumerator {
        const eastl::string_view name;
        int64_t value;
    };
    const gsl::span<Enumerator> enumerators;
    static const EnumType* FromName(eastl::string_view name);
    static void Register(const EnumType* type);
    EnumType(const skr_type_t* underlyingType, const eastl::string_view name, void (*FromString)(void* self, eastl::string_view str),
    eastl::string (*ToString)(const void* self), const gsl::span<Enumerator> enumerators)
        : skr_type_t{ SKR_TYPE_CATEGORY_ENUM }
        , underlyingType(underlyingType)
        , name(name)
        , FromString(FromString)
        , ToString(ToString)
        , enumerators(enumerators)
    {
    }
};
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
    RUNTIME_API const skr_type_t* get();
};

template <class T>
struct type_of<const T> {
    static const skr_type_t* Get()
    {
        return type_of<T>::Get();
    }
};

template <class T>
struct type_of<volatile T> {
    static const skr_type_t* Get()
    {
        return type_of<T>::Get();
    }
};

template <class T>
struct type_of<T*> {
    static const skr_type_t* Get()
    {
        static ReferenceType type{
            ReferenceType::Observed,
            true,
            type_of<T>::Get()
        };
        return &type;
    }
};

template <class T>
struct type_of<T&> {
    static const skr_type_t* Get()
    {
        static ReferenceType type{
            ReferenceType::Observed,
            false,
            type_of<T>::Get()
        };
        return &type;
    }
};

template <class T>
struct type_of<std::shared_ptr<T>> {
    static const skr_type_t* Get()
    {
        static ReferenceType type{
            ReferenceType::Shared,
            true,
            type_of<T>::Get()
        };
        return &type;
    }
};

template <class V, class T>
struct type_of_vector {
    static const skr_type_t* Get()
    {
        static DynArrayType type{
            type_of<T>::Get(),
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
    static const skr_type_t* Get()
    {
        static ArrayType type{
            type_of<T>::Get(),
            num,
            sizeof(T[num])
        };
        return &type;
    }
};

template <class T, size_t size>
struct type_of<gsl::span<T, size>> {
    static const skr_type_t* Get()
    {
        static_assert(size == -1, "only dynamic extent is supported.");
        static ArrayViewType type{
            type_of<T>::Get()
        };
        return &type;
    }
};
} // namespace type
} // namespace skr
#endif