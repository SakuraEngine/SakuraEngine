#include <type_traits>
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/type_registry.hpp"
#include "SkrRT/resource/resource_handle.h"

#include "./mask.hpp"
#include "./chunk.hpp"
#include "./chunk_view.hpp"
#include "./archetype.hpp"
#include "./storage.hpp"
#include "./scheduler.hpp"

namespace sugoi
{
static sugoi_array_comp_t* new_array(void* ptr, size_t cap, size_t elemSize, size_t align)
{
    size_t arraySize = cap - sizeof(sugoi_array_comp_t);
    void* arrayData = (char*)ptr + sizeof(sugoi_array_comp_t);
    std::align(align, elemSize, arrayData, arraySize);
    return new (ptr) sugoi_array_comp_t{ arrayData, arraySize };
}

bool is_array_small(sugoi_array_comp_t* ptr)
{
    return ptr->BeginX < ((char*)(ptr + 1) + alignof(std::max_align_t));
}

#define for_buffer(i, array, size) \
    for (char* i = (char*)array->BeginX; i != array->EndX; i += size)

static void construct_impl(sugoi_chunk_view_t view, sugoi_chunk_t::RSlice& slice, type_index_t type, EIndex offset, uint32_t size, uint32_t align, uint32_t elemSize, uint32_t maskValue, void (*constructor)(sugoi_chunk_t* chunk, EIndex index, char* data))
{
    char* dst = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    if (type.is_buffer())
        forloop (j, 0, view.count)
        {
            char* buf = (size_t)j * size + dst;
            auto array = new_array(buf, size, elemSize, align);
            if (constructor)
                for_buffer(curr, array, elemSize)
                    constructor(view.chunk, view.start + j, curr);
        }
    else if (type == kMaskComponent)
        forloop (j, 0, view.count)
            ((mask_t*)dst)[j] = maskValue;
    else if (type == kDirtyComponent)
        memset(dst, 0xFFFFFFFF, (size_t)size * view.count);
    else if (constructor)
        forloop (j, 0, view.count)
            constructor(view.chunk, view.start + j, (size_t)j * size + dst);
    else
        memset(dst, 0, (size_t)size * view.count);
}

static void destruct_impl(sugoi_chunk_view_t view, type_index_t type, EIndex offset, uint32_t size, uint32_t elemSize, resource_fields_t resourceFields, void (*destructor)(sugoi_chunk_t* chunk, EIndex index, char* data))
{
    char* src = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    auto patchResources = [&](char* data)
    {
        forloop(k, 0, resourceFields.count)
        {
            auto field = ((intptr_t*)resourceFields.offsets)[k];
            auto* resource = (skr_resource_handle_t*)(data + field);
            if(resource->is_resolved())
                resource->reset();
        }
    };
    if (type.is_buffer())
        forloop (j, 0, view.count)
        {
            auto array = (sugoi_array_comp_t*)((size_t)j * size + src);
            if (destructor)
                for_buffer(curr, array, elemSize)
                    destructor(view.chunk, view.start + j, curr);
            else if(resourceFields.count > 0)
                for_buffer(curr, array, elemSize)
                    patchResources(curr);
            if (!is_array_small(array))
                sugoi_array_comp_t::free(array->BeginX);
        }
    else if (destructor)
        forloop (j, 0, view.count)
            destructor(view.chunk, view.start + j, (size_t)j * size + src);
    else if (resourceFields.count > 0)
    {
        forloop (j, 0, view.count)
            patchResources((size_t)j * size + src);
    }
}

static void move_impl(sugoi_chunk_view_t dstV, const sugoi_chunk_t* srcC, uint32_t srcStart, type_index_t type, EIndex srcOffset, EIndex dstOffset, uint32_t size, uint32_t align, uint32_t elemSize, void (*move)(sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, char* src))
{
    SKR_ASSERT(!type.is_chunk());
    char* dst = dstV.chunk->data() + (size_t)dstOffset + (size_t)size * dstV.start;
    char* src = srcC->data() + (size_t)srcOffset + (size_t)size * srcStart;
    if (move)
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (sugoi_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (sugoi_array_comp_t*)((size_t)j * size + src);
                if (!is_array_small(arraySrc)) // memory is on heap
                    *arrayDst = *arraySrc;    // just steal it
                else                          // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                    for (char *currDst = (char*)arrayDst->BeginX, *currSrc = (char*)arraySrc->BeginX;
                         currDst != arrayDst->EndX; currDst += elemSize, currSrc += elemSize)
                        move(dstV.chunk, dstV.start + j, currDst, (sugoi_chunk_t*)srcC, srcStart + j, currSrc);
                }
            }
        }
        else
            forloop (j, 0, dstV.count)
                move(dstV.chunk, dstV.start + j, (size_t)j * size + dst, (sugoi_chunk_t*)srcC, srcStart + j, (size_t)j * size + src);
    }
    else
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (sugoi_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (sugoi_array_comp_t*)((size_t)j * size + src);
                if (!is_array_small(arraySrc)) // memory is on heap
                    *arrayDst = *arraySrc;    // just steal it
                else                          // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                    memcpy(arrayDst->BeginX, arraySrc->BeginX, arraySrc->size_in_bytes());
                }
            }
        }
        else
            memcpy(dst, src, dstV.count * (size_t)size);
    }
}

static void clone_impl(sugoi_chunk_view_t dstV, const sugoi_chunk_t* srcC, uint32_t srcStart, type_index_t type, EIndex srcOffset, EIndex dstOffset, uint32_t size, uint32_t align, uint32_t elemSize, resource_fields_t resourceFields, void (*copy)(sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, const char* src))
{
    SKR_ASSERT(!type.is_chunk());
    char* dst = dstV.chunk->data() + (size_t)dstOffset + (size_t)size * dstV.start;
    char* src = srcC->data() + (size_t)srcOffset + (size_t)size * srcStart;
    auto storage = dstV.chunk->structure->storage;
    auto patchResources = [&](char* data)
    {
        forloop(k, 0, resourceFields.count)
        {
            auto field = ((intptr_t*)resourceFields.offsets)[k];
            auto* resource = (skr_resource_handle_t*)(data + field);
            if(resource->is_resolved())
            {
                new (resource) skr_resource_handle_t(*resource, (uint64_t)storage, SKR_REQUESTER_ENTITY);
            }
        }
    };
    if (copy)
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (sugoi_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (sugoi_array_comp_t*)((size_t)j * size + src);
                if (!is_array_small(arraySrc)) // memory is on heap
                {
                    size_t cap = (char*)arraySrc->EndX - (char*)arraySrc->BeginX;
                    arrayDst->BeginX = sugoi_array_comp_t::allocate(cap);
                    arrayDst->EndX = arrayDst->CapacityX = (char*)arrayDst->BeginX + cap;
                }
                else                          // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                }
                for (char *currDst = (char*)arrayDst->BeginX, *currSrc = (char*)arraySrc->BeginX;
                        currDst != arrayDst->EndX; currDst += elemSize, currSrc += elemSize)
                    copy(dstV.chunk, dstV.start + j, currDst, (sugoi_chunk_t*)srcC, srcStart + j, currSrc);
            }
        }
        else
            forloop (j, 0, dstV.count)
                copy(dstV.chunk, dstV.start + j, (size_t)j * size + dst, (sugoi_chunk_t*)srcC, srcStart + j, (size_t)j * size + src);
    }
    else
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (sugoi_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (sugoi_array_comp_t*)((size_t)j * size + src);
                if (!is_array_small(arraySrc)) // memory is on heap
                {
                    size_t cap = (char*)arraySrc->EndX - (char*)arraySrc->BeginX;
                    arrayDst->BeginX = sugoi_array_comp_t::allocate(cap);
                    arrayDst->EndX = arrayDst->CapacityX = (char*)arrayDst->BeginX + cap;
                }
                else                          // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                }
                memcpy(arrayDst->BeginX, arraySrc->BeginX, arraySrc->size_in_bytes());
                if(resourceFields.count > 0)
                {
                    for (char* curr = (char*)arrayDst->BeginX; curr != arrayDst->EndX; curr += elemSize)
                        patchResources(curr);
                }
            }
        }
        else
        {
            memcpy(dst, src, dstV.count * (size_t)size);
            if(resourceFields.count > 0)
            {
                forloop (j, 0, dstV.count)
                    patchResources((size_t)j * size + dst);
            }
        }
    }
}

void memdup(void* dst, const void* src, size_t size, size_t count) noexcept
{
    size_t copied = 1;
    memcpy(dst, src, size);
    while (copied <= count / 2)
    {
        memcpy((char*)dst + copied * size, dst, copied * size);
        copied *= 2;
    }
    if (copied < count)
        memcpy((char*)dst + copied * size, dst, (count - copied) * size);
}

static void duplicate_impl(sugoi_chunk_view_t dstV, const sugoi_chunk_t* srcC, uint32_t srcIndex, type_index_t type, EIndex offset, EIndex dstOffset, uint32_t size, uint32_t align, uint32_t elemSize, resource_fields_t resourceFields, void (*copy)(sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, const char* src))
{
    SKR_ASSERT(!type.is_chunk());
    char* dst = dstV.chunk->data() + (size_t)dstOffset + (size_t)size * dstV.start;
    const char* src = srcC->data() + (size_t)offset + (size_t)size * srcIndex;
    if (type == kGuidComponent)
    {
        auto guidDst = (guid_t*)dst;
        auto& registry = TypeRegistry::get();
        forloop (j, 0, dstV.count)
            guidDst[j] = registry.make_guid();
        return;
    }
    auto storage = dstV.chunk->structure->storage;
    auto patchResources = [&](char* data)
    {
        forloop(k, 0, resourceFields.count)
        {
            auto field = ((intptr_t*)resourceFields.offsets)[k];
            auto* resource = (skr_resource_handle_t*)(data + field);
            if(resource->is_resolved())
            {
                new (resource) skr_resource_handle_t(*resource, (uint64_t)storage, SKR_REQUESTER_ENTITY);
            }
        }
    };
    if (copy)
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (sugoi_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (sugoi_array_comp_t*)src;
                if (!is_array_small(arraySrc))
                {
                    size_t cap = (char*)arraySrc->EndX - (char*)arraySrc->BeginX;
                    arrayDst->BeginX = sugoi_array_comp_t::allocate(cap);
                    arrayDst->EndX = arrayDst->CapacityX = (char*)arrayDst->BeginX + cap;
                }
                else
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                }

                for (char *currDst = (char*)arrayDst->BeginX, *currSrc = (char*)arraySrc->BeginX;
                     currDst != arrayDst->EndX; currDst += elemSize, currSrc += elemSize)
                    copy(dstV.chunk, dstV.start + j, currDst, (sugoi_chunk_t*)srcC, srcIndex, currSrc);
            }
        }
        else
            forloop (j, 0, dstV.count)
                copy(dstV.chunk, dstV.start + j, (size_t)j * size + dst, (sugoi_chunk_t*)srcC, srcIndex, src);
    }
    else
    {
        
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arraySrc = (sugoi_array_comp_t*)src;
                auto arrayDst = (sugoi_array_comp_t*)((size_t)j * size + dst);
                if (!is_array_small(arraySrc))
                {
                    size_t cap = (char*)arraySrc->EndX - (char*)arraySrc->BeginX;
                    arrayDst->BeginX = sugoi_array_comp_t::allocate(cap);
                    arrayDst->EndX = arrayDst->CapacityX = (char*)arrayDst->BeginX + cap;
                    memcpy(arrayDst->BeginX, arraySrc->BeginX, cap);
                }
                else
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                }
                if(resourceFields.count > 0)
                {
                    for (char* curr = (char*)arrayDst->BeginX; curr != arrayDst->EndX; curr += elemSize)
                        patchResources(curr);
                }
            }
        }
        else {
            memdup(dst, src, (size_t)size, (size_t)dstV.count);
            if(resourceFields.count > 0)
            {
                forloop (j, 0, dstV.count)
                    patchResources((size_t)j * size + dst);
            }
        }
    }
}

void construct_view(const sugoi_chunk_view_t& view) noexcept
{
    archetype_t* type = view.chunk->structure;
    EIndex* offsets = type->offsets[(int)view.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* aligns = type->aligns;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    auto maskValue = uint32_t(1 << type->type.length) - 1;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].constructor) callback = nullptr;
        if((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].constructor;

        sugoi_chunk_t::RSlice ctorSlice = view.chunk->s_lock(type->type.data[i], view);
        construct_impl(view, ctorSlice, type->type.data[i], offsets[i], sizes[i], aligns[i], elemSizes[i], maskValue,  callback);
        view.chunk->s_unlock(type->type.data[i], view);
    }
}

void destruct_view(const sugoi_chunk_view_t& view) noexcept
{
    archetype_t* type = view.chunk->structure;
    EIndex* offsets = type->offsets[(int)view.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].destructor) callback = nullptr;
        if((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].destructor;

        destruct_impl(view, type->type.data[i], offsets[i], sizes[i], elemSizes[i], type->resourceFields[i], callback);
    }
}

void construct_chunk(sugoi_chunk_t* chunk) noexcept
{
    archetype_t* type = chunk->structure;
    EIndex* offsets = type->offsets[(int)chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* aligns = type->aligns;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    auto maskValue = uint32_t(1 << type->type.length) - 1;

    for (SIndex i = type->firstChunkComponent; i < type->type.length; ++i)
    {
        decltype(type->callbacks[i].constructor) callback = nullptr;
        if((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].constructor;

        const auto ctorView = sugoi_chunk_view_t{chunk, 0, 1};
        sugoi_chunk_t::RSlice ctorSlice = chunk->s_lock(type->type.data[i], ctorView);
        construct_impl(ctorView, ctorSlice, type->type.data[i], offsets[i], sizes[i], aligns[i], elemSizes[i], maskValue, callback);
        chunk->s_unlock(type->type.data[i], ctorView);
    }
}

void destruct_chunk(sugoi_chunk_t* chunk) noexcept
{
    archetype_t* type = chunk->structure;
    EIndex* offsets = type->offsets[(int)chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    for (SIndex i = type->firstChunkComponent; i < type->type.length; ++i)
    {
        decltype(type->callbacks[i].destructor) callback = nullptr;
        if((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].destructor;

        const auto dtorView = sugoi_chunk_view_t{chunk, 0, 1};
        destruct_impl(dtorView, type->type.data[i], offsets[i], sizes[i], elemSizes[i], type->resourceFields[i], callback);
    }
}

void move_view(const sugoi_chunk_view_t& view, EIndex srcStart) noexcept
{
    move_view(view, view.chunk, srcStart);
}

void move_view(const sugoi_chunk_view_t& dstV, const sugoi_chunk_t* srcC, uint32_t srcStart) noexcept
{
    archetype_t* type = dstV.chunk->structure;
    EIndex* offsets = type->offsets[(int)dstV.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* aligns = type->aligns;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].move) callback = nullptr;
        if((callbackFlags[i] & SUGOI_CALLBACK_FLAG_MOVE) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].move;
        move_impl(dstV, srcC, srcStart, type->type.data[i], offsets[i], offsets[i], sizes[i], aligns[i], elemSizes[i], callback);
    }
}

void cast_view(const sugoi_chunk_view_t& dstV, sugoi_chunk_t* srcC, EIndex srcStart) noexcept
{
    archetype_t* srcType = srcC->structure;
    archetype_t* dstType = dstV.chunk->structure;
    EIndex* srcOffsets = srcType->offsets[srcC->pt];
    EIndex* dstOffsets = dstType->offsets[dstV.chunk->pt];
    uint32_t* srcSizes = srcType->sizes;
    uint32_t* srcAligns = srcType->aligns;
    uint32_t* dstAligns = dstType->aligns;
    uint32_t* dstSizes = dstType->sizes;
    uint32_t* srcElemSizes = srcType->elemSizes;
    uint32_t* dstElemSizes = dstType->elemSizes;
    sugoi_type_set_t srcTypes = srcType->type;
    sugoi_type_set_t dstTypes = dstType->type;
    uint32_t* srcCallbackFlags = srcType->callbackFlags;
    uint32_t* dstCallbackFlags = dstType->callbackFlags;
    uint32_t maskValue = uint32_t(1 << dstTypes.length) - 1;
    
    std::bitset<32>*srcMasks = nullptr, *dstMasks = nullptr;
    if (srcType->withMask && dstType->withMask)
    {
        SIndex srcMaskId = srcType->index(kMaskComponent);
        SIndex dstMaskId = dstType->index(kMaskComponent);
        dstMasks = (std::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcMasks = (std::bitset<32>*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset(dstMasks, 0, sizeof(uint32_t) * dstV.count);
    }

    std::bitset<32>*srcDirtys = nullptr, *dstDirtys = nullptr;
    if (srcType->withDirty && dstType->withDirty)
    {
        SIndex srcMaskId = srcType->index(kDirtyComponent);
        SIndex dstMaskId = dstType->index(kDirtyComponent);
        dstDirtys = (std::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcDirtys = (std::bitset<32>*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset(dstDirtys, 0, sizeof(uint32_t) * dstV.count);
    }

    SIndex srcI = 0, dstI = 0;

    while (srcI < srcType->firstChunkComponent && dstI < dstType->firstChunkComponent)
    {
        type_index_t srcT = srcTypes.data[srcI];
        type_index_t dstT = dstTypes.data[dstI];
        if (srcT < dstT) // destruct
        {
            decltype(srcType->callbacks[srcI].destructor) callback = nullptr;
            if((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
                callback = srcType->callbacks[srcI].destructor;

            sugoi_chunk_view_t dtorView = { srcC, srcStart, dstV.count };
            destruct_impl(dtorView, srcT, srcOffsets[srcI], srcSizes[srcI], srcElemSizes[srcI], srcType->resourceFields[srcI], callback);

            ++srcI;
        }
        else if (srcT > dstT) // construct
        {
            decltype(dstType->callbacks[dstI].constructor) callback = nullptr;
            if((dstCallbackFlags[dstI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
                callback = dstType->callbacks[dstI].constructor;

            sugoi_chunk_t::RSlice ctorSlice = dstV.chunk->s_lock(dstT, dstV);
            construct_impl(dstV, ctorSlice, dstT, dstOffsets[dstI], dstSizes[dstI], dstAligns[dstI], dstElemSizes[dstI], maskValue, callback);
            dstV.chunk->s_unlock(dstT, dstV);

            if (dstMasks)
                forloop (i, 0, dstV.count)
                    dstMasks[i]
                    .set(dstI);
            if (dstDirtys)
                forloop (i, 0, dstV.count)
                    dstDirtys[i]
                    .set(dstI);
            ++dstI;
        }
        else // move
        {
            if (srcT != kMaskComponent)
            {
                decltype(srcType->callbacks[srcI].move) callback = nullptr;
                if((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_MOVE) != 0) SUGOI_UNLIKELY
                    callback = srcType->callbacks[srcI].move;
                move_impl(dstV, srcC, srcStart, srcT, srcOffsets[srcI], dstOffsets[dstI], srcSizes[srcI], srcAligns[srcI], srcElemSizes[srcI], callback);
            }
            if (dstMasks)
            {
                if (srcMasks)
                    forloop (i, 0, dstV.count)
                        dstMasks[i]
                        .set(dstI, srcMasks[i].test(srcI));
                else
                    forloop (i, 0, dstV.count)
                        dstMasks[i]
                        .set(dstI);
            }
            if (dstDirtys)
            {
                if (srcDirtys)
                    forloop (i, 0, dstV.count)
                        dstDirtys[i]
                        .set(dstI, srcDirtys[i].test(srcI));
                else
                    forloop (i, 0, dstV.count)
                        dstDirtys[i]
                        .set(dstI);
            }
            ++srcI;
            ++dstI;
        }
    }
    while(srcI < srcType->firstChunkComponent)
    {
        type_index_t srcT = srcTypes.data[srcI];
        decltype(srcType->callbacks[srcI].destructor) callback = nullptr;
        if((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = srcType->callbacks[srcI].destructor;

        sugoi_chunk_view_t dtorView = { srcC, srcStart, dstV.count };
        destruct_impl(dtorView, srcT, srcOffsets[srcI], srcSizes[srcI], srcElemSizes[srcI], srcType->resourceFields[srcI], callback);

        ++srcI;
    }
    while(dstI < dstType->firstChunkComponent)
    {
        type_index_t dstT = dstTypes.data[dstI];
        decltype(dstType->callbacks[dstI].constructor) callback = nullptr;
        if((dstCallbackFlags[dstI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = dstType->callbacks[dstI].constructor;
        
        sugoi_chunk_t::RSlice ctorSlice = dstV.chunk->s_lock(dstT, dstV);
        construct_impl(dstV, ctorSlice, dstT, dstOffsets[dstI], dstSizes[dstI], dstAligns[dstI], dstElemSizes[dstI], maskValue, callback);
        dstV.chunk->s_unlock(dstT, dstV);

        if (dstMasks)
            forloop (i, 0, dstV.count)
                dstMasks[i]
                .set(dstI);
        if (dstDirtys)
            forloop (i, 0, dstV.count)
                dstDirtys[i]
                .set(dstI);
        ++dstI;
    }
}


void duplicate_view(const sugoi_chunk_view_t& dstV, const sugoi_chunk_t* srcC, EIndex srcStart) noexcept
{
    archetype_t* srcType = srcC->structure;
    archetype_t* dstType = dstV.chunk->structure;
    EIndex* srcOffsets = srcType->offsets[srcC->pt];
    EIndex* dstOffsets = dstType->offsets[dstV.chunk->pt];
    uint32_t* srcSizes = srcType->sizes;
    uint32_t* srcAligns = srcType->aligns;
    uint32_t* dstAligns = dstType->aligns;
    uint32_t* dstSizes = dstType->sizes;
    uint32_t* srcElemSizes = srcType->elemSizes;
    uint32_t* dstElemSizes = dstType->elemSizes;
    sugoi_type_set_t srcTypes = srcType->type;
    sugoi_type_set_t dstTypes = dstType->type;
    uint32_t* srcCallbackFlags = srcType->callbackFlags;
    uint32_t* dstCallbackFlags = dstType->callbackFlags;
    uint32_t maskValue = uint32_t(1 << dstTypes.length) - 1;
    
    std::bitset<32>* srcMasks = nullptr, *dstMasks = nullptr;
    if (srcType->withMask && dstType->withMask)
    {
        SIndex srcMaskId = srcType->index(kMaskComponent);
        SIndex dstMaskId = dstType->index(kMaskComponent);
        dstMasks = (std::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcMasks = (std::bitset<32>*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset(dstMasks, 1, sizeof(uint32_t) * dstV.count);
    }

    std::bitset<32>* dstDirtys = nullptr;
    if (srcType->withDirty && dstType->withDirty)
    {
        SIndex dstMaskId = dstType->index(kDirtyComponent);
        dstDirtys = (std::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        std::memset(dstDirtys, 1, sizeof(uint32_t) * dstV.count);
    }

    SIndex srcI = 0, dstI = 0;

    while (srcI < srcType->firstChunkComponent && dstI < dstType->firstChunkComponent)
    {
        type_index_t srcT = srcTypes.data[srcI];
        type_index_t dstT = dstTypes.data[dstI];
        if (srcT < dstT) // ignore
        {
            ++srcI;
        }
        else if (srcT > dstT) // construct
        {
            decltype(dstType->callbacks[dstI].constructor) callback = nullptr;
            if((dstCallbackFlags[dstI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
                callback = dstType->callbacks[dstI].constructor;

            sugoi_chunk_t::RSlice ctorSlice = dstV.chunk->s_lock(dstT, dstV);
            construct_impl(dstV, ctorSlice, dstT, dstOffsets[dstI], dstSizes[dstI], dstAligns[dstI], dstElemSizes[dstI], maskValue, callback);
            dstV.chunk->s_unlock(dstT, dstV);
            
            if (dstMasks)
                forloop (i, 0, dstV.count)
                    dstMasks[i]
                    .set(dstI);
            if (dstDirtys)
                forloop (i, 0, dstV.count)
                    dstDirtys[i]
                    .set(dstI);
            ++dstI;
        }
        else
        {
            if (srcT != kMaskComponent)
            {
                decltype(srcType->callbacks[srcI].copy) callback = nullptr;
                if((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_COPY) != 0) SUGOI_UNLIKELY
                    callback = srcType->callbacks[srcI].copy;
                duplicate_impl(dstV, srcC, srcStart, srcT, srcOffsets[srcI], dstOffsets[dstI], srcSizes[srcI], srcAligns[srcI], srcElemSizes[srcI], srcType->resourceFields[srcI], callback);
            }
            if (dstMasks)
            {
                if (srcMasks)
                    forloop (i, 0, dstV.count)
                        dstMasks[i]
                        .set(dstI, srcMasks[i].test(srcI));
                else
                    forloop (i, 0, dstV.count)
                        dstMasks[i]
                        .set(dstI);
            }
            if (dstDirtys)
                forloop (i, 0, dstV.count)
                    dstDirtys[i]
                    .set(dstI);
            ++srcI;
            ++dstI;
        }
    }
}

void clone_view(const sugoi_chunk_view_t& dstV, const sugoi_chunk_t* srcC, EIndex srcStart) noexcept
{
    archetype_t* srcType = srcC->structure;
    archetype_t* dstType = dstV.chunk->structure;
    EIndex* srcOffsets = srcType->offsets[srcC->pt];
    EIndex* dstOffsets = dstType->offsets[dstV.chunk->pt];
    uint32_t* srcSizes = srcType->sizes;
    uint32_t* srcAligns = srcType->aligns;
    uint32_t* srcElemSizes = srcType->elemSizes;
    sugoi_type_set_t srcTypes = srcType->type;
    uint32_t* srcCallbackFlags = srcType->callbackFlags;
    
    for(uint32_t i = 0; i < srcType->firstChunkComponent; ++i)
    {
        type_index_t srcT = srcTypes.data[i];
        decltype(srcType->callbacks[i].move) callback = nullptr;
        if((srcCallbackFlags[i] & SUGOI_CALLBACK_FLAG_MOVE) != 0) SUGOI_UNLIKELY
            callback = srcType->callbacks[i].move;
        move_impl(dstV, srcC, srcStart, srcT, srcOffsets[i], dstOffsets[i], srcSizes[i], srcAligns[i], srcElemSizes[i], callback);
    }
}

bool full_view(const sugoi_chunk_view_t& view) noexcept
{
    return view.chunk != nullptr && view.start == 0 && view.count == view.chunk->count;
}

void enable_components(const sugoi_chunk_view_t& view, const sugoi_type_set_t& types)
{
    auto group = view.chunk->group;
    auto masks = (mask_t*)sugoiV_get_owned_rw(&view, kMaskComponent);
    auto newMask = group->get_mask(types);
    if (!masks) SUGOI_UNLIKELY
        return;
    for (uint32_t i = 0; i < view.count; ++i)
        masks[i].fetch_or(newMask);
}

void disable_components(const sugoi_chunk_view_t& view, const sugoi_type_set_t& types)
{
    auto group = view.chunk->group;
    auto masks = (mask_t*)sugoiV_get_owned_rw(&view, kMaskComponent);
    auto newMask = group->get_mask(types);
    if (!masks) SUGOI_UNLIKELY
        return;
    for (uint32_t i = 0; i < view.count; ++i)
        masks[i].fetch_and(~newMask);
}
} // namespace sugoi

sugoi_type_index_t sugoiV_get_local_type(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    return view->chunk->group->index(type);
}

sugoi_type_index_t sugoiV_get_component_type(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    SKR_ASSERT(type < view->chunk->group->type.type.length);
    return view->chunk->group->type.type.data[type];
}

template <bool readonly, bool local>
auto sugoiV_get_owned(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    using namespace sugoi;
    using return_type = std::conditional_t<readonly, const void*, void*>;
    if (type_index_t(type).is_tag()) SUGOI_UNLIKELY
        return (return_type) nullptr;
    auto chunk = view->chunk;
    auto structure = chunk->structure;

    SIndex tid = type;
    SIndex slot;
    if constexpr (local)
    {
        slot = type;
        tid = structure->type.data[slot];
    }
    else
    {
        slot = structure->index(tid);
        if (slot == kInvalidSIndex)
            return (return_type) nullptr;
    }

    if constexpr (!readonly)
        chunk->set_timestamp_at(slot, structure->storage->timestamp);
    auto scheduler = structure->storage->scheduler;
    if (scheduler && scheduler->is_main_thread(structure->storage))
        SKR_ASSERT(!scheduler->sync_entry(structure, slot, readonly));

    return (return_type)chunk->get_unsafe(tid, *view).start;
}

extern "C" {
const void* sugoiV_get_component_ro(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    auto data = sugoiV_get_owned_ro(view, type);
    if (!data) SUGOI_UNLIKELY
        return view->chunk->group->get_shared_ro(type);
    return data;
}

const void* sugoiV_get_owned_ro(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    return sugoiV_get_owned<true, false>(view, type);
}

void* sugoiV_get_owned_rw(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    return sugoiV_get_owned<false, false>(view, type);
}

const void* sugoiV_get_owned_ro_local(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    return sugoiV_get_owned<true, true>(view, type);
}

void* sugoiV_get_owned_rw_local(const sugoi_chunk_view_t* view, sugoi_type_index_t type)
{
    return sugoiV_get_owned<false, true>(view, type);
}

const sugoi_entity_t* sugoiV_get_entities(const sugoi_chunk_view_t* view)
{
    auto chunk = view->chunk;
    return chunk->get_entities() + view->start;
}
}