#include "SkrProfile/profile.h"
#include "SkrRuntime/sugoi/sugoi.h"
#include "SkrRuntime/sugoi/array.hpp"
#include "SkrRuntime/sugoi/type_registry.hpp"

#include "SkrRuntime/sugoi/chunk.hpp"
#include "chunk_view.hpp"
#include "./impl/storage.hpp"
#include "./stack.hpp"
#include "SkrRuntime/sugoi/archetype.hpp"

namespace sugoi::serde_help
{
// write entities
inline static void write_entities(skr::ArchiveWrite* writer, const sugoi_entity_t* buffer, uint64_t count)
{
    if (writer->is_structured())
    {
        skr::Archive::ArrayScope arr_scope(*writer);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        for (uint64_t i = 0; i < count; i++)
        {
            SKR_FAST_CHECK(writer->value(buffer[i]), );
        }
    }
    else
    {
        SKR_FAST_CHECK(writer->bytes(buffer, count * sizeof(sugoi_entity_t)), );
    }
}
inline static void read_entities(skr::ArchiveRead* reader, sugoi_entity_t* buffer, uint64_t count)
{
    if (reader->is_structured())
    {
        skr::Archive::ArrayScope arr_scope(*reader);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        uint64_t arr_count, adjusted_count;
        SKR_FAST_CHECK(reader->array_size_structured(arr_count), );
        SKR_FAST_CHECK(reader->adjust_array_size(arr_count, count, adjusted_count), );
        for (uint64_t i = 0; i < count; i++)
        {
            SKR_FAST_CHECK(reader->value(buffer[i]), );
        }
    }
    else
    {
        SKR_FAST_CHECK(reader->bytes(buffer, count * sizeof(sugoi_entity_t)), );
    }
}

// write components
inline static void write_components(
    skr::ArchiveWrite* writer,
    const sugoi_chunk_view_t& view
)
{
    // prepare data
    archetype_t* arch_type = view.chunk->structure;
    const auto* comp_types = arch_type->type.data;
    const auto* comp_offsets = arch_type->offsets[(int)view.chunk->pt];
    const auto* comp_sizes = arch_type->sizes;
    const auto* comp_aligns = arch_type->aligns;
    const auto* comp_arr_elem_sizes = arch_type->arrElemSizes;
    const auto* comp_arr_inline_counts = arch_type->arrInlineCounts;
    const auto* comp_callbacks = arch_type->callbacks;

    skr::Archive::ArrayScope arr_scope(*writer);
    SKR_FAST_CHECK(arr_scope.is_success(), );

    for (SIndex i = 0; i < arch_type->firstChunkComponent; ++i)
    {
        // prepare component info
        auto comp_type = comp_types[i];
        auto comp_offset = comp_offsets[i];
        auto comp_size = comp_sizes[i];
        auto comp_arr_elem_size = comp_arr_elem_sizes[i];
        auto comp_arr_inline_count = comp_arr_inline_counts[i];
        auto* comp_callback = &comp_callbacks[i];
        char* comp_src = view.chunk->data() + (size_t)comp_offset + (size_t)comp_size * view.start;

        //! skip if component serialize is not implemented
        if (!comp_callback || !comp_callback->serialize)
        {
            continue;
        }

        // do write
        if (type_index_t(comp_type).is_buffer())
        { // write array component
            for (uint64_t i = 0; i < view.count; i++)
            {
                // prepare array info
                auto array = (ArrayComponentBase*)((size_t)i * comp_size + comp_src);
                uint64_t array_count = array->size();

                // write array component as object
                {
                    skr::Archive::ObjectScope obj_scope(*writer);
                    SKR_FAST_CHECK(obj_scope.is_success(), );

                    // write array info
                    SKR_FAST_CHECK(writer->key_value(u8"array_count", array_count), );

                    // write array data
                    SKR_FAST_CHECK(writer->key(u8"data"), );
                    SKR_FAST_CHECK(
                        comp_callback->serialize(
                            comp_type,
                            array->unsafe_data(),
                            array_count,
                            writer
                        ),
                    );
                }
            }
        }
        else
        { // write component as array
            SKR_FAST_CHECK(
                comp_callback->serialize(
                    comp_type,
                    comp_src,
                    view.count,
                    writer
                ),
            );
        }
    }
}
inline static void read_components(
    skr::ArchiveRead* reader,
    sugoi_chunk_view_t& view
)
{
    // prepare data
    archetype_t* arch_type = view.chunk->structure;
    const auto* comp_types = arch_type->type.data;
    const auto* comp_offsets = arch_type->offsets[(int)view.chunk->pt];
    const auto* comp_sizes = arch_type->sizes;
    const auto* comp_aligns = arch_type->aligns;
    const auto* comp_arr_elem_sizes = arch_type->arrElemSizes;
    const auto* comp_arr_inline_counts = arch_type->arrInlineCounts;
    const auto* comp_callbacks = arch_type->callbacks;

    skr::Archive::ArrayScope arr_scope(*reader);
    SKR_FAST_CHECK(arr_scope.is_success(), );

    // check array size
    //! 可能有一些不支持序列化的 component 导致数量不匹配，依赖数组越界报错比较好
    // if (reader->is_structured())
    // {
    //     uint64_t arr_count;
    //     SKR_FAST_CHECK(reader->array_size_structured(arr_count), );
    //     if (arr_count != (uint64_t)arch_type->firstChunkComponent)
    //     {
    //         reader->error(
    //             u8"sugoi::serde_help::read_components: component count mismatch, expect {}, got {}",
    //             arch_type->firstChunkComponent,
    //             arr_count
    //         );
    //     }
    // }

    for (SIndex i = 0; i < arch_type->firstChunkComponent; ++i)
    {
        // prepare component info
        auto comp_type = comp_types[i];
        auto comp_offset = comp_offsets[i];
        auto comp_size = comp_sizes[i];
        auto comp_align = comp_aligns[i];
        auto comp_arr_elem_size = comp_arr_elem_sizes[i];
        auto comp_arr_inline_count = comp_arr_inline_counts[i];
        auto* comp_callback = &comp_callbacks[i];
        char* comp_src = view.chunk->data() + (size_t)comp_offset + (size_t)comp_size * view.start;

        //! skip if component deserialize is not implemented
        if (!comp_callback || !comp_callback->deserialize)
        {
            continue;
        }

        // do read
        if (type_index_t(comp_type).is_buffer())
        { // read array component
            for (uint64_t i = 0; i < view.count; i++)
            {
                // prepare array info
                auto array = (ArrayComponentBase*)((size_t)i * comp_size + comp_src);

                // read array component as object
                {
                    skr::Archive::ObjectScope obj_scope(*reader);
                    SKR_FAST_CHECK(obj_scope.is_success(), );

                    // read array info
                    uint64_t array_count = 0;
                    SKR_FAST_CHECK(reader->key_value(u8"array_count", array_count), );

                    // alloc array data
                    if (array_count > comp_arr_inline_count)
                    { // heap alloc
                        array->unsafe_set_capacity(array_count);
                        array->set_size(array_count);
                        array->unsafe_set_data(
                            skr::SkrAllocator::alloc_raw(
                                array_count,
                                comp_arr_elem_size,
                                comp_align
                            )
                        );
                    }
                    else
                    { // inline alloc
                        array->unsafe_set_capacity(comp_arr_inline_count);
                        array->set_size(array_count);
                        array->unsafe_set_data(array->calc_inline_data_ptr(comp_align));
                    }

                    // construct array elements
                    if (auto ctor = comp_callback->constructor)
                    {
                        for (uint64_t i = 0; i < array_count; ++i)
                        {
                            auto* curr = ::skr::memory::offset_item(array->unsafe_data(), comp_arr_elem_size, i);
                            ctor(comp_type, view.chunk, view.start + i, (char*)curr);
                        }
                    }

                    // read array data
                    SKR_FAST_CHECK(reader->key(u8"data"), );
                    SKR_FAST_CHECK(
                        comp_callback->deserialize(
                            comp_type,
                            array->unsafe_data(),
                            array_count,
                            reader
                        ),
                    );
                }
            }
        }
        else
        { // read component as array
            SKR_FAST_CHECK(
                comp_callback->deserialize(
                    comp_type,
                    comp_src,
                    view.count,
                    reader
                ),
            );
        }
    }
}
} // namespace sugoi::serde_help

// serialize
void sugoi_storage_t::serialize_type(skr::ArchiveWrite* writer, const sugoi_entity_type_t& group, bool keep_meta)
{
    using namespace sugoi;

    skr::Archive::ObjectScope obj_scope(*writer);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // write types
    {
        SKR_FAST_CHECK(writer->key(u8"types"), );
        skr::Archive::ArrayScope arr_scope(*writer);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write types count
        SKR_FAST_CHECK(writer->array_size<uint64_t>(group.type.length), );

        // write each type guid
        auto& reg = TypeRegistry::get();
        for (SIndex i = 0; i < group.type.length; i++)
        {
            auto tid = type_index_t(group.type.data[i]).index();
            SKR_FAST_CHECK(writer->value<skr::GUID>(reg.get_type_desc(tid)->guid), );
        }
    }

    // write meta entities
    if (keep_meta)
    {
        SKR_FAST_CHECK(writer->key_value(u8"meta_count", (uint64_t)group.meta.length), );
        SKR_FAST_CHECK(writer->key(u8"meta"), );
        serde_help::write_entities(
            writer,
            group.meta.data,
            group.meta.length
        );
        SKR_FAST_CHECK(writer->checkpoint(), );
    }
}
void sugoi_storage_t::serialize_view(skr::ArchiveWrite* writer, const sugoi_chunk_view_t& view, bool with_entities)
{
    using namespace sugoi;

    skr::Archive::ObjectScope obj_scope(*writer);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // write count
    SKR_FAST_CHECK(writer->key_value(u8"count", view.count), );

    // write entities id
    if (with_entities)
    {
        SKR_FAST_CHECK(writer->key(u8"entities"), );
        serde_help::write_entities(
            writer,
            view.chunk->get_entities() + view.start,
            view.count
        );
        SKR_FAST_CHECK(writer->checkpoint(), );
    }

    // write components
    SKR_FAST_CHECK(writer->key(u8"components"), );
    serde_help::write_components(writer, view);
    SKR_FAST_CHECK(writer->checkpoint(), );
}
void sugoi_storage_t::serialize(skr::ArchiveWrite* writer)
{
    using namespace sugoi;
    SkrZoneScopedN("sugoi_storage_t::serialize");

    skr::Archive::ObjectScope obj_scope(*writer);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // write entity registry
    SKR_FAST_CHECK(writer->key(u8"entity_registry"), );
    entity_registry.serialize(writer);
    SKR_FAST_CHECK(writer->checkpoint(), );

    // write group and chunks
    SKR_FAST_CHECK(writer->key(u8"groups"), );
    {
        pimpl->groups.read_versioned(
            [&](const sugoi_storage_t::Impl::groups_t& groups) -> void {
                skr::Archive::ArrayScope arr_scope_group_data(*writer);
                SKR_FAST_CHECK(arr_scope_group_data.is_success(), );

                // write groups count
                SKR_FAST_CHECK(writer->array_size<uint64_t>((uint64_t)groups.size()), );

                // write each group
                for (const auto& pair : groups)
                {
                    SkrZoneScopedN("group");
                    auto* group = pair.second;

                    skr::Archive::ObjectScope obj_scope_group(*writer);
                    SKR_FAST_CHECK(obj_scope_group.is_success(), );

                    // write group types
                    SKR_FAST_CHECK(writer->key(u8"types"), );
                    serialize_type(writer, group->type, true);
                    SKR_FAST_CHECK(writer->checkpoint(), );

                    // write group chunks
                    SKR_FAST_CHECK(writer->key(u8"chunks"), );
                    {
                        skr::Archive::ArrayScope arr_scope_chunk_data(*writer);
                        SKR_FAST_CHECK(arr_scope_chunk_data.is_success(), );

                        // write chunk count
                        SKR_FAST_CHECK(writer->array_size<uint64_t>((uint64_t)group->chunks.size()), );

                        // write each chunk
                        for (auto& chunk : group->chunks)
                        {
                            SkrZoneScopedN("chunk");
                            sugoi_chunk_view_t view = { chunk, 0, chunk->count };
                            serialize_view(writer, view, true);
                            SKR_FAST_CHECK(writer->checkpoint(), );
                        }
                    }
                }
            },
            [&]() {
                return pimpl->groups_timestamp;
            }
        );
    }
}
void sugoi_storage_t::serialize_single(skr::ArchiveWrite* writer, sugoi_entity_t entity)
{
    using namespace sugoi;

    skr::Archive::ObjectScope obj_scope(*writer);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    auto view = entity_view(entity);
    auto types = view.chunk->group->type;

    // write types
    SKR_FAST_CHECK(writer->key(u8"types"), );
    serialize_type(writer, types, false);
    SKR_FAST_CHECK(writer->checkpoint(), );

    // write entity
    SKR_FAST_CHECK(writer->key(u8"data"), );
    serialize_view(writer, view, false);
    SKR_FAST_CHECK(writer->checkpoint(), );
}

// deserialize
void sugoi_storage_t::deserialize_type(skr::ArchiveRead* reader, sugoi_entity_type_t& out_type, sugoi::fixed_stack_t& stack, bool keep_meta)
{
    using namespace sugoi;

    skr::Archive::ObjectScope obj_scope(*reader);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    auto& reg = TypeRegistry::get();

    // read types
    {
        SKR_FAST_CHECK(reader->key_required(u8"types"), );
        skr::Archive::ArrayScope arr_scope(*reader);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // read types count
        uint64_t type_count = 0;
        SKR_FAST_CHECK(reader->array_size<uint64_t>(type_count), );
        out_type.type.length = (SIndex)type_count;

        // alloc temp use memory
        out_type.type.data = stack.allocate<sugoi_type_index_t>(out_type.type.length);
        auto* type_data = const_cast<sugoi_type_index_t*>(out_type.type.data);

        // read each type guid
        for (SIndex i = 0; i < out_type.type.length; i++)
        {
            // read guid
            skr::GUID guid;
            SKR_FAST_CHECK(reader->value<skr::GUID>(guid), );

            // solve type index
            type_data[i] = reg.get_type(guid);
        }

        // sort type index
        std::sort(
            type_data,
            type_data + out_type.type.length
        );
    }

    // read meta entities
    if (keep_meta)
    {
        // read meta length
        uint64_t meta_length = 0;
        SKR_FAST_CHECK(reader->key_value(u8"meta_count", meta_length), );
        out_type.meta.length = (SIndex)meta_length;
        SKR_FAST_CHECK(out_type.meta.length > 0, );

        // alloc mete data
        out_type.meta.data = stack.allocate<sugoi_entity_t>(out_type.meta.length);

        // sort meta entities
        SKR_FAST_CHECK(reader->key_required(u8"meta"), );
        serde_help::read_entities(
            reader,
            stack.allocate<sugoi_entity_t>(out_type.meta.length),
            out_type.meta.length
        );
        SKR_FAST_CHECK(reader->checkpoint(), );

        // sort meta entities
        auto* meta_data = const_cast<sugoi_entity_t*>(out_type.meta.data);
        std::sort(
            meta_data,
            meta_data + out_type.meta.length
        );
    }
}
void sugoi_storage_t::deserialize_view(skr::ArchiveRead* reader, sugoi_group_t* group, sugoi_chunk_view_t& out_view, bool with_entities)
{
    using namespace sugoi;

    skr::Archive::ObjectScope obj_scope(*reader);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // read count
    SKR_FAST_CHECK(reader->key_value(u8"count", out_view.count), );
    SKR_FAST_CHECK(out_view.count > 0, );

    // alloc view
    out_view = allocateViewStrict(group, out_view.count);

    // read entities id
    if (with_entities)
    {
        SKR_FAST_CHECK(reader->key_required(u8"entities"), );
        serde_help::read_entities(
            reader,
            const_cast<sugoi_entity_t*>(out_view.chunk->get_entities()) + out_view.start,
            out_view.count
        );
        SKR_FAST_CHECK(reader->checkpoint(), );
    }

    // construct view
    sugoi::construct_view(out_view);

    // read components
    SKR_FAST_CHECK(reader->key_required(u8"components"), );
    serde_help::read_components(reader, out_view);
    SKR_FAST_CHECK(reader->checkpoint(), );
}
void sugoi_storage_t::deserialize(skr::ArchiveRead* reader)
{
    using namespace sugoi;
    SkrZoneScopedN("sugoi_storage_t::deserialize");

    skr::Archive::ObjectScope obj_scope(*reader);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // read entry
    SKR_FAST_CHECK(reader->key_required(u8"entity_registry"), );
    entity_registry.deserialize(reader);
    SKR_FAST_CHECK(reader->checkpoint(), );

    // read group and chunks
    SKR_FAST_CHECK(reader->key_required(u8"groups"), );
    {
        skr::Archive::ArrayScope arr_scope(*reader);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // read groups count
        uint64_t group_count = 0;
        SKR_FAST_CHECK(reader->array_size(group_count), );

        // read each group
        for (uint64_t i = 0; i < group_count; i++)
        {
            SkrZoneScopedN("group");

            skr::Archive::ObjectScope obj_scope_group(*reader);
            SKR_FAST_CHECK(obj_scope_group.is_success(), );

            fixed_stack_scope_t fixed_stack_scope{ localStack };

            // read group types
            sugoi_entity_type_t group_types;
            SKR_FAST_CHECK(reader->key_required(u8"types"), );
            deserialize_type(reader, group_types, localStack, true);
            SKR_FAST_CHECK(reader->checkpoint(), );

            // alloc group
            auto group = allocateGroup(group_types);

            // read group chunks
            SKR_FAST_CHECK(reader->key_required(u8"chunks"), );
            {
                skr::Archive::ArrayScope arr_scope_chunk_data(*reader);
                SKR_FAST_CHECK(arr_scope_chunk_data.is_success(), );

                // read chunk count
                uint64_t chunk_count = 0;
                SKR_FAST_CHECK(reader->array_size<uint64_t>(chunk_count), );

                // read each chunk
                for (uint64_t j = 0; j < chunk_count; j++)
                {
                    SkrZoneScopedN("chunk");
                    sugoi_chunk_view_t view;
                    deserialize_view(reader, group, view, true);
                    SKR_FAST_CHECK(reader->checkpoint(), );

                    // update registry
                    auto entities = sugoiV_get_entities(&view);
                    for (uint64_t k = 0; k < view.count; ++k)
                    {
                        auto& entry = entity_registry.entries[e_id(entities[k])];
                        entry.chunk = view.chunk;
                        entry.indexInChunk = k + view.start;
                        entry.version = e_version(entities[k]);
                    }
                }
            }
        }
    }
}
void sugoi_storage_t::deserialize_single(skr::ArchiveRead* reader, sugoi_entity_t& out_entity)
{
    using namespace sugoi;

    fixed_stack_scope_t fixed_stack_scope(localStack);

    skr::Archive::ObjectScope obj_scope(*reader);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // read types
    sugoi_entity_type_t types;
    SKR_FAST_CHECK(reader->key_required(u8"types"), );
    deserialize_type(reader, types, localStack, false);
    SKR_FAST_CHECK(reader->checkpoint(), );

    // read entity
    auto group = get_group(types);
    sugoi_chunk_view_t view;
    SKR_FAST_CHECK(reader->key_required(u8"data"), );
    deserialize_view(reader, group, view, false);
    SKR_FAST_CHECK(reader->checkpoint(), );
    entity_registry.fill_entities(view);
    out_entity = view.chunk->get_entities()[view.start];
}
