#include "SkrBase/atomic/atomic.h"
#include "SkrTask/parallel_for.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/set.hpp"
#include "SkrRT/ecs/type_registry.hpp"

#include "./query.hpp"
#include "./impl/storage.hpp"
#include "./pool.hpp"
#include "./mask.hpp"
#include "./chunk_view.hpp"
#include "./scheduler.hpp"
#include "./iterator_ref.hpp"
#include "./utilities.hpp"

sugoi_storage_t::Impl::Impl()
    : archetypeArena(sugoi::get_default_pool())
    , groupPool(sugoi::kGroupBlockSize, sugoi::kGroupBlockCount)
    , scheduler(nullptr)
    , queryPhaseArena(sugoi::get_default_pool())
{
}

sugoi_storage_t::sugoi_storage_t(sugoi_storage_t::Impl* pimpl)
    : pimpl(pimpl)
{

}

sugoi_storage_t::~sugoi_storage_t()
{
    if (pimpl->scheduler)
        pimpl->scheduler->remove_storage(this);
    pimpl->scheduler = nullptr;
    for (auto q : pimpl->queries)
        sakura_free((void*)q);
    reset();
}

void sugoi_storage_t::reset()
{
    for (auto iter : pimpl->groups)
        iter.second->clear();
}

void sugoi_storage_t::allocate_unsafe(sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    while (count != 0)
    {
        sugoi_chunk_view_t v = allocateView(group, count);
        pimpl->entity_registry.fill_entities(v);
        construct_view(v);
        count -= v.count;
        if (callback)
            callback(u, &v);
    }
}

void sugoi_storage_t::allocate(sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_archetype(group->archetype);
    }
    allocate_unsafe(group, count, callback, u);
}

sugoi_chunk_view_t sugoi_storage_t::allocateView(sugoi_group_t* group, EIndex count)
{
    sugoi_chunk_t* freeChunk = group->get_first_free_chunk();
    if (freeChunk == nullptr)
        freeChunk = group->new_chunk(count);
    EIndex start     = freeChunk->count;
    EIndex allocated = std::min(count, freeChunk->get_capacity() - start);
    group->resize_chunk(freeChunk, start + allocated);
    structuralChange(group, freeChunk);
    return { freeChunk, start, allocated };
}

sugoi_chunk_view_t sugoi_storage_t::allocateViewStrict(sugoi_group_t* group, EIndex count)
{
    sugoi_chunk_t* freeChunk = nullptr;
    for (auto i = group->firstFree; i < (uint32_t)group->chunks.size(); ++i)
    {
        auto chunk = group->chunks[i];
        if (chunk->count + count <= chunk->get_capacity())
        {
            freeChunk = chunk;
            break;
        }
    }
    if (freeChunk == nullptr)
        freeChunk = group->new_chunk(count);
    EIndex start = freeChunk->count;
    group->resize_chunk(freeChunk, start + count);
    structuralChange(group, freeChunk);
    return { freeChunk, start, count };
}

void sugoi_storage_t::destroy(const sugoi_chunk_view_t& view)
{
    using namespace sugoi;
    auto group = view.chunk->group;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_archetype(group->archetype);
    }
    SKR_ASSERT(!group->isDead);
    auto dead = group->dead;
    if (dead)
        cast(view, dead, nullptr, nullptr);
    else
    {
        pimpl->entity_registry.free_entities(view);
        destruct_view(view);
        freeView(view);
    }
}

void sugoi_storage_t::structuralChange(sugoi_group_t* group, sugoi_chunk_t* chunk)
{
    // todo: timestamp
}

void sugoi_storage_t::linked_to_prefab(const sugoi_entity_t* src, uint32_t size, bool keepExternal)
{
    using namespace sugoi;
    struct mapper_t {
        const sugoi_entity_t* source;
        uint32_t              count;
        bool                  keepExternal;
        void                  move() {}
        void                  reset() {}

        void map(sugoi_entity_t& ent)
        {
            forloop (i, 0, count)
                if (ent == source[i])
                {
                    ent = e_make_transient(i);
                    return;
                }
            if (!keepExternal) // todo: use guid for persistent reference?
                ent = kEntityNull;
        }
    } m;
    m.count        = size;
    m.source       = src;
    m.keepExternal = keepExternal;
    forloop (i, 0, size)
        iterator_ref_view(entity_view(src[i]), m);
}

void sugoi_storage_t::prefab_to_linked(const sugoi_entity_t* src, uint32_t size)
{
    using namespace sugoi;
    struct mapper_t {
        const sugoi_entity_t* source;
        uint32_t              count;
        void                  move() {}
        void                  reset() {}

        void map(sugoi_entity_t& ent)
        {
            if (e_id(ent) > count || !e_transient(ent))
                return;
            ent = source[e_id(ent)];
        }
    } m;
    m.count  = size;
    m.source = src;
    forloop (i, 0, size)
        iterator_ref_view(entity_view(src[i]), m);
}

void sugoi_storage_t::instantiate_prefab(const sugoi_entity_t* src, uint32_t size, uint32_t count, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    skr::stl_vector<sugoi_entity_t> ents;
    ents.resize(count * size);
    pimpl->entity_registry.new_entities(ents.data(), (EIndex)ents.size());
    struct mapper_t {
        sugoi_entity_t* base;
        sugoi_entity_t* curr;
        uint32_t        size;
        void            move() { curr += size; }
        void            reset() { curr = base; }
        void            map(sugoi_entity_t& ent)
        {
            if (e_id(ent) > size || !e_transient(ent))
                return;
            ent = curr[e_id(ent)];
        }
    } m;
    m.size = size;
    skr::stl_vector<sugoi_entity_t> localEnts;
    localEnts.resize(count);
    forloop (i, 0, size)
    {
        forloop (j, 0, count)
            localEnts[j] = ents[j * size + i];
        auto view       = entity_view(src[i]);
        auto group      = view.chunk->group->cloned;
        auto localCount = 0;
        while (localCount != count)
        {
            sugoi_chunk_view_t v = allocateView(group, count - localCount);
            pimpl->entity_registry.fill_entities(v, localEnts.data() + localCount);
            duplicate_view(v, view.chunk, view.start);
            m.base = m.curr = ents.data() + localCount * size;
            localCount += v.count;
            iterator_ref_view(v, m);
            if (callback)
                callback(u, &v);
        }
    }
}

void sugoi_storage_t::instantiate(const sugoi_entity_t src, uint32_t count, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    auto view  = entity_view(src);
    auto group = view.chunk->group->cloned;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_archetype(group->archetype);
    }
    while (count != 0)
    {
        sugoi_chunk_view_t v = allocateView(group, count);
        pimpl->entity_registry.fill_entities(v);
        duplicate_view(v, view.chunk, view.start);
        count -= v.count;
        if (callback)
            callback(u, &v);
    }
}

void sugoi_storage_t::instantiate(const sugoi_entity_t src, uint32_t count, sugoi_group_t* group, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    auto view = entity_view(src);
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_archetype(group->archetype);
    }
    while (count != 0)
    {
        sugoi_chunk_view_t v = allocateView(group, count);
        pimpl->entity_registry.fill_entities(v);
        duplicate_view(v, view.chunk, view.start);
        count -= v.count;
        if (callback)
            callback(u, &v);
    }
}

void sugoi_storage_t::instantiate(const sugoi_entity_t* src, uint32_t n, uint32_t count, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        forloop (i, 0, n)
        {
            auto view = entity_view(src[i]);
            pimpl->scheduler->sync_archetype(view.chunk->structure); // data is modified by linked to prefab
            pimpl->scheduler->sync_archetype(view.chunk->group->cloned->archetype);
        }
    }
    linked_to_prefab(src, n);
    instantiate_prefab(src, n, count, callback, u);
    prefab_to_linked(src, n);
}

sugoi_chunk_view_t sugoi_storage_t::entity_view(sugoi_entity_t e) const
{
    using namespace sugoi;
    SKR_ASSERT(e_id(e) < pimpl->entity_registry.entries.size());
    auto& entry = pimpl->entity_registry.entries[e_id(e)];
    if (entry.version == e_version(e))
        return { entry.chunk, entry.indexInChunk, 1 };
    return { nullptr, 0, 1 };
}

void sugoi_storage_t::all(bool includeDisabled, bool includeDead, sugoi_view_callback_t callback, void* u)
{
    for (auto& pair : pimpl->groups)
    {
        auto group = pair.second;
        if (group->isDead && !includeDead)
            continue;
        if (group->disabled && !includeDisabled)
            continue;
        for (auto c : group->chunks)
        {
            sugoi_chunk_view_t view{ c, 0, c->count };
            callback(u, &view);
        }
    }
}

bool sugoi_storage_t::components_enabled(const sugoi_entity_t src, const sugoi_type_set_t& type)
{
    using namespace sugoi;
    auto view = entity_view(src);
    auto mask = (mask_t*)sugoiV_get_owned_ro(&view, kMaskComponent);
    if (!mask)
        return true;
    auto set = view.chunk->group->get_mask(type);
    if (set == 0)
        return false;
    return (mask->load(std::memory_order_relaxed) & set) == set;
}

bool sugoi_storage_t::exist(sugoi_entity_t e) const noexcept
{
    using namespace sugoi;
    return pimpl->entity_registry.entries.size() > e_id(e) && 
        pimpl->entity_registry.entries[e_id(e)].version == e_version(e);
}

void sugoi_storage_t::validate_meta()
{
    skr::stl_vector<sugoi_group_t*> groupsToFix;
    for (auto i = pimpl->groups.begin(); i != pimpl->groups.end(); ++i)
    {
        auto g     = i->second;
        auto type  = g->type;
        bool valid = !std::find_if(type.meta.data, type.meta.data + type.meta.length, [&](sugoi_entity_t e) {
            return !exist(e);
        });
        if (!valid)
        {
            i = pimpl->groups.erase(i);
            groupsToFix.push_back(g);
        }
    }
    for (auto g : groupsToFix)
    {
        auto& type = g->type;
        validate(type.meta);
        pimpl->groups.insert({ type, g });
    }
}

void sugoi_storage_t::validate(sugoi_entity_set_t& meta)
{
    auto end = std::remove_if(
    (sugoi_entity_t*)meta.data, (sugoi_entity_t*)meta.data + meta.length,
    [&](const sugoi_entity_t e) {
        return !exist(e);
    });
    meta.length = (SIndex)(end - meta.data);
}

void sugoi_storage_t::defragment()
{
    using namespace sugoi;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_storage(this);
    }
    for (auto& pair : pimpl->groups)
    {
        auto g = pair.second;
        if (g->chunks.size() < 2)
            continue;

        // step 1 : calculate best layout for group
        auto     total       = g->size;
        auto     arch        = g->archetype;
        uint32_t largeCount  = 0;
        uint32_t normalCount = 0;
        uint32_t smallCount  = 0;
        while (total > arch->chunkCapacity[2])
        {
            total -= arch->chunkCapacity[2];
            largeCount++;
        }
        while (total > arch->chunkCapacity[1])
        {
            total -= arch->chunkCapacity[1];
            normalCount++;
        }
        if (normalCount == 0 && largeCount == 0) // it's small group
        {
            while (total > arch->chunkCapacity[0])
            {
                total -= arch->chunkCapacity[0];
                smallCount++;
            }
        }
        else // else prefer normal size chunk
            normalCount++;

        // step 2 : grab and sort existing chunk for reuse
        skr::stl_vector<sugoi_chunk_t*> chunks = std::move(g->chunks);
        std::sort(chunks.begin(), chunks.end(), [](sugoi_chunk_t* lhs, sugoi_chunk_t* rhs) {
            return lhs->pt > rhs->pt || lhs->count > rhs->count;
        });

        // step 3 : reaverage data into new layout
        skr::stl_vector<sugoi_chunk_t*> newChunks;
        int                             o         = 0;
        int                             j         = (int)(chunks.size() - 1);
        auto                            fillChunk = [&](sugoi_chunk_t* chunk) {
            while (chunk->get_capacity() != chunk->count)
            {
                if (j <= o) // no more chunk to reaverage
                    return;
                auto source = chunks[j];
                if (source == chunk)
                    return;
                auto moveCount = chunk->get_capacity() - chunk->count;
                moveCount      = std::min(source->count, moveCount);
                move_view({ chunk, chunk->count, moveCount }, source, source->count - moveCount);
                pimpl->entity_registry.move_entities({ chunk, chunk->count, moveCount }, source, source->count - moveCount);
                source->count -= moveCount;
                chunk->count += moveCount;
                if (source->count == 0)
                {
                    destruct_chunk(source);
                    sugoi_chunk_t::destroy(source);
                    --j;
                }
            }
        };
        auto fillType = [&](uint32_t count, pool_type_t type) {
            for (uint32_t i = 0; i < count; ++i)
            {
                if (o < chunks.size() && chunks[o]->pt == type) // reuse chunk
                {
                    newChunks.push_back(chunks[o]);
                    ++o;
                }
                else // or create new chunk
                    newChunks.push_back(sugoi_chunk_t::create(type));
                fillChunk(newChunks.back());
            }
        };
        fillType(largeCount, PT_large);
        fillType(normalCount, PT_default);
        fillType(smallCount, PT_small);

        // step 4 : rebuild group chunk data
        for (auto chunk : newChunks)
            g->add_chunk(chunk);
    }
}

void sugoi_storage_t::pack_entities()
{
    using namespace sugoi;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_storage(this);
    }
    skr::stl_vector<EIndex> map;
    auto&                   entries = pimpl->entity_registry.entries;
    map.resize(entries.size());
    pimpl->entity_registry.freeEntities.clear();
    EIndex j = 0;
    forloop (i, 0, entries.size())
    {
        if (entries[i].indexInChunk != 0)
        {
            map[i] = j;
            if (i != j)
                entries[j] = entries[i];
            j++;
        }
    }
    struct mapper {
        skr::stl_vector<EIndex>* data;
        void                     move() {}
        void                     reset() {}
        void                     map(sugoi_entity_t& e)
        {
            if (e_id(e) < data->size())
                e = e_id(e, (*data)[e_id(e)]);
        }
    } m;
    m.data = &map;
    skr::stl_vector<sugoi_group_t*> gs;
    for (auto& pair : pimpl->groups)
        gs.push_back(pair.second);
    pimpl->groups.clear();
    for (auto g : gs)
    {
        for (auto c : g->chunks)
        {
            iterator_ref_chunk(c, m);
            iterator_ref_view({ c, 0, c->count }, m);
        }
        auto meta = g->type.meta;
        forloop (i, 0, meta.length)
            m.map(((sugoi_entity_t*)meta.data)[i]);
        std::sort((sugoi_entity_t*)meta.data, (sugoi_entity_t*)meta.data + meta.length);
        pimpl->groups.insert({ g->type, g });
    }
}

void sugoi_storage_t::castImpl(const sugoi_chunk_view_t& view, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u)
{
    using namespace sugoi;
    uint32_t k = 0;
    while (k < view.count)
    {
        sugoi_chunk_view_t dst = allocateView(group, view.count - k);
        pimpl->entity_registry.move_entities(dst, view.chunk, view.start + k);
        cast_view(dst, view.chunk, view.start + k);
        k += dst.count;
        if (callback)
            callback(u, &dst, (sugoi_chunk_view_t*)&view);
    }
    freeView(view);
}

void sugoi_storage_t::cast(const sugoi_chunk_view_t& view, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u)
{
    using namespace sugoi;
    auto srcGroup = view.chunk->group;
    if (srcGroup == group)
        return;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_archetype(srcGroup->archetype);
    }
    if (!group)
    {
        pimpl->entity_registry.free_entities(view);
        destruct_view(view);
        freeView(view);
        return;
    }
    if (full_view(view) && srcGroup->archetype == group->archetype)
    {
        srcGroup->remove_chunk(view.chunk);
        group->add_chunk(view.chunk);
        return;
    }
    if (pimpl->scheduler)
    {
        if (srcGroup->archetype != group->archetype)
            pimpl->scheduler->sync_archetype(group->archetype);
    }
    castImpl(view, group, callback, u);
}

void sugoi_storage_t::cast(sugoi_group_t* srcGroup, sugoi_group_t* group, sugoi_cast_callback_t callback, void* u)
{
    using namespace sugoi;
    if (srcGroup == group)
        return;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_archetype(srcGroup->archetype);
    }
    if (!group)
    {
        srcGroup->clear();
        return;
    }
    if (srcGroup->archetype == group->archetype)
    {
        for (auto chunk : srcGroup->chunks)
            group->add_chunk(chunk);
        srcGroup->chunks.clear();
        srcGroup->clear();
        return;
    }
    if (pimpl->scheduler)
    {
        pimpl->scheduler->sync_archetype(group->archetype);
    }
    // NOTE srcGroup->chunks can be modified during castImpl
    auto chunks = srcGroup->chunks;
    for (auto chunk : chunks)
    {
        sugoi_chunk_view_t view = { chunk, 0, chunk->count };
        castImpl(view, group, callback, u);
    }
}

sugoi_group_t* sugoi_storage_t::cast(sugoi_group_t* srcGroup, const sugoi_delta_type_t& diff)
{
    using namespace sugoi;
    fixed_stack_scope_t _(localStack);
    sugoi_entity_type_t type = srcGroup->type;
    sugoi_entity_type_t final;
    final.type = type.type;
    final.meta = type.meta;
    if (diff.added.type.length > 0)
    {
        auto finalType = localStack.allocate<type_index_t>(type.type.length + diff.added.type.length);
        final.type     = set_utils<sugoi_type_index_t>::merge(final.type, diff.added.type, finalType);
    }
    if (diff.added.meta.length > 0)
    {
        auto finalMeta = localStack.allocate<sugoi_entity_t>(type.meta.length + diff.added.meta.length);
        final.meta     = set_utils<sugoi_entity_t>::merge(final.meta, diff.added.meta, finalMeta);
    }
    if (diff.removed.type.length > 0)
    {
        auto finalType = localStack.allocate<type_index_t>(type.type.length + diff.added.type.length);
        final.type     = set_utils<sugoi_type_index_t>::substract(final.type, diff.removed.type, finalType);
    }
    if (diff.removed.meta.length > 0)
    {
        auto finalMeta = localStack.allocate<sugoi_entity_t>(type.meta.length + diff.added.meta.length);
        final.meta     = set_utils<sugoi_entity_t>::substract(final.meta, diff.removed.meta, finalMeta);
    }
    return get_group(final);
}

void sugoi_storage_t::batch(const sugoi_entity_t* ents, EIndex count, sugoi_view_callback_t callback, void* u)
{
    if (count == 0)
        return;
    else if (count == 1)
    {
        sugoi_chunk_view_t v = entity_view(ents[0]);
        callback(u, &v);
        return;
    }
    EIndex current = 0;
    auto   view    = entity_view(ents[current++]);
    while (current < count)
    {
        sugoi_chunk_view_t v = entity_view(ents[current]);
        if (v.chunk == view.chunk && v.start == view.start + view.count)
        {
            view.count++;
        }
        else
        {
            callback(u, &view);
            // v is not valid anymore
            view = entity_view(ents[current]);
        }
        current++;
    }
    callback(u, &view);
}

void sugoi_storage_t::merge(sugoi_storage_t& src)
{
    using namespace sugoi;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_storage(&src);
    }
    auto&                           sents = src.pimpl->entity_registry;
    skr::stl_vector<sugoi_entity_t> map;
    map.resize(sents.entries.size());
    EIndex moveCount = 0;
    for (auto& e : sents.entries)
        if (e.chunk != nullptr)
            moveCount++;
    skr::stl_vector<sugoi_entity_t> newEnts;
    newEnts.resize(moveCount);
    pimpl->entity_registry.new_entities(newEnts.data(), moveCount);
    int j = 0;
    for (int i = 0; i < sents.entries.size(); ++i)
        if (sents.entries[i].chunk != nullptr)
            map[i] = newEnts[j++];

    sents.reset();
    struct mapper {
        uint32_t        count;
        sugoi_entity_t* data;
        void            move() {}
        void            reset() {}
        void            map(sugoi_entity_t& e)
        {
            if (e_id(e) > count) SUGOI_UNLIKELY
            {
                e = kEntityNull;
                return;
            }
            e = data[e_id(e)];
        }
    } m;
    m.count = (uint32_t)map.size();
    m.data  = map.data();
    skr::stl_vector<sugoi_chunk_t*> chunks;
    struct payload_t {
        mapper*  m;
        uint32_t start, end;
    };
    skr::stl_vector<payload_t> payloads;
    payload_t                  payload;
    payload.m     = &m;
    payload.start = payload.end = 0;
    uint32_t sizePerBatch       = 1024 * 16;
    uint32_t sizeRemain         = sizePerBatch;
    for (auto& i : src.pimpl->groups)
    {
        sugoi_group_t* g = i.second;
        for (auto c : g->chunks)
        {
            chunks.push_back(c);
            auto sizeToPatch = c->count * c->structure->sizeToPatch;
            if (sizeRemain < sizeToPatch)
            {
                payload.end = (uint32_t)chunks.size();
                payloads.push_back(payload);
                payload.start = payload.end;
                sizeRemain    = sizeToPatch;
            }
            else
                sizeRemain -= sizeToPatch;
        }
    }
    if (payload.end != chunks.size())
    {
        payload.end = (uint32_t)chunks.size();
        payloads.push_back(payload);
    }
    using iter_t = decltype(payloads)::iterator;
    skr::parallel_for(payloads.begin(), payloads.end(), 1,
                      [this, &chunks](iter_t begin, iter_t end) {
                          for (auto i = begin; i < end; ++i)
                          {
                              for (auto j = i->start; j < i->end; ++j)
                              {
                                  auto c    = chunks[j];
                                  auto ents = (sugoi_entity_t*)c->get_entities();
                                  forloop (k, 0, c->count)
                                  {
                                      i->m->map(ents[k]);
                                      pimpl->entity_registry.entries[ents[k]] = { c, k, {} };
                                  }
                                  iterator_ref_chunk(c, *(i->m));
                                  iterator_ref_view({ c, 0, c->count }, *(i->m));
                              }
                          }
                      });
    for (auto& i : src.pimpl->groups)
    {
        sugoi_group_t* g    = i.second;
        auto           type = g->type;
        forloop (j, 0, type.meta.length)
            m.map((sugoi_entity_t&)type.meta.data[j]);
        sugoi_group_t* dstG = get_group(type);
        for (auto c : g->chunks)
        {
            dstG->add_chunk(c);
        }
        src.destructGroup(g);
    }
    src.pimpl->groups.clear();
    src.pimpl->queries.clear();
}

sugoi_storage_t* sugoi_storage_t::clone()
{
    sugoi_storage_t* dst = sugoiS_create();
    dst->pimpl->entity_registry = pimpl->entity_registry;
    dst->pimpl->timestamp       = pimpl->timestamp;
    for (auto group : pimpl->groups)
    {
        auto dstGroup = dst->cloneGroup(group.second);
        for (auto chunk : group.second->chunks)
        {
            auto count = chunk->count;
            auto view  = sugoi_chunk_view_t{ chunk, 0, count };
            while (count != 0)
            {
                sugoi_chunk_view_t v = dst->allocateView(dstGroup, count);
                sugoi::clone_view(v, view.chunk, view.start + (view.count - count));
                std::memcpy((sugoi_entity_t*)v.chunk->get_entities() + v.start, view.chunk->get_entities() + view.start + (view.count - count), v.count * sizeof(sugoi_entity_t));
                count -= v.count;
            }
        }
    }
    for (auto query : pimpl->queries)
    {
        dst->make_query(query->filter, query->parameters);
    }
    return dst;
}

void sugoi_storage_t::make_alias(skr::StringView name, skr::StringView aliasName)
{
    auto&               reg  = sugoi::TypeRegistry::get();
    auto                type = reg.get_type(name);
    sugoi_phase_alias_t aliasPhase{ type, ++pimpl->aliasCount };
    pimpl->aliases.insert({ aliasName, aliasPhase });
}

EIndex sugoi_storage_t::count(bool includeDisabled, bool includeDead)
{
    EIndex result = 0;
    for (auto& pair : pimpl->groups)
    {
        auto group = pair.second;
        if (group->isDead && !includeDead)
            continue;
        if (group->disabled && !includeDisabled)
            continue;
        result += group->size;
    }
    return result;
}

sugoi_timestamp_t sugoi_storage_t::timestamp() const
{
    return pimpl->timestamp;
}

sugoi::EntityRegistry& sugoi_storage_t::getEntityRegistry()
{
    return pimpl->entity_registry;
}

sugoi::scheduler_t* sugoi_storage_t::getScheduler()
{
    return pimpl->scheduler;
}

sugoi::block_arena_t& sugoi_storage_t::getArchetypeArena()
{
    return pimpl->archetypeArena;
}

void sugoi_storage_t::freeView(const sugoi_chunk_view_t& view)
{
    using namespace sugoi;
    auto group = view.chunk->group;
    structuralChange(group, view.chunk);
    uint32_t toMove = std::min(view.count, view.chunk->count - (view.start + view.count));
    if (toMove > 0)
    {
        sugoi_chunk_view_t dstView{ view.chunk, view.start, toMove };
        EIndex             srcIndex = view.chunk->count - toMove;
        pimpl->entity_registry.move_entities(dstView, srcIndex);
        move_view(dstView, srcIndex);
    }
    group->resize_chunk(view.chunk, view.chunk->count - view.count);
}

extern "C" {
sugoi_storage_t* sugoiS_create()
{
    auto pmem = (uint8_t*)sakura_malloc(sizeof(sugoi_storage_t) + sizeof(sugoi_storage_t::Impl));
    auto pimpl = new (pmem + sizeof(sugoi_storage_t)) sugoi_storage_t::Impl();
    return new (pmem) sugoi_storage_t(pimpl);
}

void sugoiS_release(sugoi_storage_t* storage)
{
    auto pimpl = (sugoi_storage_t::Impl*)(storage + 1);
    pimpl->~Impl();
    storage->~sugoi_storage_t();
    sakura_free(storage);
}

void sugoiS_allocate_type(sugoi_storage_t* storage, const sugoi_entity_type_t* type, EIndex count, sugoi_view_callback_t callback, void* u)
{
    SKR_ASSERT(sugoi::ordered(*type));
    storage->allocate(storage->get_group(*type), count, callback, u);
}

void sugoiS_allocate_group(sugoi_storage_t* storage, sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u)
{
    storage->allocate(group, count, callback, u);
}

void sugoiS_instantiate(sugoi_storage_t* storage, sugoi_entity_t prefab, EIndex count, sugoi_view_callback_t callback, void* u)
{
    storage->instantiate(prefab, count, callback, u);
}

void sugoiS_instantiate_delta(sugoi_storage_t* storage, sugoi_entity_t prefab, EIndex count, const sugoi_delta_type_t* delta, sugoi_view_callback_t callback, void* u)
{
    storage->instantiate(prefab, count, storage->cast(storage->entity_view(prefab).chunk->group->cloned, *delta), callback, u);
}

void sugoiS_instantiate_entities(sugoi_storage_t* storage, sugoi_entity_t* ents, EIndex n, EIndex count, sugoi_view_callback_t callback, void* u)
{
    storage->instantiate(ents, n, count, callback, u);
}

void sugoiS_destroy_entities(sugoi_storage_t* storage, const sugoi_entity_t* ents, EIndex n)
{
    if (n == 0) SUGOI_UNLIKELY
        return;

    auto destroy_callback = [storage](sugoi_chunk_view_t* view) {
        storage->destroy(*view);
    };
    storage->batch(ents, 1, SUGOI_LAMBDA(destroy_callback));
}

void sugoiS_destroy_in_query(const sugoi_query_t* query)
{
    query->storage->destroy(query);
}

void sugoiS_destroy_in_query_if(const sugoi_query_t* query, sugoi_destroy_callback_t callback, void* u)
{
    query->storage->destroy(query, callback, u);
}

void sugoiS_destroy_all(sugoi_storage_t* storage, const sugoi_meta_filter_t* meta)
{
    SKR_ASSERT(sugoi::ordered(*meta));
    storage->destroy(*meta);
}

void sugoiS_cast_view_delta(sugoi_storage_t* storage, const sugoi_chunk_view_t* view, const sugoi_delta_type_t* delta, sugoi_cast_callback_t callback, void* u)
{
    SKR_ASSERT(sugoi::ordered(*delta));
    storage->cast(*view, storage->cast(view->chunk->group, *delta), callback, u);
}

void sugoiS_cast_view_group(sugoi_storage_t* storage, const sugoi_chunk_view_t* view, const sugoi_group_t* group, sugoi_cast_callback_t callback, void* u)
{
    storage->cast(*view, (sugoi_group_t*)group, callback, u);
}

void sugoiS_cast_group_delta(sugoi_storage_t* storage, sugoi_group_t* group, const sugoi_delta_type_t* delta, sugoi_cast_callback_t callback, void* u)
{
    SKR_ASSERT(sugoi::ordered(*delta));
    storage->cast(group, storage->cast(group, *delta), callback, u);
}

void sugoiS_access(sugoi_storage_t* storage, sugoi_entity_t ent, sugoi_chunk_view_t* view)
{
    *view = storage->entity_view(ent);
}

void sugoiS_batch(sugoi_storage_t* storage, const sugoi_entity_t* ents, EIndex count, sugoi_view_callback_t callback, void* u)
{
    storage->batch(ents, count, callback, u);
}

void sugoiS_query(sugoi_storage_t* storage, const sugoi_filter_t* filter, const sugoi_meta_filter_t* meta, sugoi_view_callback_t callback, void* u)
{
    storage->query(*filter, *meta, callback, u);
}

void sugoiS_merge(sugoi_storage_t* storage, sugoi_storage_t* source)
{
    storage->merge(*source);
}

void sugoiS_serialize(sugoi_storage_t* storage, SBinaryWriter* v)
{
    storage->serialize(v);
}

void sugoiS_deserialize(sugoi_storage_t* storage, SBinaryReader* v)
{
    storage->deserialize(v);
}

int sugoiS_exist(sugoi_storage_t* storage, sugoi_entity_t ent)
{
    return storage->exist(ent);
}

int sugoiS_components_enabled(sugoi_storage_t* storage, sugoi_entity_t ent, const sugoi_type_set_t* types)
{
    SKR_ASSERT(sugoi::ordered(*types));
    return storage->components_enabled(ent, *types);
}

sugoi_entity_t sugoiS_deserialize_entity(sugoi_storage_t* storage, SBinaryReader* v)
{
    return storage->deserialize_prefab(v);
}

void sugoiS_serialize_entity(sugoi_storage_t* storage, sugoi_entity_t ent, SBinaryWriter* v)
{
    storage->serialize_prefab(ent, v);
}

void sugoiS_serialize_entities(sugoi_storage_t* storage, sugoi_entity_t* ents, EIndex n, SBinaryWriter* v)
{
    storage->serialize_prefab(ents, n, v);
}

void sugoiS_reset(sugoi_storage_t* storage)
{
    storage->reset();
}

void sugoiS_validate_meta(sugoi_storage_t* storage)
{
    storage->validate_meta();
}

void sugoiS_defragement(sugoi_storage_t* storage)
{
    storage->defragment();
}

void sugoiS_pack_entities(sugoi_storage_t* storage)
{
    storage->pack_entities();
}

void sugoiS_enable_components(const sugoi_chunk_view_t* view, const sugoi_type_set_t* types)
{
    using namespace sugoi;
    SKR_ASSERT(sugoi::ordered(*types));
    auto group   = view->chunk->group;
    auto masks   = (mask_t*)sugoiV_get_owned_rw(view, kMaskComponent);
    auto newMask = group->get_mask(*types);
    if (!masks) SUGOI_UNLIKELY
        return;
    for (uint32_t i = 0; i < view->count; ++i)
        masks[i].fetch_or(newMask);
}

void sugoiS_disable_components(const sugoi_chunk_view_t* view, const sugoi_type_set_t* types)
{
    using namespace sugoi;
    SKR_ASSERT(sugoi::ordered(*types));
    auto group   = view->chunk->group;
    auto masks   = (mask_t*)sugoiV_get_owned_rw(view, kMaskComponent);
    auto newMask = group->get_mask(*types);
    if (!masks) SUGOI_UNLIKELY
        return;
    for (uint32_t i = 0; i < view->count; ++i)
        masks[i].fetch_and(~newMask);
}

void sugoiQ_set_meta(sugoi_query_t* query, const sugoi_meta_filter_t* meta)
{
    if (!meta)
    {
        std::memset(&query->meta, 0, sizeof(sugoi_meta_filter_t));
    }
    else
    {
        SKR_ASSERT(sugoi::ordered(*meta));
        query->all_meta.append(meta->all_meta.data, meta->all_meta.length);
        query->none_meta.append(meta->none_meta.data, meta->none_meta.length);
        query->changed.append(meta->changed.data, meta->changed.length);
        query->meta.all_meta = { query->all_meta.data(), (SIndex)query->all_meta.size() };
        query->meta.none_meta = { query->none_meta.data(), (SIndex)query->none_meta.size() };
        query->meta.changed = { query->changed.data(), (SIndex)query->changed.size() };
        query->meta.timestamp = meta->timestamp;
    }
}

void sugoiQ_set_custom_filter(sugoi_query_t* query, sugoi_custom_filter_callback_t callback, void* u)
{
    query->customFilter         = callback;
    query->customFilterUserData = u;
}

sugoi_query_t* sugoiQ_create(sugoi_storage_t* storage, const sugoi_filter_t* filter, const sugoi_parameters_t* params)
{
    SKR_ASSERT(sugoi::ordered(*filter));
    return storage->make_query(*filter, *params);
}

void sugoiQ_make_alias(sugoi_storage_t* storage, const char8_t* component, const char8_t* alias)
{
    storage->make_alias((ochar8_t*)component, (ochar8_t*)alias);
}

void sugoiQ_release(sugoi_query_t* query)
{
    return query->storage->destroy_query(query);
}

sugoi_query_t* sugoiQ_from_literal(sugoi_storage_t* storage, const char8_t* desc)
{
    return storage->make_query(desc);
}

void sugoiQ_get_views(sugoi_query_t* query, sugoi_view_callback_t callback, void* u)
{
    return query->storage->query(query, callback, u);
}

void sugoiQ_get_groups(sugoi_query_t* query, sugoi_group_callback_t callback, void* u)
{
    return query->storage->query_groups(query, callback, u);
}

void sugoiQ_in_group(sugoi_query_t* query, sugoi_group_t* group, sugoi_view_callback_t callback, void* u)
{
    query->storage->buildQueries();
    if (!query->storage->match_group(query->filter, query->meta, group))
        return;
    query->storage->query_in_group_unsafe(&query->parameters, group, query->filter, query->meta, query->customFilter, query->customFilterUserData, callback, u);
}

const char8_t* sugoiQ_get_error()
{
    return sugoi::get_error().u8_str();
}

void sugoiQ_add_child(sugoi_query_t* query, sugoi_query_t* child)
{
    query->subqueries.push_back(child);
}

EIndex sugoiQ_get_count(sugoi_query_t* query)
{
    EIndex result      = 0;
    auto   accumulator = +[](void* u, sugoi_group_t* view) {
        EIndex* result = (EIndex*)u;
        *result += view->size;
    };
    query->storage->query_groups(query, accumulator, &result);
    return result;
}

void sugoiQ_sync(sugoi_query_t* query)
{
    if (auto scheduler = query->storage->getScheduler())
        scheduler->sync_query(query);
}

void sugoiQ_get(sugoi_query_t* query, sugoi_filter_t* filter, sugoi_parameters_t* params)
{
    query->storage->buildQueries();
    if (filter)
        *filter = query->filter;
    if (params)
        *params = query->parameters;
}

sugoi_storage_t* sugoiQ_get_storage(sugoi_query_t* query)
{
    return query->storage;
}

void sugoiS_all(sugoi_storage_t* storage, bool includeDisabled, bool includeDead, sugoi_view_callback_t callback, void* u)
{
    storage->all(includeDisabled, includeDead, callback, u);
}

EIndex sugoiS_count(sugoi_storage_t* storage, bool includeDisabled, bool includeDead)
{
    return storage->count(includeDisabled, includeDead);
}

void sugoi_set_bit(uint32_t* mask, int32_t bit)
{
    // CAS
    uint32_t oldMask = *mask;
    uint32_t newMask = oldMask | (1 << bit);
    // TODO: REMOVE THIS!
    _SAtomic(uint32_t)* pmask = (_SAtomic(uint32_t)*)mask;
    while (!skr_atomic_compare_exchange_strong(pmask, &oldMask, newMask))
    {
        oldMask = *mask;
        newMask = oldMask | (1 << bit);
    }
}
}

sugoi_type_index_t sugoi_id_of<sugoi::dirty_comp_t>::get()
{
    return sugoi::kDirtyComponent;
}

sugoi_type_index_t sugoi_id_of<sugoi::mask_comp_t>::get()
{
    return sugoi::kMaskComponent;
}

sugoi_type_index_t sugoi_id_of<sugoi::guid_comp_t>::get()
{
    return sugoi::kGuidComponent;
}