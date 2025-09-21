#pragma once
#include "SkrRuntime/io/vram_io.hpp"
#include "../common/pool.hpp"

namespace skr
{
namespace io
{

struct SKR_RUNTIME_API VRAMBuffer : public IVRAMIOBuffer {
    SKR_RC_IMPL(override)
public:
    virtual ~VRAMBuffer() SKR_NOEXCEPT;

    CGPUBufferId get_buffer() const SKR_NOEXCEPT override
    {
        return buffer;
    }
    CGPUBufferId buffer = nullptr;

public:
    void skr_rc_delete() override
    {
        SKR_ASSERT(pool && "Invalid pool detected!");
        pool->deallocate(this);
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

struct SKR_RUNTIME_API VRAMTexture : public IVRAMIOTexture {
    SKR_RC_IMPL(override)
public:
    virtual ~VRAMTexture() SKR_NOEXCEPT;

    CGPUTextureId get_texture() const SKR_NOEXCEPT override
    {
        return texture;
    }
    CGPUTextureId texture = nullptr;

public:
    void skr_rc_delete() override
    {
        SKR_ASSERT(pool && "Invalid pool detected!");
        pool->deallocate(this);
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