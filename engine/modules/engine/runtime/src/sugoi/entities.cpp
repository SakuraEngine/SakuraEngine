#include "SkrRuntime/sugoi/entity_registry.hpp"
#include "SkrRuntime/sugoi/chunk.hpp"

#ifndef forloop
    #define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

sugoi_entity_debug_proxy_t dummy;
namespace sugoi
{

void EntityRegistry::reserve(EIndex size)
{
    entries.reserve(size);
}

void EntityRegistry::reserve_free_entries(EIndex size)
{
    freeEntries.reserve(size);
}

void EntityRegistry::reset()
{
    entries.clear();
    freeEntries.clear();
}

void EntityRegistry::reserve_external(EIndex size)
{
    SKR_ASSERT(entries.size() == 0 && freeEntries.size() == 0 && externalReserved == 0);
    entries.resize_unsafe(size);
    forloop (i, 0, size)
    {
        entries[i] = { nullptr, 0, kEntityTransientVersion };
    }
    externalReserved = size;
}

void EntityRegistry::shrink()
{
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
    entries.resize_unsafe(lastValid + 1);
    entries.shrink();
    freeEntries.remove_all_if([&](EIndex i) {
        return i > lastValid;
    });
}

void EntityRegistry::pack_entities(skr::Vector<EIndex>& out_map)
{
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
        freeEntries.resize_unsafe(fn - rn);
    }
    if (i == count)
        return;

    // new entities
    EIndex newId = static_cast<EIndex>(entries.size());
    {
        SkrZoneScopedN("ResizeEntries");
        entries.resize_unsafe(entries.size() + count - i);
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

    // build freelist in input order
    freeEntries.reserve(freeEntries.size() + count);

    forloop (i, 0, count)
    {
        auto id = e_id(dst[i]);
        Entry& freeData = entries[id];
        SKR_ASSERT(e_version(dst[i]) == freeData.version);
        if (id < externalReserved)
        {
            freeData = { nullptr, 0, kEntityTransientVersion };
            continue;
        }
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

    auto ents = (sugoi_entity_t*)view.chunk->get_entities() + view.start;
    memcpy(ents, src, view.count * sizeof(sugoi_entity_t));
    forloop (i, 0, view.count)
    {
        Entry& e = entries[e_id(src[i])];
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
    }
}

void EntityRegistry::fill_entities_external(const sugoi_chunk_view_t& view, const sugoi_entity_t* src)
{
    SkrZoneScopedN("sugoi_storage_t::fill_entities_external");

    auto ents = (sugoi_entity_t*)view.chunk->get_entities() + view.start;
    memcpy(ents, src, view.count * sizeof(sugoi_entity_t));
    forloop (i, 0, view.count)
    {
        Entry& e = entries[e_id(src[i])];
        SKR_ASSERT(e.chunk == nullptr);
        e.indexInChunk = view.start + i;
        e.chunk = view.chunk;
        e.version = e_version(src[i]);
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
        Entry& e = entries[e_id(toMove[i])];
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
        entries[e_id(toMove[i])].indexInChunk = view.start + i;
    std::memcpy((sugoi_entity_t*)view.chunk->get_entities() + view.start, toMove, view.count * sizeof(sugoi_entity_t));
}

void EntityRegistry::serialize(skr::ArchiveWrite* w)
{
    SkrZoneScopedN("EntityRegistry::serialize");

    skr::Archive::ObjectScope obj_scope(*w);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // write entries
    SKR_FAST_CHECK(w->key_value(u8"entries_count", (uint64_t)entries.size()), );

    // write free entries
    SKR_FAST_CHECK(w->key(u8"free_entries"), );
    {
        skr::Archive::ArrayScope arr_scope(*w);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write free entries count
        SKR_FAST_CHECK(w->array_size<uint64_t>((uint64_t)freeEntries.size()), );

        // write free entries
        if (w->is_structured())
        {
            for (auto freeEntry : freeEntries)
            {
                SKR_FAST_CHECK(w->value(freeEntry), );
            }
        }
        else
        { // optimize for binary writer
            SKR_FAST_CHECK(w->bytes(freeEntries.data(), freeEntries.size() * sizeof(EIndex)), );
        }
    }
}
void EntityRegistry::deserialize(skr::ArchiveRead* r)
{
    SkrZoneScopedN("EntityRegistry::deserialize");

    skr::Archive::ObjectScope obj_scope(*r);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // read entries
    uint64_t entries_count = 0;
    SKR_FAST_CHECK(r->key_value_required(u8"entries_count", entries_count), );

    // resize entries
    entries.resize_unsafe((EIndex)entries_count);

    // read free entries
    SKR_FAST_CHECK(r->key_required(u8"free_entries"), );
    {
        skr::Archive::ArrayScope arr_scope(*r);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // read free entries count
        uint64_t free_entries_count = 0;
        SKR_FAST_CHECK(r->array_size<uint64_t>(free_entries_count), );

        // resize free entries
        freeEntries.resize_unsafe((EIndex)free_entries_count);

        // read free entries
        if (r->is_structured())
        {
            for (uint64_t i = 0; i < free_entries_count; i++)
            {
                SKR_FAST_CHECK(r->value(freeEntries[i]), );
            }
        }
        else
        { // optimize for binary reader
            SKR_FAST_CHECK(r->bytes(freeEntries.data(), freeEntries.size() * sizeof(EIndex)), );
        }
    }
}
} // namespace sugoi