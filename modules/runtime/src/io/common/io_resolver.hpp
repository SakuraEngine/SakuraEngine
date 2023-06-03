#pragma once
#include "io/io.h"
#include "containers/vector.hpp"

namespace skr {
namespace io {

struct IOBatchResolverBase : public IIOBatchResolver
{
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
};

struct IOBatchResolverChain : public IIOBatchResolverChain
{
    IOBatchResolverChain(IOBatchResolverId resolver) SKR_NOEXCEPT
    {
        if (resolver)
        {
            chain.emplace_back(resolver);
        }
    }

    SObjectPtr<IIOBatchResolverChain> then(IOBatchResolverId resolver) SKR_NOEXCEPT
    {
        chain.emplace_back(resolver);
        return this;
    }

    skr::vector<IOBatchResolverId> chain;
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
};

} // namespace io
} // namespace skr