#pragma once
#include "archetype.hpp"
#include "chunk.hpp"
#include "chunk_view.hpp"
#include "entity.hpp"
#include "ecs/dual.h"
#include "stack.hpp"
#include "type.hpp"
#include "query.hpp"

#include "arena.hpp"
#include "pool.hpp"
#include "ecs/entities.hpp"
#include "cache.hpp"
#include "type.hpp"
#include "set.hpp"
#include "containers/hashmap.hpp"
#include "EASTL/shared_ptr.h"
#include "task/task.hpp"
#include "utils/lazy.hpp"

namespace dual
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
//     using array_delta = eastl::vector<blob_ref_T<char>>;
//     struct vector_delta
//     {
//         size_t length;
//         array_delta content;
//     };
//     using component_delta = std::unique_ptr<array_delta[]>;
//     using buffer_delta = std::unique_ptr<eastl::vector<vector_delta>[]>;
//     struct slice_delta
//     {
//         dual_entity_type_t type;
//         blob_ref_T<guid_t> ents;
//         component_delta diffs;
//         buffer_delta bufferDiffs;
//     };
//     struct slice_data
//     {
//         dual_entity_type_t type;
//         intptr_t offset;
//     };
//     eastl::vector<slice_delta> changed;
//     eastl::vector<slice_data> created;
//     eastl::vector<dual_entity_t> destroyed;
//     eastl::vector<char> store;
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

struct query_cache_hasher {
    size_t operator()(const dual_filter_t& value) const
    {
        size_t result = hash(value.all);
        result = hash(value.any, result);
        result = hash(value.none, result);
        return result;
    }
};

struct query_cache_equal {
    bool operator()(const dual_filter_t& a, const dual_filter_t& b) const
    {
        return equal(a.all, b.all) && equal(a.any, b.any) && equal(a.none, b.none);
    }
};

struct scheduler_t;
} // namespace dual

struct dual_storage_t {
    using query_cache_t = dual::query_cache_t;
    using archetype_t = dual::archetype_t;
    using query_caches_t = skr::flat_hash_map<dual_filter_t, query_cache_t, dual::query_cache_hasher, dual::query_cache_equal>;
    using queries_t = eastl::vector<dual_query_t*>;
    using groups_t = skr::flat_hash_map<dual_entity_type_t, dual_group_t*, dual::hasher<dual_entity_type_t>, dual::equalto<dual_entity_type_t>>;
    using archetypes_t = skr::flat_hash_map<dual_type_set_t, archetype_t*, dual::hasher<dual_type_set_t>, dual::equalto<dual_type_set_t>>;
    archetypes_t archetypes;
    queries_t queries;
    bool queriesBuilt = false;
    groups_t groups;
    query_caches_t queryCaches;
    dual::block_arena_t archetypeArena;
    dual::block_arena_t queryBuildArena;
    dual::fixed_pool_t groupPool;
    dual::entity_registry_t entities;
    uint32_t timestamp;
    eastl::unique_ptr<uint32_t[]> typeTimestamps;
    mutable dual::scheduler_t* scheduler;
    mutable void* currentFiber;
    skr::task::counter_t counter;
    void* userdata;

    dual_storage_t();
    ~dual_storage_t();
    dual_group_t* construct_group(const dual_entity_type_t& type);
    archetype_t* construct_archetype(const dual_type_set_t& type);
    dual_group_t* get_group(const dual_entity_type_t& type);
    archetype_t* get_archetype(const dual_type_set_t& type);
    dual_group_t* try_get_group(const dual_entity_type_t& type) const;
    archetype_t* try_get_archetype(const dual_type_set_t& type) const;
    void destruct_group(dual_group_t* group);

    void allocate(dual_group_t* group, EIndex count, dual_view_callback_t callback, void* u);

    void get_linked_recursive(const dual_chunk_view_t& view, dual::cache_t<dual_entity_t>& result);
    void linked_to_prefab(const dual_entity_t* src, uint32_t size, bool keepExternal = true);
    void prefab_to_linked(const dual_entity_t* src, uint32_t size);
    void instantiate_prefab(const dual_entity_t* src, uint32_t size, uint32_t count, dual_view_callback_t callback, void* u);
    void instantiate(const dual_entity_t src, uint32_t count, dual_view_callback_t callback, void* u);
    void instantiate(const dual_entity_t* src, uint32_t n, uint32_t count, dual_view_callback_t callback, void* u);
    void instantiate(const dual_entity_t src, uint32_t count, dual_group_t* group, dual_view_callback_t callback, void* u);

    bool components_enabled(const dual_entity_t src, const dual_type_set_t& type);
    bool exist(dual_entity_t e) const noexcept;

    using batchmap_t = skr::flat_hash_map<dual_chunk_t*, dual_chunk_view_t>;
    void destroy(const dual_chunk_view_t& view);
    void destroy(const dual_meta_filter_t& meta);
    void free(const dual_chunk_view_t& view);

    void cast_impl(const dual_chunk_view_t& view, dual_group_t* group, dual_cast_callback_t callback, void* u);
    void cast(const dual_chunk_view_t& view, dual_group_t* group, dual_cast_callback_t callback, void* u);
    void cast(dual_group_t* srcGroup, dual_group_t* group, dual_cast_callback_t callback, void* u);
    dual_group_t* cast(dual_group_t* group, const dual_delta_type_t& diff);

    dual_chunk_view_t entity_view(dual_entity_t e) const;
    void batch(const dual_entity_t* ents, EIndex count, dual_view_callback_t callback, void* u);
    void query(const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_view_callback_t callback, void* u);
    void query_groups(const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_group_callback_t callback, void* u);
    bool match_group(const dual_filter_t& filter, const dual_meta_filter_t& meta, const dual_group_t* group);
    void query(const dual_group_t* group, const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_view_callback_t callback, void* u);
    dual_query_t* make_query(const dual_filter_t& filter, const dual_parameters_t& parameters);
    dual_query_t* make_query(const char* desc);
    void destroy_query(dual_query_t* query);
    void query(const dual_query_t* query, dual_view_callback_t callback, void* u);
    void query_groups(const dual_query_t* query, dual_group_callback_t callback, void* u);
    void build_queries();
    const query_cache_t& get_query_cache(const dual_filter_t& filter);
    void update_query_cache(dual_group_t* group, bool isAdd);

    void serialize_single(dual_entity_t e, skr_binary_writer_t* s);
    dual_entity_t deserialize_single(skr_binary_reader_t* s);
    void serialize_type(const dual_entity_type_t& g, skr_binary_writer_t* s, bool keepMeta);
    dual_entity_type_t deserialize_type(dual::fixed_stack_t& stack, skr_binary_reader_t* s, bool keepMeta);
    void serialize_prefab(dual_entity_t e, skr_binary_writer_t* s);
    void serialize_prefab(dual_entity_t* es, EIndex n, skr_binary_writer_t* s);
    dual_entity_t deserialize_prefab(skr_binary_reader_t* s);
    void serialize_view(dual_group_t* group, dual_chunk_view_t& v, skr_binary_writer_t* s, skr_binary_reader_t* ds, bool withEntities = true);
    void serialize(skr_binary_writer_t* s);
    void deserialize(skr_binary_reader_t* s);

    void merge(dual_storage_t& src);
    void reset();
    void validate_meta();
    void validate(dual_entity_set_t& meta);
    void defragment();
    void pack_entities();

    dual_chunk_view_t allocate_view(dual_group_t* group, EIndex count);
    dual_chunk_view_t allocate_view_strict(dual_group_t* group, EIndex count);
    void structural_change(dual_group_t* group, dual_chunk_t* chunk);
};