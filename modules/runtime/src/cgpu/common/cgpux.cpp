#include "string.h"
#include "common_utils.h"
#include "cgpu/cgpux.hpp"

void CGPUXBindTableValue::Initialize(const CGPUXBindTableLocation& loc, const CGPUDescriptorData& rhs)
{
    data = rhs;
    data.name = nullptr;
    data.binding = loc.binding;
    binded = false;
    resources.resize(data.count);
    offsets.resize(data.count);
    sizes.resize(data.count);
    for (uint32_t i = 0; i < data.count; i++)
    {
        resources[i] = data.ptrs[i];
        if (data.buffers_params.offsets)
        {
            offsets[i] = data.buffers_params.offsets[i];
        }
        if (data.buffers_params.sizes)
        {
            sizes[i] = data.buffers_params.sizes[i];
        }
    }
    data.ptrs = resources.data();
    data.buffers_params.offsets = offsets.data();
    data.buffers_params.sizes = sizes.data();
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
                    const_cast<uint32_t&>(pLocations[k].set) = res.set;
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
            const auto& set = sets[location.set];
            cgpu_update_descriptor_set(set, &location.value.data, 1);
            // late update value hash
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
    cgpu_free((void*)table);
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