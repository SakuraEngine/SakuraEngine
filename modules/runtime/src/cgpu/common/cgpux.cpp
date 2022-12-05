#include "string.h"
#include "common_utils.h"
#include "cgpu/cgpux.hpp"

CGPUXBindTableId CGPUXBindTable::Create(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc) SKR_NOEXCEPT
{
    auto rs = desc->root_signature;
    const auto hashes_size = desc->names_count * sizeof(uint64_t);
    const auto locations_size = desc->names_count * sizeof(CGPUXBindTable::Location);
    const auto sets_size = rs->table_count * sizeof(CGPUDescriptorSetId);
    CGPUXBindTable* table = (CGPUXBindTable*)cgpu_calloc(1, sizeof(CGPUXBindTable) + hashes_size + locations_size + sets_size);
    uint64_t* pHashes = (uint64_t*)(table + 1);
    CGPUXBindTable::Location* pLocations = (CGPUXBindTable::Location*)(pHashes + desc->names_count);
    CGPUDescriptorSetId* pSets = (CGPUDescriptorSetId*)(pLocations + desc->names_count);
    table->names_count = desc->names_count;
    table->name_hashes = pHashes;
    table->name_locations = pLocations;
    table->sets_count = rs->table_count;
    table->sets = pSets;
    // calculate hashes for each name
    for (uint32_t i = 0; i < desc->names_count; i++)
    {
        const auto name = desc->names[i];
        pHashes[i] = cgpu_hash(name, strlen(name), 0);
    }
    // calculate active sets
    for (uint32_t setIdx = 0; setIdx < rs->table_count; setIdx++)
    {
        for (uint32_t bindIdx = 0; bindIdx < rs->tables[setIdx].resources_count; bindIdx++)
        {
            const auto res = rs->tables[setIdx].resources[bindIdx];
            const auto hash = cgpu_hash(res.name, strlen(res.name), 0);
            for (uint32_t k = 0; k < desc->names_count; k++)
            {
                if (hash == pHashes[k])
                {
                    pLocations[k].set = setIdx;
                    pLocations[k].binding = bindIdx;
                    CGPUDescriptorSetDescriptor setDesc = {};
                    setDesc.root_signature = desc->root_signature;
                    setDesc.set_index = setIdx;
                    if (!pSets[setIdx]) pSets[setIdx] = cgpu_create_descriptor_set(device, &setDesc);
                    break;
                }
            }
        }
    }
    return table;
}

void CGPUXBindTable::Override(const struct CGPUXBindTable* rhs) SKR_NOEXCEPT
{

}


void CGPUXBindTable::Update(const struct CGPUDescriptorData* datas, uint32_t count) SKR_NOEXCEPT
{

}

void CGPUXBindTable::Free(CGPUXBindTableId table) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < table->sets_count; i++)
    {
        cgpu_free_descriptor_set(table->sets[i]);
    }
    cgpu_free((void*)table);
}

CGPUXBindTableId cgpux_create_bind_table(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc)
{
    return CGPUXBindTable::Create(device, desc);
}

void cgpux_free_bind_table(CGPUXBindTableId bind_table)
{
    CGPUXBindTable::Free(bind_table);
}

void cgpux_bind_table_update(CGPUXBindTableId table, const struct CGPUDescriptorData* datas, uint32_t count)
{
    return ((CGPUXBindTable*)table)->Update(datas, count);
}

void cgpux_bind_table_override(CGPUXBindTableId table, CGPUXBindTableId another)
{
    return ((CGPUXBindTable*)table)->Override(another);
}