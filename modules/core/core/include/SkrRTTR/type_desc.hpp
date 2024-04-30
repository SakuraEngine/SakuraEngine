#pragma once
#include "SkrBase/config.h"
#include <cstdint>
#include "SkrContainers/span.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrBase/meta.h"

namespace skr::rttr
{
enum ETypeDescType
{
    // empty type, used to check if TypeDescValue is initialized
    SKR_TYPE_DESC_TYPE_VOID,

    // type id
    SKR_TYPE_DESC_TYPE_TYPE_ID,
    SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID,

    // modifier
    SKR_TYPE_DESC_TYPE_CONST,
    SKR_TYPE_DESC_TYPE_POINTER,
    SKR_TYPE_DESC_TYPE_REF,
    SKR_TYPE_DESC_TYPE_RVALUE_REF,
    SKR_TYPE_DESC_TYPE_ARRAY_DIM,

    // data
    SKR_TYPE_DESC_TYPE_BOOL,
    SKR_TYPE_DESC_TYPE_INT8,
    SKR_TYPE_DESC_TYPE_INT16,
    SKR_TYPE_DESC_TYPE_INT32,
    SKR_TYPE_DESC_TYPE_INT64,
    SKR_TYPE_DESC_TYPE_UINT8,
    SKR_TYPE_DESC_TYPE_UINT16,
    SKR_TYPE_DESC_TYPE_UINT32,
    SKR_TYPE_DESC_TYPE_UINT64,
    SKR_TYPE_DESC_TYPE_FLOAT,
    SKR_TYPE_DESC_TYPE_DOUBLE,
};

struct TypeDescValue {
    // default
    inline TypeDescValue()
        : _type(SKR_TYPE_DESC_TYPE_VOID)
    {
    }

    // setter
    inline void set_void()
    {
        _type = SKR_TYPE_DESC_TYPE_VOID;
    }
    inline void set_type_id(const GUID& guid)
    {
        _type = SKR_TYPE_DESC_TYPE_TYPE_ID;
        _guid = guid;
    }
    inline void set_generic_type_id(const GUID& guid)
    {
        _type = SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID;
        _guid = guid;
    }
    inline void set_const()
    {
        _type = SKR_TYPE_DESC_TYPE_CONST;
    }
    inline void set_pointer()
    {
        _type = SKR_TYPE_DESC_TYPE_POINTER;
    }
    inline void set_ref()
    {
        _type = SKR_TYPE_DESC_TYPE_REF;
    }
    inline void set_rvalue_ref()
    {
        _type = SKR_TYPE_DESC_TYPE_RVALUE_REF;
    }
    inline void set_array_dim(uint32_t dim)
    {
        _type = SKR_TYPE_DESC_TYPE_ARRAY_DIM;
        _u32  = dim;
    }
    inline void set_bool(bool value)
    {
        _type = SKR_TYPE_DESC_TYPE_BOOL;
        _bool = value;
    }
    inline void set_int8(int8_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_INT8;
        _i8   = value;
    }
    inline void set_int16(int16_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_INT16;
        _i16  = value;
    }
    inline void set_int32(int32_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_INT32;
        _i32  = value;
    }
    inline void set_int64(int64_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_INT64;
        _i64  = value;
    }
    inline void set_uint8(uint8_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_UINT8;
        _u8   = value;
    }
    inline void set_uint16(uint16_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_UINT16;
        _u16  = value;
    }
    inline void set_uint32(uint32_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_UINT32;
        _u32  = value;
    }
    inline void set_uint64(uint64_t value)
    {
        _type = SKR_TYPE_DESC_TYPE_UINT64;
        _u64  = value;
    }
    inline void set_float(float value)
    {
        _type  = SKR_TYPE_DESC_TYPE_FLOAT;
        _float = value;
    }
    inline void set_double(double value)
    {
        _type   = SKR_TYPE_DESC_TYPE_DOUBLE;
        _double = value;
    }

    // getter
    inline ETypeDescType type() const { return _type; }
    inline const GUID&   value_guid() const { return _guid; }
    inline bool          value_bool() const { return _bool; }
    inline int8_t        value_int8() const { return _i8; }
    inline int16_t       value_int16() const { return _i16; }
    inline int32_t       value_int32() const { return _i32; }
    inline int64_t       value_int64() const { return _i64; }
    inline uint8_t       value_uint8() const { return _u8; }
    inline uint16_t      value_uint16() const { return _u16; }
    inline uint32_t      value_uint32() const { return _u32; }
    inline uint64_t      value_uint64() const { return _u64; }
    inline float         value_float() const { return _float; }
    inline double        value_double() const { return _double; }

private:
    ETypeDescType _type;
    union
    {
        GUID     _guid;
        bool     _bool;
        int8_t   _i8;
        int16_t  _i16;
        int32_t  _i32;
        int64_t  _i64;
        uint8_t  _u8;
        uint16_t _u16;
        uint32_t _u32;
        uint64_t _u64;
        float    _float;
        double   _double;
    };
};
} // namespace skr::rttr

// TypeDescTraits
// 提供静态构建 TypeDesc 的方法
namespace skr::rttr
{
template <typename T>
GUID type_id();
template <typename T>
struct TypeDescTraits {
    inline static constexpr size_t type_desc_size = 1;
    static void                    write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_type_id(type_id<T>());
    }
};

// ignore volatile
template <typename T>
struct TypeDescTraits<T volatile> : TypeDescTraits<T> {
};

// [MODIFIER] const
template <typename T>
struct TypeDescTraits<const T> : TypeDescTraits<T> {
    static constexpr size_t type_desc_size = TypeDescTraits<T>::type_desc_size + 1;
    static void             write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_const();
        TypeDescTraits<T>::write_type_desc(desc + 1);
    }
};

// [MODIFIER] pointer
template <typename T>
struct TypeDescTraits<T*> : TypeDescTraits<T> {
    static constexpr size_t type_desc_size = TypeDescTraits<T>::type_desc_size + 1;
    static void             write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_pointer();
        TypeDescTraits<T>::write_type_desc(desc + 1);
    }
};

// [MODIFIER] reference
template <typename T>
struct TypeDescTraits<T&> : TypeDescTraits<T> {
    static constexpr size_t type_desc_size = TypeDescTraits<T>::type_desc_size + 1;
    static void             write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_ref();
        TypeDescTraits<T>::write_type_desc(desc + 1);
    }
};

// [MODIFIER] rvalue reference
template <typename T>
struct TypeDescTraits<T&&> : TypeDescTraits<T> {
    static constexpr size_t type_desc_size = TypeDescTraits<T>::type_desc_size + 1;
    static void             write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_rvalue_ref();
        TypeDescTraits<T>::write_type_desc(desc + 1);
    }
};

// [MODIFIER] array
template <typename T, size_t N>
struct TypeDescTraits<T[N]> : TypeDescTraits<T> {
    static constexpr size_t type_desc_size = TypeDescTraits<T>::type_desc_size + 1;
    static void             write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_array_dim(N);
        TypeDescTraits<T>::write_type_desc(desc + 1);
    }
};

// [MODIFIER] const array
template <typename T, size_t N>
struct TypeDescTraits<const T[N]> : TypeDescTraits<T[N]> {
    static constexpr size_t type_desc_size = TypeDescTraits<T>::type_desc_size + 2;
    static void             write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_const();
        desc[1].set_array_dim(N);
        TypeDescTraits<T>::write_type_desc(desc + 2);
    }
};

// TODO. 分化为 type_desc, type_desc_fixed, type_desc_typed
template <typename T>
span<TypeDescValue> type_desc()
{
    static TypeDescValue desc[TypeDescTraits<T>::type_desc_size];
    TypeDescTraits<T>::write_type_desc(desc);
    return { desc, TypeDescTraits<T>::type_desc_size };
}
} // namespace skr::rttr

// TypeDesc help functions
namespace skr::rttr
{
// TODO. TypeDescCompare, TypeDescNormalize

}

// TypeDesc
namespace skr::rttr
{
struct TypeDesc {
};

template <size_t N>
struct FixedTypeDesc {
};

template <typename T>
struct TypedTypeDesc {
};

} // namespace skr::rttr