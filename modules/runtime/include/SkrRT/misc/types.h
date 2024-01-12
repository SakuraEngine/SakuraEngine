#pragma once
#include "SkrBase/types.h"
#include "SkrRT/config.h"

SKR_EXTERN_C SKR_RUNTIME_API void skr_make_guid(skr_guid_t* out_guid);

#ifdef __cplusplus
inline skr_guid_t skr_guid_t::Create()
{
    skr_guid_t guid;
    skr_make_guid(&guid);
    return guid;
}
#endif

extern const skr_guid_t $guid;

#ifdef __cplusplus
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
} // namespace skr::binary

namespace skr
{
template <typename T, bool EmbedRC>
struct SPtrHelper;
template <typename T>
using SPtr = SPtrHelper<T, true>;
template <typename T>
using SObjectPtr = SPtrHelper<T, false>;

using SInterfaceDeleter = void (*)(struct SInterface*);
struct SKR_RUNTIME_API         SInterface {
    virtual ~SInterface() SKR_NOEXCEPT       = default;
    virtual uint32_t          add_refcount() = 0;
    virtual uint32_t          release()      = 0;
    virtual skr_guid_t        get_type() { return {}; }
    virtual SInterfaceDeleter custom_deleter() const { return nullptr; }
};
template <class T>
constexpr bool is_object_v = std::is_base_of_v<skr::SInterface, T>;

struct SKR_RUNTIME_API SObjectHeader : public SInterface {
    uint32_t                  rc      = 1;
    skr_guid_t                type    = {};
    SInterfaceDeleter         deleter = nullptr;
    virtual uint32_t          add_refcount() override;
    virtual uint32_t          release() override;
    virtual skr_guid_t        get_type() override { return type; }
    virtual SInterfaceDeleter custom_deleter() const override { return deleter; }
};

struct SKR_RUNTIME_API IBlob : public SInterface {
    static SObjectPtr<IBlob> Create(const uint8_t* data, uint64_t size, bool move, const char* name = nullptr) SKR_NOEXCEPT;
    static SObjectPtr<IBlob> CreateAligned(const uint8_t* data, uint64_t size, uint64_t alignment, bool move, const char* name = nullptr) SKR_NOEXCEPT;

    virtual ~IBlob() SKR_NOEXCEPT                  = default;
    virtual uint8_t* get_data() const SKR_NOEXCEPT = 0;
    virtual uint64_t get_size() const SKR_NOEXCEPT = 0;
};
using BlobId = SObjectPtr<IBlob>;

} // namespace skr
    #define sobject_cast static_cast
#endif