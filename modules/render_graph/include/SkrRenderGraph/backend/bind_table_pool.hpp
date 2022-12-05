#pragma once
#include <atomic>
#include "containers/hashmap.hpp"
#include "containers/string.hpp"
#include "containers/vector.hpp"
#include "cgpu/cgpux.h"

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
        eastl::vector<CGPUXBindTableId> bind_tables;
        uint32_t cursor = 0;
    };
    const CGPURootSignatureId root_sig;
    skr::flat_hash_map<skr::string, BindTablesBlock, skr::hash<skr::string>> pool;
};
} // namespace render_graph
} // namespace skr