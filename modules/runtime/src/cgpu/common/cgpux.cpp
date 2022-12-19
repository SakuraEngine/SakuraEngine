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
    cgpu_free_aligned((void*)table);
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