#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/template/concepts.hpp"
#include "SkrCore/log.h"
#include "SkrBase/types.h"

// writer & reader
// TODO. 搬到 archive 里面去
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

// concept
template <typename T>
concept HasBinRead = requires(SBinaryReader* r, T& t) { BinSerde<T>::read(r, t); };
template <typename T>
concept HasBinWrite = requires(SBinaryWriter* w, const T& t) { BinSerde<T>::write(w, t); };

// helper
template <HasBinRead T>
inline bool bin_read(SBinaryReader* r, T&& v) { return BinSerde<T>::read(r, v); }
template <HasBinWrite T>
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
//  float4x4/rotator/quaternion
//  guid/md5
namespace skr
{
template <>
struct BinSerde<skr_float2_t> : BinSerdePOD<skr_float2_t> {
};
template <>
struct BinSerde<skr_float3_t> : BinSerdePOD<skr_float3_t> {
};
template <>
struct BinSerde<skr_float4_t> : BinSerdePOD<skr_float4_t> {
};

template <>
struct BinSerde<skr_float4x4_t> : BinSerdePOD<skr_float4x4_t> {
};
template <>
struct BinSerde<skr_rotator_t> : BinSerdePOD<skr_rotator_t> {
};
template <>
struct BinSerde<skr_quaternion_t> : BinSerdePOD<skr_quaternion_t> {
};

template <>
struct BinSerde<skr_guid_t> : BinSerdePOD<skr_guid_t> {
};
template <>
struct BinSerde<skr_md5_t> : BinSerdePOD<skr_md5_t> {
};
} // namespace skr

// 解环
//  string/vector
#include "SkrContainers/string.hpp"
#include "SkrContainers/vector.hpp"
namespace skr
{
template <>
struct BinSerde<String> {
    inline static bool read(SBinaryReader* r, String& v)
    {
        uint64_t size;
        if (!BinSerde<uint64_t>::read(r, size))
        {
            SKR_LOG_FATAL(u8"failed to read string buffer size!");
            return false;
        }
        skr::InlineVector<char8_t, 64> temp;
        temp.resize_default(size);
        if (!r->read((void*)temp.data(), temp.size()))
        {
            SKR_LOG_FATAL(u8"failed to read string buffer size!");
            return false;
        }
        v = skr::String(skr::StringView((const char8_t*)temp.data(), (int32_t)temp.size()));
        return true;
    }
    inline static bool write(SBinaryWriter* w, const String& v)
    {
        if (!BinSerde<uint64_t>::write(w, v.size()))
            return false;
        return w->write(v.u8_str(), v.size());
    }
};
template <typename V>
struct BinSerde<Vector<V>> {
    inline static bool read(SBinaryReader* r, Vector<V>& v)
    {

        // read size
        uint64_t size;
        if (!BinSerde<uint64_t>::read(r, size))
            return false;

        // read elements
        Vector<V> temp;
        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            if (!BinSerde<V>::read(r, value))
                return false;
            temp.add(std::move(value));
        }

        // result
        v = std::move(temp);
        return true;
    }
    inline static bool write(SBinaryWriter* w, const Vector<V>& v)
    {
        // write size
        if (!BinSerde<uint64_t>::write(w, v.size()))
            return false;

        // write elements
        for (const auto& value : v)
        {
            if (!BinSerde<V>::write(w, value))
                return false;
        }
        return true;
    }
};
} // namespace skr

// vector writer
namespace skr
{
struct VectorBinWriter {
    Vector<uint8_t>* buffer;

    bool write(const void* data, size_t size)
    {
        buffer->append((uint8_t*)data, size);
        return true;
    }
};

struct VectorBinWriterBitpacked {
    Vector<uint8_t>* buffer;
    uint8_t          bitOffset = 0;
    bool             write(const void* data, size_t size)
    {
        return write_bits(data, size * 8);
    }
    bool write_bits(const void* data, size_t bitSize)
    {
        uint8_t* dataPtr = (uint8_t*)data;
        if (bitOffset == 0)
        {
            buffer->append(dataPtr, (bitSize + 7) / 8);
            bitOffset = bitSize % 8;
            if (bitOffset != 0)
                buffer->last() &= (1 << bitOffset) - 1;
        }
        else
        {
            buffer->last() |= dataPtr[0] << bitOffset;
            int i = 1;
            while (bitSize > 8)
            {
                buffer->add((dataPtr[i - 1] >> (8 - bitOffset)) | (dataPtr[i] << bitOffset));
                ++i;
                bitSize -= 8;
            }
            if (bitSize > 0)
            {
                auto newBitOffset = bitOffset + bitSize;
                if (newBitOffset == 8)
                {
                    bitOffset = 0;
                    return true;
                }
                else if (newBitOffset > 8)
                {
                    buffer->add(dataPtr[i - 1] >> (8 - bitOffset));
                    newBitOffset = newBitOffset - 8;
                }
                buffer->last() &= (1 << newBitOffset) - 1;
                SKR_ASSERT(newBitOffset <= UINT8_MAX);
                bitOffset = (uint8_t)newBitOffset;
            }
        }
        return true;
    }
};
} // namespace skr