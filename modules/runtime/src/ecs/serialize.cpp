#include "storage.hpp"
#include "archetype.hpp"
#include "chunk_view.hpp"
#include "ecs/dual.h"
#include "pool.hpp"
#include "stack.hpp"
#include "type.hpp"
#include "ecs/constants.hpp"
#include "iterator_ref.hpp"
#include "ecs/array.hpp"
#include "set.hpp"
#include "scheduler.hpp"
#include "internal/utils.hpp"
#include "serde/binary/reader.h"
#include "serde/binary/writer.h"
#include "type_registry.hpp"

template<class T>
static void ArchiveBuffer(skr_binary_writer_t* writer, const T* buffer, uint32_t count)
{
    skr::binary::WriteBytes(writer, (const void*)buffer, sizeof(T) * count);
}

template<class T>
static void ArchiveBuffer(skr_binary_reader_t* reader, T* buffer, uint32_t count)
{
    skr::binary::ReadBytes(reader, (void*)buffer, sizeof(T) * count);
}

static void serialize_impl(const dual_chunk_view_t& view, dual_type_index_t type, EIndex offset, uint32_t size, uint32_t elemSize, skr_binary_writer_t* s, skr_binary_reader_t* ds
, void (*serialize)(dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* writer)
, void (*deserialize)(dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* writer))
{
    using namespace dual;
    namespace bin = skr::binary;
    bool isSeralize = s != nullptr;
    char* src = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    if (type_index_t(type).is_buffer())
    {
        // array component is pointer based and must be converted to persistent data format
        if (isSeralize)
        {
            forloop (i, 0, view.count)
            {
                auto array = (dual_array_comp_t*)((size_t)i * size + src);
                intptr_t padding = ((char*)array->BeginX - (char*)(array + 1));
                intptr_t length = ((char*)array->EndX - (char*)array->BeginX);
                bin::Archive(s, (uint32_t)padding);
                bin::Archive(s, (uint32_t)length);
                if (serialize)
                    serialize(view.chunk, view.start + i, (char*)array->BeginX, (EIndex)(length / elemSize), s);
                else
                    ArchiveBuffer(s, (uint8_t*)array->BeginX, static_cast<uint32_t>(length));
            }
        }
        else
        {
            forloop (i, 0, view.count)
            {
                auto array = (dual_array_comp_t*)((size_t)i * size + src);
                uint32_t padding = 0, length = 0;
                bin::Archive(ds, padding);
                bin::Archive(ds, length);
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
                if (deserialize)
                    deserialize(view.chunk, view.start + i, (char*)array->BeginX, (EIndex)length, ds);
                else
                    ArchiveBuffer(ds, (uint8_t*)array->BeginX, length);
            }
        }
    }
    else
    {
        if (isSeralize)
        {
            if (serialize)
                serialize(view.chunk, view.start, src, view.count, s);
            else
                ArchiveBuffer(s, src, size * view.count);
        }
        else 
        {
            if (deserialize)
                deserialize(view.chunk, view.start, src, view.count, ds);
            else
                ArchiveBuffer(ds, src, size * view.count);
        }
    }
}

void dual_storage_t::serialize_view(dual_group_t* group, dual_chunk_view_t& view, skr_binary_writer_t* s, skr_binary_reader_t* ds, bool withEntities)
{
    using namespace dual;
    namespace bin = skr::binary;
    if(s)
        bin::Archive(s, view.count);
    else
    {
        bin::Archive(ds, view.count);
        SKR_ASSERT(view.count);
        view = allocate_view_strict(group, view.count);
    }

    archetype_t* type = view.chunk->type;
    EIndex* offsets = type->offsets[(int)view.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    if (withEntities)
    {
        if(s)
            ArchiveBuffer(s, view.chunk->get_entities() + view.start, view.count);
        else
            ArchiveBuffer(ds, view.chunk->get_entities() + view.start, view.count);
    }
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
        serialize_impl(view, type->type.data[i], offsets[i], sizes[i], elemSizes[i], s, ds, type->callbacks[i].serialize, type->callbacks[i].deserialize);
}

void dual_storage_t::serialize_type(const dual_entity_type_t& type, skr_binary_writer_t* s, bool keepMeta)
{
    using namespace dual;
    namespace bin = skr::binary;
    // group is define by entity_type, so we just serialize it's type
    // todo: assert(s.is_serialize());
    bin::Archive(s, type.type.length);
    auto& reg = type_registry_t::get();
    for (auto t : type.type)
        bin::Archive(s, reg.descriptions[type_index_t(t).index()].guid);
    if(keepMeta)
    {
        bin::Archive(s, type.meta.length);
        ArchiveBuffer(s, type.meta.data, type.meta.length);
    }
}

dual_entity_type_t dual_storage_t::deserialize_type(dual::fixed_stack_t& stack, skr_binary_reader_t* s, bool keepMeta)
{
    using namespace dual;
    namespace bin = skr::binary;
    // deserialize type, and get/create group from it
    dual_entity_type_t type = {};
    bin::Archive(s, type.type.length);
    auto guids = stack.allocate<guid_t>(type.type.length);
    ArchiveBuffer(s, guids, type.type.length);
    type.type.data = stack.allocate<dual_type_index_t>(type.type.length);
    auto& reg = type_registry_t::get();
    forloop (i, 0, type.type.length) // todo: check type existence
        ((dual_type_index_t*)type.type.data)[i] = reg.guid2type[guids[i]];
    std::sort((dual_type_index_t*)type.type.data, (dual_type_index_t*)type.type.data + type.type.length);
    if(keepMeta)
    {
        bin::Archive(s, type.meta.length);
        if (type.meta.length > 0)
        {
            // todo: how to patch meta? guid?
            type.meta.data = stack.allocate<dual_entity_t>(type.meta.length);
            ArchiveBuffer(s, type.meta.data, type.meta.length);
            std::sort((dual_entity_t*)type.meta.data, (dual_entity_t*)type.meta.data + type.meta.length);
        }
    }
    return type;
}

void dual_storage_t::serialize_single(dual_entity_t e, skr_binary_writer_t* s)
{
    using namespace dual;
    auto view = entity_view(e);
    auto type = view.chunk->group->type;
    type.meta.length = 0; // remove meta
    serialize_type(type, s, false);
    serialize_view(view.chunk->group, view, s, nullptr, false);
}

dual_entity_t dual_storage_t::deserialize_single(skr_binary_reader_t* s)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    auto type = deserialize_type(localStack, s, false);
    auto group = get_group(type);
    if(scheduler)
        scheduler->sync_archetype(group->archetype);   
    dual_chunk_view_t view;
    serialize_view(group, view, nullptr, s, false);
    entities.fill_entities(view);
    return view.chunk->get_entities()[view.start];
}

//[count] ([group] [slice])*
void dual_storage_t::serialize_prefab(dual_entity_t e, skr_binary_writer_t* s)
{
    using namespace dual;
    namespace bin = skr::binary;
    bin::Archive(s, (EIndex)1);
    if(scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_archetype(entity_view(e).chunk->type);
    }
    serialize_single(e, s);
}

void dual_storage_t::serialize_prefab(dual_entity_t* es, EIndex n, skr_binary_writer_t* s)
{
    using namespace dual;
    namespace bin = skr::binary;
    bin::Archive(s, n);
    if(scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        forloop (i, 0, n)
            scheduler->sync_archetype(entity_view(es[i]).chunk->type);
    }
    linked_to_prefab(es, n);
    forloop (i, 0, n)
        serialize_single(es[i], s);
    prefab_to_linked(es, n);
}

dual_entity_t dual_storage_t::deserialize_prefab(skr_binary_reader_t* s)
{
    using namespace dual;
    namespace bin = skr::binary;
    if(scheduler)
        SKR_ASSERT(scheduler->is_main_thread(this));
    EIndex count = 0;
    bin::Archive(s, count);
    // todo: assert(count > 0)
    if (count == 1)
    {
        return deserialize_single(s);
    }
    else if(count > 1)
    {
        fixed_stack_scope_t _(localStack);
        auto prefab = localStack.allocate<dual_entity_t>(count);
        forloop (i, 0, count)
            prefab[i] = deserialize_single(s);
        prefab_to_linked(prefab, count);
        return prefab[0];
    }
    SKR_UNREACHABLE_CODE();
    return dual_entity_t();
}

void dual_storage_t::serialize(skr_binary_writer_t* s)
{
    ZoneScopedN("dual_storage_t::serialize");
    using namespace dual;
    namespace bin = skr::binary;
    if(scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_storage(this);
    }
    {
        ZoneScopedN("serialize entities");
        bin::Archive(s, (uint32_t)entities.entries.size());
        bin::Archive(s, (uint32_t)entities.freeEntries.size());
        ArchiveBuffer(s, entities.freeEntries.data(), static_cast<uint32_t>(entities.freeEntries.size()));
    }
    bin::Archive(s, (uint32_t)groups.size());
    for (auto& pair : groups)
    {
        ZoneScopedN("serialize group");
        auto group = pair.second;
        serialize_type(group->type, s, true);
        bin::Archive(s, (uint32_t)group->chunks.size());
        for(auto c : group->chunks)
        {
            ZoneScopedN("serialize chunk");
            dual_chunk_view_t view = { c, 0, c->count };
            serialize_view(group, view, s, nullptr, true);
        }
    }
}

void dual_storage_t::deserialize(skr_binary_reader_t* s)
{
    using namespace dual;
    ZoneScopedN("dual_storage_t::deserialize");
    namespace bin = skr::binary;
    if(scheduler)
    {
        SKR_ASSERT(scheduler->is_main_thread(this));
        scheduler->sync_storage(this);
    }
    // empty storage expected
    SKR_ASSERT(entities.entries.size() == 0);
    uint32_t size = 0;
    bin::Archive(s, size);
    entities.entries.resize(size);
    uint32_t freeSize = 0;
    bin::Archive(s, freeSize);
    entities.freeEntries.resize(freeSize);
    ArchiveBuffer(s, entities.freeEntries.data(), freeSize);
    uint32_t groupSize = 0;
    bin::Archive(s, groupSize);
    forloop (i, 0, groupSize)
    {
        ZoneScopedN("deserialize group");
        fixed_stack_scope_t _(localStack);
        auto type = deserialize_type(localStack, s, true);
        auto group = construct_group(type);
        uint32_t chunkCount = 0;
        bin::Archive(s, chunkCount);
        forloop (j, 0, chunkCount)
        {
            ZoneScopedN("deserialize chunk");
            dual_chunk_view_t view;
            serialize_view(group, view, nullptr, s, true);
            auto ents = dualV_get_entities(&view);
            forloop(k, 0, view.count)
            {
                entity_registry_t::entry_t entry;
                entry.chunk = view.chunk;
                entry.indexInChunk = k + view.start;
                entry.version = e_version(ents[k]);
                entities.entries[e_id(ents[k])] = entry;
            }
        }
    }
}