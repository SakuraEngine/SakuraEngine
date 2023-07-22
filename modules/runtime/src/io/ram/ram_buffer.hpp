#pragma once
#include "SkrRT/io/ram_io.hpp"
#include "../common/pool.hpp"

namespace skr {
namespace io {

struct RUNTIME_API RAMIOBuffer : public IRAMIOBuffer
{
    IO_RC_OBJECT_BODY
public:
    virtual ~RAMIOBuffer() SKR_NOEXCEPT;

    uint8_t* get_data() const SKR_NOEXCEPT { return bytes; }
    uint64_t get_size() const SKR_NOEXCEPT { return size; }

    void allocate_buffer(uint64_t n) SKR_NOEXCEPT;
    void free_buffer() SKR_NOEXCEPT;

    uint8_t* bytes = nullptr;
    uint64_t size = 0;

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<RAMIOBuffer*>(ptr);
            SKR_ASSERT(p->pool && "Invalid pool detected!");
            p->pool->deallocate(p); 
        };
    }
    friend struct SmartPool<RAMIOBuffer, IRAMIOBuffer>;
protected:
    RAMIOBuffer(ISmartPoolPtr<IRAMIOBuffer> pool) 
        : pool(pool)
    {
        SKR_ASSERT(pool && "Invalid pool detected!");
    }
    ISmartPoolPtr<IRAMIOBuffer> pool = nullptr;
};

} // namespace io
} // namespace skr