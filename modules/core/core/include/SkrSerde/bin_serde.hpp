#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/template/concepts.hpp"
#include "SkrCore/log.h"

// writer & reader
// TODO. use proxy instead
struct SBinaryWriter {
    using WriteFunc     = bool(void* user_data, const void* data, size_t size);
    using WriteBitsFunc = bool(void* user_data, const void* data, size_t size);

    template <class T>
    inline SBinaryWriter(T& user)
    {
        _user_data = &user;
        _vwrite    = +[](void* user, const void* data, size_t size) -> bool {
            return static_cast<T*>(user)->write(data, size);
        };
        auto SupportBitPacking = SKR_VALIDATOR((auto t), t.write_bits((void*)0, (size_t)0));
        if constexpr (SupportBitPacking(SKR_TYPELIST(T)))
        {
            _vwrite_bits = +[](void* user, const void* data, size_t size) -> bool {
                return static_cast<T*>(user)->write_bits(data, size);
            };
        }
    }
    inline bool write(const void* data, size_t size)
    {
        return _vwrite(_user_data, data, size);
    }
    inline bool write_bits(const void* data, size_t size)
    {
        return _vwrite_bits(_user_data, data, size);
    }

private:
    WriteFunc*     _vwrite      = nullptr;
    WriteBitsFunc* _vwrite_bits = nullptr;
    void*          _user_data   = nullptr;
};
struct SBinaryReader {
    using ReadFunc     = bool(void* user_data, void* data, size_t size);
    using ReadBitsFunc = bool(void* user_data, void* data, size_t size);

    template <class T>
    inline SBinaryReader(T& user)
    {
        _user_data = &user;
        _vread     = +[](void* user, void* data, size_t size) -> bool {
            return static_cast<T*>(user)->read(data, size);
        };
        auto SupportBitPacking = SKR_VALIDATOR((auto t), t.read_bits((void*)0, (size_t)0));
        if constexpr (SupportBitPacking(SKR_TYPELIST(T)))
        {
            _vread_bits = +[](void* user, void* data, size_t size) -> bool {
                return static_cast<T*>(user)->read_bits(data, size);
            };
        }
    }
    inline bool read(void* data, size_t size)
    {
        return _vread(_user_data, data, size);
    }
    inline bool read_bits(void* data, size_t size)
    {
        return _vread_bits(_user_data, data, size);
    }

private:
    ReadFunc*     _vread      = nullptr;
    ReadBitsFunc* _vread_bits = nullptr;
    void*         _user_data  = nullptr;
};

// traits
namespace skr
{
template <typename T>
struct BinSerde;

// helper
template <typename T>
bool bin_read(SBinaryReader* r, T&& v) { return BinSerde<T>::read(r, v); }
template <typename T>
bool bin_write(SBinaryWriter* w, const T& v) { return BinSerde<T>::write(w, v); }

// concept
template <typename T>
concept HasBinRead = requires(SBinaryReader* r, T& t) { BinSerde<T>::read(r, t); };
template <typename T>
concept HasBinWrite = requires(SBinaryWriter* w, const T& t) { BinSerde<T>::write(w, t); };
} // namespace skr

// primitive types
//  bool
//  int/uint 8/16/32/64
//  float/double
namespace skr
{
#define BIN_SERDE_PRIMITIVE(__TYPE)                           \
    template <>                                               \
    struct SKR_STATIC_API BinSerde<__TYPE> {                  \
        static bool read(SBinaryReader* r, __TYPE& v);        \
        static bool write(SBinaryWriter* w, const __TYPE& v); \
    };

BIN_SERDE_PRIMITIVE(bool)

BIN_SERDE_PRIMITIVE(int8_t)
BIN_SERDE_PRIMITIVE(int16_t)
BIN_SERDE_PRIMITIVE(int32_t)
BIN_SERDE_PRIMITIVE(int64_t)

BIN_SERDE_PRIMITIVE(uint8_t)
BIN_SERDE_PRIMITIVE(uint16_t)
BIN_SERDE_PRIMITIVE(uint32_t)
BIN_SERDE_PRIMITIVE(uint64_t)

BIN_SERDE_PRIMITIVE(float)
BIN_SERDE_PRIMITIVE(double)

#undef BIN_SERDE_PRIMITIVE
} // namespace skr

// enum & array
namespace skr
{
template <concepts::Enum T>
struct BinSerde<T> {
    inline static bool read(SBinaryReader* r, T& v)
    {
        using UT = std::underlying_type_t<T>;
        return BinSerde<UT>::read(r, reinterpret_cast<UT&>(v));
    }
    inline static bool write(SBinaryWriter* w, const T& v)
    {
        using UT = std::underlying_type_t<T>;
        return BinSerde<UT>::write(w, reinterpret_cast<const UT&>(v));
    }
};
template <typename T, size_t N>
struct BinSerde<T[N]> {
    inline static bool read(SBinaryReader* r, T (&v)[N])
    requires(HasBinRead<T>)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!BinSerde<T>::read(r, v[i]))
            {
                SKR_LOG_ERROR("[SERDE/BIN] read array failed, index: %d", i);
                return false;
            }
        }
        return true;
    }
    inline static bool write(SBinaryWriter* w, const T (&v)[N])
    requires(HasBinWrite<T>)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!BinSerde<T>::write(w, v[i]))
            {
                SKR_LOG_ERROR("[SERDE/BIN] write array failed, index: %d", i);
                return false;
            }
        }
        return true;
    }
};
} // namespace skr

// skr types, 主要指 skr base 中的类型，遵循模块依赖的规则
//  float2/float3/float4
//  float4x4
//  rotator
//  quaternion
//  guid
//  md5