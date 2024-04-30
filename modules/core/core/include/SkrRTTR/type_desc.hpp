#pragma once
#include "SkrContainers/span.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
enum ETypeDescType
{
    // empty type, used to check if TypeDescValue is initialized
    // also used to mark end of TypeDescValue array
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
    // ctor
    inline TypeDescValue()
        : _type(SKR_TYPE_DESC_TYPE_VOID)
    {
    }

    // copy & move
    inline TypeDescValue(const TypeDescValue& other)
        : _type(other._type)
    {
        switch (_type)
        {
            case SKR_TYPE_DESC_TYPE_TYPE_ID:
            case SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID:
                _guid = other._guid;
                break;
            case SKR_TYPE_DESC_TYPE_BOOL:
                _bool = other._bool;
                break;
            case SKR_TYPE_DESC_TYPE_INT8:
                _i8 = other._i8;
                break;
            case SKR_TYPE_DESC_TYPE_INT16:
                _i16 = other._i16;
                break;
            case SKR_TYPE_DESC_TYPE_INT32:
                _i32 = other._i32;
                break;
            case SKR_TYPE_DESC_TYPE_INT64:
                _i64 = other._i64;
                break;
            case SKR_TYPE_DESC_TYPE_UINT8:
                _u8 = other._u8;
                break;
            case SKR_TYPE_DESC_TYPE_UINT16:
                _u16 = other._u16;
                break;
            case SKR_TYPE_DESC_TYPE_UINT32:
                _u32 = other._u32;
                break;
            case SKR_TYPE_DESC_TYPE_UINT64:
                _u64 = other._u64;
                break;
            case SKR_TYPE_DESC_TYPE_FLOAT:
                _float = other._float;
                break;
            case SKR_TYPE_DESC_TYPE_DOUBLE:
                _double = other._double;
                break;
        }
    }
    inline TypeDescValue(TypeDescValue&& other)
        : _type(other._type)
    {
        switch (_type)
        {
            case SKR_TYPE_DESC_TYPE_TYPE_ID:
            case SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID:
                _guid = other._guid;
                break;
            case SKR_TYPE_DESC_TYPE_BOOL:
                _bool = other._bool;
                break;
            case SKR_TYPE_DESC_TYPE_INT8:
                _i8 = other._i8;
                break;
            case SKR_TYPE_DESC_TYPE_INT16:
                _i16 = other._i16;
                break;
            case SKR_TYPE_DESC_TYPE_INT32:
                _i32 = other._i32;
                break;
            case SKR_TYPE_DESC_TYPE_INT64:
                _i64 = other._i64;
                break;
            case SKR_TYPE_DESC_TYPE_UINT8:
                _u8 = other._u8;
                break;
            case SKR_TYPE_DESC_TYPE_UINT16:
                _u16 = other._u16;
                break;
            case SKR_TYPE_DESC_TYPE_UINT32:
                _u32 = other._u32;
                break;
            case SKR_TYPE_DESC_TYPE_UINT64:
                _u64 = other._u64;
                break;
            case SKR_TYPE_DESC_TYPE_FLOAT:
                _float = other._float;
                break;
            case SKR_TYPE_DESC_TYPE_DOUBLE:
                _double = other._double;
                break;
        }
    }
    inline TypeDescValue& operator=(const TypeDescValue& other)
    {
        _type = other._type;
        switch (_type)
        {
            case SKR_TYPE_DESC_TYPE_TYPE_ID:
            case SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID:
                _guid = other._guid;
                break;
            case SKR_TYPE_DESC_TYPE_BOOL:
                _bool = other._bool;
                break;
            case SKR_TYPE_DESC_TYPE_INT8:
                _i8 = other._i8;
                break;
            case SKR_TYPE_DESC_TYPE_INT16:
                _i16 = other._i16;
                break;
            case SKR_TYPE_DESC_TYPE_INT32:
                _i32 = other._i32;
                break;
            case SKR_TYPE_DESC_TYPE_INT64:
                _i64 = other._i64;
                break;
            case SKR_TYPE_DESC_TYPE_UINT8:
                _u8 = other._u8;
                break;
            case SKR_TYPE_DESC_TYPE_UINT16:
                _u16 = other._u16;
                break;
            case SKR_TYPE_DESC_TYPE_UINT32:
                _u32 = other._u32;
                break;
            case SKR_TYPE_DESC_TYPE_UINT64:
                _u64 = other._u64;
                break;
            case SKR_TYPE_DESC_TYPE_FLOAT:
                _float = other._float;
                break;
            case SKR_TYPE_DESC_TYPE_DOUBLE:
                _double = other._double;
                break;
        }
        return *this;
    }
    inline TypeDescValue& operator=(TypeDescValue&& other)
    {
        _type = other._type;
        switch (_type)
        {
            case SKR_TYPE_DESC_TYPE_TYPE_ID:
            case SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID:
                _guid = other._guid;
                break;
            case SKR_TYPE_DESC_TYPE_BOOL:
                _bool = other._bool;
                break;
            case SKR_TYPE_DESC_TYPE_INT8:
                _i8 = other._i8;
                break;
            case SKR_TYPE_DESC_TYPE_INT16:
                _i16 = other._i16;
                break;
            case SKR_TYPE_DESC_TYPE_INT32:
                _i32 = other._i32;
                break;
            case SKR_TYPE_DESC_TYPE_INT64:
                _i64 = other._i64;
                break;
            case SKR_TYPE_DESC_TYPE_UINT8:
                _u8 = other._u8;
                break;
            case SKR_TYPE_DESC_TYPE_UINT16:
                _u16 = other._u16;
                break;
            case SKR_TYPE_DESC_TYPE_UINT32:
                _u32 = other._u32;
                break;
            case SKR_TYPE_DESC_TYPE_UINT64:
                _u64 = other._u64;
                break;
            case SKR_TYPE_DESC_TYPE_FLOAT:
                _float = other._float;
                break;
            case SKR_TYPE_DESC_TYPE_DOUBLE:
                _double = other._double;
                break;
        }
        return *this;
    }

    // assign with normalize config
    inline void assign(const TypeDescValue& rhs, bool ref_as_pointer, bool rvalue_ref_as_pointer)
    {
        _type = rhs._type;
        _type = (ref_as_pointer && _type == SKR_TYPE_DESC_TYPE_REF) ? SKR_TYPE_DESC_TYPE_POINTER : _type;
        _type = (rvalue_ref_as_pointer && _type == SKR_TYPE_DESC_TYPE_RVALUE_REF) ? SKR_TYPE_DESC_TYPE_POINTER : _type;

        switch (_type)
        {
            case SKR_TYPE_DESC_TYPE_TYPE_ID:
            case SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID:
                _guid = rhs._guid;
                break;
            case SKR_TYPE_DESC_TYPE_BOOL:
                _bool = rhs._bool;
                break;
            case SKR_TYPE_DESC_TYPE_INT8:
                _i8 = rhs._i8;
                break;
            case SKR_TYPE_DESC_TYPE_INT16:
                _i16 = rhs._i16;
                break;
            case SKR_TYPE_DESC_TYPE_INT32:
                _i32 = rhs._i32;
                break;
            case SKR_TYPE_DESC_TYPE_INT64:
                _i64 = rhs._i64;
                break;
            case SKR_TYPE_DESC_TYPE_UINT8:
                _u8 = rhs._u8;
                break;
            case SKR_TYPE_DESC_TYPE_UINT16:
                _u16 = rhs._u16;
                break;
            case SKR_TYPE_DESC_TYPE_UINT32:
                _u32 = rhs._u32;
                break;
            case SKR_TYPE_DESC_TYPE_UINT64:
                _u64 = rhs._u64;
                break;
            case SKR_TYPE_DESC_TYPE_FLOAT:
                _float = rhs._float;
                break;
            case SKR_TYPE_DESC_TYPE_DOUBLE:
                _double = rhs._double;
                break;
        }
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

    // compare
    inline bool equal(const TypeDescValue& rhs, bool ref_as_pointer, bool rvalue_ref_as_pointer)
    {
        ETypeDescType self_type = _type;
        ETypeDescType rhs_type  = rhs.type();

        if (ref_as_pointer)
        {
            self_type = self_type == SKR_TYPE_DESC_TYPE_REF ? SKR_TYPE_DESC_TYPE_POINTER : self_type;
            rhs_type  = rhs_type == SKR_TYPE_DESC_TYPE_REF ? SKR_TYPE_DESC_TYPE_POINTER : rhs_type;
        }

        if (rvalue_ref_as_pointer)
        {
            self_type = self_type == SKR_TYPE_DESC_TYPE_RVALUE_REF ? SKR_TYPE_DESC_TYPE_POINTER : self_type;
            rhs_type  = rhs_type == SKR_TYPE_DESC_TYPE_RVALUE_REF ? SKR_TYPE_DESC_TYPE_POINTER : rhs_type;
        }

        if (self_type != rhs_type)
        {
            return false;
        }
        else
        {
            switch (self_type)
            {
                case SKR_TYPE_DESC_TYPE_VOID:
                    return false;
                case SKR_TYPE_DESC_TYPE_TYPE_ID:
                case SKR_TYPE_DESC_TYPE_GENERIC_TYPE_ID:
                    return _guid == rhs._guid;
                case SKR_TYPE_DESC_TYPE_CONST:
                case SKR_TYPE_DESC_TYPE_POINTER:
                case SKR_TYPE_DESC_TYPE_REF:
                case SKR_TYPE_DESC_TYPE_RVALUE_REF:
                    return true;
                case SKR_TYPE_DESC_TYPE_ARRAY_DIM:
                    return _u32 == rhs._u32;
                case SKR_TYPE_DESC_TYPE_BOOL:
                    return _bool == rhs._bool;
                case SKR_TYPE_DESC_TYPE_INT8:
                    return _i8 == rhs._i8;
                case SKR_TYPE_DESC_TYPE_INT16:
                    return _i16 == rhs._i16;
                case SKR_TYPE_DESC_TYPE_INT32:
                    return _i32 == rhs._i32;
                case SKR_TYPE_DESC_TYPE_INT64:
                    return _i64 == rhs._i64;
                case SKR_TYPE_DESC_TYPE_UINT8:
                    return _u8 == rhs._u8;
                case SKR_TYPE_DESC_TYPE_UINT16:
                    return _u16 == rhs._u16;
                case SKR_TYPE_DESC_TYPE_UINT32:
                    return _u32 == rhs._u32;
                case SKR_TYPE_DESC_TYPE_UINT64:
                    return _u64 == rhs._u64;
                case SKR_TYPE_DESC_TYPE_FLOAT:
                    return _float == rhs._float;
                case SKR_TYPE_DESC_TYPE_DOUBLE:
                    return _double == rhs._double;
            }
        }
    }

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
struct TypeDescTraits {
    inline static constexpr size_t type_desc_size = 1;
    static void                    write_type_desc(TypeDescValue* desc)
    {
        desc[0].set_type_id(skr::rttr::RTTRTraits<T>::get_guid());
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
} // namespace skr::rttr

// TypeDesc help functions
namespace skr::rttr
{
enum class ETypeDescNormalizeFlag : uint8_t
{
    IgnoreConst        = 1 << 0,
    RefAsPointer       = 1 << 1,
    RValueRefAsPointer = 1 << 2,
    Full               = IgnoreConst | RefAsPointer | RValueRefAsPointer,
};

inline bool type_desc_equal(span<TypeDescValue> lhs, span<TypeDescValue> rhs, ETypeDescNormalizeFlag compare_flag)
{
    size_t lhs_idx = 0, rhs_idx = 0;

    while (true)
    {
        // skip const
        if (flag_all(compare_flag, ETypeDescNormalizeFlag::IgnoreConst))
        {
            while (lhs[lhs_idx].type() == SKR_TYPE_DESC_TYPE_CONST && lhs_idx < lhs.size())
                ++lhs_idx;
            while (rhs[rhs_idx].type() == SKR_TYPE_DESC_TYPE_CONST && rhs_idx < rhs.size())
                ++rhs_idx;
            if (lhs_idx >= lhs.size() || rhs_idx >= rhs.size())
                break;
        }

        // compare
        if (lhs[lhs_idx].equal(
            rhs[lhs_idx],
            flag_all(compare_flag, ETypeDescNormalizeFlag::RefAsPointer),
            flag_all(compare_flag, ETypeDescNormalizeFlag::RValueRefAsPointer)))
        {
            ++lhs_idx;
            ++rhs_idx;
            if (lhs_idx >= lhs.size() || rhs_idx >= rhs.size())
                break;
        }
        else
        {
            return false;
        }
    }

    return true;
}

inline size_t type_desc_normalize(TypeDescValue* desc, size_t size, ETypeDescNormalizeFlag normalize_flag)
{
    size_t read_idx = 0, write_idx = 0;
    while (read_idx < size)
    {
        // skip const
        if (flag_all(normalize_flag, ETypeDescNormalizeFlag::IgnoreConst))
        {
            while (desc[read_idx].type() == SKR_TYPE_DESC_TYPE_CONST && read_idx < size)
                ++read_idx;
            if (read_idx >= size)
                break;
        }

        // write
        if (read_idx != write_idx)
        {
            desc[write_idx].assign(
            desc[read_idx],
            flag_all(normalize_flag, ETypeDescNormalizeFlag::RefAsPointer),
            flag_all(normalize_flag, ETypeDescNormalizeFlag::RValueRefAsPointer));
        }

        ++read_idx;
        ++write_idx;
    }

    size_t new_size = write_idx;
    while (write_idx < size)
    {
        desc[write_idx].set_void();
        ++write_idx;
    }
    return new_size;
}
} // namespace skr::rttr

// TypeDesc
namespace skr::rttr
{
struct TypeDesc {
    using TypeDescWriter = void (*)(TypeDescValue* desc);

    // ctor
    inline TypeDesc() = default;
    inline TypeDesc(span<TypeDescValue> desc)
        : _desc(desc.begin(), desc.size())
    {
    }
    inline TypeDesc(size_t size, TypeDescWriter writer)
    {
        _desc.resize_unsafe(size);
        writer(_desc.data());
    }

    // copy & move
    inline TypeDesc(const TypeDesc& other)            = default;
    inline TypeDesc(TypeDesc&& other)                 = default;
    inline TypeDesc& operator=(const TypeDesc& other) = default;
    inline TypeDesc& operator=(TypeDesc&& other)      = default;

    // to span
    operator span<TypeDescValue>() const { return { const_cast<TypeDescValue*>(_desc.data()), _desc.size() }; }

    // compare
    inline bool equal(const TypeDesc& other, ETypeDescNormalizeFlag compare_flag) const
    {
        return type_desc_equal(*this, other, compare_flag);
    }

    // normalize
    inline void normalize(ETypeDescNormalizeFlag normalize_flag)
    {
        _desc.resize_unsafe(type_desc_normalize(_desc.data(), _desc.size(), normalize_flag));
    }

    // TODO. invoke
    // TODO. invoke_extern
    // TODO. set_field
    // TODO. get_field

private:
    Vector<TypeDescValue> _desc = {};
};

template <typename T>
struct TypedTypeDesc {
    // ctor
    inline TypedTypeDesc() { TypeDescTraits<T>::write_type_desc(_desc); }

    // copy & move
    inline TypedTypeDesc(const TypedTypeDesc& other)            = default;
    inline TypedTypeDesc(TypedTypeDesc&& other)                 = default;
    inline TypedTypeDesc& operator=(const TypedTypeDesc& other) = default;
    inline TypedTypeDesc& operator=(TypedTypeDesc&& other)      = default;

    // compare
    inline bool equal(const TypedTypeDesc& other, ETypeDescNormalizeFlag compare_flag) const
    {
        return type_desc_equal(*this, other, compare_flag);
    }

    // normalize
    inline void normalize(ETypeDescNormalizeFlag normalize_flag)
    {
        _desc.resize_unsafe(type_desc_normalize(_desc.data(), _desc.size(), normalize_flag));
    }

    // TODO. invoke
    // TODO. invoke_extern
    // TODO. set_field
    // TODO. get_field

private:
    TypeDescValue _desc[TypeDescTraits<T>::type_desc_size];
};

// make TypeDesc
template <typename T>
inline TypeDesc type_desc_of()
{
    return TypeDesc(TypeDescTraits<T>::type_desc_size, &TypeDescTraits<T>::write_type_desc);
}
template <typename T>
inline TypedTypeDesc<T> typed_type_desc_of()
{
    return TypedTypeDesc<T>();
}
} // namespace skr::rttr

// TODO. TypeDescView，代替 Span 进行轻量的 TypeDesc 传输