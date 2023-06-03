#include "platform/memory.h"
#include "platform/atomic.h"
#include "misc/types.h"
#include "containers/sptr.hpp"

namespace skr
{
const char* kSimpleBlobName = "SimpleBlob";
struct SimpleBlob : public IBlob
{
public:
    SimpleBlob(const uint8_t* data, uint64_t size, bool move) SKR_NOEXCEPT
        : size(size)
    {
        if (move)
        {
            bytes = (uint8_t*)data;
        }
        else if (size)
        {
            bytes = (uint8_t*)sakura_mallocN(size, kSimpleBlobName);
            if (data)
                memcpy(bytes, data, size);
        }
    }

    ~SimpleBlob() SKR_NOEXCEPT
    {
        if (bytes)
        {
            sakura_freeN(bytes, kSimpleBlobName);
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
    uint64_t size = 0;
    uint8_t* bytes = nullptr;
    SAtomicU32 rc = 0;
};
}

skr::BlobId skr_create_blob(const uint8_t* data, uint64_t size, bool move) SKR_NOEXCEPT
{
    return skr::SObjectPtr<skr::SimpleBlob>::Create(data, size, move);
}
