#pragma once
#include "pool.hpp"

#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

struct IOBatchBase : public IIOBatch
{
    void reserve(uint64_t n) SKR_NOEXCEPT
    {
        requests.reserve(n);
    }

    void add_request(IORequest request) SKR_NOEXCEPT
    {
        requests.emplace_back(request);
    }

    eastl::fixed_vector<IORequest, 1> requests;

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
            auto* p = static_cast<IOBatchBase*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    friend struct SmartPool<IOBatchBase, IIOBatch>;
protected:
    IOBatchBase(ISmartPool<IIOBatch>* pool, const uint64_t sequence) : sequence(sequence), pool(pool) {}
    
    const uint64_t sequence;
    ISmartPool<IIOBatch>* pool = nullptr;
};

} // namespace io
} // namespace skr