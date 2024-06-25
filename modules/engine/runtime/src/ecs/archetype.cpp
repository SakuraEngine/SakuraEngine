#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/set.hpp"
#include "SkrRT/ecs/type_index.hpp"
#include "SkrRT/ecs/type_registry.hpp"

#include "./utilities.hpp"
#include "./stack.hpp"
#include "./chunk.hpp"
#include "./chunk_view.hpp"
#include "./impl/storage.hpp"
#include "./archetype.hpp"
#include <algorithm>
#include <array>

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

sugoi::archetype_t* sugoi_storage_t::construct_archetype(const sugoi_type_set_t& inType)
{
    using namespace sugoi;
    fixed_stack_scope_t _(localStack);
    char* buffer = (char*)pimpl->archetypeArena.allocate(data_size(inType), 1);
    archetype_t& proto = *pimpl->archetypeArena.allocate<archetype_t>();
    proto.storage = this;
    proto.type = sugoi::clone(inType, buffer);
    proto.withMask = false;
    proto.withDirty = false;
    proto.sizeToPatch = 0;
    proto.firstChunkComponent = proto.type.length;
    forloop (i, 0, proto.type.length)
        if(type_index_t(proto.type.data[i]).is_chunk())
        {
            proto.firstChunkComponent = i;
            break;
        }
    forloop (i, 0, 3)
        proto.offsets[i] = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.elemSizes = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.callbackFlags = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.aligns = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.sizes = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.resourceFields = pimpl->archetypeArena.allocate<sugoi::resource_fields_t>(proto.type.length);
    proto.callbacks = pimpl->archetypeArena.allocate<sugoi_callback_v>(proto.type.length);
    ::memset(proto.callbacks, 0, sizeof(sugoi_callback_v) * proto.type.length);
    proto.stableOrder = pimpl->archetypeArena.allocate<SIndex>(proto.type.length);
    auto& registry = TypeRegistry::get();
    forloop (i, 0, proto.type.length)
    {
        const auto tid = type_index_t(proto.type.data[i]).index();
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
        proto.callbackFlags[i] = callbackFlag;
        proto.callbacks[i] = desc.callback;
        proto.resourceFields[i] = { desc.resourceFields, desc.resourceFieldsCount };
    }
    auto guids = localStack.allocate<guid_t>(proto.type.length);
    proto.entitySize = sizeof(sugoi_entity_t);
    uint32_t padding = 0;
    forloop (i, 0, proto.type.length)
    {
        auto t = proto.type.data[i];
        if (t == kMaskComponent)
            proto.withMask = true;
        if (t == kDirtyComponent)
            proto.withDirty = true;
        auto ti = type_index_t(t);
        auto& desc = *registry.get_type_desc(ti.index());
        proto.sizes[i] = desc.size;
        proto.elemSizes[i] = desc.elementSize;
        guids[i] = desc.guid;
        proto.aligns[i] = desc.alignment;
        proto.stableOrder[i] = i;
        proto.entitySize += desc.size;
        if(!ti.is_chunk())
            padding += desc.alignment;
        if (!ti.is_chunk() && desc.entityFieldsCount != 0)
            proto.sizeToPatch += desc.size;
    }
    std::sort(proto.stableOrder, proto.stableOrder + proto.type.length, [&](SIndex lhs, SIndex rhs) {
        return guid_compare_t{}(guids[lhs], guids[rhs]);
    });
    size_t caps[] = { kSmallBinSize - sizeof(sugoi_chunk_t), kFastBinSize - sizeof(sugoi_chunk_t), kLargeBinSize - sizeof(sugoi_chunk_t) };
    const uint32_t sliceDataSize = sizeof(sugoi::slice_data_t) * proto.type.length;
    forloop (i, 0, 3)
    {
        uint32_t* offsets = proto.offsets[i];
        uint32_t& capacity = proto.chunkCapacity[i];
        proto.sliceDataOffsets[i] = static_cast<sugoi_timestamp_t>(caps[i] - sliceDataSize);
        uint32_t ccOffset = (uint32_t)(caps[i] - sliceDataSize);
        forloop (j, 0, proto.type.length)
        {
            SIndex id = proto.stableOrder[j];
            TIndex t = proto.type.data[id];
            auto ti = type_index_t(t);
            if(ti.is_chunk())
            {
                ccOffset -= proto.sizes[id] * capacity;
                ccOffset = (uint32_t)(proto.aligns[id] * (ccOffset / proto.aligns[id]));
                offsets[id] = ccOffset;
            }
        }
        capacity = (uint32_t)(ccOffset - padding) / proto.entitySize;
        if (capacity == 0)
            continue;
        uint32_t offset = sizeof(sugoi_entity_t) * capacity;
        forloop (j, 0, proto.type.length)
        {
            SIndex id = proto.stableOrder[j];
            TIndex t = proto.type.data[id];
            auto ti = type_index_t(t);
            if(!ti.is_chunk())
            {
                offset = (uint32_t)(proto.aligns[id] * ((offset + proto.aligns[id] - 1) / proto.aligns[id]));
                offsets[id] = offset;
                offset += proto.sizes[id] * capacity;
            }
        }
    }

    return pimpl->archetypes.insert({ proto.type, &proto }).first->second;
}

sugoi::archetype_t* sugoi_storage_t::clone_archetype(archetype_t *src)
{
    using namespace sugoi;
    if(auto a = try_get_archetype(src->type))
        return a;
    char* buffer = (char*)pimpl->archetypeArena.allocate(data_size(src->type), 1);
    archetype_t& proto = *pimpl->archetypeArena.allocate<archetype_t>();
    proto.storage = this;
    proto.type = sugoi::clone(src->type, buffer);
    proto.withMask = src->withMask;
    proto.withDirty = src->withMask;
    proto.sizeToPatch = src->sizeToPatch;
    proto.firstChunkComponent = src->withMask;
    forloop (i, 0, 3)
    {
        proto.offsets[i] = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
        memcpy(proto.offsets[i], src->offsets[i], sizeof(uint32_t) * proto.type.length);
    }
    proto.elemSizes = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    memcpy(proto.elemSizes, src->elemSizes, sizeof(uint32_t) * proto.type.length);
    proto.callbackFlags = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    memcpy(proto.callbackFlags, src->callbackFlags, sizeof(uint32_t) * proto.type.length);
    proto.aligns = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    memcpy(proto.aligns, src->aligns, sizeof(uint32_t) * proto.type.length);
    proto.sizes = pimpl->archetypeArena.allocate<uint32_t>(proto.type.length);
    memcpy(proto.sizes, src->sizes, sizeof(uint32_t) * proto.type.length);
    proto.resourceFields = pimpl->archetypeArena.allocate<sugoi::resource_fields_t>(proto.type.length);
    memcpy(proto.resourceFields, src->resourceFields, sizeof(sugoi::resource_fields_t) * proto.type.length);
    proto.callbacks = pimpl->archetypeArena.allocate<sugoi_callback_v>(proto.type.length);
    memcpy(proto.callbacks, src->callbacks, sizeof(sugoi_callback_v) * proto.type.length);
    proto.stableOrder = pimpl->archetypeArena.allocate<SIndex>(proto.type.length);
    memcpy(proto.stableOrder, src->stableOrder, sizeof(SIndex) * proto.type.length);
    proto.entitySize = src->entitySize;
    proto.sliceDataOffsets[0] = src->sliceDataOffsets[0];
    proto.sliceDataOffsets[1] = src->sliceDataOffsets[1];
    proto.sliceDataOffsets[2] = src->sliceDataOffsets[2];
    proto.chunkCapacity[0] = src->chunkCapacity[0];
    proto.chunkCapacity[1] = src->chunkCapacity[1];
    proto.chunkCapacity[2] = src->chunkCapacity[2];

    return pimpl->archetypes.insert({ proto.type, &proto }).first->second;
}

sugoi_group_t* sugoi_storage_t::construct_group(const sugoi_entity_type_t& inType)
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
    sugoi_group_t& proto = *new (pimpl->groupPool.allocate()) sugoi_group_t();
    char* buffer = (char*)(&proto + 1);
    proto.firstFree = 0;
    sugoi_entity_type_t type = sugoi::clone(inType, buffer);
    proto.type = type;
    auto toClean = localStack.allocate<TIndex>(proto.type.type.length + 1);
    SIndex toCleanCount = 0;
    auto toClone = localStack.allocate<TIndex>(proto.type.type.length + 1);
    SIndex toCloneCount = 0;
    proto.isDead = false;
    proto.disabled = false;
    bool hasTracked = false;
    forloop (i, 0, proto.type.type.length)
    {
        type_index_t t = proto.type.type.data[i];
        if(t > kDeadComponent)
            toClean[toCleanCount++] = kDeadComponent;
        toClean[toCleanCount++] = t;
        if (!t.is_pinned())
            toClone[toCloneCount++] = t;
        else
            hasTracked = true;
        if (t == kDeadComponent)
            proto.isDead = true;
        if (t == kDisableComponent)
            proto.disabled = true;
    }
    if(toCleanCount == proto.type.type.length)
        toClean[toCleanCount++] = kDeadComponent;
    // std::sort(&toClean[0], &toClean[toCleanCount]); dead is always smaller
    proto.archetype = archetype;
    proto.size = 0;
    proto.timestamp = 0;
    proto.dead = nullptr;
    proto.cloned = proto.isDead ? nullptr : &proto;
    pimpl->groups.insert({ type, &proto });
    update_query_cache(&proto, true);
    if (hasTracked && !proto.isDead)
    {
        auto deadType = make_zeroed<sugoi_entity_type_t>();
        deadType.type = { toClean, toCleanCount };
        proto.dead = get_group(deadType);
        sugoi_entity_type_t cloneType = make_zeroed<sugoi_entity_type_t>();
        cloneType.type = { toClone, toCloneCount };
        proto.cloned = get_group(cloneType);
    }
    return &proto;
}

sugoi_group_t* sugoi_storage_t::clone_group(sugoi_group_t* srcG)
{
    if(auto g = try_get_group(srcG->type))
        return g;
    sugoi_group_t& proto = *new (pimpl->groupPool.allocate()) sugoi_group_t();
    std::memcpy(&proto, srcG, sizeof(sugoi_group_t));
    char* buffer = (char*)(&proto + 1);
    proto.type = sugoi::clone(srcG->type, buffer);
    proto.archetype = clone_archetype(srcG->archetype);
    if (srcG->dead)
    {
        proto.dead = clone_group(srcG->dead);
    }
    if (srcG->cloned)
    {
        proto.cloned = clone_group(srcG->cloned);
    }
    pimpl->groups.insert({ proto.type, &proto });
    return &proto;
}

sugoi::archetype_t* sugoi_storage_t::try_get_archetype(const sugoi_type_set_t& type) const
{
    if (auto i = pimpl->archetypes.find(type); i != pimpl->archetypes.end())
        return i->second;
    return nullptr;
}

sugoi_group_t* sugoi_storage_t::try_get_group(const sugoi_entity_type_t& type) const
{
    if (auto i = pimpl->groups.find(type); i != pimpl->groups.end())
        return i->second;
    return nullptr;
}

sugoi::archetype_t* sugoi_storage_t::get_archetype(const sugoi_type_set_t& type)
{
    archetype_t* archetype = try_get_archetype(type);
    if (!archetype)
        archetype = construct_archetype(type);
    return archetype;
}

sugoi_group_t* sugoi_storage_t::get_group(const sugoi_entity_type_t& type)
{
    using namespace sugoi;
    bool withPinned = false;
    bool dead = false;
    for(SIndex i=0; i < type.type.length; ++i)
    {
        type_index_t t = type.type.data[i];
        if(t.is_pinned())
            withPinned = true;
        if(t == kDeadComponent)
            dead = true;
    }
    if(dead && !withPinned)
        return nullptr;
    sugoi_group_t* group = try_get_group(type);
    if (!group)
        group = construct_group(type);
    return group;
}

void sugoi_storage_t::destruct_group(sugoi_group_t* group)
{
    update_query_cache(group, false);
    pimpl->groups.erase(group->type);
    pimpl->groupPool.free(group);
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
    if(chunk->count < chunk->get_capacity())
    {
        chunk->index = (uint32_t)chunks.size();
        chunks.push_back(chunk);
    }
    else
    {
        chunks.resize(chunks.size() + 1);
        for(uint32_t i = firstFree; i < (uint32_t)chunks.size() - 1; ++i)
        {
            chunks[i]->index = i+1;
            chunks[i + 1] = chunks[i];
        }
        chunks[firstFree] = chunk;
        chunk->index = firstFree;
        firstFree++;
    }
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
    if(chunk->index < firstFree)
        firstFree--;
    for(uint32_t i = chunk->index; i < (uint32_t)chunks.size(); ++i)
    {
        if (i + 1 < (uint32_t)chunks.size())
        {
            chunks[i] = chunks[i + 1];
            chunks[i]->index = i;
        }
        else
            chunks.pop_back();
    }
    chunk->group = nullptr;
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
    auto& slot = chunks[chunk->index];
    std::swap(chunks[firstFree]->index, chunk->index);
    std::swap(chunks[firstFree], slot);
    firstFree++;
}

void sugoi_group_t::clear()
{
    using namespace sugoi;
    for(auto chunk : chunks)
    {
        archetype->storage->pimpl->entity_registry.free_entities({ chunk, 0, chunk->count });
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
        auto metaGroup = storage->pimpl->entity_registry.entries[e_id(type.meta.data[i])].chunk->group;
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
        auto metaGroup = storage->pimpl->entity_registry.entries[e_id(type.meta.data[i])].chunk->group;
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
    std::bitset<32> mask;
    for (SIndex i = 0; i < subtype.length; ++i)
    {
        if (share(subtype.data[i]))
        {
            mask.set(i);
            break;
        }
    }
    return mask.to_ulong();
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
        auto metaGroup = storage->pimpl->entity_registry.entries[e_id(type.meta.data[i])].chunk->group;
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
    std::bitset<32> mask;
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
            mask.set(i);
            ++i;
            ++j;
        }
    }
    return mask.to_ulong();
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

uint32_t sugoiG_get_stable_order(const sugoi_group_t* group, sugoi_type_index_t localType)
{
    return group->archetype->stableOrder[localType];
}

const void* sugoiG_get_shared_ro(const sugoi_group_t* group, sugoi_type_index_t type)
{
    return group->get_shared_ro(type);
}
}