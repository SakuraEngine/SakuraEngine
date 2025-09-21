#pragma once
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/span.hpp"
#include "SkrContainersDef/optional.hpp"
#include "SkrRuntime/sugoi/sugoi.h"

namespace sugoi
{
struct SKR_RUNTIME_API EntityRegistry
{
    EntityRegistry() = default;
    EntityRegistry(const EntityRegistry& rhs);
    EntityRegistry& operator=(const EntityRegistry& rhs);

    void reserve(EIndex size);
    void reserve_free_entries(EIndex size);
    void reset();
    void shrink();
    void reserve_external(EIndex size);
    void sync_external(sugoi_entity_t* src, EIndex count);
    void pack_entities(skr::Vector<EIndex>& out_map);
    void new_entities(sugoi_entity_t* dst, EIndex count);
    void free_entities(const sugoi_entity_t* dst, EIndex count);
    void fill_entities(const sugoi_chunk_view_t& view);
    void fill_entities(const sugoi_chunk_view_t& view, const sugoi_entity_t* src);
    void fill_entities_external(const sugoi_chunk_view_t& view, const sugoi_entity_t* src);
    void free_entities(const sugoi_chunk_view_t& view);
    void move_entities(const sugoi_chunk_view_t& view, const sugoi_chunk_t* src, EIndex srcIndex);
    void move_entities(const sugoi_chunk_view_t& view, EIndex srcIndex);

    void serialize(skr::ArchiveWrite* w);
    void deserialize(skr::ArchiveRead* r);

    struct Entry
    {
        sugoi_chunk_t* chunk;
        uint32_t indexInChunk : 24;
        uint32_t version : 8;
    };

    template <typename F>
    void visit_entries(const F& f) const
    {
        skr::Span<const Entry> entries_view = entries;
        f(entries_view);
    }

    template <typename F>
    void visit_free_entries(const F& f) const
    {
        skr::Span<const EIndex> entries_view = freeEntries;
        f(entries_view);
    }

    SKR_FORCEINLINE skr::Optional<Entry> try_get_entry(sugoi_entity_t e) const
    {
        const auto id = e_id(e);
        if (id < entries.size()) [[likely]]
            return entries[id];
        return {};
    }

private:
    friend struct ::sugoi_storage_t;
    skr::Vector<Entry> entries;
    skr::Vector<EIndex> freeEntries;
    EIndex externalReserved = 0;
};

inline EntityRegistry::EntityRegistry(const EntityRegistry& rhs)
    : entries(rhs.entries)
    , freeEntries(rhs.freeEntries)
{
}

inline EntityRegistry& EntityRegistry::operator=(const EntityRegistry& rhs)
{
    entries = rhs.entries;
    freeEntries = rhs.freeEntries;
    return *this;
}

} // namespace sugoi