#pragma once
#include "cgpu/cgpux.h"
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include <EASTL/fixed_vector.h>

struct CGPUXBindTableLocation;
struct CGPUXBindTable;
struct CGPUXMergedBindTable;

struct CGPUXBindTableValue
{
    friend struct CGPUXBindTable;
    friend struct CGPUXMergedBindTable;
public:
    CGPUXBindTableValue() = default;
    CGPUXBindTableValue(const CGPUXBindTableValue&) = delete;
    CGPUXBindTableValue& operator=(const CGPUXBindTableValue&) = delete;

    void Initialize(const CGPUXBindTableLocation& loc, const CGPUDescriptorData& rhs);

protected:
    bool binded = false;
    CGPUDescriptorData data = {};
    // arena
    eastl::fixed_vector<const void*, 1> resources;
    eastl::fixed_vector<uint64_t, 1> offsets;
    eastl::fixed_vector<uint64_t, 1> sizes;
};

struct CGPUXBindTableLocation
{
    const uint32_t tbl_idx = 0;
    const uint32_t binding = 0;
    CGPUXBindTableValue value;
};

// | tex0 | buffer0 |    set0
// | tex1 | buffer1 |    set1
// | buffer2 | tex2 |    set2
// desc: buffer0, buffer2
// overriden_sets_count: 2, overriden_sets: set0 & set2
struct CGPUXBindTable 
{
    friend struct CGPUXMergedBindTable;
public:
    RUNTIME_API static CGPUXBindTableId Create(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc) SKR_NOEXCEPT;
    RUNTIME_API static void Free(CGPUXBindTableId table) SKR_NOEXCEPT;

    RUNTIME_API void Update(const struct CGPUDescriptorData* datas, uint32_t count) SKR_NOEXCEPT;
    RUNTIME_API void Bind(CGPURenderPassEncoderId encoder) const SKR_NOEXCEPT;
    RUNTIME_API void Bind(CGPUComputePassEncoderId encoder) const SKR_NOEXCEPT;

    inline CGPURootSignatureId GetRootSignature() const SKR_NOEXCEPT
    {
        return root_signature;
    }

protected:
    void updateDescSetsIfDirty() const SKR_NOEXCEPT;

    CGPURootSignatureId root_signature = nullptr;
    // flatten name hashes 
    uint64_t* name_hashes = nullptr;
    // set index location for flattened name hashes
    CGPUXBindTableLocation* name_locations = nullptr;
    // count of flattened name hashes
    uint32_t names_count = 0;
    // all sets
    uint32_t sets_count = 0;
    CGPUDescriptorSetId* sets = nullptr;
};

struct CGPUXMergedBindTable
{
    // on initialize we create no descriptor sets for the table
    // on merge:
    // 1. detect overlap sets, for example, multiple tables update set-1, then we'll create a new set-1 and update it with these tables
    // 2. for no-overlap sets, we'll just copy them to the merged table
public:
    RUNTIME_API static CGPUXMergedBindTableId Create(CGPUDeviceId device, const struct CGPUXMergedBindTableDescriptor* desc) SKR_NOEXCEPT;
    RUNTIME_API static void Free(CGPUXMergedBindTableId table) SKR_NOEXCEPT;

    RUNTIME_API void Merge(const CGPUXBindTableId* tables, uint32_t count) SKR_NOEXCEPT;
    RUNTIME_API void Bind(CGPURenderPassEncoderId encoder) const SKR_NOEXCEPT;
    RUNTIME_API void Bind(CGPUComputePassEncoderId encoder) const SKR_NOEXCEPT;

protected:
    void mergeUpdateForTable(const CGPUXBindTableId* bind_tables, uint32_t count, uint32_t tbl_idx) SKR_NOEXCEPT;

    CGPURootSignatureId root_signature = nullptr;
    uint32_t sets_count = 0;
    CGPUDescriptorSetId* copied = nullptr;
    CGPUDescriptorSetId* merged = nullptr;
    CGPUDescriptorSetId* result = nullptr;
};

namespace cgpux
{
template <typename T> struct hash;
template <typename T> struct equal_to;

template <typename T, typename... Rest>
void hash_combine(size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= cgpux::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

template <>
struct hash<size_t> {
    size_t operator()(const size_t& val) const { return val; }
};
template <>
struct hash<const void*> {
    size_t operator()(const void* val) const { return (size_t)val; }
};
template <>
struct hash<CGPUVertexLayout> {
    size_t operator()(const CGPUVertexLayout& val) const { return skr_hash(&val, sizeof(CGPUVertexLayout), CGPU_NAME_HASH_SEED); }
};
static const CGPUVertexLayout kZeroCGPUVertexLayout = make_zeroed<CGPUVertexLayout>();

template <>
struct equal_to<CGPUVertexLayout> {
    size_t operator()(const CGPUVertexLayout& a, const CGPUVertexLayout& b) const
    {
        if (a.attribute_count != b.attribute_count) return false;
        for (uint32_t i = 0; i < a.attribute_count; i++)
        {
            const bool vequal = (a.attributes[i].array_size == b.attributes[i].array_size) &&
                                (a.attributes[i].format == b.attributes[i].format) &&
                                (a.attributes[i].binding == b.attributes[i].binding) &&
                                (a.attributes[i].offset == b.attributes[i].offset) &&
                                (a.attributes[i].elem_stride == b.attributes[i].elem_stride) &&
                                (a.attributes[i].rate == b.attributes[i].rate) &&
                                (0 == strcmp(a.attributes[i].semantic_name, b.attributes[i].semantic_name));
            if (!vequal) return false;
        }
        return true;
    }
};

template <>
struct equal_to<CGPUDescriptorData> {
    size_t operator()(const CGPUDescriptorData& a, const CGPUDescriptorData& b) const
    {
        if (a.binding != b.binding) 
            return false;
        if (a.binding_type != b.binding_type) 
            return false;
        if (a.count != b.count) 
            return false;
        for (uint32_t i = 0; i < a.count; i++)
        {
            if (a.ptrs[i] != b.ptrs[i]) 
                return false;
        }
        // extra parameters
        if (a.buffers_params.offsets)
        {
            if (!b.buffers_params.offsets) 
                return false;
            for (uint32_t i = 0; i < a.count; i++)
            {
                if (a.buffers_params.offsets[i] != b.buffers_params.offsets[i]) 
                    return false;
            }
        }
        if (a.buffers_params.sizes)
        {
            if (a.buffers_params.sizes) 
                return false;
            for (uint32_t i = 0; i < a.count; i++)
            {
                if (a.buffers_params.sizes[i] != b.buffers_params.sizes[i]) 
                    return false;
            }
        }
        return true;
    }
};

template <>
struct equal_to<CGPUPipelineShaderDescriptor> {
    size_t operator()(const CGPUPipelineShaderDescriptor& a, const CGPUPipelineShaderDescriptor& b) const
    {
        if (a.library != b.library) return false;
        if (a.stage != b.stage) return false;
        if (a.num_constants != b.num_constants) return false;
        if (::strcmp(a.entry, b.entry) != 0) return false;
        for (uint32_t i = 0; i < a.num_constants; i++)
        {
            if (a.constants[i].constantID != b.constants[i].constantID) return false;
            if (a.constants[i].u != b.constants[i].u) return false;
        }
        return true;
    }
};

template <>
struct hash<CGPUPipelineShaderDescriptor> {
    size_t operator()(const CGPUPipelineShaderDescriptor& val) const 
    {
        size_t result = val.stage;
        const auto entry_hash = skr_hash(val.entry, strlen(val.entry), CGPU_NAME_HASH_SEED); 
        const auto constants_hash = skr_hash(val.constants, sizeof(CGPUConstantSpecialization) * val.num_constants, CGPU_NAME_HASH_SEED);
        const auto pLibrary = static_cast<const void*>(val.library);
        hash_combine(result, entry_hash, constants_hash, pLibrary);    
        return result;   
    }
};
static const CGPUPipelineShaderDescriptor kZeroCGPUPipelineShaderDescriptor = make_zeroed<CGPUPipelineShaderDescriptor>();

template <>
struct equal_to<CGPUBlendStateDescriptor> {
    size_t operator()(const CGPUBlendStateDescriptor& a, const CGPUBlendStateDescriptor& b) const
    {
        if (a.alpha_to_coverage != b.alpha_to_coverage) return false;            
        if (a.independent_blend != b.independent_blend) return false;       
        for (uint32_t i = 0; i < count; i++)
        {
            if (a.src_factors[i] != b.src_factors[i]) return false;            
            if (a.dst_factors[i] != b.dst_factors[i]) return false;            
            if (a.src_alpha_factors[i] != b.src_alpha_factors[i]) return false;            
            if (a.dst_alpha_factors[i] != b.dst_alpha_factors[i]) return false;            
            if (a.blend_modes[i] != b.blend_modes[i]) return false;            
            if (a.blend_alpha_modes[i] != b.blend_alpha_modes[i]) return false;            
            if (a.masks[i] != b.masks[i]) return false;            
        }
        return true;
    }
    uint32_t count = CGPU_MAX_MRT_COUNT;
};

template <>
struct hash<CGPUBlendStateDescriptor> {
    size_t operator()(const CGPUBlendStateDescriptor& val) const 
    {
        return skr_hash(&val, sizeof(CGPUBlendStateDescriptor), CGPU_NAME_HASH_SEED);
    }
};
static const CGPUBlendStateDescriptor kZeroCGPUBlendStateDescriptor = make_zeroed<CGPUBlendStateDescriptor>();

template <>
struct equal_to<CGPUDepthStateDesc> {
    size_t operator()(const CGPUDepthStateDesc& a, const CGPUDepthStateDesc& b) const
    {
        if (a.depth_test != b.depth_test) return false;            
        if (a.depth_write != b.depth_write) return false;            
        if (a.depth_func != b.depth_func) return false;            
        if (a.stencil_test != b.stencil_test) return false;            
        if (a.stencil_read_mask != b.stencil_read_mask) return false;            
        if (a.stencil_write_mask != b.stencil_write_mask) return false;            
        if (a.stencil_front_func != b.stencil_front_func) return false;            
        if (a.stencil_front_fail != b.stencil_front_fail) return false;            
        if (a.depth_front_fail != b.depth_front_fail) return false;            
        if (a.stencil_front_pass != b.stencil_front_pass) return false;            
        if (a.stencil_back_func != b.stencil_back_func) return false;            
        if (a.stencil_back_fail != b.stencil_back_fail) return false;            
        if (a.depth_back_fail != b.depth_back_fail) return false;            
        if (a.stencil_back_pass != b.stencil_back_pass) return false;            
        return true;
    }
};

template <>
struct hash<CGPUDepthStateDesc> {
    size_t operator()(const CGPUDepthStateDesc& val) const 
    {
        return skr_hash(&val, sizeof(CGPUDepthStateDesc), CGPU_NAME_HASH_SEED);
    }
};
static const CGPUDepthStateDesc kZeroCGPUDepthStateDesc = make_zeroed<CGPUDepthStateDesc>();

template <>
struct equal_to<CGPURasterizerStateDescriptor> {
    size_t operator()(const CGPURasterizerStateDescriptor& a, const CGPURasterizerStateDescriptor& b) const
    {
        if (a.cull_mode != b.cull_mode) return false;            
        if (a.depth_bias != b.depth_bias) return false;            
        if (a.slope_scaled_depth_bias != b.slope_scaled_depth_bias) return false;            
        if (a.fill_mode != b.fill_mode) return false;            
        if (a.front_face != b.front_face) return false;            
        if (a.enable_multi_sample != b.enable_multi_sample) return false;            
        if (a.enable_scissor != b.enable_scissor) return false;            
        if (a.enable_depth_clamp != b.enable_depth_clamp) return false;            
        return true;
    }
};

template <>
struct hash<CGPURasterizerStateDescriptor> {
    size_t operator()(const CGPURasterizerStateDescriptor& val) const 
    {
        return skr_hash(&val, sizeof(CGPURasterizerStateDescriptor), CGPU_NAME_HASH_SEED);
    }
};
static const CGPURasterizerStateDescriptor kZeroCGPURasterizerStateDescriptor = make_zeroed<CGPURasterizerStateDescriptor>();

template <>
struct equal_to<CGPURenderPipelineDescriptor> {
    size_t operator()(const CGPURenderPipelineDescriptor& a, const CGPURenderPipelineDescriptor& b) const
    {
        if (a.vertex_layout->attribute_count != b.vertex_layout->attribute_count) 
            return false;
        if (a.render_target_count != b.render_target_count) 
            return false;

        // equal root signature
        const auto rs_a = a.root_signature->pool_sig ? a.root_signature->pool_sig : a.root_signature;
        const auto rs_b = b.root_signature->pool_sig ? b.root_signature->pool_sig : b.root_signature;
        if (rs_a != rs_b) return false;
        
        // equal sample quality & count
        if (a.sample_quality != b.sample_quality)  return false;
        if (a.sample_count != b.sample_count) return false;

        // equal out formats
        if (a.depth_stencil_format != b.depth_stencil_format) return false;
        for (uint32_t i = 0; i < a.render_target_count; i++)
        {
            if (a.color_formats[i] != b.color_formats[i]) return false;
        }

        if (a.color_resolve_disable_mask != b.color_resolve_disable_mask) return false;
        if (a.prim_topology != b.prim_topology) return false;
        if (a.enable_indirect_command != b.enable_indirect_command) return false;

        // equal shaders
        if (a.vertex_shader && !b.vertex_shader) return false;
        if (!a.vertex_shader && b.vertex_shader) return false;
        if (a.vertex_shader && !equal_to<CGPUPipelineShaderDescriptor>()(*a.vertex_shader, *b.vertex_shader)) return false;

        if (a.tesc_shader && !b.tesc_shader) return false;
        if (!a.tesc_shader && b.tesc_shader) return false;
        if (a.tesc_shader && !equal_to<CGPUPipelineShaderDescriptor>()(*a.tesc_shader, *b.tesc_shader)) return false;

        if (a.tese_shader && !b.tese_shader) return false;
        if (!a.tese_shader && b.tese_shader) return false;
        if (a.tese_shader && !equal_to<CGPUPipelineShaderDescriptor>()(*a.tese_shader, *b.tese_shader)) return false;

        if (a.geom_shader && !b.geom_shader) return false;
        if (!a.geom_shader && b.geom_shader) return false;
        if (a.geom_shader && !equal_to<CGPUPipelineShaderDescriptor>()(*a.geom_shader, *b.geom_shader)) return false;

        if (a.fragment_shader && !b.fragment_shader) return false;
        if (!a.fragment_shader && b.fragment_shader) return false;
        if (a.fragment_shader && !equal_to<CGPUPipelineShaderDescriptor>()(*a.fragment_shader, *b.fragment_shader)) return false;
        
        // equal vertex layout
        if (a.vertex_layout && !b.vertex_layout) return false;
        if (!a.vertex_layout && b.vertex_layout) return false;
        if (a.vertex_layout && !equal_to<CGPUVertexLayout>()(*a.vertex_layout, *b.vertex_layout)) return false;

        // equal blend state
        if (a.blend_state && !b.blend_state) return false;
        if (!a.blend_state && b.blend_state) return false;
        auto bs_equal = equal_to<CGPUBlendStateDescriptor>(); bs_equal.count = a.render_target_count;
        if (a.blend_state && !bs_equal(*a.blend_state, *b.blend_state)) return false;

        // equal depth state
        if (a.depth_state && !b.depth_state) return false;
        if (!a.depth_state && b.depth_state) return false;
        if (a.depth_state && !equal_to<CGPUDepthStateDesc>()(*a.depth_state, *b.depth_state)) return false;

        // equal raster state
        if (a.rasterizer_state && !b.rasterizer_state) return false;
        if (!a.rasterizer_state && b.rasterizer_state) return false;
        if (a.rasterizer_state && !equal_to<CGPURasterizerStateDescriptor>()(*a.rasterizer_state, *b.rasterizer_state)) return false;

        return true;
    }
};

template <>
struct hash<CGPURenderPipelineDescriptor> {
    struct ParameterBlock
    {
        ParameterBlock(const CGPURenderPipelineDescriptor& desc)
            : render_target_count(desc.render_target_count), 
            sample_count(desc.sample_count), 
            sample_quality(desc.sample_quality),
            color_resolve_disable_mask(desc.color_resolve_disable_mask),
            depth_stencil_format(desc.depth_stencil_format),
            prim_topology(desc.prim_topology),
            enable_indirect_command(desc.enable_indirect_command)
        {
            for (uint32_t i = 0; i < render_target_count; i++)
            {
                color_formats[i] = desc.color_formats[i];
            }
        }
        ECGPUFormat color_formats[CGPU_MAX_MRT_COUNT];
        const uint32_t render_target_count;
        const ECGPUSampleCount sample_count;
        const uint32_t sample_quality;
        const ECGPUSlotMask color_resolve_disable_mask;
        const ECGPUFormat depth_stencil_format;
        const ECGPUPrimitiveTopology prim_topology;
        const bool enable_indirect_command;
    };

    size_t operator()(const CGPURenderPipelineDescriptor& a) const 
    {
        size_t result = 0;
        const auto block = make_zeroed<ParameterBlock>(a);
        const void* rs_a = a.root_signature->pool_sig ? a.root_signature->pool_sig : a.root_signature;
        const auto& vertex_shader = a.vertex_shader ? *a.vertex_shader : kZeroCGPUPipelineShaderDescriptor;
        const auto& tesc_shader = a.tesc_shader ? *a.tesc_shader : kZeroCGPUPipelineShaderDescriptor;
        const auto& tese_shader = a.tese_shader ? *a.tese_shader : kZeroCGPUPipelineShaderDescriptor;
        const auto& geom_shader = a.geom_shader ? *a.geom_shader : kZeroCGPUPipelineShaderDescriptor;
        const auto& fragment_shader = a.fragment_shader ? *a.fragment_shader : kZeroCGPUPipelineShaderDescriptor;
        const auto& vertex_layout = a.vertex_layout ? *a.vertex_layout : kZeroCGPUVertexLayout;
        const auto& blend_state = a.blend_state ? *a.blend_state : kZeroCGPUBlendStateDescriptor;
        const auto& depth_state = a.depth_state ? *a.depth_state : kZeroCGPUDepthStateDesc;
        const auto& rasterizer_state = a.rasterizer_state ? *a.rasterizer_state : kZeroCGPURasterizerStateDescriptor;
        hash_combine(result, rs_a,
            vertex_shader, tesc_shader, tese_shader, geom_shader, fragment_shader, 
            vertex_layout, blend_state, depth_state, rasterizer_state, block);
        return 0;
    }
};

template <>
struct hash<hash<CGPURenderPipelineDescriptor>::ParameterBlock> {
    size_t operator()(const hash<CGPURenderPipelineDescriptor>::ParameterBlock& val) const 
    {
        return skr_hash(&val, sizeof(hash<CGPURenderPipelineDescriptor>::ParameterBlock), CGPU_NAME_HASH_SEED);
    }
};
} // namespace cgpux