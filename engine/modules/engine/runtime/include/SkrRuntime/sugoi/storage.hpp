#pragma once
#include "SkrRuntime/sugoi/entity_registry.hpp"
#include "SkrRuntime/sugoi/set.hpp"

struct sugoi_storage_t;
namespace sugoi
{
template <class T>
struct cache_t;

struct archetype_t;
struct fixed_stack_t;
struct block_arena_t;
struct EntityRegistry;
using Storage = sugoi_storage_t;

template <class T>
struct hasher
{
    size_t operator()(const T& value) const
    {
        return hash(value);
    }
};

template <class T>
struct equalto
{
    size_t operator()(const T& a, const T& b) const
    {
        return equal(a, b);
    }
};
} // namespace sugoi

struct sugoi_phase_alias_t
{
    sugoi_type_index_t type;
    uint32_t phase;
};

struct SKR_RUNTIME_API sugoi_storage_t
{
    struct Impl;
    using archetype_t = sugoi::archetype_t;

    sugoi_storage_t(Impl* pimpl);
    ~sugoi_storage_t();
    sugoi_group_t* get_group(const sugoi_entity_type_t& type);
    archetype_t* get_archetype(const sugoi_type_set_t& type);

    void allocate_unsafe(sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u);
    void allocate(sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u);
    void reserve_entities(EIndex count);
    void allocate_reserved_unsafe(sugoi_group_t* group, const sugoi_entity_t* ents, EIndex count, sugoi_view_callback_t callback, void* u);
    void allocate_reserved(sugoi_group_t* group, const sugoi_entity_t* ents, EIndex count, sugoi_view_callback_t callback, void* u);

    void get_linked_recursive(const sugoi_chunk_view_t& view, sugoi::cache_t<sugoi_entity_t>& result);
    void linked_to_prefab(const sugoi_entity_t* src, uint32_t size, bool keepExternal = true);
    void prefab_to_linked(const sugoi_entity_t* src, uint32_t size);
    void instantiate_prefab(const sugoi_entity_t* src, uint32_t size, uint32_t count, sugoi_view_callback_t callback, void* u);
    void instantiate(const sugoi_entity_t src, uint32_t count, sugoi_view_callback_t callback, void* u);
    void instantiate(const sugoi_entity_t* src, uint32_t n, uint32_t count, sugoi_view_callback_t callback, void* u);
    void instantiate(const sugoi_entity_t src, uint32_t count, sugoi_group_t* group, sugoi_view_callback_t callback, void* u);

    bool components_enabled(const sugoi_entity_t src, const sugoi_type_set_t& type);
    bool exist(sugoi_entity_t e) const noexcept;
    bool alive(sugoi_entity_t e) const noexcept;

    void destroy_entities(const sugoi_chunk_view_t& view);
    void destroy_entities(const sugoi_query_t* view);
    void destroy_entities(const sugoi_query_t* view, sugoi_destroy_callback_t callback, void* u);
    void destroy_entities(const sugoi_meta_filter_t& meta);
    void destroy_entities(const sugoi_entity_t* ents, EIndex n);

    void cast(const sugoi_chunk_view_t& view, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
    void cast(sugoi_group_t* srcGroup, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);
    sugoi_group_t* cast(sugoi_group_t* group, const sugoi_delta_type_t& diff);

    SUGOI_FORCEINLINE sugoi_chunk_view_t entity_view(sugoi_entity_t e) const
    {
        auto entry = entity_registry.try_get_entry(e);
        if (entry.value().version == sugoi::e_version(e)) [[likely]]
            return { entry.value().chunk, entry.value().indexInChunk, 1 };
        return { nullptr, 0, 1 };
    }

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
    bool match_entity(const sugoi_query_t* query, sugoi_entity_t entity);
    bool match_group(const sugoi_query_t* query, const sugoi_group_t* group);

    sugoi_query_t* make_query(const sugoi_filter_t& filter, const sugoi_parameters_t& parameters);
    sugoi_query_t* make_query(const char8_t* desc);
    void destroy_query(sugoi_query_t* query);

    // serialize
    void serialize_type(skr::ArchiveWrite* writer, const sugoi_entity_type_t& group, bool keep_meta);
    void serialize_view(skr::ArchiveWrite* writer, const sugoi_chunk_view_t& view, bool with_entities = true);
    void serialize(skr::ArchiveWrite* writer);
    void serialize_single(skr::ArchiveWrite* writer, sugoi_entity_t entity);

    // deserialize
    void deserialize_type(skr::ArchiveRead* reader, sugoi_entity_type_t& out_type, sugoi::fixed_stack_t& stack, bool keep_meta);
    void deserialize_view(skr::ArchiveRead* reader, sugoi_group_t* group, sugoi_chunk_view_t& out_view, bool with_entities = true);
    void deserialize(skr::ArchiveRead* reader);
    void deserialize_single(skr::ArchiveRead* reader, sugoi_entity_t& out_entity);

    void merge(sugoi_storage_t& src);
    sugoi_storage_t* clone();
    void reset();
    void validate_meta();
    void validate(sugoi_entity_set_t& meta);
    void defragment();
    void pack_entities();
    void redirect(sugoi_entity_t* ents, sugoi_entity_t* newEnts, EIndex n);

    void make_alias(skr::StringView name, skr::StringView alias);

    // getters
    EIndex count(bool includeDisabled, bool includeDead);
    sugoi_timestamp_t timestamp() const;
    sugoi::EntityRegistry& getEntityRegistry();
    void buildQueryOverloads();

protected:
    sugoi::block_arena_t& getArchetypeArena();
    Impl* pimpl = nullptr;
    sugoi::EntityRegistry entity_registry;

private:
    sugoi_chunk_view_t allocateView(sugoi_group_t* group, EIndex count);
    sugoi_chunk_view_t allocateViewStrict(sugoi_group_t* group, EIndex count);

    sugoi_group_t* allocateGroup(const sugoi_entity_type_t& type);
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