#include "SkrProfile/profile.h"
#include "SkrGraphics/cgpux.hpp"
#include "SkrGraphics/containers.hpp"
#include "common_utils.h"

// CGPUX bind table apis

void CGPUXBindTableValue::Initialize(const CGPUXBindTableLocation& loc, const CGPUDescriptorData& rhs)
{
    data = rhs;
    data.by_name.name = nullptr;
    data.by_index.binding = loc.binding;
    data.by_index.view_usage = loc.view_usage;
    data.by_index.type = loc.type;
    binded = false;
    resources.resize_default(data.count);
    for (uint32_t i = 0; i < data.count; i++)
    {
        resources[i] = data.ptrs[i];
    }
    data.ptrs = resources.data();
}

CGPUXBindTableId CGPUXBindTable::Create(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc) SKR_NOEXCEPT
{
    auto rs = desc->root_signature;
    const auto hashes_size = desc->names_count * sizeof(uint64_t);
    const auto locations_size = desc->names_count * sizeof(CGPUXBindTableLocation);
    const auto total_size = sizeof(CGPUXBindTable) + hashes_size + locations_size;
    CGPUXBindTable* table = (CGPUXBindTable*)cgpu_calloc_aligned(1, total_size, alignof(CGPUXBindTable));
    new (table) CGPUXBindTable();
    uint64_t* pHashes = (uint64_t*)(table + 1);
    CGPUXBindTableLocation* pLocations = (CGPUXBindTableLocation*)(pHashes + desc->names_count);
    table->names_count = desc->names_count;
    table->name_hashes = pHashes;
    table->name_locations = pLocations;
    table->root_signature = desc->root_signature;
    // calculate hashes for each name
    for (uint32_t i = 0; i < desc->names_count; i++)
    {
        const auto name = desc->names[i];
        pHashes[i] = skr_hash_of(name, strlen((const char*)name));
    }
    // calculate active sets
    for (uint32_t i = 0; i < rs->table_count; i++)
    {
        for (uint32_t bindIdx = 0; bindIdx < rs->tables[i].resources_count; bindIdx++)
        {
            const auto setIdx = rs->tables[i].set_index;
            const auto res = rs->tables[i].resources[bindIdx];
            const auto hash = skr_hash_of(res.name, strlen((const char*)res.name));
            for (uint32_t k = 0; k < desc->names_count; k++)
            {
                if (hash == pHashes[k])
                {
                    // initialize location set/binding
                    new (pLocations + k) CGPUXBindTableLocation();
                    const_cast<uint32_t&>(pLocations[k].logicalSetIdx) = setIdx;
                    const_cast<uint32_t&>(pLocations[k].binding) = res.binding;
                    const_cast<CGPUViewUsages&>(pLocations[k].view_usage) = res.view_usages;
                    const_cast<ECGPUResourceType&>(pLocations[k].type) = res.type;

                    CGPUDescriptorSetDescriptor setDesc = {};
                    setDesc.root_signature = desc->root_signature;
                    setDesc.set_index = setIdx;
                    if (!table->sets[setIdx])
                    {
                        table->sets[setIdx] = cgpu_create_descriptor_set(device, &setDesc);
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
        const auto& data = datas[i];
        if (data.by_name.name)
        {
            const auto name_hash = skr_hash_of(data.by_name.name, strlen((const char*)data.by_name.name));
            for (uint32_t j = 0; j < names_count; j++)
            {
                if (name_hash == name_hashes[j])
                {
                    auto& loc = name_locations[j];
                    loc.value.Initialize(loc, data);
                    break;
                }
            }
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }

    skr::FixedSet<uint32_t, 4> needsUpdateIndices;
    for (uint32_t i = 0; i < names_count; i++)
    {
        const auto& location = name_locations[i];
        if (!location.value.binded)
        {
            needsUpdateIndices.add(location.logicalSetIdx);
        }
    }
    for (auto setIdx : needsUpdateIndices)
    {
        skr::InlineVector<CGPUDescriptorData, 4> datas;
        for (uint32_t i = 0; i < names_count; i++)
        {
            const auto& location = name_locations[i];
            if (!location.value.binded && location.logicalSetIdx == setIdx)
            {
                // const auto& set = sets[location.tbl_idx];
                // TODO: batch update for better performance
                // this update is kinda dangerous during draw-call because update-after-bind may happen
                // TODO: fix this
                datas.emplace(location.value.data);
                const_cast<bool&>(location.value.binded) = true;
            }
        }
        if (const auto updateDataCount = static_cast<uint32_t>(datas.size()))
        {
            cgpu_update_descriptor_set(sets[setIdx], datas.data(), updateDataCount);
        }
    }
}

void CGPUXBindTable::Bind(CGPURenderPassEncoderId encoder) const SKR_NOEXCEPT
{
    for (auto&& [i, set] : sets)
    {
        cgpu_render_encoder_bind_descriptor_set(encoder, set);
    }
}

void CGPUXBindTable::Bind(CGPUComputePassEncoderId encoder) const SKR_NOEXCEPT
{
    for (auto&& [i, set] : sets)
    {
        cgpu_compute_encoder_bind_descriptor_set(encoder, set);
    }
}

void CGPUXBindTable::Free(CGPUXBindTableId table) SKR_NOEXCEPT
{
    for (auto&& [i, set] : table->sets)
    {
        cgpu_free_descriptor_set(set);
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

CGPUXMergedBindTableId CGPUXMergedBindTable::Create(CGPUDeviceId device, const struct CGPUXMergedBindTableDescriptor* desc) SKR_NOEXCEPT
{
    SKR_ASSERT(desc->root_signature);

    const auto total_size = sizeof(CGPUXMergedBindTable);
    CGPUXMergedBindTable* table = (CGPUXMergedBindTable*)cgpu_calloc_aligned(1, total_size, alignof(CGPUXMergedBindTable));
    new (table) CGPUXMergedBindTable();
    table->root_signature = desc->root_signature;
    table->sets_count = desc->root_signature->table_count;
    return table;
}

void CGPUXMergedBindTable::Merge(const CGPUXBindTableId* bind_tables, uint32_t count) SKR_NOEXCEPT
{
    SkrZoneScopedN("CGPUXMergedBindTable::Merge");

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
            if (bind_tables[j]->sets.find(tblIdx)->second != nullptr)
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
            SkrZoneScopedN("CGPUXMergedBindTable::MergeOverlap");

            auto setIdx = root_signature->tables[tblIdx].set_index;
            if (!merged[setIdx])
            {
                CGPUDescriptorSetDescriptor setDesc = {};
                setDesc.root_signature = root_signature;
                setDesc.set_index = setIdx;
                merged[setIdx] = cgpu_create_descriptor_set(root_signature->device, &setDesc);
            }
            // update merged value
            mergeUpdateForTable(bind_tables, count, setIdx);
            result[setIdx] = merged[setIdx];
        }
        else // direct copy from source table
        {
            auto setIdx = root_signature->tables[tblIdx].set_index;
            copied[setIdx] = bind_tables[source_table]->sets.find(setIdx)->second;
            result[setIdx] = copied[setIdx];
        }
    }
}

void CGPUXMergedBindTable::mergeUpdateForTable(const CGPUXBindTableId* bind_tables, uint32_t count, uint32_t setIdx) SKR_NOEXCEPT
{
    SkrZoneScopedN("CGPUXMergedBindTable::UpdateDescriptors");

    auto to_update = merged[setIdx];
    // TODO: refactor & remove this vector
    skr::Vector<CGPUDescriptorData> datas;
    // foreach table location to update values
    for (uint32_t i = 0; i < count; i++)
    {
        for (uint32_t j = 0; j < bind_tables[i]->names_count; j++)
        {
            const auto& location = bind_tables[i]->name_locations[j];
            if (location.logicalSetIdx == setIdx)
            {
                SkrZoneScopedN("CGPUXMergedBindTable::UpdateDescriptor");
                // batch update for better performance
                datas.add(location.value.data);
            }
        }
    }
    // this update is kinda dangerous during draw-call because update-after-bind may happen
    // TODO: give some runtime warning
    cgpu_update_descriptor_set(to_update, datas.data(), (uint32_t)datas.size());
}

void CGPUXMergedBindTable::Bind(CGPURenderPassEncoderId encoder) const SKR_NOEXCEPT
{
    for (auto&& [i, set] : result)
    {
        if (set == nullptr) 
            continue;
        cgpu_render_encoder_bind_descriptor_set(encoder, set);
    }
}

void CGPUXMergedBindTable::Bind(CGPUComputePassEncoderId encoder) const SKR_NOEXCEPT
{
    for (auto&& [i, set] : result)
    {
        if (set == nullptr) 
            continue;
        cgpu_compute_encoder_bind_descriptor_set(encoder, set);
    }
}

void CGPUXMergedBindTable::Free(CGPUXMergedBindTableId table) SKR_NOEXCEPT
{
    for (auto&& [i, set] : table->merged)
    {
        if (set == nullptr) 
            continue;
        cgpu_free_descriptor_set(set);
    }
    ((CGPUXMergedBindTable*)table)->~CGPUXMergedBindTable();
    cgpu_free_aligned((void*)table, alignof(CGPUXMergedBindTable));
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
skr_hash hash<CGPUVertexLayout>::operator()(const CGPUVertexLayout& val) const
{
    SkrZoneScopedN("hash<CGPUVertexLayout>");

    return skr_hash_of(&val, sizeof(CGPUVertexLayout));
}

skr_hash equal_to<CGPUVertexLayout>::operator()(const CGPUVertexLayout& a, const CGPUVertexLayout& b) const
{
    SkrZoneScopedN("equal_to<CGPUVertexLayout>");

    if (a.attribute_count != b.attribute_count) return false;
    for (uint32_t i = 0; i < a.attribute_count; i++)
    {
        const bool vequal = (a.attributes[i].array_size == b.attributes[i].array_size) &&
            (a.attributes[i].format == b.attributes[i].format) &&
            (a.attributes[i].binding == b.attributes[i].binding) &&
            (a.attributes[i].offset == b.attributes[i].offset) &&
            (a.attributes[i].elem_stride == b.attributes[i].elem_stride) &&
            (a.attributes[i].rate == b.attributes[i].rate) &&
            (0 == strcmp((const char*)a.attributes[i].semantic_name, (const char*)b.attributes[i].semantic_name));
        if (!vequal) return false;
    }
    return true;
}

skr_hash equal_to<CGPUShaderEntryDescriptor>::operator()(const CGPUShaderEntryDescriptor& a, const CGPUShaderEntryDescriptor& b) const
{
    SkrZoneScopedN("equal_to<CGPUShaderEntryDescriptor>");

    if (a.library != b.library) return false;
    if (a.entry && !b.entry) return false;
    if (!a.entry && b.entry) return false;
    if (a.entry && ::strcmp((const char*)a.entry, (const char*)b.entry) != 0) return false;
    /*
    for (uint32_t i = 0; i < a.num_constants; i++)
    {
        if (a.constants[i].constantID != b.constants[i].constantID) return false;
        if (a.constants[i].u != b.constants[i].u) return false;
    }
    */
    return true;
}

skr_hash hash<CGPUShaderEntryDescriptor>::operator()(const CGPUShaderEntryDescriptor& val) const
{
    SkrZoneScopedN("hash<CGPUShaderEntryDescriptor>");

    skr_hash result = 0;
    const auto entry_hash = val.entry ? skr_hash_of((const char*)val.entry, strlen((const char*)val.entry)) : 0;
    // const auto constants_hash = val.constants ? skr_hash_of(val.constants, sizeof(CGPUConstantSpecialization) * val.num_constants) : 0;
    const auto pLibrary = static_cast<const void*>(val.library);
    hash_combine(result, entry_hash /*, constants_hash*/, pLibrary);
    return result;
}

skr_hash equal_to<CGPUBlendStateDescriptor>::operator()(const CGPUBlendStateDescriptor& a, const CGPUBlendStateDescriptor& b) const
{
    SkrZoneScopedN("equal_to<CGPUBlendStateDescriptor>");

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

skr_hash hash<CGPUBlendStateDescriptor>::operator()(const CGPUBlendStateDescriptor& val) const
{
    SkrZoneScopedN("hash<CGPUBlendStateDescriptor>");

    return skr_hash_of(&val, sizeof(CGPUBlendStateDescriptor));
}

skr_hash equal_to<CGPUDepthStateDesc>::operator()(const CGPUDepthStateDesc& a, const CGPUDepthStateDesc& b) const
{
    SkrZoneScopedN("equal_to<CGPUDepthStateDesc>");

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

skr_hash hash<CGPUDepthStateDesc>::operator()(const CGPUDepthStateDesc& val) const
{
    SkrZoneScopedN("hash<CGPUDepthStateDesc>");

    return skr_hash_of(&val, sizeof(CGPUDepthStateDesc));
}

skr_hash equal_to<CGPURasterizerStateDescriptor>::operator()(const CGPURasterizerStateDescriptor& a, const CGPURasterizerStateDescriptor& b) const
{
    SkrZoneScopedN("equal_to<CGPURasterizerStateDescriptor>");

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

skr_hash hash<CGPURasterizerStateDescriptor>::operator()(const CGPURasterizerStateDescriptor& val) const
{
    SkrZoneScopedN("hash<CGPURasterizerStateDescriptor>");

    return skr_hash_of(&val, sizeof(CGPURasterizerStateDescriptor));
}

skr_hash equal_to<CGPURenderPipelineDescriptor>::operator()(const CGPURenderPipelineDescriptor& a, const CGPURenderPipelineDescriptor& b) const
{
    SkrZoneScopedN("equal_to<CGPURenderPipelineDescriptor>");

    if (a.vertex_layout->attribute_count != b.vertex_layout->attribute_count)
        return false;
    if (a.render_target_count != b.render_target_count)
        return false;

    // equal root signature
    const auto rs_a = a.root_signature->pool_sig ? a.root_signature->pool_sig : a.root_signature;
    const auto rs_b = b.root_signature->pool_sig ? b.root_signature->pool_sig : b.root_signature;
    if (rs_a != rs_b) return false;

    // equal sample quality & count
    if (a.sample_quality != b.sample_quality) return false;
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
    if (a.vertex_shader && !equal_to<CGPUShaderEntryDescriptor>()(*a.vertex_shader, *b.vertex_shader)) return false;

    if (a.tesc_shader && !b.tesc_shader) return false;
    if (!a.tesc_shader && b.tesc_shader) return false;
    if (a.tesc_shader && !equal_to<CGPUShaderEntryDescriptor>()(*a.tesc_shader, *b.tesc_shader)) return false;

    if (a.tese_shader && !b.tese_shader) return false;
    if (!a.tese_shader && b.tese_shader) return false;
    if (a.tese_shader && !equal_to<CGPUShaderEntryDescriptor>()(*a.tese_shader, *b.tese_shader)) return false;

    if (a.geom_shader && !b.geom_shader) return false;
    if (!a.geom_shader && b.geom_shader) return false;
    if (a.geom_shader && !equal_to<CGPUShaderEntryDescriptor>()(*a.geom_shader, *b.geom_shader)) return false;

    if (a.fragment_shader && !b.fragment_shader) return false;
    if (!a.fragment_shader && b.fragment_shader) return false;
    if (a.fragment_shader && !equal_to<CGPUShaderEntryDescriptor>()(*a.fragment_shader, *b.fragment_shader)) return false;

    // equal vertex layout
    if (a.vertex_layout && !b.vertex_layout) return false;
    if (!a.vertex_layout && b.vertex_layout) return false;
    if (a.vertex_layout && !equal_to<CGPUVertexLayout>()(*a.vertex_layout, *b.vertex_layout)) return false;

    // equal blend state
    if (a.blend_state && !b.blend_state) return false;
    if (!a.blend_state && b.blend_state) return false;
    auto bs_equal = equal_to<CGPUBlendStateDescriptor>();
    bs_equal.count = a.render_target_count;
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
    : render_target_count(desc.render_target_count)
    , sample_count(desc.sample_count)
    , sample_quality(desc.sample_quality)
    , color_resolve_disable_mask(desc.color_resolve_disable_mask)
    , depth_stencil_format(desc.depth_stencil_format)
    , prim_topology(desc.prim_topology)
    , enable_indirect_command(desc.enable_indirect_command)
{
    for (uint32_t i = 0; i < render_target_count; i++)
    {
        color_formats[i] = desc.color_formats[i];
    }
}

skr_hash hash<CGPURenderPipelineDescriptor>::operator()(const CGPURenderPipelineDescriptor& a) const
{
    SkrZoneScopedN("hash<CGPURenderPipelineDescriptor>");

    skr_hash result = 0;
    const auto block = make_zeroed<ParameterBlock>(a);
    const void* rs_a = a.root_signature->pool_sig ? a.root_signature->pool_sig : a.root_signature;
    const auto& vertex_shader = a.vertex_shader ? *a.vertex_shader : kZeroCGPUShaderEntryDescriptor;
    const auto& tesc_shader = a.tesc_shader ? *a.tesc_shader : kZeroCGPUShaderEntryDescriptor;
    const auto& tese_shader = a.tese_shader ? *a.tese_shader : kZeroCGPUShaderEntryDescriptor;
    const auto& geom_shader = a.geom_shader ? *a.geom_shader : kZeroCGPUShaderEntryDescriptor;
    const auto& fragment_shader = a.fragment_shader ? *a.fragment_shader : kZeroCGPUShaderEntryDescriptor;
    const auto& vertex_layout = a.vertex_layout ? *a.vertex_layout : kZeroCGPUVertexLayout;
    const auto& blend_state = a.blend_state ? *a.blend_state : kZeroCGPUBlendStateDescriptor;
    const auto& depth_state = a.depth_state ? *a.depth_state : kZeroCGPUDepthStateDesc;
    const auto& rasterizer_state = a.rasterizer_state ? *a.rasterizer_state : kZeroCGPURasterizerStateDescriptor;
    hash_combine(result, rs_a, vertex_shader, tesc_shader, tese_shader, geom_shader, fragment_shader, vertex_layout, blend_state, depth_state, rasterizer_state, block);
    return 0;
}

skr_hash hash<hash<CGPURenderPipelineDescriptor>::ParameterBlock>::operator()(const hash<CGPURenderPipelineDescriptor>::ParameterBlock& val) const
{
    SkrZoneScopedN("hash<CGPURenderPipelineDescriptor::ParameterBlock>");

    return skr_hash_of(&val, sizeof(hash<CGPURenderPipelineDescriptor>::ParameterBlock));
}
} // namespace cgpux