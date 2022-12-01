#pragma once
#include "arena.hpp"
#include "type.hpp"
#include <EASTL/vector.h>
#include <EASTL/array.h>
#include <containers/hashmap.hpp>
#include "platform/guid.hpp"

namespace dual
{
using guid_t = dual_guid_t;

struct guid_compare_t {
    bool operator()(const guid_t& a, const guid_t& b) const
    {
        using value_type = eastl::array<char, 16>;
        return reinterpret_cast<const value_type&>(a) < reinterpret_cast<const value_type&>(b);
    }
};

using type_description_t = dual_type_description_t;

static constexpr type_index_t kDisableComponent = type_index_t(0, false, false, true, false);
static constexpr type_index_t kDeadComponent = type_index_t(1, false, false, true, false);
static constexpr type_index_t kLinkComponent = type_index_t(2, false, true, false, false);
static constexpr type_index_t kMaskComponent = type_index_t(3, false, false, false, false);
static constexpr type_index_t kGuidComponent = type_index_t(4, false, false, false, false);
static constexpr type_index_t kDirtyComponent = type_index_t(5, false, false, false, false);

struct type_registry_t {
    type_registry_t(pool_t& pool);
    type_registry_t(const type_registry_t&) = delete;
    eastl::vector<type_description_t> descriptions;
    eastl::vector<intptr_t> entityFields;
    block_arena_t nameArena;
    skr::flat_hash_map<skr::string_view, type_index_t, skr::hash<skr::string_view>> name2type;
    skr::flat_hash_map<guid_t, type_index_t, skr::guid::hash> guid2type;
    guid_func_t guid_func = nullptr;
    type_index_t register_type(const type_description_t& desc);
    type_index_t get_type(const guid_t& guid);
    type_index_t get_type(skr::string_view name);
    guid_t make_guid();
    static type_registry_t& get();
};
} // namespace dual