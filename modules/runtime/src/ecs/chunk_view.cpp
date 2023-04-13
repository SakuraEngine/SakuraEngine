#include <type_traits>
#include <EASTL/bitset.h>
#include "ecs/dual.h"
#include "ecs/dual_config.h"
#include "type.hpp"
#include "mask.hpp"
#include "ecs/constants.hpp"
#include "type.hpp"
#include "chunk_view.hpp"
#include "chunk.hpp"
#include "archetype.hpp"
#include "ecs/array.hpp"
#include "storage.hpp"
#include "set.hpp"
#include "scheduler.hpp"
#include "resource/resource_handle.h"
#include "type_registry.hpp"

namespace dual
{
static dual_array_comp_t* new_array(void* ptr, size_t cap, size_t elemSize, size_t align)
{
    size_t arraySize = cap - sizeof(dual_array_comp_t);
    void* arrayData = (char*)ptr + sizeof(dual_array_comp_t);
    eastl::align(align, elemSize, arrayData, arraySize);
    return new (ptr) dual_array_comp_t{ arrayData, arraySize };
}

bool is_array_small(dual_array_comp_t* ptr)
{
    return ptr->BeginX < ((char*)(ptr + 1) + alignof(std::max_align_t));
}

static void construct_impl(dual_chunk_view_t view, type_index_t type, EIndex offset, uint32_t size, uint32_t align, uint32_t elemSize, uint32_t maskValue, void (*constructor)(dual_chunk_t* chunk, EIndex index, char* data))
{
    char* dst = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    if (type.is_buffer())
        forloop (j, 0, view.count)
        {
            char* buf = (size_t)j * size + dst;
            auto array = new_array(buf, size, elemSize, align);
            if (constructor)
                for (char* curr = (char*)array->BeginX; curr != array->EndX; curr += elemSize)
                    constructor(view.chunk, view.start + j, curr);
        }
    else if (type == kMaskComponent)
        forloop (j, 0, view.count)
            ((mask_t*)dst)[j] = maskValue;
    else if (type == kDirtyComponent)
        memset(dst, 0xFFFFFFFF, (size_t)size * view.count);
    // else if (type == kGuidComponent)
    // {
    //     auto guidDst = (guid_t*)dst;
    //     auto& registry = type_registry_t::get();
    //     forloop (j, 0, view.count)
    //         guidDst[j] = registry.make_guid();
    //     return;
    // }
    else if (constructor)
        forloop (j, 0, view.count)
            constructor(view.chunk, view.start + j, (size_t)j * size + dst);
    else
        memset(dst, 0, (size_t)size * view.count);
}

static void destruct_impl(dual_chunk_view_t view, type_index_t type, EIndex offset, uint32_t size, uint32_t elemSize, resource_fields_t resourceFields, void (*destructor)(dual_chunk_t* chunk, EIndex index, char* data))
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
            auto array = (dual_array_comp_t*)((size_t)j * size + src);
            if (destructor)
                for (char* curr = (char*)array->BeginX; curr != array->EndX; curr += elemSize)
                    destructor(view.chunk, view.start + j, curr);
            else if(resourceFields.count > 0)
                for (char* curr = (char*)array->BeginX; curr != array->EndX; curr += elemSize)
                    patchResources(curr);
            if (!is_array_small(array))
                dual_array_comp_t::free(array->BeginX);
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

static void move_impl(dual_chunk_view_t dstV, const dual_chunk_t* srcC, uint32_t srcStart, type_index_t type, EIndex offset, uint32_t size, uint32_t align, uint32_t elemSize, void (*move)(dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, char* src))
{
    SKR_ASSERT(!type.is_chunk());
    char* dst = dstV.chunk->data() + (size_t)offset + (size_t)size * dstV.start;
    char* src = srcC->data() + (size_t)offset + (size_t)size * srcStart;
    if (move)
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (dual_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (dual_array_comp_t*)((size_t)j * size + src);
                if (!is_array_small(arraySrc)) // memory is on heap
                    *arrayDst = *arraySrc;    // just steal it
                else                          // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                    for (char *currDst = (char*)arrayDst->BeginX, *currSrc = (char*)arraySrc->BeginX;
                         currDst != arrayDst->EndX; currDst += elemSize, currSrc += elemSize)
                        move(dstV.chunk, dstV.start + j, currDst, (dual_chunk_t*)srcC, srcStart + j, currSrc);
                }
            }
        }
        else
            forloop (j, 0, dstV.count)
                move(dstV.chunk, dstV.start + j, (size_t)j * size + dst, (dual_chunk_t*)srcC, srcStart + j, (size_t)j * size + src);
    }
    else
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (dual_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (dual_array_comp_t*)((size_t)j * size + src);
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

static void duplicate_impl(dual_chunk_view_t dstV, const dual_chunk_t* srcC, uint32_t srcIndex, type_index_t type, EIndex offset, EIndex dstOffset, uint32_t size, uint32_t align, uint32_t elemSize, resource_fields_t resourceFields, void (*copy)(dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, const char* src))
{
    SKR_ASSERT(!type.is_chunk());
    char* dst = dstV.chunk->data() + (size_t)dstOffset + (size_t)size * dstV.start;
    const char* src = srcC->data() + (size_t)offset + (size_t)size * srcIndex;
    if (type == kGuidComponent)
    {
        auto guidDst = (guid_t*)dst;
        auto& registry = type_registry_t::get();
        forloop (j, 0, dstV.count)
            guidDst[j] = registry.make_guid();
        return;
    }
    auto storage = dstV.chunk->type->storage;
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
                auto arrayDst = (dual_array_comp_t*)((size_t)j * size + dst);
                auto arraySrc = (dual_array_comp_t*)src;
                if (!is_array_small(arraySrc))
                {
                    size_t cap = (char*)arraySrc->EndX - (char*)arraySrc->BeginX;
                    arrayDst->BeginX = dual_array_comp_t::allocate(cap);
                    arrayDst->EndX = arrayDst->CapacityX = (char*)arrayDst->BeginX + cap;
                }
                else
                {
                    new_array(arrayDst, size, elemSize, align);
                    arrayDst->EndX = (char*)arrayDst->BeginX + arraySrc->size_in_bytes();
                }

                for (char *currDst = (char*)arrayDst->BeginX, *currSrc = (char*)arraySrc->BeginX;
                     currDst != arrayDst->EndX; currDst += elemSize, currSrc += elemSize)
                    copy(dstV.chunk, dstV.start + j, currDst, (dual_chunk_t*)srcC, srcIndex, currSrc);
            }
        }
        else
            forloop (j, 0, dstV.count)
                copy(dstV.chunk, dstV.start + j, (size_t)j * size + dst, (dual_chunk_t*)srcC, srcIndex, src);
    }
    else
    {
        memdup(dst, src, (size_t)size, (size_t)dstV.count);
        
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arraySrc = (dual_array_comp_t*)src;
                if (!is_array_small(arraySrc))
                {
                    auto arrayDst = (dual_array_comp_t*)((size_t)j * size + dst);
                    size_t cap = (char*)arraySrc->EndX - (char*)arraySrc->BeginX;
                    arrayDst->BeginX = dual_array_comp_t::allocate(cap);
                    arrayDst->EndX = arrayDst->CapacityX = (char*)arrayDst->BeginX + cap;
                    memcpy(arrayDst->BeginX, arraySrc->BeginX, cap);
                }
                if(resourceFields.count > 0)
                {
                    auto arrayDst = (dual_array_comp_t*)((size_t)j * size + dst);
                    for (char* curr = (char*)arrayDst->BeginX; curr != arrayDst->EndX; curr += elemSize)
                        patchResources(curr);
                }
            }
        }
        else if(resourceFields.count > 0)
        {
            forloop (j, 0, dstV.count)
                patchResources((size_t)j * size + dst);
        }
    }
}

void construct_view(const dual_chunk_view_t& view) noexcept
{
    archetype_t* type = view.chunk->type;
    EIndex* offsets = type->offsets[(int)view.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* aligns = type->aligns;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    auto maskValue = uint32_t(1 << type->type.length) - 1;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].constructor) callback = nullptr;
        if((callbackFlags[i] & DCF_CTOR) != 0) DUAL_UNLIKELY
            callback = type->callbacks[i].constructor;
        construct_impl(view, type->type.data[i], offsets[i], sizes[i], aligns[i], elemSizes[i], maskValue,  callback);
    }
}

void destruct_view(const dual_chunk_view_t& view) noexcept
{
    archetype_t* type = view.chunk->type;
    EIndex* offsets = type->offsets[(int)view.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].destructor) callback = nullptr;
        if((callbackFlags[i] & DCF_CTOR) != 0) DUAL_UNLIKELY
            callback = type->callbacks[i].destructor;
        destruct_impl(view, type->type.data[i], offsets[i], sizes[i], elemSizes[i], type->resourceFields[i], callback);
    }
}

void construct_chunk(dual_chunk_t* chunk) noexcept
{
    archetype_t* type = chunk->type;
    EIndex* offsets = type->offsets[(int)chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* aligns = type->aligns;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    auto maskValue = uint32_t(1 << type->type.length) - 1;

    for (SIndex i = type->firstChunkComponent; i < type->type.length; ++i)
    {
        decltype(type->callbacks[i].constructor) callback = nullptr;
        if((callbackFlags[i] & DCF_CTOR) != 0) DUAL_UNLIKELY
            callback = type->callbacks[i].constructor;
        construct_impl(dual_chunk_view_t{chunk, 0, 1}, type->type.data[i], offsets[i], sizes[i], aligns[i], elemSizes[i], maskValue, callback);
    }
}

void destruct_chunk(dual_chunk_t* chunk) noexcept
{
    archetype_t* type = chunk->type;
    EIndex* offsets = type->offsets[(int)chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    for (SIndex i = type->firstChunkComponent; i < type->type.length; ++i)
    {
        decltype(type->callbacks[i].destructor) callback = nullptr;
        if((callbackFlags[i] & DCF_CTOR) != 0) DUAL_UNLIKELY
            callback = type->callbacks[i].destructor;
        destruct_impl(dual_chunk_view_t{chunk, 0, 1}, type->type.data[i], offsets[i], sizes[i], elemSizes[i], type->resourceFields[i], callback);
    }
}

void move_view(const dual_chunk_view_t& view, EIndex srcStart) noexcept
{
    move_view(view, view.chunk, srcStart);
}

void move_view(const dual_chunk_view_t& dstV, const dual_chunk_t* srcC, uint32_t srcStart) noexcept
{
    archetype_t* type = dstV.chunk->type;
    EIndex* offsets = type->offsets[(int)dstV.chunk->pt];
    uint32_t* sizes = type->sizes;
    uint32_t* aligns = type->aligns;
    uint32_t* elemSizes = type->elemSizes;
    uint32_t* callbackFlags = type->callbackFlags;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].move) callback = nullptr;
        if((callbackFlags[i] & DCF_MOVE) != 0) DUAL_UNLIKELY
            callback = type->callbacks[i].move;
        move_impl(dstV, srcC, srcStart, type->type.data[i], offsets[i], sizes[i], aligns[i], elemSizes[i], callback);
    }
}

void cast_view(const dual_chunk_view_t& dstV, dual_chunk_t* srcC, EIndex srcStart) noexcept
{
    archetype_t* srcType = srcC->type;
    archetype_t* dstType = dstV.chunk->type;
    EIndex* srcOffsets = srcType->offsets[srcC->pt];
    EIndex* dstOffsets = dstType->offsets[dstV.chunk->pt];
    uint32_t* srcSizes = srcType->sizes;
    uint32_t* srcAligns = srcType->aligns;
    uint32_t* dstAligns = dstType->aligns;
    uint32_t* dstSizes = dstType->sizes;
    uint32_t* srcElemSizes = srcType->elemSizes;
    uint32_t* dstElemSizes = dstType->elemSizes;
    dual_type_set_t srcTypes = srcType->type;
    dual_type_set_t dstTypes = dstType->type;
    uint32_t* srcCallbackFlags = srcType->callbackFlags;
    uint32_t* dstCallbackFlags = dstType->callbackFlags;
    uint32_t maskValue = uint32_t(1 << dstTypes.length) - 1;
    
    eastl::bitset<32>*srcMasks = nullptr, *dstMasks = nullptr;
    if (srcType->withMask && dstType->withMask)
    {
        SIndex srcMaskId = srcType->index(kMaskComponent);
        SIndex dstMaskId = dstType->index(kMaskComponent);
        dstMasks = (eastl::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcMasks = (eastl::bitset<32>*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset(dstMasks, 0, sizeof(uint32_t) * dstV.count);
    }

    eastl::bitset<32>*srcDirtys = nullptr, *dstDirtys = nullptr;
    if (srcType->withDirty && dstType->withDirty)
    {
        SIndex srcMaskId = srcType->index(kDirtyComponent);
        SIndex dstMaskId = dstType->index(kDirtyComponent);
        dstDirtys = (eastl::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcDirtys = (eastl::bitset<32>*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
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
            if((srcCallbackFlags[srcI] & DCF_CTOR) != 0) DUAL_UNLIKELY
                callback = srcType->callbacks[srcI].destructor;
            destruct_impl({ srcC, srcStart, dstV.count }, srcT, srcOffsets[srcI], srcSizes[srcI], srcElemSizes[srcI], srcType->resourceFields[srcI], callback);
            ++srcI;
        }
        else if (srcT > dstT) // construct
        {
            decltype(dstType->callbacks[dstI].constructor) callback = nullptr;
            if((dstCallbackFlags[dstI] & DCF_CTOR) != 0) DUAL_UNLIKELY
                callback = dstType->callbacks[dstI].constructor;
            construct_impl(dstV, dstT, dstOffsets[dstI], dstSizes[dstI], dstAligns[dstI], dstElemSizes[dstI], maskValue, callback);
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
                if((srcCallbackFlags[srcI] & DCF_MOVE) != 0) DUAL_UNLIKELY
                    callback = srcType->callbacks[srcI].move;
                move_impl(dstV, srcC, srcStart, srcT, srcOffsets[srcI], srcSizes[srcI], srcAligns[srcI], srcElemSizes[srcI], callback);
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
}


void duplicate_view(const dual_chunk_view_t& dstV, const dual_chunk_t* srcC, EIndex srcStart) noexcept
{
    archetype_t* srcType = srcC->type;
    archetype_t* dstType = dstV.chunk->type;
    EIndex* srcOffsets = srcType->offsets[srcC->pt];
    EIndex* dstOffsets = dstType->offsets[dstV.chunk->pt];
    uint32_t* srcSizes = srcType->sizes;
    uint32_t* srcAligns = srcType->aligns;
    uint32_t* dstAligns = dstType->aligns;
    uint32_t* dstSizes = dstType->sizes;
    uint32_t* srcElemSizes = srcType->elemSizes;
    uint32_t* dstElemSizes = dstType->elemSizes;
    dual_type_set_t srcTypes = srcType->type;
    dual_type_set_t dstTypes = dstType->type;
    uint32_t* srcCallbackFlags = srcType->callbackFlags;
    uint32_t* dstCallbackFlags = dstType->callbackFlags;
    uint32_t maskValue = uint32_t(1 << dstTypes.length) - 1;
    
    eastl::bitset<32>*srcMasks = nullptr, *dstMasks = nullptr;
    if (srcType->withMask && dstType->withMask)
    {
        SIndex srcMaskId = srcType->index(kMaskComponent);
        SIndex dstMaskId = dstType->index(kMaskComponent);
        dstMasks = (eastl::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcMasks = (eastl::bitset<32>*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset(dstMasks, 1, sizeof(uint32_t) * dstV.count);
    }

    eastl::bitset<32>* dstDirtys = nullptr;
    if (srcType->withDirty && dstType->withDirty)
    {
        SIndex dstMaskId = dstType->index(kDirtyComponent);
        dstDirtys = (eastl::bitset<32>*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
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
            if((dstCallbackFlags[dstI] & DCF_CTOR) != 0) DUAL_UNLIKELY
                callback = dstType->callbacks[dstI].constructor;
            construct_impl(dstV, dstT, dstOffsets[dstI], dstSizes[dstI], dstAligns[dstI], dstElemSizes[dstI], maskValue, callback);
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
                if((srcCallbackFlags[srcI] & DCF_COPY) != 0) DUAL_UNLIKELY
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

bool full_view(const dual_chunk_view_t& view) noexcept
{
    return view.chunk != nullptr && view.start == 0 && view.count == view.chunk->count;
}

void enable_components(const dual_chunk_view_t& view, const dual_type_set_t& types)
{
    auto group = view.chunk->group;
    auto masks = (mask_t*)dualV_get_owned_rw(&view, kMaskComponent);
    auto newMask = group->get_mask(types);
    if (!masks) DUAL_UNLIKELY
        return;
    for (uint32_t i = 0; i < view.count; ++i)
        masks[i].fetch_or(newMask);
}

void disable_components(const dual_chunk_view_t& view, const dual_type_set_t& types)
{
    auto group = view.chunk->group;
    auto masks = (mask_t*)dualV_get_owned_rw(&view, kMaskComponent);
    auto newMask = group->get_mask(types);
    if (!masks) DUAL_UNLIKELY
        return;
    for (uint32_t i = 0; i < view.count; ++i)
        masks[i].fetch_and(~newMask);
}
} // namespace dual

template <bool readonly, bool local>
auto dualV_get_owned(const dual_chunk_view_t* view, dual_type_index_t type)
{
    using namespace dual;
    using return_type = std::conditional_t<readonly, const void*, void*>;
    if (type_index_t(type).is_tag()) DUAL_UNLIKELY
        return (return_type) nullptr;
    auto chunk = view->chunk;
    auto structure = chunk->type;
    SIndex id;
    if constexpr (local)
        id = type;
    else
        id = structure->index(type);
    if (id == kInvalidSIndex)
        return (return_type) nullptr;
    if constexpr (!readonly)
        chunk->timestamps()[id] = structure->storage->timestamp;
    auto scheduler = structure->storage->scheduler;
    if (scheduler && scheduler->is_main_thread(structure->storage))
        scheduler->sync_entry(structure, id, readonly);
    EIndex offset = 0;
    if (!type_index_t(type).is_chunk())
        offset = structure->sizes[id] * view->start;
    return (return_type)(chunk->data() + offset + structure->offsets[chunk->pt][id]);
}

extern "C" {
const void* dualV_get_component_ro(const dual_chunk_view_t* view, dual_type_index_t type)
{
    auto data = dualV_get_owned_ro(view, type);
    if (!data) DUAL_UNLIKELY
        return view->chunk->group->get_shared_ro(type);
    return data;
}

const void* dualV_get_owned_ro(const dual_chunk_view_t* view, dual_type_index_t type)
{
    return dualV_get_owned<true, false>(view, type);
}

void* dualV_get_owned_rw(const dual_chunk_view_t* view, dual_type_index_t type)
{
    return dualV_get_owned<false, false>(view, type);
}

const void* dualV_get_owned_ro_local(const dual_chunk_view_t* view, dual_type_index_t type)
{
    return dualV_get_owned<true, true>(view, type);
}

void* dualV_get_owned_rw_local(const dual_chunk_view_t* view, dual_type_index_t type)
{
    return dualV_get_owned<false, true>(view, type);
}

const dual_entity_t* dualV_get_entities(const dual_chunk_view_t* view)
{
    auto chunk = view->chunk;
    return chunk->get_entities() + view->start;
}
}