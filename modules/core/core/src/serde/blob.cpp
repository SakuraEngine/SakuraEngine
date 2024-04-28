#include "SkrSerde/blob.h"
#include "SkrMemory/memory.h"
#include "SkrOS/atomic.h"
#include "SkrContainers/sptr.hpp"

namespace skr
{
const char* kSimpleBlobName = "SimpleBlob";
struct SimpleBlob : public IBlob {
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

    uint8_t* get_data() const SKR_NOEXCEPT { return bytes; }
    uint64_t get_size() const SKR_NOEXCEPT { return size; }

public:
    uint32_t add_refcount()
    {
        return 1 + skr_atomicu32_add_relaxed(&rc, 1);
    }
    uint32_t release()
    {
        skr_atomicu32_add_relaxed(&rc, -1);
        return skr_atomicu32_load_acquire(&rc);
    }

private:
    uint64_t   size      = 0;
    uint64_t   alignment = 0;
    uint8_t*   bytes     = nullptr;
    SAtomicU32 rc        = 0;
};
} // namespace skr

skr::BlobId skr::IBlob::Create(const uint8_t* data, uint64_t size, bool move, const char* name) SKR_NOEXCEPT
{
    return skr::SObjectPtr<skr::SimpleBlob>::Create(data, size, alignof(uint8_t), move, name);
}

skr::BlobId skr::IBlob::CreateAligned(const uint8_t* data, uint64_t size, uint64_t alignment, bool move, const char* name) SKR_NOEXCEPT
{
    return skr::SObjectPtr<skr::SimpleBlob>::Create(data, size, alignment, move, name);
}
