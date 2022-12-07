#pragma once
#include "SkrRenderGraph/module.configure.h"
#include "containers/hashmap.hpp"
#include "containers/string.hpp"
#include "containers/vector.hpp"
#include <EASTL/fixed_vector.h>
#include "cgpu/cgpux.hpp"

namespace skr
{
namespace render_graph
{
// thread-unsafe descriptor set heap
// it's supposed to be resized only once at compile
class BindTablePool
{
    friend class RenderGraphBackend;
public:
    void expand(const char* keys, const CGPUXName* names, uint32_t names_count, size_t set_count = 1);
    CGPUXBindTableId pop(const char* keys, const CGPUXName* names, uint32_t names_count);
    void reset();
    void destroy();

    BindTablePool(CGPURootSignatureId root_sig)
        : root_sig(root_sig)
    {
    }
protected:
    struct BindTablesBlock
    {
        skr::vector<CGPUXBindTableId> bind_tables;
        uint32_t cursor = 0;
    };
    const CGPURootSignatureId root_sig;
    skr::flat_hash_map<skr::string, BindTablesBlock, skr::hash<skr::string>> pool;
};

class MergedBindTablePool
{
    struct Key
    {
        struct View
        {
            const CGPUXBindTableId* tables;
            uint32_t count;
        };
        struct hasher
        {
            using is_transparent = void;

            SKR_RENDER_GRAPH_API size_t operator() (const Key& val) const;
            SKR_RENDER_GRAPH_API size_t operator() (const View& val) const;
        };
        struct equal_to
        {
            using is_transparent = void;

            SKR_RENDER_GRAPH_API size_t operator()(const Key& lhs, const Key& rhs) const;
            SKR_RENDER_GRAPH_API size_t operator()(const Key& lhs, const View& other) const;
        };
        eastl::fixed_vector<CGPUXBindTableId, 3> tables;
    };
    static_assert(sizeof(Key) <= 8 * sizeof(size_t), "Key should be under single cacheline!");

    MergedBindTablePool(CGPURootSignatureId root_sig)
        : root_sig(root_sig)
    {
    }
    
    CGPUXMergedBindTableId pop(const CGPUXBindTableId* tables, uint32_t count);
    void reset();
    void destroy();

protected:
    const CGPURootSignatureId root_sig;
    skr::flat_hash_map<Key, CGPUXMergedBindTableId, Key::hasher, Key::equal_to> pool;
};
} // namespace render_graph
} // namespace skr