#include "string.h"
#include "common_utils.h"
#include "cgpu/cgpux.hpp"

#include "tracy/Tracy.hpp"

// CGPUX bind table apis

void CGPUXBindTableValue::Initialize(const CGPUXBindTableLocation& loc, const CGPUDescriptorData& rhs)
{
    data = rhs;
    data.name = nullptr;
    data.binding = loc.binding;
    data.binding_type = rhs.binding_type;
    binded = false;
    resources.resize(data.count);
    for (uint32_t i = 0; i < data.count; i++)
    {
        resources[i] = data.ptrs[i];
    }
    data.ptrs = resources.data();

    if (data.buffers_params.offsets)
    {
        offsets.resize(data.count);
        for (uint32_t i = 0; i < data.count; i++)
        {
            offsets[i] = data.buffers_params.offsets[i];
        }
        data.buffers_params.offsets = offsets.data();
    }
    if (data.buffers_params.sizes)
    {
        sizes.resize(data.count);
        for (uint32_t i = 0; i < data.count; i++)
        {
            sizes[i] = data.buffers_params.sizes[i];
        }
        data.buffers_params.sizes = sizes.data();
    }
}

CGPUXBindTableId CGPUXBindTable::Create(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc) SKR_NOEXCEPT
{
    auto rs = desc->root_signature;
    const auto hashes_size = desc->names_count * sizeof(uint64_t);
    const auto locations_size = desc->names_count * sizeof(CGPUXBindTableLocation);
    const auto sets_size = rs->table_count * sizeof(CGPUDescriptorSetId);
    const auto total_size = sizeof(CGPUXBindTable) + hashes_size + locations_size + sets_size;
    CGPUXBindTable* table = (CGPUXBindTable*)cgpu_calloc_aligned(1, total_size, alignof(CGPUXBindTable));
    uint64_t* pHashes = (uint64_t*)(table + 1);
    CGPUXBindTableLocation* pLocations = (CGPUXBindTableLocation*)(pHashes + desc->names_count);
    CGPUDescriptorSetId* pSets = (CGPUDescriptorSetId*)(pLocations + desc->names_count);
    table->names_count = desc->names_count;
    table->name_hashes = pHashes;
    table->name_locations = pLocations;
    table->sets_count = rs->table_count;
    table->sets = pSets;
    table->root_signature = desc->root_signature;
    // calculate hashes for each name
    for (uint32_t i = 0; i < desc->names_count; i++)
    {
        const auto name = desc->names[i];
        pHashes[i] = cgpu_name_hash(name, strlen(name));
    }
    // calculate active sets
    for (uint32_t setIdx = 0; setIdx < rs->table_count; setIdx++)
    {
        for (uint32_t bindIdx = 0; bindIdx < rs->tables[setIdx].resources_count; bindIdx++)
        {
            const auto res = rs->tables[setIdx].resources[bindIdx];
            const auto hash = cgpu_name_hash(res.name, strlen(res.name));
            for (uint32_t k = 0; k < desc->names_count; k++)
            {
                if (hash == pHashes[k])
                {
                    // initialize location set/binding
                    new (pLocations + k) CGPUXBindTableLocation();
                    const_cast<uint32_t&>(pLocations[k].tbl_idx) = setIdx;
                    const_cast<uint32_t&>(pLocations[k].binding) = res.binding;

                    CGPUDescriptorSetDescriptor setDesc = {};
                    setDesc.root_signature = desc->root_signature;
                    setDesc.set_index = setIdx;
                    if (!pSets[setIdx]) 
                    {
                        pSets[setIdx] = cgpu_create_descriptor_set(device, &setDesc);
                    }
                    break;
                }
            }
        }
    }
    return table;
}

void CGPUXBindTable::Update(const struct CGPUDescriptorData* datas, uint32_t count) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < count; i++)
    {
        bool updated = false;
        const auto& data = datas[i];
        if (data.name)
        {
            const auto name_hash = cgpu_name_hash(data.name, strlen(data.name));
            for (uint32_t j = 0; j < names_count; j++)
            {
                if (name_hash == name_hashes[j])
                {
                    const auto& location = name_locations[j];
                    if (!cgpux::equal_to<CGPUDescriptorData>()(data, location.value.data))
                    {
                        auto& loc = name_locations[j];
                        loc.value.Initialize(loc, data);
                    }
                    updated = true;
                    break;
                }
            }
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
        (void)updated;
    }
    updateDescSetsIfDirty();
}

void CGPUXBindTable::updateDescSetsIfDirty() const SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < names_count; i++)
    {
        const auto& location = name_locations[i];
        if (!location.value.binded)
        {
            const auto& set = sets[location.tbl_idx];
            // TODO: batch update for better performance
            // this update is kinda dangerous during draw-call because update-after-bind may happen
            // TODO: fix this
            const auto& table = set->root_signature->tables[location.tbl_idx];
            const auto rsBindingType = table.resources[location.binding].type; 
            const auto thisBindingType = location.value.data.binding_type;
            SKR_ASSERT(rsBindingType == thisBindingType && "Binding type mismatch");
            cgpu_update_descriptor_set(set, &location.value.data, 1);
            const_cast<bool&>(location.value.binded) = true;
        }
    }
}

void CGPUXBindTable::Bind(CGPURenderPassEncoderId encoder) const SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++)
    {
        if (sets[i] != nullptr)
        {
            cgpu_render_encoder_bind_descriptor_set(encoder, sets[i]);
        }
    }
}

void CGPUXBindTable::Bind(CGPUComputePassEncoderId encoder) const SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++)
    {
        if (sets[i] != nullptr)
        {
            cgpu_compute_encoder_bind_descriptor_set(encoder, sets[i]);
        }
    }
}

void CGPUXBindTable::Free(CGPUXBindTableId table) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < table->sets_count; i++)
    {
        if (table->sets[i]) cgpu_free_descriptor_set(table->sets[i]);
    }
    for (uint32_t i = 0; i < table->names_count; i++)
    {
        table->name_locations[i].~CGPUXBindTableLocation();
    }
    ((CGPUXBindTable*)table)->~CGPUXBindTable();
    cgpu_free_aligned((void*)table, alignof(CGPUXBindTable));
}

CGPUXBindTableId cgpux_create_bind_table(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc)
{
    return CGPUXBindTable::Create(device, desc);
}

void cgpux_bind_table_update(CGPUXBindTableId table, const struct CGPUDescriptorData* datas, uint32_t count)
{
    return ((CGPUXBindTable*)table)->Update(datas, count);
}

void cgpux_render_encoder_bind_bind_table(CGPURenderPassEncoderId encoder, CGPUXBindTableId table)
{
    table->Bind(encoder);
}

void cgpux_compute_encoder_bind_bind_table(CGPUComputePassEncoderId encoder, CGPUXBindTableId table)
{
    table->Bind(encoder);
}

void cgpux_free_bind_table(CGPUXBindTableId bind_table)
{
    CGPUXBindTable::Free(bind_table);
}

// CGPUX merged bind table apis

CGPUXMergedBindTableId CGPUXMergedBindTable::Create(CGPUDeviceId device, const struct CGPUXMergedBindTableDescriptor *desc) SKR_NOEXCEPT
{
    SKR_ASSERT(desc->root_signature);

    const auto total_size = sizeof(CGPUXMergedBindTable) + 3 * desc->root_signature->table_count * sizeof(CGPUDescriptorSetId);
    CGPUXMergedBindTable* table = (CGPUXMergedBindTable*)cgpu_calloc_aligned(1, total_size, alignof(CGPUXMergedBindTable));
    table->root_signature = desc->root_signature;
    table->sets_count = desc->root_signature->table_count;
    table->copied = (CGPUDescriptorSetId*)(table + 1);
    table->merged = table->copied + table->sets_count;
    table->result = table->merged + table->sets_count;
    return table;
}

void CGPUXMergedBindTable::Merge(const CGPUXBindTableId* bind_tables, uint32_t count) SKR_NOEXCEPT
{
    ZoneScopedN("CGPUXMergedBindTable::Merge");

    // reset result slots
    for (uint32_t tblIdx = 0; tblIdx < root_signature->table_count; tblIdx++)
    {
        result[tblIdx] = nullptr;
    }

    // detect overlap sets at ${i}
    const auto notfound_index = root_signature->table_count;
    const auto overlap_index = UINT32_MAX;
    for (uint32_t tblIdx = 0; tblIdx < root_signature->table_count; tblIdx++)
    {
        uint32_t source_table = notfound_index;
        for (uint32_t j = 0; j < count; j++)
        {
            if (bind_tables[j]->sets[tblIdx] != nullptr)
            {
                if (source_table == notfound_index)
                {
                    source_table = j;
                }
                else
                {
                    // overlap detected
                    source_table = overlap_index;
                    break;
                }
            }
        }
        if (source_table == notfound_index) // not found set
        {
            // ... do nothing now
        }
        else if (source_table == overlap_index)
        {
            ZoneScopedN("CGPUXMergedBindTable::MergeOverlap");
            
            if (!merged[tblIdx]) 
            {
                CGPUDescriptorSetDescriptor setDesc = {};
                setDesc.root_signature = root_signature;
                setDesc.set_index = tblIdx;
                merged[tblIdx] = cgpu_create_descriptor_set(root_signature->device, &setDesc);
            }
            // update merged value
            mergeUpdateForTable(bind_tables, count, tblIdx);
            result[tblIdx] = merged[tblIdx];
        }
        else // direct copy from source table
        {
            copied[tblIdx] = bind_tables[source_table]->sets[tblIdx];
            result[tblIdx] = copied[tblIdx];
        }
    }
}

void CGPUXMergedBindTable::mergeUpdateForTable(const CGPUXBindTableId* bind_tables, uint32_t count, uint32_t tbl_idx) SKR_NOEXCEPT
{
    ZoneScopedN("CGPUXMergedBindTable::UpdateDescriptors");

    auto to_update = merged[tbl_idx];
    // TODO: refactor & remove this vector
    eastl::vector<CGPUDescriptorData> datas;
    // foreach table location to update values
    for (uint32_t i = 0; i < count; i++)
    {
        for (uint32_t j = 0; j < bind_tables[i]->names_count; j++)
        {
            const auto& location = bind_tables[i]->name_locations[j];
            if (location.tbl_idx == tbl_idx)
            {
                ZoneScopedN("CGPUXMergedBindTable::UpdateDescriptor");
                // batch update for better performance
                datas.emplace_back(location.value.data);
            }
        }
    }
    // this update is kinda dangerous during draw-call because update-after-bind may happen
    // TODO: give some runtime warning
    cgpu_update_descriptor_set(to_update, datas.data(), (uint32_t)datas.size());
}

void CGPUXMergedBindTable::Bind(CGPURenderPassEncoderId encoder) const SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++)
    {
        if (result[i] != nullptr)
        {
            cgpu_render_encoder_bind_descriptor_set(encoder, result[i]);
        }
    }
}

void CGPUXMergedBindTable::Bind(CGPUComputePassEncoderId encoder) const SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++)
    {
        if (result[i] != nullptr)
        {
            cgpu_compute_encoder_bind_descriptor_set(encoder, result[i]);
        }
    }
}

void CGPUXMergedBindTable::Free(CGPUXMergedBindTableId table) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < table->sets_count; i++)
    {
        // free merged sets
        if (table->merged[i]) cgpu_free_descriptor_set(table->merged[i]);
    }
    ((CGPUXMergedBindTable*)table)->~CGPUXMergedBindTable();
}

CGPUXMergedBindTableId cgpux_create_megred_bind_table(CGPUDeviceId device, const struct CGPUXMergedBindTableDescriptor* desc)
{
    return CGPUXMergedBindTable::Create(device, desc);
}

void cgpux_merged_bind_table_merge(CGPUXMergedBindTableId table, const CGPUXBindTableId* tables, uint32_t count)
{
    return ((CGPUXMergedBindTable*)table)->Merge(tables, count);
}

void cgpux_render_encoder_bind_merged_bind_table(CGPURenderPassEncoderId encoder, CGPUXMergedBindTableId table)
{
    table->Bind(encoder);
}

void cgpux_compute_encoder_bind_merged_bind_table(CGPUComputePassEncoderId encoder, CGPUXMergedBindTableId table)
{
    table->Bind(encoder);
}

void cgpux_free_merged_bind_table(CGPUXMergedBindTableId merged_table)
{
    CGPUXMergedBindTable::Free(merged_table);
}

// equals & hashes
namespace cgpux
{
size_t hash<CGPUVertexLayout>::operator()(const CGPUVertexLayout& val) const 
{
    ZoneScopedN("hash<CGPUVertexLayout>");

    return skr_hash(&val, sizeof(CGPUVertexLayout), CGPU_NAME_HASH_SEED); 
}

size_t equal_to<CGPUVertexLayout>::operator()(const CGPUVertexLayout& a, const CGPUVertexLayout& b) const
{
    ZoneScopedN("equal_to<CGPUVertexLayout>");

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

size_t equal_to<CGPUDescriptorData>::operator()(const CGPUDescriptorData& a, const CGPUDescriptorData& b) const
{
    ZoneScopedN("equal_to<CGPUDescriptorData>");

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

size_t equal_to<CGPUPipelineShaderDescriptor>::operator()(const CGPUPipelineShaderDescriptor& a, const CGPUPipelineShaderDescriptor& b) const
{
    ZoneScopedN("equal_to<CGPUPipelineShaderDescriptor>");

    if (a.library != b.library) return false;
    if (a.stage != b.stage) return false;
    if (a.num_constants != b.num_constants) return false;
    if (a.entry && !b.entry) return false;
    if (!a.entry && b.entry) return false;
    if (a.entry && ::strcmp(a.entry, b.entry) != 0) return false;
    for (uint32_t i = 0; i < a.num_constants; i++)
    {
        if (a.constants[i].constantID != b.constants[i].constantID) return false;
        if (a.constants[i].u != b.constants[i].u) return false;
    }
    return true;
}

size_t hash<CGPUPipelineShaderDescriptor>::operator()(const CGPUPipelineShaderDescriptor& val) const 
{
    ZoneScopedN("hash<CGPUPipelineShaderDescriptor>");

    size_t result = val.stage;
    const auto entry_hash = val.entry ? skr_hash(val.entry, strlen(val.entry), CGPU_NAME_HASH_SEED) : 0; 
    const auto constants_hash = val.constants ? skr_hash(val.constants, sizeof(CGPUConstantSpecialization) * val.num_constants, CGPU_NAME_HASH_SEED) : 0;
    const auto pLibrary = static_cast<const void*>(val.library);
    hash_combine(result, entry_hash, constants_hash, pLibrary);    
    return result;   
}

size_t equal_to<CGPUBlendStateDescriptor>::operator()(const CGPUBlendStateDescriptor& a, const CGPUBlendStateDescriptor& b) const
{
    ZoneScopedN("equal_to<CGPUBlendStateDescriptor>");

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

size_t hash<CGPUBlendStateDescriptor>::operator()(const CGPUBlendStateDescriptor& val) const 
{
    ZoneScopedN("hash<CGPUBlendStateDescriptor>");

    return skr_hash(&val, sizeof(CGPUBlendStateDescriptor), CGPU_NAME_HASH_SEED);
}

size_t equal_to<CGPUDepthStateDesc>::operator()(const CGPUDepthStateDesc& a, const CGPUDepthStateDesc& b) const
{
    ZoneScopedN("equal_to<CGPUDepthStateDesc>");

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

size_t hash<CGPUDepthStateDesc>::operator()(const CGPUDepthStateDesc& val) const 
{
    ZoneScopedN("hash<CGPUDepthStateDesc>");

    return skr_hash(&val, sizeof(CGPUDepthStateDesc), CGPU_NAME_HASH_SEED);
}

size_t equal_to<CGPURasterizerStateDescriptor>::operator()(const CGPURasterizerStateDescriptor& a, const CGPURasterizerStateDescriptor& b) const
{
    ZoneScopedN("equal_to<CGPURasterizerStateDescriptor>");

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

size_t hash<CGPURasterizerStateDescriptor>::operator()(const CGPURasterizerStateDescriptor& val) const 
{
    ZoneScopedN("hash<CGPURasterizerStateDescriptor>");

    return skr_hash(&val, sizeof(CGPURasterizerStateDescriptor), CGPU_NAME_HASH_SEED);
}

size_t equal_to<CGPURenderPipelineDescriptor>::operator()(const CGPURenderPipelineDescriptor& a, const CGPURenderPipelineDescriptor& b) const
{
    ZoneScopedN("equal_to<CGPURenderPipelineDescriptor>");

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

hash<CGPURenderPipelineDescriptor>::ParameterBlock::ParameterBlock(const CGPURenderPipelineDescriptor& desc)
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

size_t hash<CGPURenderPipelineDescriptor>::operator()(const CGPURenderPipelineDescriptor& a) const 
{
    ZoneScopedN("hash<CGPURenderPipelineDescriptor>");

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

size_t hash<hash<CGPURenderPipelineDescriptor>::ParameterBlock>::operator()(const hash<CGPURenderPipelineDescriptor>::ParameterBlock& val) const 
{
    ZoneScopedN("hash<CGPURenderPipelineDescriptor::ParameterBlock>");

    return skr_hash(&val, sizeof(hash<CGPURenderPipelineDescriptor>::ParameterBlock), CGPU_NAME_HASH_SEED);
}
}