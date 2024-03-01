#pragma once
#include "SkrRT/io/vram_io.hpp"
#include "../common/pool.hpp"

namespace skr {
namespace io {

struct SKR_RUNTIME_API VRAMBuffer : public IVRAMIOBuffer
{
    IO_RC_OBJECT_BODY
public:
    virtual ~VRAMBuffer() SKR_NOEXCEPT;

    CGPUBufferId get_buffer() const SKR_NOEXCEPT
    {
        return buffer;
    }
    CGPUBufferId buffer = nullptr;

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<VRAMBuffer*>(ptr);
            SKR_ASSERT(p->pool && "Invalid pool detected!");
            p->pool->deallocate(p); 
        };
    }
    friend struct SmartPool<VRAMBuffer, IVRAMIOBuffer>;
protected:
    VRAMBuffer(ISmartPoolPtr<IVRAMIOBuffer> pool) 
        : pool(pool)
    {
        SKR_ASSERT(pool && "Invalid pool detected!");
    }
    const ISmartPoolPtr<IVRAMIOBuffer> pool = nullptr;
};

struct SKR_RUNTIME_API VRAMTexture : public IVRAMIOTexture
{
    IO_RC_OBJECT_BODY
public:
    virtual ~VRAMTexture() SKR_NOEXCEPT;

    CGPUTextureId get_texture() const SKR_NOEXCEPT
    {
        return texture;
    }
    CGPUTextureId texture = nullptr;
    
public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<VRAMTexture*>(ptr);
            SKR_ASSERT(p->pool && "Invalid pool detected!");
            p->pool->deallocate(p); 
        };
    }
    friend struct SmartPool<VRAMTexture, IVRAMIOTexture>;
protected:
    VRAMTexture(ISmartPoolPtr<IVRAMIOTexture> pool) 
        : pool(pool)
    {
        SKR_ASSERT(pool && "Invalid pool detected!");
    }
    const ISmartPoolPtr<IVRAMIOTexture> pool = nullptr;
};

} // namespace io
} // namespace skr