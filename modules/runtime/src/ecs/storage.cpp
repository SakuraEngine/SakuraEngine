#include "ecs/SmallVector.h"
#include "chunk_view.hpp"

#include "ecs/dual.h"
#include "entity.hpp"
#include "query.hpp"
#include "set.hpp"
#include "storage.hpp"
#include "ecs/constants.hpp"
#include "pool.hpp"
#include "mask.hpp"
#include "iterator_ref.hpp"
#include "scheduler.hpp"
#include "utils/parallel_for.hpp"
#include "type_registry.hpp"

dual_storage_t::dual_storage_t()
    : archetypeArena(dual::get_default_pool())
    , queryBuildArena(dual::get_default_pool())
    , groupPool(dual::kGroupBlockSize, dual::kGroupBlockCount)
    , scheduler(nullptr)
{
}

dual_storage_t::~dual_storage_t()
{
    if(scheduler)
        scheduler->remove_storage(this);
    scheduler = nullptr;
    for(auto q : queries)
        sakura_free((void*)q);
    for (auto iter : groups)
        iter.second->clear();
}

void dual_storage_t::reset()
{
    for (auto iter : groups)
        iter.second->clear();
    groups.clear();
    archetypes.clear();
    queries.clear();
    queryCaches.clear();
    entities.reset();
    archetypeArena.reset();
    queryBuildArena.reset();
    groupPool.reset();
    queriesBuilt = false;
}

void dual_storage_t::allocate(dual_group_t* group, EIndex count, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_archetype(group->archetype);
    }
    while (count != 0)
    {
        dual_chunk_view_t v = allocate_view(group, count);
        entities.fill_entities(v);
        construct_view(v);
        count -= v.count;
        if (callback)
            callback(u, &v);
    }
}

dual_chunk_view_t dual_storage_t::allocate_view(dual_group_t* group, EIndex count)
{
    dual_chunk_t* freeChunk = group->get_first_free_chunk();
    if (freeChunk == nullptr)
        freeChunk = group->new_chunk(count);
    EIndex start = freeChunk->count;
    EIndex allocated = std::min(count, freeChunk->get_capacity() - start);
    group->resize_chunk(freeChunk, start + allocated);
    structural_change(group, freeChunk);
    return { freeChunk, start, allocated };
}

dual_chunk_view_t dual_storage_t::allocate_view_strict(dual_group_t* group, EIndex count)
{
    dual_chunk_t* freeChunk = nullptr;
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
    structural_change(group, freeChunk);
    return { freeChunk, start, count };
}

void dual_storage_t::destroy(const dual_chunk_view_t& view)
{
    using namespace dual;
    auto group = view.chunk->group;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_archetype(group->archetype);
    }
    SKR_ASSERT(!group->isDead);
    auto dead = group->dead;
    if (dead)
        cast(view, dead, nullptr, nullptr);
    else
    {
        entities.free_entities(view);
        destruct_view(view);
        free(view);
    }
}

void dual_storage_t::free(const dual_chunk_view_t& view)
{
    using namespace dual;
    auto group = view.chunk->group;
    structural_change(group, view.chunk);
    uint32_t toMove = std::min(view.count, view.chunk->count - (view.start + view.count));
    if (toMove > 0)
    {
        dual_chunk_view_t dstView{ view.chunk, view.start, toMove };
        EIndex srcIndex = view.chunk->count - toMove;
        entities.move_entities(dstView, srcIndex);
        move_view(dstView, srcIndex);
    }
    group->resize_chunk(view.chunk, view.chunk->count - view.count);
}

void dual_storage_t::structural_change(dual_group_t* group, dual_chunk_t* chunk)
{
    // todo: timestamp
}

void dual_storage_t::linked_to_prefab(const dual_entity_t* src, uint32_t size, bool keepExternal)
{
    using namespace dual;
    struct mapper_t {
        const dual_entity_t* source;
        uint32_t count;
        bool keepExternal;
        void move() {}
        void reset() {}

        void map(dual_entity_t& ent)
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
    m.count = size;
    m.source = src;
    m.keepExternal = keepExternal;
    forloop (i, 0, size)
        iterator_ref_view(entity_view(src[i]), m);
}

void dual_storage_t::prefab_to_linked(const dual_entity_t* src, uint32_t size)
{
    using namespace dual;
    struct mapper_t {
        const dual_entity_t* source;
        uint32_t count;
        void move() {}
        void reset() {}

        void map(dual_entity_t& ent)
        {
            if (e_id(ent) > count || !e_transient(ent))
                return;
            ent = source[e_id(ent)];
        }
    } m;
    m.count = size;
    m.source = src;
    forloop (i, 0, size)
        iterator_ref_view(entity_view(src[i]), m);
}

void dual_storage_t::instantiate_prefab(const dual_entity_t* src, uint32_t size, uint32_t count, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    eastl::vector<dual_entity_t> ents;
    ents.resize(count * size);
    entities.new_entities(ents.data(), (EIndex)ents.size());
    struct mapper_t {
        dual_entity_t* base;
        dual_entity_t* curr;
        uint32_t size;
        void move() { curr += size; }
        void reset() { curr = base; }
        void map(dual_entity_t& ent)
        {
            if (e_id(ent) > size || !e_transient(ent))
                return;
            ent = curr[e_id(ent)];
        }
    } m;
    m.size = size;
    eastl::vector<dual_entity_t> localEnts;
    localEnts.resize(count);
    forloop (i, 0, size)
    {
        forloop (j, 0, count)
            localEnts[j] = ents[j * size + i];
        auto view = entity_view(src[i]);
        auto group = view.chunk->group->cloned;
        auto localCount = 0;
        while (localCount != count)
        {
            dual_chunk_view_t v = allocate_view(group, count - localCount);
            entities.fill_entities(v, localEnts.data() + localCount);
            duplicate_view(v, view.chunk, view.start);
            m.base = m.curr = ents.data() + localCount * size;
            localCount += v.count;
            iterator_ref_view(v, m);
            if (callback)
                callback(u, &v);
        }
    }
}

void dual_storage_t::instantiate(const dual_entity_t src, uint32_t count, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    auto view = entity_view(src);
    auto group = view.chunk->group->cloned;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_archetype(group->archetype);
    }
    while (count != 0)
    {
        dual_chunk_view_t v = allocate_view(group, count);
        entities.fill_entities(v);
        duplicate_view(v, view.chunk, view.start);
        count -= v.count;
        if (callback)
            callback(u, &v);
    }
}

void dual_storage_t::instantiate(const dual_entity_t src, uint32_t count, dual_group_t* group, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    auto view = entity_view(src);
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_archetype(group->archetype);
    }
    while (count != 0)
    {
        dual_chunk_view_t v = allocate_view(group, count);
        entities.fill_entities(v);
        duplicate_view(v, view.chunk, view.start);
        count -= v.count;
        if (callback)
            callback(u, &v);
    }
}

void dual_storage_t::instantiate(const dual_entity_t* src, uint32_t n, uint32_t count, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        forloop (i, 0, n)
        {
            auto view = entity_view(src[i]);
            scheduler->sync_archetype(view.chunk->type); // data is modified by linked to prefab
            scheduler->sync_archetype(view.chunk->group->cloned->archetype);
        }
    }
    linked_to_prefab(src, n);
    instantiate_prefab(src, n, count, callback, u);
    prefab_to_linked(src, n);
}

dual_chunk_view_t dual_storage_t::entity_view(dual_entity_t e) const
{
    using namespace dual;
    SKR_ASSERT(e_id(e) < entities.entries.size());
    auto& entry = entities.entries[e_id(e)];
    if(entry.version == e_version(e))
        return { entry.chunk, entry.indexInChunk, 1 };
    return { nullptr, 0, 1 };
}

bool dual_storage_t::components_enabled(const dual_entity_t src, const dual_type_set_t& type)
{
    using namespace dual;
    auto view = entity_view(src);
    auto mask = (mask_t*)dualV_get_owned_ro(&view, kMaskComponent);
    if (!mask)
        return true;
    auto set = view.chunk->group->get_mask(type);
    if (set == 0)
        return false;
    return (mask->load(std::memory_order_relaxed) & set) == set;
}

bool dual_storage_t::exist(dual_entity_t e) const noexcept
{
    using namespace dual;
    return entities.entries.size() > e_id(e) && entities.entries[e_id(e)].version == e_version(e);
}

void dual_storage_t::validate_meta()
{
    eastl::vector<dual_group_t*> groupsToFix;
    for (auto i = groups.begin(); i != groups.end(); ++i)
    {
        auto g = i->second;
        auto type = g->type;
        bool valid = !std::find_if(type.meta.data, type.meta.data + type.meta.length, [&](dual_entity_t e) {
            return !exist(e);
        });
        if (!valid)
        {
            i = groups.erase(i);
            groupsToFix.push_back(g);
        }
    }
    for (auto g : groupsToFix)
    {
        auto& type = g->type;
        validate(type.meta);
        groups.insert({ type, g });
    }
}

void dual_storage_t::validate(dual_entity_set_t& meta)
{
    auto end = std::remove_if(
    (dual_entity_t*)meta.data, (dual_entity_t*)meta.data + meta.length,
    [&](const dual_entity_t e) {
        return !exist(e);
    });
    meta.length = (SIndex)(end - meta.data);
}

void dual_storage_t::defragment()
{
    using namespace dual;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_storage(this);
    }
    for (auto& pair : groups)
    {
        auto g = pair.second;
        if (g->chunks.size() < 2)
            continue;

        // step 1 : calculate best layout for group
        auto total = g->size;
        auto arch = g->archetype;
        uint32_t largeCount = 0;
        uint32_t normalCount = 0;
        uint32_t smallCount = 0;
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
        eastl::vector<dual_chunk_t*> chunks = std::move(g->chunks);
        std::sort(chunks.begin(), chunks.end(), [](dual_chunk_t* lhs, dual_chunk_t* rhs) {
            return lhs->pt > rhs->pt || lhs->count > rhs->count;
        });

        // step 3 : reaverage data into new layout
        eastl::vector<dual_chunk_t*> newChunks;
        int o = 0;
        int j = (int)(chunks.size() - 1);
        auto fillChunk = [&](dual_chunk_t* chunk) {
            while (chunk->get_capacity() != chunk->count)
            {
                if (j <= o) // no more chunk to reaverage
                    return;
                auto source = chunks[j];
                if (source == chunk)
                    return;
                auto moveCount = chunk->get_capacity() - chunk->count;
                moveCount = std::min(source->count, moveCount);
                move_view({ chunk, chunk->count, moveCount }, source, source->count - moveCount);
                entities.move_entities({ chunk, chunk->count, moveCount }, source, source->count - moveCount);
                source->count -= moveCount;
                chunk->count += moveCount;
                if (source->count == 0)
                {
                    destruct_chunk(source);
                    dual_chunk_t::destroy(source);
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
                    newChunks.push_back(dual_chunk_t::create(type));
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

void dual_storage_t::pack_entities()
{
    using namespace dual;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_storage(this);
    }
    eastl::vector<EIndex> map;
    auto& entries = entities.entries;
    map.resize(entries.size());
    entities.freeEntries.clear();
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
        eastl::vector<EIndex>* data;
        void move() {}
        void reset() {}
        void map(dual_entity_t& e)
        {
            if (e_id(e) < data->size())
                e = e_id(e, (*data)[e_id(e)]);
        }
    } m;
    m.data = &map;
    eastl::vector<dual_group_t*> gs;
    for (auto& pair : groups)
        gs.push_back(pair.second);
    groups.clear();
    for (auto g : gs)
    {
        for(auto c : g->chunks)
        {
            iterator_ref_chunk( c, m );
            iterator_ref_view({ c, 0, c->count }, m);
        }
        auto meta = g->type.meta;
        forloop (i, 0, meta.length)
            m.map(((dual_entity_t*)meta.data)[i]);
        std::sort((dual_entity_t*)meta.data, (dual_entity_t*)meta.data + meta.length);
        groups.insert({ g->type, g });
    }
}

void dual_storage_t::cast_impl(const dual_chunk_view_t& view, dual_group_t* group, dual_cast_callback_t callback, void* u)
{
    using namespace dual;
    uint32_t k = 0;
    while (k < view.count)
    {
        dual_chunk_view_t dst = allocate_view(group, view.count - k);
        entities.move_entities(dst, view.chunk, view.start + k);
        cast_view(dst, view.chunk, view.start + k);
        k += dst.count;
        if (callback)
            callback(u, &dst, (dual_chunk_view_t*)&view);
    }
    free(view);
}

void dual_storage_t::cast(const dual_chunk_view_t& view, dual_group_t* group, dual_cast_callback_t callback, void* u)
{
    using namespace dual;
    auto srcGroup = view.chunk->group;
    if (srcGroup == group)
        return;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_archetype(srcGroup->archetype);
    }
    if (!group)
    {
        entities.free_entities(view);
        destruct_view(view);
        free(view);
        return;
    }
    if (full_view(view) && srcGroup->archetype == group->archetype)
    {
        srcGroup->remove_chunk(view.chunk);
        group->add_chunk(view.chunk);
        return;
    }
    if (scheduler)
    {
        if (srcGroup->archetype != group->archetype)
            scheduler->sync_archetype(group->archetype);
    }
    cast_impl(view, group, callback, u);
}

void dual_storage_t::cast(dual_group_t* srcGroup, dual_group_t* group, dual_cast_callback_t callback, void* u)
{
    using namespace dual;
    if (srcGroup == group)
        return;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_archetype(srcGroup->archetype);
    }
    if (!group)
    {
        srcGroup->clear();
        return;
    }
    if (srcGroup->archetype == group->archetype)
    {
        for(auto chunk : srcGroup->chunks)
            group->add_chunk(chunk);
        srcGroup->chunks.clear();
        srcGroup->clear();
        return;
    }
    if (scheduler)
    {
        scheduler->sync_archetype(group->archetype);
    }
    //NOTE srcGroup->chunks can be modified during cast_impl
    auto chunks = srcGroup->chunks;
    for (auto chunk : chunks)
    {
        dual_chunk_view_t view = { chunk, 0, chunk->count };
        cast_impl(view, group, callback, u);
    }
}

dual_group_t* dual_storage_t::cast(dual_group_t* srcGroup, const dual_delta_type_t& diff)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    dual_entity_type_t type = srcGroup->type;
    dual_entity_type_t final;
    final.type = type.type;
    final.meta = type.meta;
    if (diff.added.type.length > 0)
    {
        auto finalType = localStack.allocate<type_index_t>(type.type.length + diff.added.type.length);
        final.type = set_utils<dual_type_index_t>::merge(final.type, diff.added.type, finalType);
    }
    if (diff.added.meta.length > 0)
    {
        auto finalMeta = localStack.allocate<dual_entity_t>(type.meta.length + diff.added.meta.length);
        final.meta = set_utils<dual_entity_t>::merge(final.meta, diff.added.meta, finalMeta);
    }
    if (diff.removed.type.length > 0)
    {
        auto finalType = localStack.allocate<type_index_t>(type.type.length + diff.added.type.length);
        final.type = set_utils<dual_type_index_t>::substract(final.type, diff.removed.type, finalType);
    }
    if (diff.removed.meta.length > 0)
    {
        auto finalMeta = localStack.allocate<dual_entity_t>(type.meta.length + diff.added.meta.length);
        final.meta = set_utils<dual_entity_t>::substract(final.meta, diff.removed.meta, finalMeta);
    }
    return get_group(final);
}

void dual_storage_t::batch(const dual_entity_t* ents, EIndex count, dual_view_callback_t callback, void* u)
{
    if(count == 0)
        return;
    else if(count == 1)
    {
        dual_chunk_view_t v = entity_view(ents[0]);
        callback(u, &v);
        return;
    }
    EIndex current = 0;
    auto view = entity_view(ents[current++]);
    while (current < count)
    {
        dual_chunk_view_t v = entity_view(ents[current]);
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

void dual_storage_t::merge(dual_storage_t& src)
{
    using namespace dual;
    if (scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_storage(&src);
    }
    auto& sents = src.entities;
    eastl::vector<dual_entity_t> map;
    map.resize(sents.entries.size());
    EIndex moveCount = 0;
    for (auto& e : sents.entries)
        if (e.chunk != nullptr)
            moveCount++;
    eastl::vector<dual_entity_t> newEnts;
    newEnts.resize(moveCount);
    entities.new_entities(newEnts.data(), moveCount);
    int j = 0;
    for (int i = 0; i < sents.entries.size(); ++i)
        if (sents.entries[i].chunk != nullptr)
            map[i] = newEnts[j++];

    sents.reset();
    struct mapper {
        uint32_t count;
        dual_entity_t* data;
        void move() {}
        void reset() {}
        void map(dual_entity_t& e)
        {
            if (e_id(e) > count) DUAL_UNLIKELY
                {
                    e = kEntityNull;
                    return;
                }
            e = data[e_id(e)];
        }
    } m;
    m.count = (uint32_t)map.size();
    m.data = map.data();
    eastl::vector<dual_chunk_t*> chunks;
    struct payload_t {
        mapper* m;
        uint32_t start, end;
    };
    eastl::vector<payload_t> payloads;
    payload_t payload;
    payload.m = &m;
    payload.start = payload.end = 0;
    uint32_t sizePerBatch = 1024 * 16;
    uint32_t sizeRemain = sizePerBatch;
    for (auto& i : src.groups)
    {
        dual_group_t* g = i.second;
        for(auto c : g->chunks)
        {
            chunks.push_back(c);
            auto sizeToPatch = c->count * c->type->sizeToPatch;
            if (sizeRemain < sizeToPatch)
            {
                payload.end = (uint32_t)chunks.size();
                payloads.push_back(payload);
                payload.start = payload.end;
                sizeRemain = sizeToPatch;
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
    [this, &chunks](iter_t begin, iter_t end)
    {
        for(auto i = begin; i<end; ++i)
        {
            for(auto j=i->start; j<i->end; ++j)
            {
                auto c = chunks[j];
                auto ents = (dual_entity_t*)c->get_entities();
                forloop (k, 0, c->count)
                {
                    i->m->map(ents[k]);
                    entities.entries[ents[k]] = { c, k };
                }
                iterator_ref_chunk( c, *(i->m) );
                iterator_ref_view({ c, 0, c->count }, *(i->m));
            }
        }
    });
    for (auto& i : src.groups)
    {
        dual_group_t* g = i.second;
        auto type = g->type;
        forloop (j, 0, type.meta.length)
            m.map((dual_entity_t&)type.meta.data[j]);
        dual_group_t* dstG = get_group(type);
        for(auto c : g->chunks)
        {
            dstG->add_chunk(c);
        }
        src.destruct_group(g);
    }
    src.groups.clear();
    src.queries.clear();
}

extern "C" {
dual_storage_t* dualS_create()
{
    return new dual_storage_t;
}

void dualS_release(dual_storage_t* storage)
{
    delete storage;
}

void dualS_set_userdata(dual_storage_t* storage, void* u)
{
    storage->userdata = u;
}

void* dualS_get_userdata(dual_storage_t* storage)
{
    return storage->userdata;
}

void dualS_allocate_type(dual_storage_t* storage, const dual_entity_type_t* type, EIndex count, dual_view_callback_t callback, void* u)
{
    SKR_ASSERT(dual::ordered(*type));
    storage->allocate(storage->get_group(*type), count, callback, u);
}

void dualS_allocate_group(dual_storage_t* storage, dual_group_t* group, EIndex count, dual_view_callback_t callback, void* u)
{
    storage->allocate(group, count, callback, u);
}

void dualS_instantiate(dual_storage_t* storage, dual_entity_t prefab, EIndex count, dual_view_callback_t callback, void* u)
{
    storage->instantiate(prefab, count, callback, u);
}

void dualS_instantiate_delta(dual_storage_t* storage, dual_entity_t prefab, EIndex count, const dual_delta_type_t* delta, dual_view_callback_t callback, void* u)
{
    storage->instantiate(prefab, count, storage->cast(storage->entity_view(prefab).chunk->group->cloned, *delta), callback, u);
}

void dualS_instantiate_entities(dual_storage_t* storage, dual_entity_t* ents, EIndex n, EIndex count, dual_view_callback_t callback, void* u)
{
    storage->instantiate(ents, n, count, callback, u);
}

void dualS_destroy(dual_storage_t* storage, const dual_chunk_view_t* view)
{
    storage->destroy(*view);
}

void dualS_destroy_all(dual_storage_t* storage, const dual_meta_filter_t* meta)
{
    SKR_ASSERT(dual::ordered(*meta));
    storage->destroy(*meta);
}

void dualS_cast_view_delta(dual_storage_t* storage, const dual_chunk_view_t* view, const dual_delta_type_t* delta, dual_cast_callback_t callback, void* u)
{
    SKR_ASSERT(dual::ordered(*delta));
    storage->cast(*view, storage->cast(view->chunk->group, *delta), callback, u);
}

void dualS_cast_view_group(dual_storage_t* storage, const dual_chunk_view_t* view, const dual_group_t* group, dual_cast_callback_t callback, void* u)
{
    storage->cast(*view, (dual_group_t*)group, callback, u);
}

void dualS_cast_group_delta(dual_storage_t* storage, dual_group_t* group, const dual_delta_type_t* delta, dual_cast_callback_t callback, void* u)
{
    SKR_ASSERT(dual::ordered(*delta));
    storage->cast(group, storage->cast(group, *delta), callback, u);
}

void dualS_access(dual_storage_t* storage, dual_entity_t ent, dual_chunk_view_t* view)
{
    *view = storage->entity_view(ent);
}

void dualS_batch(dual_storage_t* storage, const dual_entity_t* ents, EIndex count, dual_view_callback_t callback, void* u)
{
    storage->batch(ents, count, callback, u);
}

void dualS_query(dual_storage_t* storage, const dual_filter_t* filter, const dual_meta_filter_t* meta, dual_view_callback_t callback, void* u)
{
    SKR_ASSERT(dual::ordered(*filter));
    SKR_ASSERT(dual::ordered(*meta));
    
    if (storage->scheduler)
        SKR_ASSERT(storage->scheduler->is_main_thread(storage));
    storage->query(*filter, *meta, callback, u);
}

void dualS_merge(dual_storage_t* storage, dual_storage_t* source)
{
    storage->merge(*source);
}

void dualS_serialize(dual_storage_t* storage, skr_binary_writer_t* v)
{
    storage->serialize(v);
}

void dualS_deserialize(dual_storage_t* storage, skr_binary_reader_t* v)
{
    storage->deserialize(v);
}

int dualS_exist(dual_storage_t* storage, dual_entity_t ent)
{
    return storage->exist(ent);
}

int dualS_components_enabled(dual_storage_t* storage, dual_entity_t ent, const dual_type_set_t* types)
{
    SKR_ASSERT(dual::ordered(*types));
    return storage->components_enabled(ent, *types);
}

dual_entity_t dualS_deserialize_entity(dual_storage_t* storage, skr_binary_reader_t* v)
{
    return storage->deserialize_prefab(v);
}

void dualS_serialize_entity(dual_storage_t* storage, dual_entity_t ent, skr_binary_writer_t* v)
{
    storage->serialize_prefab(ent, v);
}

void dualS_serialize_entities(dual_storage_t* storage, dual_entity_t* ents, EIndex n, skr_binary_writer_t* v)
{
    storage->serialize_prefab(ents, n, v);
}

void dualS_reset(dual_storage_t* storage)
{
    storage->reset();
}

void dualS_validate_meta(dual_storage_t* storage)
{
    storage->validate_meta();
}

void dualS_defragement(dual_storage_t* storage)
{
    storage->defragment();
}

void dualS_pack_entities(dual_storage_t* storage)
{
    storage->pack_entities();
}

void dualS_enable_components(const dual_chunk_view_t* view, const dual_type_set_t* types)
{
    using namespace dual;
    SKR_ASSERT(dual::ordered(*types));
    auto group = view->chunk->group;
    auto masks = (mask_t*)dualV_get_owned_rw(view, kMaskComponent);
    auto newMask = group->get_mask(*types);
    if (!masks) DUAL_UNLIKELY
        return;
    for (uint32_t i = 0; i < view->count; ++i)
        masks[i].fetch_or(newMask);
}

void dualS_disable_components(const dual_chunk_view_t* view, const dual_type_set_t* types)
{
    using namespace dual;
    SKR_ASSERT(dual::ordered(*types));
    auto group = view->chunk->group;
    auto masks = (mask_t*)dualV_get_owned_rw(view, kMaskComponent);
    auto newMask = group->get_mask(*types);
    if (!masks) DUAL_UNLIKELY
        return;
    for (uint32_t i = 0; i < view->count; ++i)
        masks[i].fetch_and(~newMask);
}

void dualQ_set_meta(dual_query_t* query, const dual_meta_filter_t* meta)
{
    if (!meta)
    {
        std::memset(&query->meta, 0, sizeof(dual_meta_filter_t));
    }
    else
    {
        SKR_ASSERT(dual::ordered(*meta));
        query->meta = *meta;
    }
}

dual_query_t* dualQ_create(dual_storage_t* storage, const dual_filter_t* filter, const dual_parameters_t* params)
{
    SKR_ASSERT(dual::ordered(*filter));
    return storage->make_query(*filter, *params);
}

void dualQ_release(dual_query_t *query)
{
    return query->storage->destroy_query(query);
}

dual_query_t* dualQ_from_literal(dual_storage_t* storage, const char* desc)
{
    return storage->make_query(desc);
}

void dualQ_get_views(dual_query_t* query, dual_view_callback_t callback, void* u)
{
    return query->storage->query(query, callback, u);
}

void dualQ_get_groups(dual_query_t* query, dual_group_callback_t callback, void* u)
{
    return query->storage->query_groups(query, callback, u);
}

void dualQ_get_views_group(dual_query_t* query, dual_group_t* group, dual_view_callback_t callback, void* u)
{
    query->storage->build_queries();
    if(!query->storage->match_group(query->buildedFilter, query->meta, group))
        return;
    query->storage->query(group, query->buildedFilter, query->meta, callback, u);
}

const char* dualQ_get_error()
{
    return dual::get_error().c_str();
}

EIndex dualQ_get_count(dual_query_t* query)
{
    EIndex result = 0;
    auto accumulator = +[](void* u, dual_group_t* view)
    {
        EIndex* result = (EIndex*)u;
        *result += view->size;
    };
    query->storage->query_groups(query, accumulator, &result);
    return result;
}

void dualQ_get(dual_query_t* query, dual_filter_t* filter, dual_parameters_t* params)
{
    if(!query->storage->queriesBuilt)
        query->storage->build_queries();
    *filter = query->buildedFilter;
    *params = query->parameters;
}

dual_storage_t* dualQ_get_storage(dual_query_t* query)
{
    return query->storage;
}

void dualS_all(dual_storage_t *storage, bool includeDisabled, bool includeDead, dual_view_callback_t callback, void *u)
{
    for(auto& pair : storage->groups)
    {
        auto group = pair.second;
        if(group->isDead && !includeDead)
            continue;
        if(group->disabled && !includeDisabled)
            continue;
        for(auto c : group->chunks)
        {
            dual_chunk_view_t view {c, 0, c->count};
            callback(u, &view);
        }
    }
}
}