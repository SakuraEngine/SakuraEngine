#pragma once
#include "cgpu/cgpux.h"
#include "utils/hash.h"
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

template <>
struct hash<CGPUVertexLayout> {
    size_t operator()(const CGPUVertexLayout& val) const { return skr_hash(&val, sizeof(CGPUVertexLayout), 0); }
};
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
} // namespace cgpux