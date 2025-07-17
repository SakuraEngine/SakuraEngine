#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/set.hpp"
#include "SkrRT/ecs/type_index.hpp"
#include "SkrRT/ecs/type_registry.hpp"

#include "./stack.hpp"
#include "./chunk.hpp"
#include "./chunk_view.hpp"
#include "./impl/storage.hpp"
#include "./archetype.hpp"
#include <algorithm>
#include <array>

#ifndef forloop
#define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

namespace sugoi
{

thread_local fixed_stack_t localStack(4096 * 8);

struct guid_compare_t {
    bool operator()(const guid_t& a, const guid_t& b) const
    {
        using value_type = std::array<char, 16>;
        return reinterpret_cast<const value_type&>(a) < reinterpret_cast<const value_type&>(b);
    }
};

bool archetype_t::with_chunk_component() const noexcept
{
    return firstChunkComponent != type.length;
}

SIndex archetype_t::index(sugoi_type_index_t inType) const noexcept
{
    auto end = type.data + type.length;
    const sugoi_type_index_t* result = std::lower_bound(type.data, end, inType);
    if (result != end && *result == inType)
        return (SIndex)(result - type.data);
    else
        return kInvalidSIndex;
}
} // namespace sugoi

template <typename Field, typename ValueType>
void write_const(const Field& cref, ValueType v)
{
    Field& ref = const_cast<Field&>(cref);
    ref = v;
}

sugoi::archetype_t* sugoi_storage_t::constructArchetype(const sugoi_type_set_t& inType)
{
    using namespace sugoi;

    fixed_stack_scope_t _(localStack);
    auto& atypeArena = getArchetypeArena();
    char* buffer = (char*)atypeArena.allocate(data_size(inType), 1);
    archetype_t& archetype = *atypeArena.allocate<archetype_t>();
    write_const(archetype.storage, this);
    write_const(archetype.type, sugoi::clone(inType, buffer));
    write_const(archetype.withMask, false);
    write_const(archetype.withDirty, false);
    write_const(archetype.sizeToPatch, 0);
    write_const(archetype.firstChunkComponent, archetype.type.length);
    forloop (i, 0, archetype.type.length)
        if(type_index_t(archetype.type.data[i]).is_chunk())
        {
            write_const(archetype.firstChunkComponent, i);
            break;
        }
    forloop (i, 0, 3)
        write_const(archetype.offsets[i], atypeArena.allocate<uint32_t>(archetype.type.length));
    write_const(archetype.elemSizes, atypeArena.allocate<uint32_t>(archetype.type.length));
    write_const(archetype.callbackFlags, atypeArena.allocate<uint32_t>(archetype.type.length));
    write_const(archetype.aligns, atypeArena.allocate<uint32_t>(archetype.type.length));
    write_const(archetype.sizes, atypeArena.allocate<uint32_t>(archetype.type.length));
    write_const(archetype.resourceFields, atypeArena.allocate<sugoi::resource_fields_t>(archetype.type.length));
    write_const(archetype.callbacks, atypeArena.allocate<sugoi_callback_v>(archetype.type.length));
    ::memset((void*)archetype.callbacks, 0, sizeof(sugoi_callback_v) * archetype.type.length);
    write_const(archetype.stableOrder, atypeArena.allocate<SIndex>(archetype.type.length));
    auto& registry = TypeRegistry::get();
    forloop (i, 0, archetype.type.length)
    {
        const auto tid = type_index_t(archetype.type.data[i]).index();
        const auto& desc = *registry.get_type_desc(tid);
        uint32_t callbackFlag = 0;
        if (desc.callback.constructor)
            callbackFlag |= SUGOI_CALLBACK_FLAG_CTOR;
        if (desc.callback.destructor)
            callbackFlag |= SUGOI_CALLBACK_FLAG_DTOR;
        if (desc.callback.copy)
            callbackFlag |= SUGOI_CALLBACK_FLAG_COPY;
        if (desc.callback.move)
            callbackFlag |= SUGOI_CALLBACK_FLAG_MOVE;
        write_const(archetype.callbackFlags[i], callbackFlag);
        write_const(archetype.callbacks[i], desc.callback);
        write_const(archetype.resourceFields[i], resource_fields_t{ desc.resourceFields, desc.resourceFieldsCount });
    }
    auto guids = localStack.allocate<guid_t>(archetype.type.length);
    write_const(archetype.entitySize,sizeof(sugoi_entity_t));
    uint32_t padding = 0;
    forloop (i, 0, archetype.type.length)
    {
        auto t = archetype.type.data[i];
        if (t == kMaskComponent)
            write_const(archetype.withMask, true);
        if (t == kDirtyComponent)
            write_const(archetype.withDirty, true);
        auto ti = type_index_t(t);
        auto& desc = *registry.get_type_desc(ti.index());
        write_const(archetype.sizes[i], desc.size);
        write_const(archetype.elemSizes[i], desc.elementSize);
        guids[i] = desc.guid;
        write_const(archetype.aligns[i], desc.alignment);
        write_const(archetype.stableOrder[i], i);
        write_const(archetype.entitySize, archetype.entitySize + desc.size);
        if(!ti.is_chunk())
            padding += desc.alignment;
        if (!ti.is_chunk() && desc.entityFieldsCount != 0)
            write_const(archetype.sizeToPatch, archetype.sizeToPatch + desc.size);
    }
    std::sort(const_cast<uint32_t*>(archetype.stableOrder), const_cast<uint32_t*>(archetype.stableOrder) + archetype.type.length, 
        [&](SIndex lhs, SIndex rhs) {
            return guid_compare_t{}(guids[lhs], guids[rhs]);
        });
    size_t caps[] = { kSmallBinSize - sizeof(sugoi_chunk_t), kFastBinSize - sizeof(sugoi_chunk_t), kLargeBinSize - sizeof(sugoi_chunk_t) };
    const uint32_t sliceDataSize = sizeof(sugoi::slice_data_t) * archetype.type.length;
    forloop (i, 0, 3)
    {
        uint32_t* offsets = const_cast<uint32_t*>(archetype.offsets[i]);
        uint32_t& capacity = const_cast<uint32_t&>(archetype.chunkCapacity[i]);
        write_const(archetype.sliceDataOffsets[i], static_cast<sugoi_timestamp_t>(caps[i] - sliceDataSize));
        uint32_t ccOffset = (uint32_t)(caps[i] - sliceDataSize);
        forloop (j, 0, archetype.type.length)
        {
            SIndex id = archetype.stableOrder[j];
            TIndex t = archetype.type.data[id];
            auto ti = type_index_t(t);
            if(ti.is_chunk())
            {
                ccOffset -= archetype.sizes[id] * capacity;
                ccOffset = (uint32_t)(archetype.aligns[id] * (ccOffset / archetype.aligns[id]));
                offsets[id] = ccOffset;
            }
        }
        capacity = (uint32_t)(ccOffset - padding) / archetype.entitySize;
        if (capacity == 0)
            continue;
        uint32_t offset = sizeof(sugoi_entity_t) * capacity;
        forloop (j, 0, archetype.type.length)
        {
            SIndex id = archetype.stableOrder[j];
            TIndex t = archetype.type.data[id];
            auto ti = type_index_t(t);
            if(!ti.is_chunk())
            {
                offset = (uint32_t)(archetype.aligns[id] * ((offset + archetype.aligns[id] - 1) / archetype.aligns[id]));
                offsets[id] = offset;
                offset += archetype.sizes[id] * capacity;
            }
        }
    }
    sugoi::archetype_t* new_type = nullptr;
    {
        pimpl->archetypes.update_versioned([&](auto& archetypes){
            new_type = archetypes.insert({ archetype.type, &archetype }).first->second;
            pimpl->archetype_timestamp += 1;
        }, 
        [&](){ 
            return pimpl->archetype_timestamp;
        });
    }
    return new_type;
}

sugoi::archetype_t* sugoi_storage_t::cloneArchetype(archetype_t *src)
{
    using namespace sugoi;
    if(auto a = tryGetArchetype(src->type))
        return a;

    auto& archetypeArena = getArchetypeArena();
    char* buffer = (char*)archetypeArena.allocate(data_size(src->type), 1);
    archetype_t& archetype = *archetypeArena.allocate<archetype_t>();
    write_const(archetype.storage, this);
    write_const(archetype.type, sugoi::clone(src->type, buffer));
    write_const(archetype.withMask, src->withMask);
    write_const(archetype.withDirty, src->withMask);
    write_const(archetype.sizeToPatch, src->sizeToPatch);
    write_const(archetype.firstChunkComponent, src->withMask);
    forloop (i, 0, 3)
    {
        write_const(archetype.offsets[i], archetypeArena.allocate<uint32_t>(archetype.type.length));
        memcpy((void*)archetype.offsets[i], src->offsets[i], sizeof(uint32_t) * archetype.type.length);
    }
    write_const(archetype.elemSizes, archetypeArena.allocate<uint32_t>(archetype.type.length));
    memcpy((void*)archetype.elemSizes, src->elemSizes, sizeof(uint32_t) * archetype.type.length);
    write_const(archetype.callbackFlags, archetypeArena.allocate<uint32_t>(archetype.type.length));
    memcpy((void*)archetype.callbackFlags, src->callbackFlags, sizeof(uint32_t) * archetype.type.length);
    write_const(archetype.aligns, archetypeArena.allocate<uint32_t>(archetype.type.length));
    memcpy((void*)archetype.aligns, src->aligns, sizeof(uint32_t) * archetype.type.length);
    write_const(archetype.sizes, archetypeArena.allocate<uint32_t>(archetype.type.length));
    memcpy((void*)archetype.sizes, src->sizes, sizeof(uint32_t) * archetype.type.length);
    write_const(archetype.resourceFields, archetypeArena.allocate<sugoi::resource_fields_t>(archetype.type.length));
    memcpy((void*)archetype.resourceFields, src->resourceFields, sizeof(sugoi::resource_fields_t) * archetype.type.length);
    write_const(archetype.callbacks, archetypeArena.allocate<sugoi_callback_v>(archetype.type.length));
    memcpy((void*)archetype.callbacks, src->callbacks, sizeof(sugoi_callback_v) * archetype.type.length);
    write_const(archetype.stableOrder, archetypeArena.allocate<SIndex>(archetype.type.length));
    memcpy((void*)archetype.stableOrder, src->stableOrder, sizeof(SIndex) * archetype.type.length);
    write_const(archetype.entitySize, src->entitySize);
    write_const(archetype.sliceDataOffsets[0], src->sliceDataOffsets[0]);
    write_const(archetype.sliceDataOffsets[1], src->sliceDataOffsets[1]);
    write_const(archetype.sliceDataOffsets[2], src->sliceDataOffsets[2]);
    write_const(archetype.chunkCapacity[0], src->chunkCapacity[0]);
    write_const(archetype.chunkCapacity[1], src->chunkCapacity[1]);
    write_const(archetype.chunkCapacity[2], src->chunkCapacity[2]);
    sugoi::archetype_t* new_type = nullptr;
    {
        pimpl->archetypes.update_versioned([&](auto& archetypes){
            new_type = archetypes.insert({ archetype.type, &archetype }).first->second;
            pimpl->archetype_timestamp += 1;
        }, 
        [&](){
            return pimpl->archetype_timestamp;
        });
    }
    return new_type;
}

sugoi_group_t* sugoi_storage_t::constructGroup(const sugoi_entity_type_t& inType)
{
    using namespace sugoi;
    fixed_stack_scope_t _(localStack);
    sugoi_type_set_t structure;
    SIndex firstTag = 0;
    for (; firstTag < inType.type.length; ++firstTag)
        if (type_index_t(inType.type.data[firstTag]).is_tag())
            break;
    structure.data = inType.type.data;
    structure.length = firstTag;
    archetype_t* archetype = get_archetype(structure);
    const auto typeSize = static_cast<uint32_t>(data_size(inType));
    SKR_ASSERT((sizeof(sugoi_group_t) + typeSize) < kGroupBlockSize);(void)typeSize;
    sugoi_group_t& group = *new (pimpl->groupPool.allocate()) sugoi_group_t();
    char* buffer = (char*)(&group + 1);
    group.firstFree = 0;
    sugoi_entity_type_t type = sugoi::clone(inType, buffer);
    group.type = type;
    auto toClean = localStack.allocate<TIndex>(group.type.type.length + 1);
    SIndex toCleanCount = 0;
    auto toClone = localStack.allocate<TIndex>(group.type.type.length + 1);
    SIndex toCloneCount = 0;
    group.isDead = false;
    group.disabled = false;
    bool hasTracked = false;
    forloop (i, 0, group.type.type.length)
    {
        type_index_t t = group.type.type.data[i];
        if(t > kDeadComponent)
            toClean[toCleanCount++] = kDeadComponent;
        toClean[toCleanCount++] = t;
        if (!t.is_pinned())
            toClone[toCloneCount++] = t;
        else
            hasTracked = true;
        if (t == kDeadComponent)
            group.isDead = true;
        if (t == kDisableComponent)
            group.disabled = true;
    }
    if(toCleanCount == group.type.type.length)
        toClean[toCleanCount++] = kDeadComponent;
    // std::sort(&toClean[0], &toClean[toCleanCount]); dead is always smaller
    group.archetype = archetype;
    group.size = 0;
    group.timestamp = 0;
    group.dead = nullptr;
    group.cloned = group.isDead ? nullptr : &group;
    {
        pimpl->groups.update_versioned([&](auto& groups){
            groups.insert({ type, &group });
            pimpl->groups_timestamp += 1;
        }, 
        [&](){
            return pimpl->groups_timestamp;
        });
    }
    updateQueryCache(&group, true);
    if (hasTracked && !group.isDead)
    {
        auto deadType = make_zeroed<sugoi_entity_type_t>();
        deadType.type = { toClean, toCleanCount };
        group.dead = get_group(deadType);
        sugoi_entity_type_t cloneType = make_zeroed<sugoi_entity_type_t>();
        cloneType.type = { toClone, toCloneCount };
        group.cloned = get_group(cloneType);
    }
    return &group;
}

sugoi_group_t* sugoi_storage_t::cloneGroup(sugoi_group_t* srcG)
{
    if(auto g = tryGetGroup(srcG->type))
        return g;
    sugoi_group_t& group = *new (pimpl->groupPool.allocate()) sugoi_group_t();
    std::memcpy((void*)&group, srcG, sizeof(sugoi_group_t));
    char* buffer = (char*)(&group + 1);
    group.type = sugoi::clone(srcG->type, buffer);
    group.archetype = cloneArchetype(srcG->archetype);
    if (srcG->dead)
    {
        group.dead = cloneGroup(srcG->dead);
    }
    if (srcG->cloned)
    {
        group.cloned = cloneGroup(srcG->cloned);
    }
    
    pimpl->groups.update_versioned([&](auto& groups){
        groups.insert({ group.type, &group });
        pimpl->groups_timestamp += 1;
    }, 
    [&](){
        return pimpl->groups_timestamp;
    });

    return &group;
}

sugoi::archetype_t* sugoi_storage_t::tryGetArchetype(const sugoi_type_set_t& type) const
{
    sugoi::archetype_t* found = nullptr;
    pimpl->archetypes.read_versioned([&](auto& archetypes){
        if (auto i = archetypes.find(type); i != archetypes.end())
            found = i->second;
    }, 
    [&](){
        return pimpl->archetype_timestamp;
    });
    return found;
}

sugoi_group_t* sugoi_storage_t::tryGetGroup(const sugoi_entity_type_t& type) const
{
    sugoi_group_t* found = nullptr;
    pimpl->groups.read_versioned([&](auto& groups){
        if (auto i = groups.find(type); i != groups.end())
            found = i->second;
    }, 
    [&](){
        return pimpl->groups_timestamp;
    });
    return found;
}

sugoi::archetype_t* sugoi_storage_t::get_archetype(const sugoi_type_set_t& type)
{
    archetype_t* archetype = tryGetArchetype(type);
    return archetype ? archetype : constructArchetype(type);
}

sugoi_group_t* sugoi_storage_t::get_group(const sugoi_entity_type_t& type)
{
    bool withPinned = false;
    bool dead = false;
    for(SIndex i = 0; i < type.type.length; ++i)
    {
        sugoi::type_index_t t = type.type.data[i];
        if(t.is_pinned())
            withPinned = true;
        if(t == sugoi::kDeadComponent)
            dead = true;
    }
    if(dead && !withPinned)
        return nullptr;
    sugoi_group_t* existed_group = tryGetGroup(type);
    return existed_group ? existed_group : constructGroup(type);
}

void sugoi_storage_t::destructGroup(sugoi_group_t* group)
{
    updateQueryCache(group, false);
    pimpl->groups.update_versioned([&](auto& groups){
        groups.erase(group->type);
        pimpl->groupPool.free(group);
        pimpl->groups_timestamp += 1;
    }, 
    [&](){
        return pimpl->groups_timestamp;
    });
}

sugoi_chunk_t* sugoi_group_t::get_first_free_chunk() const noexcept
{
    if (firstFree == (uint32_t)chunks.size())
        return nullptr;
    return chunks[firstFree];
}

sugoi_chunk_t* sugoi_group_t::new_chunk(uint32_t hint)
{
    using namespace sugoi;
    pool_type_t pt;
    if (chunks.size() < kSmallBinThreshold && hint < archetype->chunkCapacity[PT_small])
        pt = PT_small;
    else if (hint > archetype->chunkCapacity[PT_default] * 8u)
        pt = PT_large;
    else
        pt = PT_default;
    sugoi_chunk_t* chunk = sugoi_chunk_t::create(pt);
    add_chunk(chunk);
    construct_chunk(chunk);
    return chunk;
}

void sugoi_group_t::add_chunk(sugoi_chunk_t* chunk)
{
    using namespace sugoi;
    size += chunk->count;
    chunk->structure = archetype;
    chunk->group = this;
    bool isFull      = chunk->count == chunk->get_capacity();
    if (!isFull || firstFree == (uint32_t)chunks.size())
    {
        chunk->index = (uint32_t)chunks.size();
        chunks.push_back(chunk);
    }
    else
    {
        chunks.resize(chunks.size() + 1);
        for (uint32_t i = firstFree; i < (uint32_t)chunks.size() - 1; ++i)
        {
            auto j = (uint32_t)chunks.size() - 1 + firstFree - i;
            chunks[j] = chunks[j - 1];
            chunks[j]->index = j;
        }
        chunks[firstFree] = chunk;
        chunk->index = firstFree;
    }
    if (isFull)
        firstFree++;
}

void sugoi_group_t::resize_chunk(sugoi_chunk_t* chunk, EIndex newSize)
{
    using namespace sugoi;
    bool full = chunk->count == chunk->get_capacity();
    size = size + newSize - chunk->count;
    chunk->count = newSize;
    if (newSize == 0)
    {
        destruct_chunk(chunk);
        remove_chunk(chunk);
        sugoi_chunk_t::destroy(chunk);
    }
    else
    {
        if (!full && chunk->get_capacity() == newSize)
            mark_full(chunk);
        else if((full && chunk->get_capacity() > newSize))
            mark_free(chunk);
    }
}

void sugoi_group_t::remove_chunk(sugoi_chunk_t* chunk)
{
    using namespace sugoi;
    size -= chunk->count;
    for(uint32_t i = chunk->index; i < (uint32_t)chunks.size() - 1; ++i)
    {
        chunks[i] = chunks[i + 1];
        chunks[i]->index = i;
    }
    chunks.pop_back();
    chunk->group = nullptr;
    if (chunk->index < firstFree)
        firstFree--;
}

void sugoi_group_t::mark_free(sugoi_chunk_t* chunk)
{
    using namespace sugoi;
    SKR_ASSERT(chunk->index < firstFree);
    firstFree--;
    auto& slot = chunks[chunk->index];
    std::swap(chunks[firstFree]->index, chunk->index);
    std::swap(chunks[firstFree], slot);
}

void sugoi_group_t::mark_full(sugoi_chunk_t* chunk)
{
    using namespace sugoi;
    
    SKR_ASSERT(chunk->index >= firstFree);
    if (chunk->index != firstFree)
    {
        auto& slot = chunks[chunk->index];
        std::swap(chunks[firstFree]->index, chunk->index);
        std::swap(chunks[firstFree], slot);
    }
    firstFree++;
}

void sugoi_group_t::clear()
{
    using namespace sugoi;
    SkrZoneScopedN("sugoi_group_t::clear");

    auto& entity_registry = archetype->storage->getEntityRegistry();
    size_t freeCount = 0;
    for (auto chunk : chunks)
    {
        freeCount += chunk->count;
    }
    entity_registry.reserve_free_entries(freeCount);

    for (auto chunk : chunks)
    {
        entity_registry.free_entities({ chunk, 0, chunk->count });
        destruct_view({ chunk, 0, chunk->count });
        destruct_chunk(chunk);
        sugoi_chunk_t::destroy(chunk);
    }
    chunks.clear();
    firstFree = 0;
    size = 0;
}

TIndex sugoi_group_t::index(sugoi_type_index_t inType) const noexcept
{
    using namespace sugoi;
    auto end = type.type.data + type.type.length;
    const sugoi_type_index_t* result = std::lower_bound(type.type.data, end, inType);
    if (result != end && *result == inType)
        return (TIndex)(result - type.type.data);
    else
        return kInvalidSIndex;
}

bool sugoi_group_t::share(sugoi_type_index_t t) const noexcept
{
    using namespace sugoi;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto entry = storage->getEntityRegistry().try_get_entry(type.meta.data[i]);
        SKR_ASSERT(entry.has_value());
        auto metaGroup = entry.value().chunk->group;
        if (metaGroup->index(t) != kInvalidSIndex)
            return true;
        if (metaGroup->share(t))
            return true;
    }
    return false;
}

bool sugoi_group_t::own(const sugoi_type_set_t& subtype) const noexcept
{
    return sugoi::set_utils<sugoi_type_index_t>::all(type.type, subtype);
}

bool sugoi_group_t::share(const sugoi_type_set_t& subtype) const noexcept
{
    using namespace sugoi;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto entry = storage->getEntityRegistry().try_get_entry(type.meta.data[i]);
        SKR_ASSERT(entry.has_value());
        auto metaGroup = entry.value().chunk->group;
        if (metaGroup->own(subtype))
            return true;
        if (metaGroup->share(subtype))
            return true;
    }
    return false;
}

sugoi_mask_comp_t sugoi_group_t::get_shared_mask(const sugoi_type_set_t& subtype) const noexcept
{
    using namespace sugoi;
    sugoi::bitset32 mask;
    for (SIndex i = 0; i < subtype.length; ++i)
    {
        if (share(subtype.data[i]))
        {
            mask.set(i, true);
            break;
        }
    }
    return mask.to_uint32();
}

void sugoi_group_t::get_shared_type(sugoi_type_set_t& result, void* buffer) const noexcept
{
    using namespace sugoi;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto view = storage->entity_view(type.meta.data[i]);
        auto metaGroup = view.chunk->group;
        sugoi_type_set_t merged;
        if (metaGroup->archetype->withMask)
        {
            auto mask = *(sugoi_mask_comp_t*)sugoiV_get_owned_ro(&view, kMaskComponent);
            merged = set_utils<sugoi_type_index_t>::merge_with_mask(result, metaGroup->type.type, mask, buffer);
        }
        else
        {
            merged = set_utils<sugoi_type_index_t>::merge(result, metaGroup->type.type, buffer);
        }
        buffer = (void*)result.data;
        result = merged;
        metaGroup->get_shared_type(result, buffer);
    }
}

const sugoi_group_t* sugoi_group_t::get_owner(sugoi_type_index_t t) const noexcept
{
    using namespace sugoi;
    if (index(t) != kInvalidSIndex)
        return this;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto entry = storage->getEntityRegistry().try_get_entry(type.meta.data[i]);
        SKR_ASSERT(entry.has_value());
        auto metaGroup = entry.value().chunk->group;
        if (auto g = metaGroup->get_owner(t))
            return g;
    }
    return nullptr;
}

const void* sugoi_group_t::get_shared_ro(sugoi_type_index_t t) const noexcept
{
    using namespace sugoi;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto view = storage->entity_view(type.meta.data[i]);
        if (auto data = sugoiV_get_component_ro(&view, t))
            return data;
        return view.chunk->group->get_shared_ro(t);
    }
    return nullptr;
}

sugoi_mask_comp_t sugoi_group_t::get_mask(const sugoi_type_set_t& subtype) const noexcept
{
    using namespace sugoi;
    sugoi::bitset32 mask;
    SIndex i = 0, j = 0;
    auto stype = type.type;
    while (i < stype.length && j < subtype.length)
    {
        if (stype.data[i] > subtype.data[j])
            ++j;
        else if (stype.data[i] < subtype.data[j])
            ++i;
        else
        {
            mask.set(i, true);
            ++i;
            ++j;
        }
    }
    return mask.to_uint32();
}

extern "C" {
int sugoiG_has_components(const sugoi_group_t* group, const sugoi_type_set_t* types)
{
    return group->own(*types) || group->share(*types);
}

int sugoiG_own_components(const sugoi_group_t* group, const sugoi_type_set_t* types)
{
    return group->own(*types);
}

int sugoiG_share_components(const sugoi_group_t* group, const sugoi_type_set_t* types)
{
    return group->share(*types);
}

void sugoiG_get_type(const sugoi_group_t* group, sugoi_entity_type_t* type)
{
    *type = group->type;
}

uint32_t sugoiG_get_size(const sugoi_group_t* group)
{
    return group->size;
}

uint32_t sugoiG_get_stable_order(const sugoi_group_t* group, sugoi_type_index_t localType)
{
    return group->archetype->stableOrder[localType];
}

const void* sugoiG_get_shared_ro(const sugoi_group_t* group, sugoi_type_index_t type)
{
    return group->get_shared_ro(type);
}
}