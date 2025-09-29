#include "SkrRenderGraph/frontend/base_types.hpp"

namespace skr
{
namespace RG
{
static const auto NULL_TEXTURE_HANDLE = TextureHandle();
// tex
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

TextureSRVHandle TextureHandle::read_cube(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureRTVHandle TextureHandle::write_mip(uint32_t base) const
{
    ShaderWriteHandle _ = *this;
    _.mip_level = base;
    return _;
}

TextureRTVHandle TextureHandle::write_array(uint32_t base, uint32_t count) const
{
    ShaderWriteHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureRTVHandle TextureHandle::write_cube(uint32_t base, uint32_t count) const
{
    ShaderWriteHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureUAVHandle TextureHandle::readwrite_mip(uint32_t mip) const
{
    TextureUAVHandle _ = *this;
    _.mip_level = mip;
    return _;
}

TextureUAVHandle TextureHandle::readwrite_array(uint32_t base, uint32_t count) const
{
    TextureUAVHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureUAVHandle TextureHandle::readwrite_cube(uint32_t base, uint32_t count) const
{
    TextureUAVHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

// srv
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

TextureSRVHandle TextureSRVHandle::read_cube(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureSRVHandle::ShaderReadHandle(const HandleStorage _this, const uint32_t mip_base, const uint32_t mip_count, const uint32_t array_base, const uint32_t array_count)
    : _this(_this)
    , mip_base(mip_base)
    , mip_count(mip_count)
    , array_base(array_base)
    , array_count(array_count)
{
}

// rtv
TextureRTVHandle::ShaderWriteHandle(const HandleStorage _this)
    : _this(_this)
{
}

TextureRTVHandle TextureRTVHandle::write_mip(uint32_t mip) const
{
    TextureRTVHandle _ = *this;
    _.mip_level = mip;
    return _;
}

TextureRTVHandle TextureRTVHandle::write_array(uint32_t base, uint32_t count) const
{
    TextureRTVHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureRTVHandle TextureRTVHandle::write_cube(uint32_t base, uint32_t count) const
{
    TextureRTVHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

// UAV
TextureUAVHandle::ShaderReadWriteHandle(const HandleStorage _this)
    : _this(_this)
{
}

TextureUAVHandle TextureUAVHandle::readwrite_mip(uint32_t mip) const
{
    TextureUAVHandle _ = *this;
    _.mip_level = mip;
    return _;
}

TextureUAVHandle TextureUAVHandle::readwrite_array(uint32_t base, uint32_t count) const
{
    TextureUAVHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

TextureUAVHandle TextureUAVHandle::readwrite_cube(uint32_t base, uint32_t count) const
{
    TextureUAVHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

// Subresource
TextureSubresourceHandle::SubresourceHandle(const HandleStorage _this)
    : _this(_this)
{
}

// CBV
BufferCBVHandle::ShaderReadHandle(const HandleStorage _this)
    : _this(_this)
{
}

// UAV
BufferUAVHandle::ShaderReadWriteHandle(const HandleStorage _this)
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
PipelineBufferHandle::PipelineReferenceHandle(const HandleStorage _this)
    : _this(_this)
{
}

// AccelerationStructure SRV
AccelerationStructureSRVHandle::ShaderReadHandle(const HandleStorage _this)
    : _this(_this)
{
}
} // namespace RG
} // namespace skr