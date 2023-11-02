#pragma once
#include "SkrRT/platform/configure.h"
#ifdef __cplusplus
#include <initializer_list>
#endif

typedef struct skr_guid_t {
#ifdef __cplusplus
    SKR_CONSTEXPR skr_guid_t() = default;
    SKR_CONSTEXPR skr_guid_t(uint32_t b0, uint16_t b1, uint16_t b2, const uint8_t b3s[8])
        : Storage0(b0), Storage1((uint32_t)b1 << 16 | (uint32_t)b2), 
#if SKR_IS_LITTLE_ENDIAN
        Storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24), 
        Storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
#else
        Storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24), 
        Storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
#endif
    {

    }
    SKR_CONSTEXPR skr_guid_t(uint32_t b0, uint16_t b1, uint16_t b2, std::initializer_list<uint8_t> b3s_l)
        : Storage0(b0), Storage1((uint32_t)b1 << 16 | (uint32_t)b2), 
#define b3s (b3s_l).begin()
    #if SKR_IS_LITTLE_ENDIAN
            Storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24), 
            Storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
    #else
            Storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24), 
            Storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
    #endif
#undef b3s
    {

    }
    SKR_CONSTEXPR bool isZero() const { return !(Storage0 && Storage1 && Storage2 && Storage3); }
    SKR_CONSTEXPR uint32_t Data1() const { return Storage0; }
    SKR_CONSTEXPR uint16_t Data2() const { return (uint16_t)(Storage1 >> 16); }
    SKR_CONSTEXPR uint16_t Data3() const { return (uint16_t)(Storage1 & UINT16_MAX); }
    SKR_CONSTEXPR uint16_t Data4(uint8_t idx0_7) const { return ((uint8_t*)&Storage2)[idx0_7]; }
#endif
    uint32_t Storage0 SKR_IF_CPP( = 0);
    uint32_t Storage1 SKR_IF_CPP( = 0);
    uint32_t Storage2 SKR_IF_CPP( = 0);
    uint32_t Storage3 SKR_IF_CPP( = 0);
} skr_guid_t;

SKR_EXTERN_C SKR_RUNTIME_API void skr_make_guid(skr_guid_t* out_guid);

#define SKR_MD5_DIGEST_LENGTH 128 / 8
typedef struct skr_md5_t {
    uint8_t digest[SKR_MD5_DIGEST_LENGTH];
} skr_md5_t;

typedef struct skr_md5_u32x4_view_t {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} skr_md5_u32x4_view_t;

SKR_EXTERN_C SKR_RUNTIME_API bool skr_parse_md5(const char8_t* str32, skr_md5_t* out_md5);
SKR_EXTERN_C SKR_RUNTIME_API void skr_make_md5(const char8_t* str, uint32_t str_size, skr_md5_t* out_md5);

extern const skr_guid_t $guid;

#ifdef __cplusplus
#define SKR_DECLARE_VEC2_BODY(TT, NAME) \
    TT x, y;\
    SKR_FORCEINLINE bool operator == ( const NAME& vec ) const { return x==vec.x && y==vec.y; }\
    SKR_FORCEINLINE bool operator != ( const NAME& vec ) const { return x!=vec.x || y!=vec.y; }\
    SKR_FORCEINLINE NAME() = default;\
    SKR_FORCEINLINE NAME(const NAME&) = default;\
    SKR_FORCEINLINE NAME(TT X, TT Y) : x(X), y(Y) {} \
    SKR_FORCEINLINE NAME(TT t) : x(t), y(t) {}
#else
#define SKR_DECLARE_VEC2_BODY(TT, NAME) TT x, y;
#endif

typedef struct skr_uint64x2_t {
    SKR_DECLARE_VEC2_BODY(uint64_t, skr_uint64x2_t)
} skr_uint64x2_t;

typedef struct skr_uint32x2_t {
    SKR_DECLARE_VEC2_BODY(uint32_t, skr_uint32x2_t)
} skr_uint32x2_t;

typedef struct skr_float2_t {
    SKR_DECLARE_VEC2_BODY(float, skr_float2_t)
} skr_float2_t;

typedef struct skr_double2_t {
    SKR_DECLARE_VEC2_BODY(double, skr_double2_t)
} skr_double2_t;

typedef struct skr_float3_t {
    float x SKR_IF_CPP( = 0.f);
    float y SKR_IF_CPP( = 0.f);
    float z SKR_IF_CPP( = 0.f);
} skr_float3_t;

#ifdef __cplusplus
namespace skr
{
namespace scalar_math
{
    inline skr_float3_t operator* (float s, skr_float3_t v) { return skr_float3_t{ s * v.x, s * v.y, s * v.z }; }
    inline skr_float3_t operator* (skr_float3_t v, float s) { return skr_float3_t{ s * v.x, s * v.y, s * v.z }; }
    inline skr_float3_t operator+ (skr_float3_t a, skr_float3_t b) { return skr_float3_t{ a.x + b.x, a.y + b.y, a.z + b.z }; }
}
}
#endif

typedef struct skr_rotator_t {
    struct
    {
        float pitch SKR_IF_CPP( = 0.f);
        float yaw SKR_IF_CPP( = 0.f);
        float roll SKR_IF_CPP( = 0.f);
    };
} skr_rotator_t;

typedef struct SKR_ALIGNAS(16) skr_float4_t {
    struct 
    {
        float x SKR_IF_CPP( = 0.f);
        float y SKR_IF_CPP( = 0.f);
        float z SKR_IF_CPP( = 0.f);
        float w SKR_IF_CPP( = 0.f);
    };
} skr_float4_t;

typedef struct SKR_ALIGNAS(16) skr_quaternion_t {
    struct 
    {
        float x SKR_IF_CPP( = 0.f);
        float y SKR_IF_CPP( = 0.f);
        float z SKR_IF_CPP( = 0.f);
        float w SKR_IF_CPP( = 0.f);
    };
} skr_quaternion_t;

typedef struct SKR_ALIGNAS(16) skr_transform_t {
    struct 
    {
        skr_rotator_t rotation SKR_IF_CPP( = { 0.f, 0.f, 0.f });
        skr_float3_t translation SKR_IF_CPP( = { 0.f, 0.f, 0.f });
        skr_float3_t scale SKR_IF_CPP( = { 1.f, 1.f, 1.f });
    };
} skr_transform_t;

typedef struct SKR_ALIGNAS(16) skr_float4x4_t {
    float M[4][4];
} skr_float4x4_t;

#ifdef __cplusplus
inline static SKR_CONSTEXPR bool operator==(skr_uint32x2_t l, uint32_t r)
{
    return (l.x == r) && (l.y == r);
} 
inline static SKR_CONSTEXPR bool operator!=(skr_uint32x2_t l, uint32_t r)
{
    return (l.x != r) || (l.y != r);
}
inline static SKR_CONSTEXPR bool operator==(uint32_t l, skr_uint32x2_t r)
{
    return (l == r.x) && (l == r.y);
} 
inline static SKR_CONSTEXPR bool operator!=(uint32_t l, skr_uint32x2_t r)
{
    return (l != r.x) || (l != r.y);
}
inline static SKR_CONSTEXPR bool operator==(skr_float2_t l, skr_float2_t r) 
{
    return (l.x == r.x) && (l.y == r.y);
}
inline static SKR_CONSTEXPR bool operator!=(skr_float2_t l, skr_float2_t r) 
{
    return (l.x != r.x) || (l.y != r.y);
}
inline bool operator==(skr_md5_t a, skr_md5_t b)
{
    const skr_md5_u32x4_view_t* va = (skr_md5_u32x4_view_t*)&a;
    const skr_md5_u32x4_view_t* vb = (skr_md5_u32x4_view_t*)&b;
    int result = true;
    result &= (va->a == vb->a);
    result &= (va->b == vb->b);
    result &= (va->c == vb->c);
    result &= (va->d == vb->d);
    return result;
}

#include "SkrRT/serde/binary/blob_fwd.h"
namespace skr::binary
{
BLOB_POD(skr_float2_t);
BLOB_POD(skr_float3_t);
BLOB_POD(skr_float4_t);
BLOB_POD(skr_quaternion_t);
BLOB_POD(skr_float4x4_t);
BLOB_POD(skr_rotator_t);
BLOB_POD(skr_guid_t);
BLOB_POD(skr_md5_t);
}

namespace skr
{
template <typename T, bool EmbedRC>
struct SPtrHelper;
template <typename T>
using SPtr = SPtrHelper<T, true>;
template <typename T>
using SObjectPtr = SPtrHelper<T, false>;

using SInterfaceDeleter = void(*)(struct SInterface*);
struct SKR_RUNTIME_API SInterface
{
    virtual ~SInterface() SKR_NOEXCEPT = default;
    virtual uint32_t add_refcount() = 0;
    virtual uint32_t release() = 0;
    virtual skr_guid_t get_type() { return {}; }
    virtual SInterfaceDeleter custom_deleter() const { return nullptr; }
};
template <class T>
constexpr bool is_object_v = std::is_base_of_v<skr::SInterface, T>;

struct SKR_RUNTIME_API SObjectHeader : public SInterface
{
    uint32_t rc = 1;
    skr_guid_t type = {};
    SInterfaceDeleter deleter = nullptr;
    virtual uint32_t add_refcount() override;
    virtual uint32_t release() override;
    virtual skr_guid_t get_type() override { return type; }
    virtual SInterfaceDeleter custom_deleter() const override { return deleter; }
};
namespace type {
template <class T> struct type_id;
}
template<class T>
struct SBoxed : public SObjectHeader
{
    T value;
    T* get() { return &value; }
    const T* get() const { return &value; }
    SBoxed(T&& value, SInterfaceDeleter deleter = nullptr) : value(std::move(value)) 
    {
        type = skr::type::type_id<T>::get();
        this->deleter = deleter;
    }
    static SBoxed<T>* from(T* ptr)
    {
        return (SBoxed<T>*)((uint8_t*)(ptr) - offsetof(SBoxed<T>, value));
    }
};
template<class T>
SBoxed<T>* box(T* ptr)
{
    return SBoxed<T>::from(ptr);
}

struct SKR_RUNTIME_API IBlob : public SInterface
{
    static SObjectPtr<IBlob> Create(const uint8_t* data, uint64_t size, bool move, const char* name = nullptr) SKR_NOEXCEPT;
    static SObjectPtr<IBlob> CreateAligned(const uint8_t* data, uint64_t size, uint64_t alignment, bool move, const char* name = nullptr) SKR_NOEXCEPT;
    
    virtual ~IBlob() SKR_NOEXCEPT = default;
    virtual uint8_t* get_data() const SKR_NOEXCEPT = 0;
    virtual uint64_t get_size() const SKR_NOEXCEPT = 0;
};
using BlobId = SObjectPtr<IBlob>;

}
#define sobject_cast static_cast
#endif