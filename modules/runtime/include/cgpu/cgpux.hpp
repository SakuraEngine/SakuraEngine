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
    RUNTIME_API size_t operator()(const CGPUVertexLayout& val) const;
};
static const CGPUVertexLayout kZeroCGPUVertexLayout = make_zeroed<CGPUVertexLayout>();

template <>
struct equal_to<CGPUVertexLayout> {
    RUNTIME_API size_t operator()(const CGPUVertexLayout& a, const CGPUVertexLayout& b) const;
};

template <>
struct equal_to<CGPUDescriptorData> {
    RUNTIME_API size_t operator()(const CGPUDescriptorData& a, const CGPUDescriptorData& b) const;
};

template <>
struct equal_to<CGPUShaderEntryDescriptor> {
    RUNTIME_API size_t operator()(const CGPUShaderEntryDescriptor& a, const CGPUShaderEntryDescriptor& b) const;
};

template <>
struct hash<CGPUShaderEntryDescriptor> {
    RUNTIME_API size_t operator()(const CGPUShaderEntryDescriptor& val) const;
};
static const CGPUShaderEntryDescriptor kZeroCGPUShaderEntryDescriptor = make_zeroed<CGPUShaderEntryDescriptor>();

template <>
struct equal_to<CGPUBlendStateDescriptor> {
    RUNTIME_API size_t operator()(const CGPUBlendStateDescriptor& a, const CGPUBlendStateDescriptor& b) const;

    uint32_t count = CGPU_MAX_MRT_COUNT;
};

template <>
struct hash<CGPUBlendStateDescriptor> {
    RUNTIME_API size_t operator()(const CGPUBlendStateDescriptor& val) const;
};
static const CGPUBlendStateDescriptor kZeroCGPUBlendStateDescriptor = make_zeroed<CGPUBlendStateDescriptor>();

template <>
struct equal_to<CGPUDepthStateDesc> {
    RUNTIME_API size_t operator()(const CGPUDepthStateDesc& a, const CGPUDepthStateDesc& b) const;
};

template <>
struct hash<CGPUDepthStateDesc> {
    RUNTIME_API size_t operator()(const CGPUDepthStateDesc& val) const;
};
static const CGPUDepthStateDesc kZeroCGPUDepthStateDesc = make_zeroed<CGPUDepthStateDesc>();

template <>
struct equal_to<CGPURasterizerStateDescriptor> {
    RUNTIME_API size_t operator()(const CGPURasterizerStateDescriptor& a, const CGPURasterizerStateDescriptor& b) const;
};

template <>
struct hash<CGPURasterizerStateDescriptor> {
    RUNTIME_API size_t operator()(const CGPURasterizerStateDescriptor& val) const;
};
static const CGPURasterizerStateDescriptor kZeroCGPURasterizerStateDescriptor = make_zeroed<CGPURasterizerStateDescriptor>();

template <>
struct equal_to<CGPURenderPipelineDescriptor> {
    RUNTIME_API size_t operator()(const CGPURenderPipelineDescriptor& a, const CGPURenderPipelineDescriptor& b) const;
};

template <>
struct hash<CGPURenderPipelineDescriptor> {
    struct ParameterBlock
    {
        RUNTIME_API ParameterBlock(const CGPURenderPipelineDescriptor& desc);

        ECGPUFormat color_formats[CGPU_MAX_MRT_COUNT];
        const uint32_t render_target_count;
        const ECGPUSampleCount sample_count;
        const uint32_t sample_quality;
        const ECGPUSlotMask color_resolve_disable_mask;
        const ECGPUFormat depth_stencil_format;
        const ECGPUPrimitiveTopology prim_topology;
        const bool enable_indirect_command;
    };

    RUNTIME_API size_t operator()(const CGPURenderPipelineDescriptor& a) const;
};

template <>
struct hash<hash<CGPURenderPipelineDescriptor>::ParameterBlock> {
    RUNTIME_API size_t operator()(const hash<CGPURenderPipelineDescriptor>::ParameterBlock& val) const;
};
} // namespace cgpux