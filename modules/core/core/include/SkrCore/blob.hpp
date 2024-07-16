#pragma once
#include "SkrBase/types.h"
#include "SkrContainersDef/sptr.hpp"

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