#include "SkrRT/ecs/entity_registry.hpp"
#include "./utilities.hpp"
#include "./chunk.hpp"

sugoi_entity_debug_proxy_t dummy;
namespace sugoi
{

void EntityRegistry::reset()
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    entries.clear();
    freeEntities.clear();
}

void EntityRegistry::shrink()
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    if (entries.size() == 0)
        return;
    EIndex lastValid = (EIndex)(entries.size() - 1);
    while (lastValid != 0 && entries[lastValid].chunk == nullptr)
        --lastValid;
    if (entries[lastValid].indexInChunk == 0)
    {
        entries.clear();
        return;
    }
    entries.resize_default(lastValid + 1);
    entries.shrink();
    freeEntities.remove_all_if([&](EIndex i) {
        return i > lastValid;
    });
}

void EntityRegistry::new_entities(sugoi_entity_t* dst, EIndex count)
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    EIndex i = 0;
    // recycle entities

    auto fn = (EIndex)freeEntities.size();
    auto rn = std::min((EIndex)freeEntities.size(), count);
    forloop (j, 0, rn)
    {
        auto id = freeEntities[fn - rn + j];
        dst[i] = e_version(id, entries[id].version);
        i++;
    }
    freeEntities.resize_default(fn - rn);
    if (i == count)
        return;
    // new entities
    EIndex newId = static_cast<EIndex>(entries.size());
    entries.resize_default(entries.size() + count - i);
    while (i < count)
    {
        dst[i] = e_version(newId, entries[newId].version);
        i++;
        newId++;
    }
}

void EntityRegistry::free_entities(const sugoi_entity_t* dst, EIndex count)
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    // build freelist in input order
    freeEntities.reserve(freeEntities.size() + count);

    forloop (i, 0, count)
    {
        auto id = e_id(dst[i]);
        entry_t& freeData = entries[id];
        freeData = { nullptr, 0, e_inc_version(freeData.version) };
        freeEntities.add(id);
    }
}

void EntityRegistry::fill_entities(const sugoi_chunk_view_t& view)
{
    auto ents = (sugoi_entity_t*)view.chunk->get_entities() + view.start;
    new_entities(ents, view.count);
    forloop (i, 0, view.count)
    {
        entry_t& e = entries[e_id(ents[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
}

void EntityRegistry::fill_entities(const sugoi_chunk_view_t& view, const sugoi_entity_t* src)
{
    auto ents = (sugoi_entity_t*)view.chunk->get_entities() + view.start;
    memcpy(ents, src, view.count * sizeof(sugoi_entity_t));
    forloop (i, 0, view.count)
    {
        entry_t& e = entries[e_id(src[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
}

void EntityRegistry::free_entities(const sugoi_chunk_view_t& view)
{
    free_entities(view.chunk->get_entities() + view.start, view.count);
}

void EntityRegistry::move_entities(const sugoi_chunk_view_t& view, const sugoi_chunk_t* src, EIndex srcIndex)
{
    SKR_ASSERT(src != view.chunk || (srcIndex >= view.start + view.count));
    const sugoi_entity_t* toMove = src->get_entities() + srcIndex;
    forloop (i, 0, view.count)
    {
        entry_t& e = entries[e_id(toMove[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
    std::memcpy((sugoi_entity_t*)view.chunk->get_entities() + view.start, toMove, view.count * sizeof(sugoi_entity_t));
}

void EntityRegistry::move_entities(const sugoi_chunk_view_t& view, EIndex srcIndex)
{
    SKR_ASSERT(srcIndex >= view.start + view.count);
    const sugoi_entity_t* toMove = view.chunk->get_entities() + srcIndex;
    forloop (i, 0, view.count)
        entries[e_id(toMove[i])]
        .indexInChunk = view.start + i;
    std::memcpy((sugoi_entity_t*)view.chunk->get_entities() + view.start, toMove, view.count * sizeof(sugoi_entity_t));
}
} // namespace sugoi