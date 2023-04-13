#include "type_registry.hpp"
#include "entity.hpp"
#include "ecs/dual.h"
#include "mask.hpp"
#include "storage.hpp"
#include "archetype.hpp"
#include "type.hpp"

#include "ecs/constants.hpp"
#include "set.hpp"
#include "type.hpp"
#include "stack.hpp"
#include "storage.hpp"
#include "utils/make_zeroed.hpp"
#include <EASTL/sort.h>
#include <EASTL/bitset.h>
#include "internal/utils.hpp"

namespace dual
{

thread_local fixed_stack_t localStack(4096 * 8);

bool archetype_t::with_chunk_component() const noexcept
{
    return firstChunkComponent != type.length;
}

SIndex archetype_t::index(dual_type_index_t inType) const noexcept
{
    auto end = type.data + type.length;
    const dual_type_index_t* result = eastl::lower_bound(type.data, end, inType);
    if (result != end && *result == inType)
        return (SIndex)(result - type.data);
    else
        return kInvalidSIndex;
}
} // namespace dual

dual::archetype_t* dual_storage_t::construct_archetype(const dual_type_set_t& inType)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    char* buffer = (char*)archetypeArena.allocate(data_size(inType), 1);
    archetype_t& proto = *archetypeArena.allocate<archetype_t>();
    proto.storage = this;
    proto.type = clone(inType, buffer);
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
        proto.offsets[i] = archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.elemSizes = archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.callbackFlags = archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.aligns = archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.sizes = archetypeArena.allocate<uint32_t>(proto.type.length);
    proto.resourceFields = archetypeArena.allocate<dual::resource_fields_t>(proto.type.length);
    proto.callbacks = archetypeArena.allocate<dual_callback_v>(proto.type.length);
    ::memset(proto.callbacks, 0, sizeof(dual_callback_v) * proto.type.length);
    auto& registry = type_registry_t::get();
    forloop (i, 0, proto.type.length)
    {
        const auto& desc = registry.descriptions[type_index_t(proto.type.data[i]).index()];
        uint32_t callbackFlag = 0;
        if (desc.callback.constructor)
            callbackFlag |= DCF_CTOR;
        if (desc.callback.destructor)
            callbackFlag |= DCF_DTOR;
        if (desc.callback.copy)
            callbackFlag |= DCF_COPY;
        if (desc.callback.move)
            callbackFlag |= DCF_MOVE;
        if (desc.callback.serialize || desc.callback.deserialize)
            callbackFlag |= DCF_SERDE;
        proto.callbackFlags[i] = callbackFlag;
        proto.callbacks[i] = desc.callback;
        proto.resourceFields[i] = { desc.resourceFields, desc.resourceFieldsCount };
    }
    auto guids = localStack.allocate<guid_t>(proto.type.length);
    auto stableOrder = localStack.allocate<SIndex>(proto.type.length);
    proto.entitySize = sizeof(dual_entity_t);
    uint32_t padding = 0;
    forloop (i, 0, proto.type.length)
    {
        auto t = proto.type.data[i];
        if (t == kMaskComponent)
            proto.withMask = true;
        if (t == kDirtyComponent)
            proto.withDirty = true;
        auto ti = type_index_t(t);
        auto& desc = registry.descriptions[ti.index()];
        proto.sizes[i] = desc.size;
        proto.elemSizes[i] = desc.elementSize;
        guids[i] = desc.guid;
        proto.aligns[i] = desc.alignment;
        stableOrder[i] = i;
        proto.entitySize += desc.size;
        if(!ti.is_chunk())
            padding += desc.alignment;
        if (!ti.is_chunk() && desc.entityFieldsCount != 0)
            proto.sizeToPatch += desc.size;
    }
    eastl::sort(stableOrder, stableOrder + proto.type.length, [&](SIndex lhs, SIndex rhs) {
        return guid_compare_t{}(guids[lhs], guids[rhs]);
    });
    size_t caps[] = { kSmallBinSize - sizeof(dual_chunk_t), kFastBinSize - sizeof(dual_chunk_t), kLargeBinSize - sizeof(dual_chunk_t) };
    const uint32_t versionSize = sizeof(uint32_t) * proto.type.length;
    forloop (i, 0, 3)
    {
        uint32_t* offsets = proto.offsets[i];
        uint32_t& capacity = proto.chunkCapacity[i];
        proto.versionOffset[i] = static_cast<uint32_t>(caps[i] - versionSize);
        uint32_t ccOffset = (uint32_t)(caps[i] - versionSize);
        forloop (j, 0, proto.type.length)
        {
            SIndex id = stableOrder[j];
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
        uint32_t offset = sizeof(dual_entity_t) * capacity;
        forloop (j, 0, proto.type.length)
        {
            SIndex id = stableOrder[j];TIndex t = proto.type.data[id];
            auto ti = type_index_t(t);
            if(!ti.is_chunk())
            {
                offset = (uint32_t)(proto.aligns[id] * ((offset + proto.aligns[id] - 1) / proto.aligns[id]));
                offsets[id] = offset;
                offset += proto.sizes[id] * capacity;
            }
        }
    }

    return archetypes.insert({ proto.type, &proto }).first->second;
}

dual_group_t* dual_storage_t::construct_group(const dual_entity_type_t& inType)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    dual_type_set_t structure;
    SIndex firstTag = 0;
    for (; firstTag < inType.type.length; ++firstTag)
        if (type_index_t(inType.type.data[firstTag]).is_tag())
            break;
    structure.data = inType.type.data;
    structure.length = firstTag;
    archetype_t* archetype = get_archetype(structure);
    const auto typeSize = static_cast<uint32_t>(data_size(inType));
    assert((sizeof(dual_group_t) + typeSize) < kGroupBlockSize);(void)typeSize;
    dual_group_t& proto = *new (groupPool.allocate()) dual_group_t();
    char* buffer = (char*)(&proto + 1);
    proto.firstFree = 0;
    buffer += sizeof(dual_group_t);
    dual_entity_type_t type = clone(inType, buffer);
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
        if (!t.is_tracked())
            toClone[toCloneCount++] = t;
        else
            hasTracked = true;
        if (t == kDeadComponent)
            proto.isDead = true;
        if (t == kDisableComponent)
            proto.disabled = true;
    }
    // eastl::sort(&toClean[0], &toClean[toCleanCount]); dead is always smaller
    proto.archetype = archetype;
    proto.size = 0;
    proto.timestamp = 0;
    proto.dead = nullptr;
    proto.cloned = proto.isDead ? nullptr : &proto;
    groups.insert({ type, &proto });
    update_query_cache(&proto, true);
    if (hasTracked && !proto.isDead)
    {
        auto deadType = make_zeroed<dual_entity_type_t>();
        deadType.type = { toClean, toCleanCount };
        proto.dead = get_group(deadType);
        dual_entity_type_t cloneType = make_zeroed<dual_entity_type_t>();
        cloneType.type = { toClone, toCloneCount };
        proto.cloned = get_group(cloneType);
    }
    return &proto;
}

dual::archetype_t* dual_storage_t::try_get_archetype(const dual_type_set_t& type) const
{
    if (auto i = archetypes.find(type); i != archetypes.end())
        return i->second;
    return nullptr;
}

dual_group_t* dual_storage_t::try_get_group(const dual_entity_type_t& type) const
{
    if (auto i = groups.find(type); i != groups.end())
        return i->second;
    return nullptr;
}

dual::archetype_t* dual_storage_t::get_archetype(const dual_type_set_t& type)
{
    archetype_t* archetype = try_get_archetype(type);
    if (!archetype)
        archetype = construct_archetype(type);
    return archetype;
}

dual_group_t* dual_storage_t::get_group(const dual_entity_type_t& type)
{
    using namespace dual;
    bool withTracked = false;
    bool dead = false;
    for(SIndex i=0; i < type.type.length; ++i)
    {
        type_index_t t = type.type.data[i];
        if(t.is_tracked())
            withTracked = true;
        if(t == kDeadComponent)
            dead = true;
    }
    if(dead && !withTracked)
        return nullptr;
    dual_group_t* group = try_get_group(type);
    if (!group)
        group = construct_group(type);
    return group;
}

void dual_storage_t::destruct_group(dual_group_t* group)
{
    update_query_cache(group, false);
    groups.erase(group->type);
    groupPool.free(group);
}

dual_chunk_t* dual_group_t::get_first_free_chunk() const noexcept
{
    if (firstFree == (uint32_t)chunks.size())
        return nullptr;
    return chunks[firstFree];
}

dual_chunk_t* dual_group_t::new_chunk(uint32_t hint)
{
    using namespace dual;
    pool_type_t pt;
    if (chunks.size() < kSmallBinThreshold && hint < archetype->chunkCapacity[PT_small])
        pt = PT_small;
    else if (hint > archetype->chunkCapacity[PT_default] * 8u)
        pt = PT_large;
    else
        pt = PT_default;
    dual_chunk_t* chunk = dual_chunk_t::create(pt);
    add_chunk(chunk);
    construct_chunk(chunk);
    return chunk;
}

void dual_group_t::add_chunk(dual_chunk_t* chunk)
{
    using namespace dual;
    size += chunk->count;
    chunk->type = archetype;
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

void dual_group_t::resize_chunk(dual_chunk_t* chunk, EIndex newSize)
{
    using namespace dual;
    bool full = chunk->count == chunk->get_capacity();
    size = size + newSize - chunk->count;
    chunk->count = newSize;
    if (newSize == 0)
    {
        destruct_chunk(chunk);
        remove_chunk(chunk);
        dual_chunk_t::destroy(chunk);
    }
    else
    {
        if (!full && chunk->get_capacity() == newSize)
            mark_full(chunk);
        else if((full && chunk->get_capacity() > newSize))
            mark_free(chunk);
    }
}

void dual_group_t::remove_chunk(dual_chunk_t* chunk)
{
    using namespace dual;
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

void dual_group_t::mark_free(dual_chunk_t* chunk)
{
    using namespace dual;
    SKR_ASSERT(chunk->index < firstFree);
    firstFree--;
    auto& slot = chunks[chunk->index];
    eastl::swap(chunks[firstFree]->index, chunk->index);
    eastl::swap(chunks[firstFree], slot);
}

void dual_group_t::mark_full(dual_chunk_t* chunk)
{
    using namespace dual;
    
    SKR_ASSERT(chunk->index >= firstFree);
    auto& slot = chunks[chunk->index];
    eastl::swap(chunks[firstFree]->index, chunk->index);
    eastl::swap(chunks[firstFree], slot);
    firstFree++;
}

void dual_group_t::clear()
{
    using namespace dual;
    for(auto chunk : chunks)
    {
        destruct_view({ chunk, 0, chunk->count });
        destruct_chunk(chunk);
        dual_chunk_t::destroy(chunk);
    }
    chunks.clear();
    size = 0;
}

TIndex dual_group_t::index(dual_type_index_t inType) const noexcept
{
    using namespace dual;
    auto end = type.type.data + type.type.length;
    const dual_type_index_t* result = eastl::lower_bound(type.type.data, end, inType);
    if (result != end && *result == inType)
        return (TIndex)(result - type.type.data);
    else
        return kInvalidSIndex;
}

bool dual_group_t::share(dual_type_index_t t) const noexcept
{
    using namespace dual;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto metaGroup = storage->entities.entries[e_id(type.meta.data[i])].chunk->group;
        if (metaGroup->index(t) != kInvalidSIndex)
            return true;
        if (metaGroup->share(t))
            return true;
    }
    return false;
}

bool dual_group_t::own(const dual_type_set_t& subtype) const noexcept
{
    return dual::set_utils<dual_type_index_t>::all(type.type, subtype);
}

bool dual_group_t::share(const dual_type_set_t& subtype) const noexcept
{
    using namespace dual;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto metaGroup = storage->entities.entries[e_id(type.meta.data[i])].chunk->group;
        if (metaGroup->own(subtype))
            return true;
        if (metaGroup->share(subtype))
            return true;
    }
    return false;
}

dual_mask_comp_t dual_group_t::get_shared_mask(const dual_type_set_t& subtype) const noexcept
{
    using namespace dual;
    eastl::bitset<32> mask;
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

void dual_group_t::get_shared_type(dual_type_set_t& result, void* buffer) const noexcept
{
    using namespace dual;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto view = storage->entity_view(type.meta.data[i]);
        auto metaGroup = view.chunk->group;
        dual_type_set_t merged;
        if (metaGroup->archetype->withMask)
        {
            auto mask = *(dual_mask_comp_t*)dualV_get_owned_ro(&view, kMaskComponent);
            merged = set_utils<dual_type_index_t>::merge_with_mask(result, metaGroup->type.type, mask, buffer);
        }
        else
        {
            merged = set_utils<dual_type_index_t>::merge(result, metaGroup->type.type, buffer);
        }
        buffer = (void*)result.data;
        result = merged;
        metaGroup->get_shared_type(result, buffer);
    }
}

const dual_group_t* dual_group_t::get_owner(dual_type_index_t t) const noexcept
{
    using namespace dual;
    if (index(t) != kInvalidSIndex)
        return this;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto metaGroup = storage->entities.entries[e_id(type.meta.data[i])].chunk->group;
        if (auto g = metaGroup->get_owner(t))
            return g;
    }
    return nullptr;
}

const void* dual_group_t::get_shared_ro(dual_type_index_t t) const noexcept
{
    using namespace dual;
    auto storage = archetype->storage;
    for (EIndex i = 0; i < type.meta.length; ++i)
    {
        auto view = storage->entity_view(type.meta.data[i]);
        if (auto data = dualV_get_component_ro(&view, t))
            return data;
        return view.chunk->group->get_shared_ro(t);
    }
    return nullptr;
}

dual_mask_comp_t dual_group_t::get_mask(const dual_type_set_t& subtype) const noexcept
{
    using namespace dual;
    eastl::bitset<32> mask;
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
int dualG_has_components(const dual_group_t* group, const dual_type_set_t* types)
{
    return group->own(*types) || group->share(*types);
}

int dualG_own_components(const dual_group_t* group, const dual_type_set_t* types)
{
    return group->own(*types);
}

int dualG_share_components(const dual_group_t* group, const dual_type_set_t* types)
{
    return group->share(*types);
}

void dualG_get_type(const dual_group_t* group, dual_entity_type_t* type)
{
    *type = group->type;
}

const void* dualG_get_shared_ro(const dual_group_t* group, dual_type_index_t type)
{
    return group->get_shared_ro(type);
}
}