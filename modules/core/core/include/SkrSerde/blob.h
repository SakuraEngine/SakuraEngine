#pragma once
#include "SkrBase/types.h"
#include "SkrContainers/sptr.hpp"

namespace skr
{
struct SKR_CORE_API IBlob : public SInterface {
    static SObjectPtr<IBlob> Create(const uint8_t* data, uint64_t size, bool move, const char* name = nullptr) SKR_NOEXCEPT;
    static SObjectPtr<IBlob> CreateAligned(const uint8_t* data, uint64_t size, uint64_t alignment, bool move, const char* name = nullptr) SKR_NOEXCEPT;

    virtual ~IBlob() SKR_NOEXCEPT                  = default;
    virtual uint8_t* get_data() const SKR_NOEXCEPT = 0;
    virtual uint64_t get_size() const SKR_NOEXCEPT = 0;
};
using BlobId = SObjectPtr<IBlob>;
} // namespace skr

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