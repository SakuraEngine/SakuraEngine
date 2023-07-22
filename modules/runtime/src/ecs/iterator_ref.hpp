#pragma once
#include "type.hpp"
#include "archetype.hpp"
#include "chunk.hpp"
#include "chunk_view.hpp"
#include "SkrRT/ecs/array.hpp"
#include "type_registry.hpp"
#include <type_traits>

namespace dual
{
template <class F>
static void iter_ref_impl(dual_chunk_view_t view, type_index_t type, EIndex offset, uint32_t size, uint32_t elemSize, F&& iter)
{
    char* src = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    auto& reg = type_registry_t::get();
    const auto& desc = reg.descriptions[type.index()];
    auto map = desc.callback.map;
    if (desc.entityFieldsCount == 0 && !map)
        return;
    auto iter_element = [&](dual_chunk_t* chunk, EIndex index, char* curr) {
        dual_mapper_t mapper;
        mapper.map = +[](void* user, dual_entity_t* ent) {
            ((std::remove_reference_t<F>*)user)->map(*(dual_entity_t*)ent);
        };
        mapper.user = &iter;
        if (map)
            map(chunk, index, curr, &mapper);
        else
        {
            forloop (i, 0, desc.entityFieldsCount)
            {
                auto field = reg.entityFields[desc.entityFields + i];
                iter.map(*(dual_entity_t*)(curr + field));
            }
        }
    };
    if (type.is_buffer())
    {
        if (type == kLinkComponent)
        {
            forloop (j, 0, view.count)
            {
                auto link = (link_array_t*)((size_t)j * size + src);
                for (auto& e : *link)
                    iter.map(e);
                iter.move();
            }
        }
        else
        {
            forloop (j, 0, view.count)
            {
                auto array = (dual_array_comp_t*)((size_t)j * size + src);
                for (char* curr = (char*)array->BeginX; curr < array->EndX; curr += elemSize)
                    iter_element(view.chunk, view.start + j, curr);
                iter.move();
            }
        }
    }
    else
        forloop (j, 0, view.count)
        {
            iter_element(view.chunk, view.start + j, src + size * j);
            iter.move();
        }
    iter.reset();
}

template <class F>
void iterator_ref_view(const dual_chunk_view_t& view, F&& iter) noexcept
{
    archetype_t* type = view.chunk->type;
    EIndex* offsets = type->offsets[(int)view.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    forloop (i, 0, type->firstChunkComponent)
        iter_ref_impl(view, type->type.data[i], offsets[i], sizes[i], elemSizes[i], std::forward<F>(iter));
}
template<class F>
void iterator_ref_chunk(dual_chunk_t* chunk, F&& iter) noexcept
{
    archetype_t* type = chunk->type;
    EIndex* offsets = type->offsets[(int)chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    forloop (i, type->firstChunkComponent, type->type.length)
        iter_ref_impl({ chunk, 0, 1 }, type->type.data[i], offsets[i], sizes[i], elemSizes[i], std::forward<F>(iter));
}
} // namespace dual