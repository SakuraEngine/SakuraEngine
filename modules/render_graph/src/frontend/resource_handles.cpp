#include "SkrRenderGraph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
TextureSRVHandle TextureSRVHandle::read_mip(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.mip_base = base;
    _.mip_count = count;
    return _;
}

TextureSRVHandle TextureSRVHandle::read_array(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureSRVHandle TextureSRVHandle::dimension(ECGPUTextureDimension dim) const
{
    ShaderReadHandle _ = *this;
    _.dim = dim;
    return _;
}

TextureSRVHandle::ShaderReadHandle(const handle_t _this, 
    const uint32_t mip_base, const uint32_t mip_count, const uint32_t array_base, const uint32_t array_count)
    : _this(_this)
    , mip_base(mip_base)
    , mip_count(mip_count)
    , array_base(array_base)
    , array_count(array_count)
{
}

TextureSRVHandle TextureHandle::read_mip(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.mip_base = base;
    _.mip_count = count;
    return _;
}

TextureSRVHandle TextureHandle::read_array(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureRTVHandle::ShaderWriteHandle(const handle_t _this)
    : _this(_this)
{
}

TextureRTVHandle TextureRTVHandle::write_mip(uint32_t mip) const
{
    TextureRTVHandle _ = *this;
    _.mip_level = mip;
    return _;
}

// UAV
TextureUAVHandle::ShaderReadWriteHandle(const handle_t _this)
    : _this(_this)
{
}

// Subresource
TextureSubresourceHandle::SubresourceHandle(const handle_t _this)
    : _this(_this)
{
}

// CBV
BufferCBVHandle::ShaderReadHandle(const handle_t _this)
    : _this(_this)
{
}

// UAV
BufferUAVHandle::ShaderReadWriteHandle(const handle_t _this)
    : _this(_this)
{
}

// ds
TextureDSVHandle TextureHandle::clear_depth(float depth) const
{
    TextureDSVHandle _ = *this;
    _.cleardepth = depth;
    return _;
}

TextureDSVHandle TextureDSVHandle::clear_depth(float depth) const
{
    TextureDSVHandle _ = *this;
    _.cleardepth = depth;
    return _;
}

// VB/IB
PipelineBufferHandle::PipelineReferenceHandle(const handle_t _this)
    : _this(_this)
{
}
} // namespace render_graph
} // namespace skr