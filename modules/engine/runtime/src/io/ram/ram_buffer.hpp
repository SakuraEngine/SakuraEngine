#pragma once
#include "SkrRT/io/ram_io.hpp"
#include "../common/pool.hpp"

namespace skr
{
namespace io
{

struct SKR_RUNTIME_API RAMIOBuffer : public IRAMIOBuffer {
    SKR_RC_IMPL(override)
    virtual ~RAMIOBuffer() SKR_NOEXCEPT;

    uint8_t* get_data() const SKR_NOEXCEPT override { return bytes; }
    uint64_t get_size() const SKR_NOEXCEPT override { return size; }

    void allocate_buffer(uint64_t n) SKR_NOEXCEPT;
    void free_buffer() SKR_NOEXCEPT;

public:
    void skr_rc_delete() override
    {
        SKR_ASSERT(pool && "Invalid pool detected!");
        pool->deallocate(this);
    }

    friend struct AllocateIOBufferResolver;
    friend struct SmartPool<RAMIOBuffer, IRAMIOBuffer>;

protected:
    uint8_t* bytes = nullptr;
    uint64_t size  = 0;
    RAMIOBuffer(ISmartPoolPtr<IRAMIOBuffer> pool)
        : pool(pool)
    {
        SKR_ASSERT(pool && "Invalid pool detected!");
    }
    const ISmartPoolPtr<IRAMIOBuffer> pool = nullptr;
};

} // namespace io
} // namespace skr