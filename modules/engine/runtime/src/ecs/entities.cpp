#include "SkrRT/ecs/entity_registry.hpp"
#include "./chunk.hpp"

#ifndef forloop
#define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

sugoi_entity_debug_proxy_t dummy;
namespace sugoi
{

void EntityRegistry::reserve(size_t size)
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });
    entries.reserve(size);
}

void EntityRegistry::reset()
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    entries.clear();
    freeEntries.clear();
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
    freeEntries.remove_all_if([&](EIndex i) {
        return i > lastValid;
    });
}

void EntityRegistry::pack_entities(skr::Vector<EIndex>& out_map)
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    out_map.resize_unsafe(entries.size());
    freeEntries.clear();
    EIndex j = 0;
    forloop (i, 0, entries.size())
    {
        if (entries[i].indexInChunk != 0)
        {
            out_map[i] = j;
            if (i != j)
                entries[j] = entries[i];
            j++;
        }
    }
}

void EntityRegistry::new_entities(sugoi_entity_t* dst, EIndex count)
{
    SkrZoneScopedN("sugoi_storage_t::new_entities");
    
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    EIndex i = 0;
    // recycle entities
    auto fn = (EIndex)freeEntries.size();
    auto rn = std::min((EIndex)freeEntries.size(), count);
    forloop (j, 0, rn)
    {
        auto id = freeEntries[fn - rn + j];
        dst[i] = e_version(id, entries[id].version);
        i++;
    }
    {
        SkrZoneScopedN("ResizeFreeEntries");
        freeEntries.resize_default(fn - rn);
    }
    if (i == count)
        return;
    // new entities
    EIndex newId = static_cast<EIndex>(entries.size());
    {
        SkrZoneScopedN("ResizeEntries");
        entries.resize_default(entries.size() + count - i);
    }
    {
        SkrZoneScopedN("InitializeEntryValues");
        while (i < count)
        {
            dst[i] = e_version(newId, entries[newId].version);
            i++;
            newId++;
        }
    }
}

void EntityRegistry::free_entities(const sugoi_entity_t* dst, EIndex count)
{
    SkrZoneScopedN("sugoi_storage_t::free_entities");
    
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    // build freelist in input order
    freeEntries.reserve(freeEntries.size() + count);

    forloop (i, 0, count)
    {
        auto id = e_id(dst[i]);
        Entry& freeData = entries[id];
        freeData = { nullptr, 0, e_inc_version(freeData.version) };
        freeEntries.add(id);
    }
}

void EntityRegistry::fill_entities(const sugoi_chunk_view_t& view)
{
    SkrZoneScopedN("sugoi_storage_t::fill_entities");

    auto ents = (sugoi_entity_t*)view.chunk->get_entities() + view.start;
    new_entities(ents, view.count);
    {
        mutex.lock();
        SKR_DEFER({ mutex.unlock(); });
        forloop (i, 0, view.count)
        {
            Entry& e = entries[e_id(ents[i])];
            e.indexInChunk = view.start + i;
            e.chunk = view.chunk;
        }
    }
}

void EntityRegistry::fill_entities(const sugoi_chunk_view_t& view, const sugoi_entity_t* src)
{
    SkrZoneScopedN("sugoi_storage_t::fill_entities");
    
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    auto ents = (sugoi_entity_t*)view.chunk->get_entities() + view.start;
    memcpy(ents, src, view.count * sizeof(sugoi_entity_t));
    forloop (i, 0, view.count)
    {
        Entry& e = entries[e_id(src[i])];
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
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    SKR_ASSERT(src != view.chunk || (srcIndex >= view.start + view.count));
    const sugoi_entity_t* toMove = src->get_entities() + srcIndex;
    forloop (i, 0, view.count)
    {
        Entry& e = entries[e_id(toMove[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
    std::memcpy((sugoi_entity_t*)view.chunk->get_entities() + view.start, toMove, view.count * sizeof(sugoi_entity_t));
}

void EntityRegistry::move_entities(const sugoi_chunk_view_t& view, EIndex srcIndex)
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    SKR_ASSERT(srcIndex >= view.start + view.count);
    const sugoi_entity_t* toMove = view.chunk->get_entities() + srcIndex;
    forloop (i, 0, view.count)
        entries[e_id(toMove[i])].indexInChunk = view.start + i;
    std::memcpy((sugoi_entity_t*)view.chunk->get_entities() + view.start, toMove, view.count * sizeof(sugoi_entity_t));
}

void EntityRegistry::serialize(SBinaryWriter* writer)
{
    visit_entries([&](const auto& entriesView){
        skr::bin_write(writer, (uint32_t)entriesView.size());
    });
    visit_free_entries([&](const auto& freeEntriesView){
        skr::bin_write(writer, (uint32_t)freeEntriesView.size());
        writer->write(freeEntriesView.data(), sizeof(EIndex) * static_cast<uint32_t>(freeEntriesView.size()));
    });
}

void EntityRegistry::deserialize(SBinaryReader* reader)
{
    mutex.lock();
    SKR_DEFER({ mutex.unlock(); });

    // empty storage expected
    SKR_ASSERT(entries.size() == 0);
    uint32_t size = 0;
    skr::bin_read(reader, size);
    entries.resize_default(size);
    uint32_t freeSize = 0;
    skr::bin_read(reader, freeSize);
    freeEntries.resize_default(freeSize);
    reader->read((void*)freeEntries.data(), sizeof(EIndex) * freeSize);
}

} // namespace sugoi