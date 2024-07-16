#pragma once
#include "SkrContainers/vector.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/set.hpp"

struct sugoi_storage_t;
namespace sugoi
{
template<class T>
struct cache_t;

struct archetype_t;
struct fixed_stack_t;
struct block_arena_t;
struct JobScheduler;
struct EntityRegistry;
using Storage = sugoi_storage_t;

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
} // namespace sugoi

struct sugoi_phase_alias_t {
    sugoi_type_index_t type;
    uint32_t phase;
};

struct SKR_RUNTIME_API sugoi_storage_t {
    struct Impl;
    using archetype_t = sugoi::archetype_t;

    sugoi_storage_t(Impl* pimpl);
    ~sugoi_storage_t();
    sugoi_group_t* get_group(const sugoi_entity_type_t& type);
    archetype_t* get_archetype(const sugoi_type_set_t& type);

    void allocate_unsafe(sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u);
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

    void destroy(const sugoi_chunk_view_t& view);
    void destroy(const sugoi_query_t* view);
    void destroy(const sugoi_query_t* view, sugoi_destroy_callback_t callback, void* u);
    void destroy(const sugoi_meta_filter_t& meta);

    void cast(const sugoi_chunk_view_t& view, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
    void cast(sugoi_group_t* srcGroup, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
    sugoi_group_t* cast(sugoi_group_t* group, const sugoi_delta_type_t& diff);

    sugoi_chunk_view_t entity_view(sugoi_entity_t e) const;
    void all(bool includeDisabled, bool includeDead, sugoi_view_callback_t callback, void* u);
    void batch(const sugoi_entity_t* ents, EIndex count, sugoi_view_callback_t callback, void* u);

    bool match_group(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, const sugoi_group_t* group);

    void filter_unsafe(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_view_callback_t callback, void* u);
    void filter_groups(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_group_callback_t callback, void* u);
    void filter_in_single_group(const sugoi_parameters_t* params, const sugoi_group_t* group, const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_custom_filter_callback_t customFilter, void* u1, sugoi_view_callback_t callback, void* u);
    // TODO: add this to scheduler API
    void filter_safe(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_view_callback_t callback, void* u);
    void query(const sugoi_query_t* query, sugoi_view_callback_t callback, void* u);
    void query_groups(const sugoi_query_t* query, sugoi_group_callback_t callback, void* u);
    
    sugoi_query_t* make_query(const sugoi_filter_t& filter, const sugoi_parameters_t& parameters);
    sugoi_query_t* make_query(const char8_t* desc);
    void destroy_query(sugoi_query_t* query);
    struct QueryBuilder;
    QueryBuilder new_query();

    void serialize_single(sugoi_entity_t e, SBinaryWriter* s);
    sugoi_entity_t deserialize_single(SBinaryReader* s);
    void serialize_type(const sugoi_entity_type_t& g, SBinaryWriter* s, bool keepMeta);
    sugoi_entity_type_t deserialize_type(sugoi::fixed_stack_t& stack, SBinaryReader* s, bool keepMeta);
    void serialize_prefab(sugoi_entity_t e, SBinaryWriter* s);
    void serialize_prefab(sugoi_entity_t* es, EIndex n, SBinaryWriter* s);
    sugoi_entity_t deserialize_prefab(SBinaryReader* s);
    void serialize_view(sugoi_group_t* group, sugoi_chunk_view_t& v, SBinaryWriter* s, SBinaryReader* ds, bool withEntities = true);
    void serialize(SBinaryWriter* s);
    void deserialize(SBinaryReader* s);

    void merge(sugoi_storage_t& src);
    sugoi_storage_t* clone();
    void reset();
    void validate_meta();
    void validate(sugoi_entity_set_t& meta);
    void defragment();
    void pack_entities();

    void make_alias(skr::StringView name, skr::StringView alias);
    
    // getters
    EIndex count(bool includeDisabled, bool includeDead);
    sugoi_timestamp_t timestamp() const;
    sugoi::EntityRegistry& getEntityRegistry();

    // TODO: REMOVE THESE
    friend struct sugoi::JobScheduler;
    sugoi::JobScheduler* getScheduler();
    void buildQueryOverloads();

protected:
    sugoi::block_arena_t& getArchetypeArena();
    Impl* pimpl = nullptr;

private:
    sugoi_chunk_view_t allocateView(sugoi_group_t* group, EIndex count);
    sugoi_chunk_view_t allocateViewStrict(sugoi_group_t* group, EIndex count);

    sugoi_group_t* constructGroup(const sugoi_entity_type_t& type);
    sugoi_group_t* cloneGroup(sugoi_group_t* src);
    sugoi_group_t* tryGetGroup(const sugoi_entity_type_t& type) const;
    void destructGroup(sugoi_group_t* group);

    archetype_t* constructArchetype(const sugoi_type_set_t& type);
    archetype_t* cloneArchetype(archetype_t* src);
    archetype_t* tryGetArchetype(const sugoi_type_set_t& type) const;

    void buildQueryCache(sugoi_query_t* query);
    void updateQueryCache(sugoi_group_t* group, bool isAdd);

    void structuralChange(sugoi_group_t* group, sugoi_chunk_t* chunk);
    void freeView(const sugoi_chunk_view_t& view);
    void castImpl(const sugoi_chunk_view_t& view, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
};

enum class QueryBuilderError
{
    UnknownError,
};

using QueryBuilderResult = skr::Expected<QueryBuilderError, sugoi_query_t*>;

struct sugoi_storage_t::QueryBuilder
{
    QueryBuilder(sugoi_storage_t* storage)
        : storage(storage)
    {

    }

    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadWriteAll(Idxs... idxs) { return All<false, Ts...>(idxs...); }
    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadAll(Idxs... idxs) { return All<true, Ts...>(idxs...); }
    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadWriteAny(Idxs... idxs) { return Any<false, Ts...>(idxs...); }
    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadAny(Idxs... idxs) { return Any<true, Ts...>(idxs...);  }

    template <typename...Ts, typename...Idxs>
    QueryBuilder& None(Idxs... idxs)
    {
        if constexpr (sizeof...(Ts) > 0)
        {
            (none.push_back(sugoi_id_of<Ts>::get()), ...);
            (types.push_back(sugoi_id_of<Ts>::get()), ...);
            (ops.push_back({0 * (int)sugoi_id_of<Ts>::get(), true, false, SOS_SEQ}), ...);
        }
        (none.push_back(idxs), ...);
        (types.push_back(idxs), ...);
        (ops.push_back({0 * (int)idxs, true, false, SOS_SEQ}), ...);
        return *this;
    }

    template <sugoi::EntityConcept...Ents>
    QueryBuilder& WithMetaEntity(Ents... ents)
    {
        (all_meta.push_back(ents), ...);
        return *this;
    }

    template <sugoi::EntityConcept...Ents>
    QueryBuilder& WithoutMetaEntity(Ents... ents)
    {
        (none_meta.push_back(ents), ...);
        return *this;
    }

    QueryBuilderResult commit() SKR_NOEXCEPT
    {
        sugoi_parameters_t parameters;
        parameters.types = types.data();
        parameters.accesses = ops.data();
        parameters.length = types.size();    

        SKR_DECLARE_ZERO(sugoi_filter_t, filter);
        all.sort([](auto a, auto b) { return a < b; });
        filter.all.data = all.data();
        filter.all.length = all.size();
        filter.none.data = none.data();
        filter.none.length = none.size();

        auto q = sugoiQ_create(storage, &filter, &parameters);
        if (q)
        {
            SKR_DECLARE_ZERO(sugoi_meta_filter_t, meta_filter);
            if (all_meta.size())
            {
                meta_filter.all_meta.data = all_meta.data();
                meta_filter.all_meta.length = all_meta.size();
            }
            if (none_meta.size())
            {
                meta_filter.none_meta.data = none_meta.data();
                meta_filter.none_meta.length = none_meta.size();
            }
            sugoiQ_set_meta(q, &meta_filter);
        }
        return q;
    }

private:
    template <bool readonly, typename...Ts, typename...Idxs>
    QueryBuilder& All(Idxs... idxs)
    {
        if constexpr (sizeof...(Ts) > 0)
        {
            (all.push_back(sugoi_id_of<Ts>::get()), ...);
            (types.push_back(sugoi_id_of<Ts>::get()), ...);
            (ops.push_back({0 * (int)sugoi_id_of<Ts>::get(), readonly, false, SOS_SEQ}), ...);
        }
        (all.push_back(idxs), ...);
        (types.push_back(idxs), ...);
        (ops.push_back({0 * (int)idxs, readonly, false, SOS_SEQ}), ...);
        return *this;
    }
    template <bool readonly, typename...Ts, typename...Idxs>
    QueryBuilder& Any(Idxs... idxs)
    {
        if constexpr (sizeof...(Ts) > 0)
        {
            (any.push_back(idxs), ...);
            (types.push_back(idxs), ...);
            (ops.push_back({0 * (int)idxs, readonly, false, SOS_SEQ}), ...);
        }
        (any.push_back(sugoi_id_of<Ts>::get()), ...);
        (types.push_back(sugoi_id_of<Ts>::get()), ...);
        (ops.push_back({0 * (int)sugoi_id_of<Ts>::get(), readonly, false, SOS_SEQ}), ...);
        return *this;
    }

    sugoi_storage_t* storage = nullptr;

    skr::InlineVector<sugoi_entity_t, 4> all_meta;
    skr::InlineVector<sugoi_entity_t, 4> none_meta;

    skr::InlineVector<sugoi_type_index_t, 8> all;
    skr::InlineVector<sugoi_type_index_t, 4> any;
    skr::InlineVector<sugoi_type_index_t, 4> none;
    skr::InlineVector<sugoi_type_index_t, 8> types;
    skr::InlineVector<sugoi_operation_t, 8> ops;
};

inline sugoi_storage_t::QueryBuilder sugoi_storage_t::new_query()
{
    return QueryBuilder(this);
}