

#include "archetype.hpp"
#include "chunk_view.hpp"
#include "pool.hpp"
#include "serialize.hpp"
#include "stack.hpp"
#include "storage.hpp"
#include "type.hpp"
#include "type_registry.hpp"
#include "constants.hpp"
#include "iterator_ref.hpp"
#include "array.hpp"
#include "phmap.h"
#include <atomic>
#include "set.hpp"
#include "scheduler.hpp"
#ifndef forloop
    #define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

namespace dual
{
void serializer_t::archive(void* data, uint32_t bytes)
{
    v->stream(t, data, bytes);
}

void serializer_t::peek(void* data, uint32_t bytes)
{
    v->peek(t, data, bytes);
}

bool serializer_t::is_serialize()
{
    return v->is_serialize(t);
}
} // namespace dual

static void serialize_impl(const dual_chunk_view_t& view, dual_type_index_t type, EIndex offset, uint32_t size, uint32_t elemSize, dual::serializer_t s, void (*serialize)(dual_chunk_t* chunk, EIndex index, char* data, EIndex count, const dual_serializer_v* v, void* s))
{
    using namespace dual;
    char* src = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    if (type_index_t(type).is_buffer())
    {
        // array component is pointer based and must be converted to persistent data format
        if (s.is_serialize())
        {
            forloop(i, 0, view.count)
            {
                auto array = (dual_array_component_t*)((size_t)i * size + src);
                auto temp = *array;
                *(intptr_t*)temp.BeginX = ((char*)temp.BeginX - (char*)(array + 1)); // padding
                *(intptr_t*)temp.EndX = ((char*)temp.EndX - (char*)temp.BeginX);     // length
                s.archive(temp);
                if (serialize)
                    serialize(view.chunk, view.start + i, (char*)array->BeginX, (EIndex)(((char*)array->EndX - (char*)array->BeginX) / elemSize), s.v, s.t);
                else
                    s.archive(array->BeginX, (uint32_t)((char*)array->EndX - (char*)array->BeginX));
            }
        }
        else
        {
            forloop(i, 0, view.count)
            {
                auto array = (dual_array_component_t*)((size_t)i * size + src);
                s.archive(*array);
                auto padding = (intptr_t)array->BeginX;
                auto length = (intptr_t)array->EndX;
                if (padding > elemSize) // array on heap
                {
                    array->BeginX = llvm_vecsmall::SmallVectorBase::allocate(length);
                    array->CapacityX = array->EndX = (char*)array->BeginX + length;
                }
                else
                {
                    array->BeginX = (char*)(array + 1) + padding;
                    array->EndX = (char*)array->BeginX + length;
                    array->CapacityX = (char*)array + size;
                }
                if (serialize)
                    serialize(view.chunk, view.start + i, (char*)array->BeginX, (EIndex)length, s.v, s.t);
                else
                    s.archive(array->BeginX, size);
            }
        }
    }
    else
    {
        if (serialize)
            serialize(view.chunk, view.start, src, view.count, s.v, s.t);
        else
            s.archive(src, size * view.count);
    }
}

void dual::serialize_view(const dual_chunk_view_t& view, dual::serializer_t s, bool withEntities)
{
    using namespace dual;
    s.archive(view.count);

    archetype_t* type = view.chunk->type;
    EIndex* offsets = type->offsets[(int)view.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    if (withEntities)
        s.archive(view.chunk->get_entities() + view.start, view.count);
    for (int i = 0; i < type->type.length; ++i)
        serialize_impl(view, type->type.data[i], offsets[i], sizes[i], elemSizes[i], s, type->callbacks[i].serialize);
}

void dual_storage_t::serialize_type(const dual_entity_type_t& type, dual::serializer_t s)
{
    using namespace dual;
    // group is define by entity_type, so we just serialize it's type
    // todo: assert(s.is_serialize());
    s.archive(type.type.length);
    auto reg = type_registry_t::get();
    for (auto t : type.type)
        s.archive(reg.descriptions[type_index_t(t).index()].guid);
    s.archive(type.meta.length);
    s.archive(type.meta.data, type.meta.length);
}

dual_entity_type_t dual_storage_t::deserialize_type(dual::fixed_stack_t& stack, dual::serializer_t s)
{
    using namespace dual;
    // deserialize type, and get/create group from it
    // todo: assert(!s.is_serialize());
    dual_entity_type_t type;
    s.archive(type.type.length);
    auto guids = stack.allocate<guid_t>(type.type.length);
    s.archive(guids, type.type.length);
    type.type.data = stack.allocate<dual_type_index_t>(type.type.length);
    auto reg = type_registry_t::get();
    forloop(i, 0, type.type.length) // todo: check type existence
    ((dual_type_index_t*)type.type.data)[i] = reg.guid2type[guids[i]];
    std::sort((dual_type_index_t*)type.type.data, (dual_type_index_t*)type.type.data + type.type.length);
    s.archive(type.meta.length);
    if (type.meta.length > 0)
    {
        // todo: how to patch meta? guid?
        type.meta.data = stack.allocate<dual_entity_t>(type.meta.length);
        s.archive(type.meta.data, type.meta.length);
        std::sort((dual_entity_t*)type.meta.data, (dual_entity_t*)type.meta.data + type.meta.length);
    }
    return type;
}

void dual_storage_t::serialize_single(dual_entity_t e, dual::serializer_t s)
{
    using namespace dual;
    auto view = entity_view(e);
    auto type = view.chunk->group->type;
    type.meta.length = 0; // remove meta
    serialize_type(type, s);
    serialize_view(view, s, false);
}

dual_entity_t dual_storage_t::deserialize_single(dual::serializer_t s)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    auto type = deserialize_type(localStack, s);
    auto group = get_group(type);
    scheduler->sync_archetype(group->archetype);
    auto view = allocate_view_strict(group, 1);
    serialize_view(view, s, false);
    entities.fill_entities(view);
    return view.chunk->get_entities()[view.start];
}

//[count] ([group] [slice])*
void dual_storage_t::serialize_prefab(dual_entity_t e, dual::serializer_t s)
{
    using namespace dual;
    s.archive((EIndex)1);
    assert(scheduler->is_main_thread(this));
    scheduler->sync_archetype(entity_view(e).chunk->type);
    serialize_single(e, s);
}

void dual_storage_t::serialize_prefab(dual_entity_t* es, EIndex n, serializer_t s)
{
    using namespace dual;
    s.archive(n);
    assert(scheduler->is_main_thread(this));
    forloop(i, 0, n)
    scheduler->sync_archetype(entity_view(es[i]).chunk->type);
    linked_to_prefab(es, n);
    forloop(i, 0, n)
    serialize_single(es[i], s);
    prefab_to_linked(es, n);
}

dual_entity_t dual_storage_t::deserialize_prefab(dual::serializer_t s)
{
    using namespace dual;
    assert(scheduler->is_main_thread(this));
    EIndex count;
    s.archive(count);
    // todo: assert(count > 0)
    if (count == 1)
    {
        return deserialize_single(s);
    }
    else
    {
        fixed_stack_scope_t _(localStack);
        auto prefab = localStack.allocate<dual_entity_t>(count);
        forloop(i, 0, count)
        prefab[i] = deserialize_single(s);
        prefab_to_linked(prefab, count);
        return prefab[0];
    }
}

void dual_storage_t::serialize(dual::serializer_t s)
{
    using namespace dual;
    assert(scheduler->is_main_thread(this));
    scheduler->sync_storage(this);
    s.archive(entities.entries.size());
    s.archive(entities.entries.data(), entities.entries.size());
    s.archive(entities.freeEntries.size());
    s.archive(entities.freeEntries.data(), entities.freeEntries.size());
    s.archive(entities.size);
    s.archive(groups.size());
    for (auto& pair : groups)
    {
        auto group = pair.second;
        serialize_type(group->type, s);
        s.archive(group->chunkCount);
        for (dual_chunk_t* c = group->firstChunk; c; c = c->next)
            serialize_view({ c, 0, c->count }, s, true);
    }
}

void dual_storage_t::deserialize(dual::serializer_t s)
{
    using namespace dual;
    assert(scheduler->is_main_thread(this));
    scheduler->sync_storage(this);
    // todo: assert(entities.entries.size() == 0)
    uint32_t size;
    s.archive(size);
    entities.entries.resize(size);
    s.archive(entities.entries.data(), size);
    uint32_t freeSize;
    s.archive(freeSize);
    entities.freeEntries.resize(freeSize);
    s.archive(entities.freeEntries.data(), freeSize);
    s.archive(entities.size);
    uint32_t groupSize;
    s.archive(groupSize);
    forloop(i, 0, groupSize)
    {
        fixed_stack_scope_t _(localStack);
        auto type = deserialize_type(localStack, s);
        auto group = construct_group(type);
        uint32_t chunkCount;
        s.archive(chunkCount);
        forloop(j, 0, chunkCount)
        {
            EIndex chunkSize;
            s.peek(chunkSize);
            auto view = allocate_view_strict(group, chunkSize);
            serialize_view(view, s, true);
        }
    }
}