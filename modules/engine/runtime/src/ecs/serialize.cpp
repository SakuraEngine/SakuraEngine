#include "SkrProfile/profile.h"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/type_registry.hpp"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"

#include "./chunk.hpp"
#include "./impl/storage.hpp"
#include "./stack.hpp"
#include "./archetype.hpp"
#include "./scheduler.hpp"
#include "./utilities.hpp"

template <class T>
static void WriteBuffer(SBinaryWriter* writer, const T* buffer, uint32_t count)
{
    writer->write((const void*)buffer, sizeof(T) * count);
}

template <class T>
static void ReadBuffer(SBinaryReader* reader, T* buffer, uint32_t count)
{
    reader->read((void*)buffer, sizeof(T) * count);
}

static void serialize_impl(const sugoi_chunk_view_t& view, sugoi_type_index_t type, EIndex offset, uint32_t size, uint32_t elemSize, SBinaryWriter* s, SBinaryReader* ds, void (*serialize)(sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryWriter* writer), void (*deserialize)(sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryReader* writer))
{
    using namespace sugoi;
    namespace bin    = skr::binary;
    bool  isSeralize = s != nullptr;
    char* src        = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    if (type_index_t(type).is_buffer())
    {
        // array component is pointer based and must be converted to persistent data format
        if (isSeralize)
        {
            forloop (i, 0, view.count)
            {
                auto     array   = (sugoi_array_comp_t*)((size_t)i * size + src);
                intptr_t padding = ((char*)array->BeginX - (char*)(array + 1));
                intptr_t length  = ((char*)array->EndX - (char*)array->BeginX);
                bin::Write(s, (uint32_t)padding);
                bin::Write(s, (uint32_t)length);
                if (serialize)
                    serialize(view.chunk, view.start + i, (char*)array->BeginX, (EIndex)(length / elemSize), s);
                else
                    WriteBuffer(s, (uint8_t*)array->BeginX, static_cast<uint32_t>(length));
            }
        }
        else
        {
            forloop (i, 0, view.count)
            {
                auto     array   = (sugoi_array_comp_t*)((size_t)i * size + src);
                uint32_t padding = 0, length = 0;
                bin::Read(ds, padding);
                bin::Read(ds, length);
                if (padding > elemSize) // array on heap
                {
                    array->BeginX    = llvm_vecsmall::SmallVectorBase::allocate(length);
                    array->CapacityX = array->EndX = (char*)array->BeginX + length;
                }
                else
                {
                    array->BeginX    = (char*)(array + 1) + padding;
                    array->EndX      = (char*)array->BeginX + length;
                    array->CapacityX = (char*)array + size;
                }
                if (deserialize)
                    deserialize(view.chunk, view.start + i, (char*)array->BeginX, (EIndex)length, ds);
                else
                    ReadBuffer(ds, (uint8_t*)array->BeginX, length);
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
                WriteBuffer(s, src, size * view.count);
        }
        else
        {
            if (deserialize)
                deserialize(view.chunk, view.start, src, view.count, ds);
            else
                ReadBuffer(ds, src, size * view.count);
        }
    }
}

void sugoi_storage_t::serialize_view(sugoi_group_t* group, sugoi_chunk_view_t& view, SBinaryWriter* s, SBinaryReader* ds, bool withEntities)
{
    using namespace sugoi;
    namespace bin = skr::binary;
    if (s)
        bin::Write(s, view.count);
    else
    {
        bin::Read(ds, view.count);
        SKR_ASSERT(view.count);
        view = allocateViewStrict(group, view.count);
    }

    archetype_t* type      = view.chunk->structure;
    EIndex*      offsets   = type->offsets[(int)view.chunk->pt];
    uint32_t*    sizes     = type->sizes;
    uint32_t*    elemSizes = type->elemSizes;
    if (withEntities)
    {
        if (s)
            WriteBuffer(s, view.chunk->get_entities() + view.start, view.count);
        else
            ReadBuffer(ds, view.chunk->get_entities() + view.start, view.count);
    }
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
        serialize_impl(view, type->type.data[i], offsets[i], sizes[i], elemSizes[i], s, ds, type->callbacks[i].serialize, type->callbacks[i].deserialize);
}

void sugoi_storage_t::serialize_type(const sugoi_entity_type_t& type, SBinaryWriter* s, bool keepMeta)
{
    using namespace sugoi;
    namespace bin = skr::binary;
    // group is define by entity_type, so we just serialize it's type
    // todo: assert(s.is_serialize());
    bin::Write(s, type.type.length);
    auto& reg = TypeRegistry::get();
    for (SIndex i = 0; i < type.type.length; i++)
    {
        auto tid = type_index_t(type.type.data[i]).index();
        bin::Write(s, reg.get_type_desc(tid)->guid);
    }
    if (keepMeta)
    {
        bin::Write(s, type.meta.length);
        WriteBuffer(s, type.meta.data, type.meta.length);
    }
}

sugoi_entity_type_t sugoi_storage_t::deserialize_type(sugoi::fixed_stack_t& stack, SBinaryReader* s, bool keepMeta)
{
    using namespace sugoi;
    namespace bin = skr::binary;
    // deserialize type, and get/create group from it
    sugoi_entity_type_t type = {};
    bin::Read(s, type.type.length);
    auto guids = stack.allocate<guid_t>(type.type.length);
    ReadBuffer(s, guids, type.type.length);
    type.type.data = stack.allocate<sugoi_type_index_t>(type.type.length);
    auto& reg      = TypeRegistry::get();
    forloop (i, 0, type.type.length) // todo: check type existence
        ((sugoi_type_index_t*)type.type.data)[i] = reg.get_type(guids[i]);
    std::sort((sugoi_type_index_t*)type.type.data, (sugoi_type_index_t*)type.type.data + type.type.length);
    if (keepMeta)
    {
        bin::Read(s, type.meta.length);
        if (type.meta.length > 0)
        {
            // todo: how to patch meta? guid?
            type.meta.data = stack.allocate<sugoi_entity_t>(type.meta.length);
            ReadBuffer(s, type.meta.data, type.meta.length);
            std::sort((sugoi_entity_t*)type.meta.data, (sugoi_entity_t*)type.meta.data + type.meta.length);
        }
    }
    return type;
}

void sugoi_storage_t::serialize_single(sugoi_entity_t e, SBinaryWriter* s)
{
    using namespace sugoi;
    auto view        = entity_view(e);
    auto type        = view.chunk->group->type;
    type.meta.length = 0; // remove meta
    serialize_type(type, s, false);
    serialize_view(view.chunk->group, view, s, nullptr, false);
}

sugoi_entity_t sugoi_storage_t::deserialize_single(SBinaryReader* s)
{
    using namespace sugoi;
    fixed_stack_scope_t _(localStack);
    auto                type  = deserialize_type(localStack, s, false);
    auto                group = get_group(type);
    if (pimpl->scheduler)
        pimpl->scheduler->sync_archetype(group->archetype);
    sugoi_chunk_view_t view;
    serialize_view(group, view, nullptr, s, false);
    pimpl->entity_registry.fill_entities(view);
    return view.chunk->get_entities()[view.start];
}

//[count] ([group] [slice])*
void sugoi_storage_t::serialize_prefab(sugoi_entity_t e, SBinaryWriter* s)
{
    using namespace sugoi;
    namespace bin = skr::binary;
    bin::Write(s, (EIndex)1);
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_archetype(entity_view(e).chunk->structure);
    }
    serialize_single(e, s);
}

void sugoi_storage_t::serialize_prefab(sugoi_entity_t* es, EIndex n, SBinaryWriter* s)
{
    using namespace sugoi;
    namespace bin = skr::binary;
    bin::Write(s, n);
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        forloop (i, 0, n)
            pimpl->scheduler->sync_archetype(entity_view(es[i]).chunk->structure);
    }
    linked_to_prefab(es, n);
    forloop (i, 0, n)
        serialize_single(es[i], s);
    prefab_to_linked(es, n);
}

sugoi_entity_t sugoi_storage_t::deserialize_prefab(SBinaryReader* s)
{
    using namespace sugoi;
    namespace bin = skr::binary;
    if (pimpl->scheduler)
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
    EIndex count = 0;
    bin::Read(s, count);
    // todo: assert(count > 0)
    if (count == 1)
    {
        return deserialize_single(s);
    }
    else if (count > 1)
    {
        fixed_stack_scope_t _(localStack);
        auto                prefab = localStack.allocate<sugoi_entity_t>(count);
        forloop (i, 0, count)
            prefab[i] = deserialize_single(s);
        prefab_to_linked(prefab, count);
        return prefab[0];
    }
    SKR_UNREACHABLE_CODE();
    return sugoi_entity_t();
}

void sugoi_storage_t::serialize(SBinaryWriter* s)
{
    SkrZoneScopedN("sugoi_storage_t::serialize");
    using namespace sugoi;
    namespace bin = skr::binary;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_storage(this);
    }
    {
        SkrZoneScopedN("serialize entities");
        bin::Write(s, (uint32_t)pimpl->entity_registry.entries.size());
        bin::Write(s, (uint32_t)pimpl->entity_registry.freeEntries.size());
        WriteBuffer(s, pimpl->entity_registry.freeEntries.data(), static_cast<uint32_t>(pimpl->entity_registry.freeEntries.size()));
    }
    bin::Write(s, (uint32_t)pimpl->groups.size());
    for (auto& pair : pimpl->groups)
    {
        SkrZoneScopedN("serialize group");
        auto group = pair.second;
        serialize_type(group->type, s, true);
        bin::Write(s, (uint32_t)group->chunks.size());
        for (auto c : group->chunks)
        {
            SkrZoneScopedN("serialize chunk");
            sugoi_chunk_view_t view = { c, 0, c->count };
            serialize_view(group, view, s, nullptr, true);
        }
    }
}

void sugoi_storage_t::deserialize(SBinaryReader* s)
{
    using namespace sugoi;
    SkrZoneScopedN("sugoi_storage_t::deserialize");
    namespace bin = skr::binary;
    if (pimpl->scheduler)
    {
        SKR_ASSERT(pimpl->scheduler->is_main_thread(this));
        pimpl->scheduler->sync_storage(this);
    }
    // empty storage expected
    SKR_ASSERT(pimpl->entity_registry.entries.size() == 0);
    uint32_t size = 0;
    bin::Read(s, size);
    pimpl->entity_registry.entries.resize_default(size);
    uint32_t freeSize = 0;
    bin::Read(s, freeSize);
    pimpl->entity_registry.freeEntries.resize_default(freeSize);
    ReadBuffer(s, pimpl->entity_registry.freeEntries.data(), freeSize);
    uint32_t groupSize = 0;
    bin::Read(s, groupSize);
    forloop (i, 0, groupSize)
    {
        SkrZoneScopedN("deserialize group");
        fixed_stack_scope_t _(localStack);
        auto                type       = deserialize_type(localStack, s, true);
        auto                group      = constructGroup(type);
        uint32_t            chunkCount = 0;
        bin::Read(s, chunkCount);
        forloop (j, 0, chunkCount)
        {
            SkrZoneScopedN("deserialize chunk");
            sugoi_chunk_view_t view;
            serialize_view(group, view, nullptr, s, true);
            auto ents = sugoiV_get_entities(&view);
            forloop (k, 0, view.count)
            {
                EntityRegistry::entry_t entry;
                entry.chunk                                   = view.chunk;
                entry.indexInChunk                            = k + view.start;
                entry.version                                 = e_version(ents[k]);
                pimpl->entity_registry.entries[e_id(ents[k])] = entry;
            }
        }
    }
}