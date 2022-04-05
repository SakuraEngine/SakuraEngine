#pragma once
#include "api.h"

namespace cgpux
{

} // namespace cgpux

#include <EASTL/hash_set.h>
#include "utils/hash.h"
namespace eastl
{
template <>
struct hash<CGpuVertexLayout> {
    size_t operator()(const CGpuVertexLayout& val) const { return skr_hash(&val, sizeof(CGpuVertexLayout), 0); }
};
template <>
struct equal_to<CGpuVertexLayout> {
    size_t operator()(const CGpuVertexLayout& a, const CGpuVertexLayout& b) const
    {
        if (a.attribute_count != b.attribute_count) return false;
        for (uint32_t i = 0; i < a.attribute_count; i++)
        {
            const bool equal = (a.attributes[i].binding == b.attributes[i].binding) &&
                               (a.attributes[i].format == b.attributes[i].format) &&
                               (a.attributes[i].offset == b.attributes[i].offset) &&
                               (a.attributes[i].rate == b.attributes[i].rate) &&
                               (0 == strcmp(a.attributes[i].semantic_name, b.attributes[i].semantic_name));
            if (!equal) return false;
        }
        return true;
    }
};
} // namespace eastl