#include "SkrBase/atomic/atomic.h"
#include "SkrCore/blob.hpp"
#include "SkrCore/memory/memory.h"

namespace skr
{
const char* kSimpleBlobName = "SimpleBlob";
struct SimpleBlob : public IBlob {
    SKR_RC_IMPL(override);
    SKR_RC_DELETER_IMPL_DEFAULT(override)
public:
    SimpleBlob(const uint8_t* data, uint64_t size, uint64_t alignment, bool move, const char* name) SKR_NOEXCEPT
        : size(size),
          alignment(alignment)
    {
        if (move)
        {
            bytes = (uint8_t*)data;
        }
        else if (size)
        {
            bytes = (uint8_t*)sakura_malloc_alignedN(size, alignment, name ? name : kSimpleBlobName);
            if (data)
                memcpy(bytes, data, size);
        }
    }

    ~SimpleBlob() SKR_NOEXCEPT
    {
        if (bytes)
        {
            sakura_free_alignedN(bytes, alignment, kSimpleBlobName);
        }
        bytes = nullptr;
    }

    uint8_t* get_data() const SKR_NOEXCEPT override { return bytes; }
    uint64_t get_size() const SKR_NOEXCEPT override { return size; }

private:
    uint64_t size      = 0;
    uint64_t alignment = 0;
    uint8_t* bytes     = nullptr;
};
} // namespace skr

skr::BlobId skr::IBlob::Create(const uint8_t* data, uint64_t size, bool move, const char* name) SKR_NOEXCEPT
{
    return skr::RC<skr::SimpleBlob>::New(data, size, alignof(uint8_t), move, name);
}

skr::BlobId skr::IBlob::CreateAligned(const uint8_t* data, uint64_t size, uint64_t alignment, bool move, const char* name) SKR_NOEXCEPT
{
    return skr::RC<skr::SimpleBlob>::New(data, size, alignment, move, name);
}
