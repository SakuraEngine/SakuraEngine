#pragma once
#include "SkrRT/platform/configure.h"
#include <cstdint>
#include "SkrRT/rttr/guid.hpp"

namespace skr::rttr
{
enum ETypeDescType
{
    SKR_TYPE_DESC_TYPE_VOID,
    SKR_TYPE_DESC_TYPE_GUID,
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

struct TypeDesc {
    SKR_INLINE TypeDesc()
        : _type(SKR_TYPE_DESC_TYPE_VOID)
    {
    }
    SKR_INLINE TypeDesc(const GUID& guid)
        : _type(SKR_TYPE_DESC_TYPE_GUID)
        , _guid(guid)
    {
    }
    SKR_INLINE TypeDesc(bool b)
        : _type(SKR_TYPE_DESC_TYPE_BOOL)
        , _bool(b)
    {
    }
    SKR_INLINE TypeDesc(int8_t i8)
        : _type(SKR_TYPE_DESC_TYPE_INT8)
        , _i8(i8)
    {
    }
    SKR_INLINE TypeDesc(int16_t i16)
        : _type(SKR_TYPE_DESC_TYPE_INT16)
        , _i16(i16)
    {
    }
    SKR_INLINE TypeDesc(int32_t i32)
        : _type(SKR_TYPE_DESC_TYPE_INT32)
        , _i32(i32)
    {
    }
    SKR_INLINE TypeDesc(int64_t i64)
        : _type(SKR_TYPE_DESC_TYPE_INT64)
        , _i64(i64)
    {
    }
    SKR_INLINE TypeDesc(uint8_t u8)
        : _type(SKR_TYPE_DESC_TYPE_UINT8)
        , _u8(u8)
    {
    }
    SKR_INLINE TypeDesc(uint16_t u16)
        : _type(SKR_TYPE_DESC_TYPE_UINT16)
        , _u16(u16)
    {
    }
    SKR_INLINE TypeDesc(uint32_t u32)
        : _type(SKR_TYPE_DESC_TYPE_UINT32)
        , _u32(u32)
    {
    }
    SKR_INLINE TypeDesc(uint64_t u64)
        : _type(SKR_TYPE_DESC_TYPE_UINT64)
        , _u64(u64)
    {
    }

    SKR_INLINE ETypeDescType type() const { return _type; }
    SKR_INLINE const GUID&   value_guid() const { return _guid; }
    SKR_INLINE bool          value_bool() const { return _bool; }
    SKR_INLINE int8_t        value_int8() const { return _i8; }
    SKR_INLINE int16_t       value_int16() const { return _i16; }
    SKR_INLINE int32_t       value_int32() const { return _i32; }
    SKR_INLINE int64_t       value_int64() const { return _i64; }
    SKR_INLINE uint8_t       value_uint8() const { return _u8; }
    SKR_INLINE uint16_t      value_uint16() const { return _u16; }
    SKR_INLINE uint32_t      value_uint32() const { return _u32; }
    SKR_INLINE uint64_t      value_uint64() const { return _u64; }

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
    };
};
} // namespace skr::rttr
