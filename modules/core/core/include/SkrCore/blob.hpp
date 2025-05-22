#pragma once
#include "SkrBase/types.h"
#include <SkrCore/memory/rc.hpp>

namespace skr
{
struct SKR_CORE_API IBlob : public IRCAble {
    static RC<IBlob> Create(const uint8_t* data, uint64_t size, bool move, const char* name = nullptr) SKR_NOEXCEPT;
    static RC<IBlob> CreateAligned(const uint8_t* data, uint64_t size, uint64_t alignment, bool move, const char* name = nullptr) SKR_NOEXCEPT;

    virtual ~IBlob() SKR_NOEXCEPT                  = default;
    virtual uint8_t* get_data() const SKR_NOEXCEPT = 0;
    virtual uint64_t get_size() const SKR_NOEXCEPT = 0;
};
using BlobId = RC<IBlob>;
} // namespace skr