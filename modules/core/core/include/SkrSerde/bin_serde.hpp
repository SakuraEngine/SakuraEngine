#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/template/concepts.hpp"
#include "SkrCore/log.h"
#include "SkrBase/types.h"
#include "SkrBase/math.h"

namespace skr::concepts
{
template <typename T>
concept BinWriterSupportBitPacking = requires(T t, void* data, size_t size) {
    t.write_bits(data, size);
};
template <typename T>
concept BinReaderSupportBitPacking = requires(T t, void* data, size_t size) {
    t.read_bits(data, size);
};
} // namespace skr::concepts

// writer & reader
// TODO. proxy 样式
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
        if constexpr (skr::concepts::BinWriterSupportBitPacking<T>)
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
        if constexpr (skr::concepts::BinReaderSupportBitPacking<T>)
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

// concept
namespace concepts
{
template <typename T>
concept HasBinRead = requires(SBinaryReader* r, T& t) { BinSerde<T>::read(r, t); };
template <typename T>
concept HasBinWrite = requires(SBinaryWriter* w, const T& t) { BinSerde<T>::write(w, t); };
} // namespace concepts

// helper
template <concepts::HasBinRead T>
inline bool bin_read(SBinaryReader* r, T& v) { return BinSerde<T>::read(r, v); }
template <concepts::HasBinWrite T>
inline bool bin_write(SBinaryWriter* w, const T& v) { return BinSerde<T>::write(w, v); }

// POD writer
template <typename T>
struct BinSerdePOD {
    inline static bool read(SBinaryReader* r, T& v)
    {
        return r->read(&v, sizeof(v));
    }
    inline static bool write(SBinaryWriter* w, const T& v)
    {
        return w->write(&v, sizeof(v));
    }
};
} // namespace skr

// primitive types
//  bool
//  int/uint 8/16/32/64
//  float/double
namespace skr
{
template <>
struct BinSerde<bool> : BinSerdePOD<bool> {
};

template <>
struct BinSerde<int8_t> : BinSerdePOD<int8_t> {
};
template <>
struct BinSerde<int16_t> : BinSerdePOD<int16_t> {
};
template <>
struct BinSerde<int32_t> : BinSerdePOD<int32_t> {
};
template <>
struct BinSerde<int64_t> : BinSerdePOD<int64_t> {
};

template <>
struct BinSerde<uint8_t> : BinSerdePOD<uint8_t> {
};
template <>
struct BinSerde<uint16_t> : BinSerdePOD<uint16_t> {
};
template <>
struct BinSerde<uint32_t> : BinSerdePOD<uint32_t> {
};
template <>
struct BinSerde<uint64_t> : BinSerdePOD<uint64_t> {
};

template <>
struct BinSerde<float> : BinSerdePOD<float> {
};
template <>
struct BinSerde<double> : BinSerdePOD<double> {
};

#if SKR_PLAT_MACOSX
template <>
struct BinSerde<size_t> : BinSerdePOD<size_t> {
};
#endif
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
        requires(concepts::HasBinRead<T>)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!BinSerde<T>::read(r, v[i]))
            {
                SKR_LOG_ERROR(u8"[SERDE/BIN] read array failed, index: %d", i);
                return false;
            }
        }
        return true;
    }
    inline static bool write(SBinaryWriter* w, const T (&v)[N])
        requires(concepts::HasBinWrite<T>)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!BinSerde<T>::write(w, v[i]))
            {
                SKR_LOG_ERROR(u8"[SERDE/BIN] write array failed, index: %d", i);
                return false;
            }
        }
        return true;
    }
};
} // namespace skr

// guid / md5
namespace skr
{
template <>
struct BinSerde<GUID> : BinSerdePOD<GUID> {
};
template <>
struct BinSerde<MD5> : BinSerdePOD<MD5> {
};
} // namespace skr

// math misc
namespace skr
{
// rotator
template <>
struct BinSerde<RotatorF> : BinSerdePOD<RotatorF> {
};
template <>
struct BinSerde<RotatorD> : BinSerdePOD<RotatorD> {
};

// quat
template <>
struct BinSerde<QuatF> : BinSerdePOD<QuatF> {
};
template <>
struct BinSerde<QuatD> : BinSerdePOD<QuatD> {
};

// transform
template <>
struct BinSerde<TransformF> : BinSerdePOD<TransformF> {
};
template <>
struct BinSerde<TransformD> : BinSerdePOD<TransformD> {
};
} // namespace skr

// vector & matrix
namespace skr
{
template <MathVector T>
struct BinSerde<T> : BinSerdePOD<T> {
};

template <MathMatrix T>
struct BinSerde<T> : BinSerdePOD<T> {
};
} // namespace skr