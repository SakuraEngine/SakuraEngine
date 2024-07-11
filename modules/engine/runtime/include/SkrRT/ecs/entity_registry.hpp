#pragma once
#include "SkrOS/thread.h"
#include "SkrBase/atomic/atomic_mutex.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/span.hpp"
#include "SkrRT/ecs/sugoi.h"

namespace sugoi
{
struct SKR_RUNTIME_API EntityRegistry {
    EntityRegistry() = default;
    EntityRegistry(const EntityRegistry& rhs);
    EntityRegistry& operator=(const EntityRegistry& rhs);

    void reset();
    void shrink();
    void new_entities(sugoi_entity_t* dst, EIndex count);
    void free_entities(const sugoi_entity_t* dst, EIndex count);
    void fill_entities(const sugoi_chunk_view_t& view);
    void fill_entities(const sugoi_chunk_view_t& view, const sugoi_entity_t* src);
    void free_entities(const sugoi_chunk_view_t& view);
    void move_entities(const sugoi_chunk_view_t& view, const sugoi_chunk_t* src, EIndex srcIndex);
    void move_entities(const sugoi_chunk_view_t& view, EIndex srcIndex);

    struct entry_t {
        sugoi_chunk_t* chunk;
        uint32_t indexInChunk : 24;
        uint32_t version : 8;
    };

    template<typename F>
    void visit_entries(const F& f)
    {
        mutex.lock_shared();
        skr::span<const entry_t> entries_view = entries;
        f(entries_view);
        mutex.unlock_shared();
    }
    
    template<typename F>
    void visit_free_entities(const F& f)
    {
        mutex.lock_shared();
        skr::span<const EIndex> entries_view = freeEntities;
        f(entries_view);
        mutex.unlock_shared();
    }

// private:
    skr::Vector<entry_t> entries;
    skr::Vector<EIndex> freeEntities;
    skr::shared_atomic_mutex mutex;
};

inline EntityRegistry::EntityRegistry(const EntityRegistry& rhs)
    : entries(rhs.entries), freeEntities(rhs.freeEntities)
{

}

inline EntityRegistry& EntityRegistry::operator=(const EntityRegistry& rhs)
{
    entries = rhs.entries;
    freeEntities = rhs.freeEntities;
    return *this;
}

} // namespace sugoi