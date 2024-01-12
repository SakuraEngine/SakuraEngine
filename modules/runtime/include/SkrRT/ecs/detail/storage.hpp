#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/set.hpp"
#include "SkrRT/ecs/entities.hpp"
#include "SkrContainers/hashmap.hpp"
#include "SkrRT/async/fib_task.hpp"

#include <memory>
#include "stack.hpp"
#include "query.hpp"
#include "archetype.hpp"
#include "arena.hpp"
#include "pool.hpp"
#include "cache.hpp"

namespace sugoi
{
extern thread_local fixed_stack_t localStack;

// template<class T>
// struct blob_ref_T
// {
//     intptr_t offset;
//     size_t count;
//     T& resolve(void* store) { return *(T*)((char*)store + offset); }
// };
// struct storage_delta_t
// {
//     using array_delta = skr::stl_vector<blob_ref_T<char>>;
//     struct vector_delta
//     {
//         size_t length;
//         array_delta content;
//     };
//     using component_delta = std::unique_ptr<array_delta[]>;
//     using buffer_delta = std::unique_ptr<skr::stl_vector<vector_delta>[]>;
//     struct slice_delta
//     {
//         sugoi_entity_type_t type;
//         blob_ref_T<guid_t> ents;
//         component_delta diffs;
//         buffer_delta bufferDiffs;
//     };
//     struct slice_data
//     {
//         sugoi_entity_type_t type;
//         intptr_t offset;
//     };
//     skr::stl_vector<slice_delta> changed;
//     skr::stl_vector<slice_data> created;
//     skr::stl_vector<sugoi_entity_t> destroyed;
//     skr::stl_vector<char> store;
// };

template <class T>
struct hasher {
    size_t operator()(const T& value) const
    {
        return hash(value);
    }
};

template <class T>
struct equalto {
    size_t operator()(const T& a, const T& b) const
    {
        return equal(a, b);
    }
};

struct scheduler_t;
} // namespace sugoi

struct sugoi_phase_alias_t {
    sugoi_type_index_t type;
    uint32_t phase;
};

struct sugoi_storage_t {
    using archetype_t = sugoi::archetype_t;
    using queries_t = skr::stl_vector<sugoi_query_t*>;
    using groups_t = skr::FlatHashMap<sugoi_entity_type_t, sugoi_group_t*, sugoi::hasher<sugoi_entity_type_t>, sugoi::equalto<sugoi_entity_type_t>>;
    using archetypes_t = skr::FlatHashMap<sugoi_type_set_t, archetype_t*, sugoi::hasher<sugoi_type_set_t>, sugoi::equalto<sugoi_type_set_t>>;
    using phase_alias_t = skr::FlatHashMap<skr::StringView, sugoi_phase_alias_t, skr::Hash<skr::StringView>>;
    archetypes_t archetypes;
    queries_t queries;
    phase_alias_t aliases;
    uint32_t aliasCount = 0;
    sugoi::phase_entry** phases = nullptr;
    uint32_t phaseCount = 0;
    bool queriesBuilt = false;
    groups_t groups;
    sugoi::block_arena_t archetypeArena;
    sugoi::block_arena_t queryBuildArena;
    sugoi::fixed_pool_t groupPool;
    sugoi::entity_registry_t entities;
    uint32_t timestamp;
    std::unique_ptr<uint32_t[]> typeTimestamps;
    mutable sugoi::scheduler_t* scheduler;
    mutable void* currentFiber;
    skr::task::counter_t counter;
    void* userdata;

    sugoi_storage_t();
    ~sugoi_storage_t();
    sugoi_group_t* construct_group(const sugoi_entity_type_t& type);
    archetype_t* construct_archetype(const sugoi_type_set_t& type);
    sugoi_group_t* get_group(const sugoi_entity_type_t& type);
    archetype_t* get_archetype(const sugoi_type_set_t& type);
    sugoi_group_t* try_get_group(const sugoi_entity_type_t& type) const;
    archetype_t* try_get_archetype(const sugoi_type_set_t& type) const;
    void destruct_group(sugoi_group_t* group);

    void allocate(sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u);

    void get_linked_recursive(const sugoi_chunk_view_t& view, sugoi::cache_t<sugoi_entity_t>& result);
    void linked_to_prefab(const sugoi_entity_t* src, uint32_t size, bool keepExternal = true);
    void prefab_to_linked(const sugoi_entity_t* src, uint32_t size);
    void instantiate_prefab(const sugoi_entity_t* src, uint32_t size, uint32_t count, sugoi_view_callback_t callback, void* u);
    void instantiate(const sugoi_entity_t src, uint32_t count, sugoi_view_callback_t callback, void* u);
    void instantiate(const sugoi_entity_t* src, uint32_t n, uint32_t count, sugoi_view_callback_t callback, void* u);
    void instantiate(const sugoi_entity_t src, uint32_t count, sugoi_group_t* group, sugoi_view_callback_t callback, void* u);

    bool components_enabled(const sugoi_entity_t src, const sugoi_type_set_t& type);
    bool exist(sugoi_entity_t e) const noexcept;

    using batchmap_t = skr::FlatHashMap<sugoi_chunk_t*, sugoi_chunk_view_t>;
    void destroy(const sugoi_chunk_view_t& view);
    void destroy(const sugoi_meta_filter_t& meta);
    void free(const sugoi_chunk_view_t& view);

    void cast_impl(const sugoi_chunk_view_t& view, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
    void cast(const sugoi_chunk_view_t& view, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
    void cast(sugoi_group_t* srcGroup, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
    sugoi_group_t* cast(sugoi_group_t* group, const sugoi_delta_type_t& diff);

    sugoi_chunk_view_t entity_view(sugoi_entity_t e) const;
    void batch(const sugoi_entity_t* ents, EIndex count, sugoi_view_callback_t callback, void* u);
    void query(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_view_callback_t callback, void* u);
    void query_groups(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_group_callback_t callback, void* u);
    bool match_group(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, const sugoi_group_t* group);
    void query(const sugoi_group_t* group, const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_view_callback_t callback, void* u);
    sugoi_query_t* make_query(const sugoi_filter_t& filter, const sugoi_parameters_t& parameters);
    sugoi_query_t* make_query(const char* desc);
    void destroy_query(sugoi_query_t* query);
    void query(const sugoi_query_t* query, sugoi_view_callback_t callback, void* u);
    void query_groups(const sugoi_query_t* query, sugoi_group_callback_t callback, void* u);
    void build_queries();
    void build_query_cache(sugoi_query_t* query);
    void update_query_cache(sugoi_group_t* group, bool isAdd);

    void serialize_single(sugoi_entity_t e, skr_binary_writer_t* s);
    sugoi_entity_t deserialize_single(skr_binary_reader_t* s);
    void serialize_type(const sugoi_entity_type_t& g, skr_binary_writer_t* s, bool keepMeta);
    sugoi_entity_type_t deserialize_type(sugoi::fixed_stack_t& stack, skr_binary_reader_t* s, bool keepMeta);
    void serialize_prefab(sugoi_entity_t e, skr_binary_writer_t* s);
    void serialize_prefab(sugoi_entity_t* es, EIndex n, skr_binary_writer_t* s);
    sugoi_entity_t deserialize_prefab(skr_binary_reader_t* s);
    void serialize_view(sugoi_group_t* group, sugoi_chunk_view_t& v, skr_binary_writer_t* s, skr_binary_reader_t* ds, bool withEntities = true);
    void serialize(skr_binary_writer_t* s);
    void deserialize(skr_binary_reader_t* s);

    void merge(sugoi_storage_t& src);
    archetype_t* clone_archetype(archetype_t* src);
    sugoi_group_t* clone_group(sugoi_group_t* src);
    sugoi_storage_t* clone();
    void reset();
    void validate_meta();
    void validate(sugoi_entity_set_t& meta);
    void defragment();
    void pack_entities();

    sugoi_chunk_view_t allocate_view(sugoi_group_t* group, EIndex count);
    sugoi_chunk_view_t allocate_view_strict(sugoi_group_t* group, EIndex count);
    void structural_change(sugoi_group_t* group, sugoi_chunk_t* chunk);

    void make_alias(skr::StringView name, skr::StringView alias);
};