#include "entities.hpp"
#include "chunk_view.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include <cstring>
#ifndef forloop
    #define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

namespace dual
{
void entity_registry_t::reset()
{
    entries.clear();
    freeEntries.clear();
    size = 0;
}

void entity_registry_t::shrink()
{
    if (entries.size() == 0)
        return;
    EIndex lastValid = entries.size() - 1;
    while (lastValid != 0 && entries[lastValid].chunk == nullptr)
        --lastValid;
    if (entries[lastValid].indexInChunk == 0)
    {
        entries.clear();
        return;
    }
    entries.resize(lastValid + 1);
    entries.shrink_to_fit();
    freeEntries.erase(std::remove_if(freeEntries.begin(), freeEntries.end(), [&](EIndex i) {
        return i > lastValid;
    }),
        freeEntries.end());
}

void entity_registry_t::new_entities(dual_entity_t* dst, EIndex count)
{
    size += count;
    EIndex i = 0;
    // recycle entities

    auto fn = (EIndex)freeEntries.size();
    auto rn = std::min((EIndex)freeEntries.size(), count);
    forloop(j, 0, rn)
    {
        auto id = freeEntries[fn - rn + j];
        dst[i] = e_version(id, entries[id].version);
        i++;
    }
    freeEntries.resize(fn - rn);
    if (i == count)
        return;
    // new entities
    EIndex newId = static_cast<EIndex>(entries.size());
    entries.resize(entries.size() + count - i);
    while (i < count)
    {
        dst[i] = e_version(newId, entries[newId].version);
        i++;
        newId++;
    }
}

void entity_registry_t::free_entities(const dual_entity_t* dst, EIndex count)
{
    size -= count;
    // build freelist in input order
    freeEntries.reserve(freeEntries.size() + count);

    forloop(i, 0, count)
    {
        auto id = e_id(dst[i]);
        entry_t& freeData = entries[id];
        freeData = { nullptr, 0, e_inc_version(freeData.version) };
        freeEntries.push_back(id);
    }
}

void entity_registry_t::fill_entities(const dual_chunk_view_t& view)
{
    auto ents = (dual_entity_t*)view.chunk->get_entities() + view.start;
    new_entities(ents, view.count);
    forloop(i, 0, view.count)
    {
        entry_t& e = entries[e_id(ents[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
}

void entity_registry_t::fill_entities(const dual_chunk_view_t& view, const dual_entity_t* src)
{
    auto ents = (dual_entity_t*)view.chunk->get_entities() + view.start;
    memcpy(ents, src, view.count * sizeof(dual_entity_t));
    forloop(i, 0, view.count)
    {
        entry_t& e = entries[e_id(src[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
}

void entity_registry_t::free_entities(const dual_chunk_view_t& view)
{
    free_entities(view.chunk->get_entities() + view.start, view.count);
}

void entity_registry_t::move_entities(const dual_chunk_view_t& view, const dual_chunk_t* src, EIndex srcIndex)
{
    const dual_entity_t* toMove = src->get_entities() + srcIndex;
    forloop(i, 0, view.count)
    {
        entry_t& e = entries[e_id(toMove[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
    std::memcpy((dual_entity_t*)view.chunk->get_entities() + view.start, toMove, view.count * sizeof(dual_entity_t));
}

void entity_registry_t::move_entities(const dual_chunk_view_t& view, EIndex srcIndex)
{
    const dual_entity_t* toMove = view.chunk->get_entities() + srcIndex;
    forloop(i, 0, view.count)
        entries[e_id(toMove[i])]
            .indexInChunk = view.start + i;
    memcpy((dual_entity_t*)view.chunk->get_entities() + view.start, toMove, view.count * sizeof(dual_entity_t));
}
} // namespace dual