#pragma once
#include "../common/pool.hpp"

#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

struct RUNTIME_API RAMIOBuffer : public IRAMIOBuffer
{
public:
    virtual ~RAMIOBuffer() SKR_NOEXCEPT;

    uint8_t* get_data() const SKR_NOEXCEPT { return bytes; }
    uint64_t get_size() const SKR_NOEXCEPT { return size; }

    void allocate_buffer(uint64_t n) SKR_NOEXCEPT;
    void free_buffer() SKR_NOEXCEPT;

    uint8_t* bytes = nullptr;
    uint64_t size = 0;

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
    SAtomicU32 rc = 0;

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<RAMIOBuffer*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    friend struct SmartPool<RAMIOBuffer, IRAMIOBuffer>;
protected:
    RAMIOBuffer(skr::SObjectPtr<ISmartPool<IRAMIOBuffer>> pool) : pool(pool) {}
    skr::SObjectPtr<ISmartPool<IRAMIOBuffer>> pool = nullptr;
};

} // namespace io
} // namespace skr