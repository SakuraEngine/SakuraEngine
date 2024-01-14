#pragma once
#include "SkrBase/config.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrGraphics/cgpux.h"

namespace skr
{
namespace render_graph
{
// thread-unsafe descriptor set heap
// it's supposed to be resized only once at compile
// TODO: lifetime management (GC)
class BindTablePool
{
    friend class RenderGraphBackend;
public:
    void expand(const char8_t* keys, const CGPUXName* names, uint32_t names_count, size_t set_count = 1);
    CGPUXBindTableId pop(const char8_t* keys, const CGPUXName* names, uint32_t names_count);
    void reset();
    void destroy();

    BindTablePool(CGPURootSignatureId root_sig)
        : root_sig(root_sig)
    {
    }
protected:
    struct BindTablesBlock
    {
        skr::Vector<CGPUXBindTableId> bind_tables;
        uint32_t cursor = 0;
    };
    const CGPURootSignatureId root_sig;
    skr::FlatHashMap<skr::String, BindTablesBlock, skr::Hash<skr::String>> pool;
};

// TODO: lifetime management (GC)
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
        inline Key() = default;
        inline Key(const CGPUXBindTableId* tables, uint32_t count)
            : tables(tables, count)
        {
        }
        skr::InlineVector<CGPUXBindTableId, 3> tables;
    };
    static_assert(sizeof(Key) <= 8 * sizeof(size_t), "Key should be under single cacheline!");
    struct GuradedMergedBindTable
    {
        CGPUXMergedBindTableId table;
        bool update_gurad = false;
    };

public:
    MergedBindTablePool(CGPURootSignatureId root_sig)
        : root_sig(root_sig)
    {
    }
    CGPUXMergedBindTableId pop(const CGPUXBindTableId* tables, uint32_t count);
    void reset();
    void destroy();

protected:
    const CGPURootSignatureId root_sig;
    skr::FlatHashMap<Key, GuradedMergedBindTable, Key::hasher, Key::equal_to> pool;
};
} // namespace render_graph
} // namespace skr