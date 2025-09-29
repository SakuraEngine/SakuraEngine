#include <type_traits>
#include "SkrRuntime/sugoi/sugoi.h"
#include "SkrRuntime/sugoi/array.hpp"
#include "SkrRuntime/sugoi/type_registry.hpp"
#include "./mask.hpp"
#include "SkrRuntime/sugoi/chunk.hpp"
#include "./chunk_view.hpp"
#include "SkrRuntime/sugoi/archetype.hpp"
#include "./impl/storage.hpp"

namespace sugoi
{
static ArrayComponentBase* new_array(
    void* ptr,
    size_t arrMemSize,
    size_t elemSize,
    size_t inlineCount,
    size_t align
)
{
    auto* array = (ArrayComponentBase*)ptr;
    array->unsafe_setup_inline(align, inlineCount);
    return array;
}

static void construct_impl(
    sugoi_chunk_view_t view,
    sugoi_chunk_t::RSlice& slice,
    type_index_t type,
    EIndex offset,
    uint32_t size,
    uint32_t align,
    uint32_t elemSize,
    uint32_t inlineCount,
    uint32_t maskValue,
    void (*constructor)(sugoi_type_index_t type, sugoi_chunk_t* chunk, EIndex index, char* data)
)
{
    char* dst = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    if (type.is_buffer())
        forloop (j, 0, view.count)
        {
            char* buf = (size_t)j * size + dst;
            auto array = new_array(
                buf,
                size,
                elemSize,
                inlineCount,
                align
            );
            // 初始的时候是空的，没必要构造
            // if (constructor)
            //     for_buffer(curr, array, elemSize)
            //         constructor(type, view.chunk, view.start + j, curr);
        }
    else if (type == kMaskComponent)
        forloop (j, 0, view.count)
            ((mask_t*)dst)[j] = maskValue;
    else if (type == kDirtyComponent)
        memset(dst, 0xFFFFFFFF, (size_t)size * view.count);
    else if (constructor)
        forloop (j, 0, view.count)
            constructor(type, view.chunk, view.start + j, (size_t)j * size + dst);
    else
        memset(dst, 0, (size_t)size * view.count);
}

static void destruct_impl(
    sugoi_chunk_view_t view,
    type_index_t type,
    EIndex offset,
    uint32_t size,
    uint32_t elemSize,
    uint32_t inlineCount,
    uint32_t align,
    void (*destructor)(sugoi_type_index_t type, sugoi_chunk_t* chunk, EIndex index, char* data)
)
{
    char* src = view.chunk->data() + (size_t)offset + (size_t)size * view.start;
    if (type.is_buffer())
        forloop (j, 0, view.count)
        {
            auto array = (ArrayComponentBase*)((size_t)j * size + src);
            if (destructor)
            {
                for (uint64_t arr_idx = 0; arr_idx < array->size(); ++arr_idx)
                {
                    auto* curr = ::skr::memory::offset_item(array->unsafe_data(), elemSize, arr_idx);
                    destructor(type, view.chunk, view.start + j, (char*)curr);
                }
            }
            // free memory if needed
            if (array->capacity() > inlineCount)
                skr::SkrAllocator::free_raw(array->unsafe_data(), align);
        }
    else if (destructor)
        forloop (j, 0, view.count)
            destructor(type, view.chunk, view.start + j, (size_t)j * size + src);
}

static void move_impl(
    sugoi_chunk_view_t dstV,
    const sugoi_chunk_t* srcC,
    uint32_t srcStart,
    type_index_t type,
    EIndex srcOffset,
    EIndex dstOffset,
    uint32_t size,
    uint32_t align,
    uint32_t elemSize,
    uint32_t inlineCount,
    void (*move)(sugoi_type_index_t type, sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, char* src)
)
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
                auto arrayDst = (ArrayComponentBase*)((size_t)j * size + dst);
                auto arraySrc = (ArrayComponentBase*)((size_t)j * size + src);
                if (arraySrc->capacity() > inlineCount) // memory is on heap
                    *arrayDst = *arraySrc;              // just steal it
                else                                    // memory is in chunk
                {
                    new_array(
                        arrayDst,
                        size,
                        elemSize,
                        inlineCount,
                        align
                    );
                    arrayDst->set_size(arraySrc->size());
                    for (uint64_t arr_idx = 0; arr_idx < arraySrc->size(); ++arr_idx)
                    {
                        auto* currSrc = ::skr::memory::offset_item(arraySrc->unsafe_data(), elemSize, arr_idx);
                        auto* currDst = ::skr::memory::offset_item(arrayDst->unsafe_data(), elemSize, arr_idx);

                        move(
                            type,
                            dstV.chunk,
                            dstV.start + j,
                            (char*)currDst,
                            (sugoi_chunk_t*)srcC,
                            srcStart + j,
                            (char*)currSrc
                        );
                    }
                }
            }
        }
        else
            forloop (j, 0, dstV.count)
                move(
                    type,
                    dstV.chunk,
                    dstV.start + j,
                    (size_t)j * size + dst,
                    (sugoi_chunk_t*)srcC,
                    srcStart + j,
                    (size_t)j * size + src
                );
    }
    else
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (ArrayComponentBase*)((size_t)j * size + dst);
                auto arraySrc = (ArrayComponentBase*)((size_t)j * size + src);
                if (arraySrc->capacity() > inlineCount) // memory is on heap
                    *arrayDst = *arraySrc;              // just steal it
                else                                    // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, inlineCount, align);
                    arrayDst->set_size(arraySrc->size());
                    memcpy(arrayDst->unsafe_data(), arraySrc->unsafe_data(), arraySrc->size() * elemSize);
                }
            }
        }
        else
            memcpy(dst, src, dstV.count * (size_t)size);
    }
}

static void clone_impl(
    sugoi_chunk_view_t dstV,
    const sugoi_chunk_t* srcC,
    uint32_t srcStart,
    type_index_t type,
    EIndex srcOffset,
    EIndex dstOffset,
    uint32_t size,
    uint32_t align,
    uint32_t elemSize,
    uint32_t inlineCount,
    void (*copy)(sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, const char* src)
)
{
    SKR_ASSERT(!type.is_chunk());
    char* dst = dstV.chunk->data() + (size_t)dstOffset + (size_t)size * dstV.start;
    char* src = srcC->data() + (size_t)srcOffset + (size_t)size * srcStart;
    if (copy)
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (ArrayComponentBase*)((size_t)j * size + dst);
                auto arraySrc = (ArrayComponentBase*)((size_t)j * size + src);
                if (arraySrc->capacity() > inlineCount) // memory is on heap
                {
                    arrayDst->unsafe_set_data(
                        skr::SkrAllocator::alloc_raw(
                            arraySrc->capacity(),
                            elemSize,
                            align
                        )
                    );
                    arrayDst->unsafe_set_capacity(arraySrc->capacity());
                    arrayDst->set_size(arraySrc->size());
                }
                else // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, inlineCount, align);
                    arrayDst->set_size(arraySrc->size());
                }
                for (uint64_t arr_idx = 0; arr_idx < arraySrc->size(); ++arr_idx)
                {
                    auto* currSrc = ::skr::memory::offset_item(arraySrc->unsafe_data(), elemSize, arr_idx);
                    auto* currDst = ::skr::memory::offset_item(arrayDst->unsafe_data(), elemSize, arr_idx);

                    copy(
                        dstV.chunk,
                        dstV.start + j,
                        (char*)currDst,
                        (sugoi_chunk_t*)srcC,
                        srcStart + j,
                        (char*)currSrc
                    );
                }
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
                auto arrayDst = (ArrayComponentBase*)((size_t)j * size + dst);
                auto arraySrc = (ArrayComponentBase*)((size_t)j * size + src);
                if (arraySrc->capacity() > inlineCount) // memory is on heap
                {
                    arrayDst->unsafe_set_data(
                        skr::SkrAllocator::alloc_raw(
                            arraySrc->capacity(),
                            elemSize,
                            align
                        )
                    );
                    arrayDst->unsafe_set_capacity(arraySrc->capacity());
                    arrayDst->set_size(arraySrc->size());
                }
                else // memory is in chunk
                {
                    new_array(arrayDst, size, elemSize, inlineCount, align);
                    arrayDst->set_size(arraySrc->size());
                }
                memcpy(arrayDst->unsafe_data(), arraySrc->unsafe_data(), arraySrc->size() * elemSize);
            }
        }
        else
        {
            memcpy(dst, src, dstV.count * (size_t)size);
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

static void duplicate_impl(
    sugoi_chunk_view_t dstV,
    const sugoi_chunk_t* srcC,
    uint32_t srcIndex,
    type_index_t type,
    EIndex offset,
    EIndex dstOffset,
    uint32_t size,
    uint32_t align,
    uint32_t elemSize,
    uint32_t inlineCount,
    void (*copy)(sugoi_type_index_t type, sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, const char* src)
)
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
    if (copy)
    {
        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arrayDst = (ArrayComponentBase*)((size_t)j * size + dst);
                auto arraySrc = (ArrayComponentBase*)src;
                if (arraySrc->capacity() > inlineCount)
                {
                    arrayDst->unsafe_set_data(
                        skr::SkrAllocator::alloc_raw(
                            arraySrc->capacity(),
                            elemSize,
                            align
                        )
                    );
                    arrayDst->unsafe_set_capacity(arraySrc->capacity());
                    arrayDst->set_size(arraySrc->size());
                }
                else
                {
                    new_array(arrayDst, size, elemSize, inlineCount, align);
                    arrayDst->set_size(arraySrc->size());
                }

                for (uint64_t arr_idx = 0; arr_idx < arraySrc->size(); ++arr_idx)
                {
                    auto* currSrc = ::skr::memory::offset_item(arraySrc->unsafe_data(), elemSize, arr_idx);
                    auto* currDst = ::skr::memory::offset_item(arrayDst->unsafe_data(), elemSize, arr_idx);

                    copy(type, dstV.chunk, dstV.start + j, (char*)currDst, (sugoi_chunk_t*)srcC, srcIndex, (char*)currSrc);
                }
            }
        }
        else
            forloop (j, 0, dstV.count)
                copy(type, dstV.chunk, dstV.start + j, (size_t)j * size + dst, (sugoi_chunk_t*)srcC, srcIndex, src);
    }
    else
    {

        if (type.is_buffer())
        {
            forloop (j, 0, dstV.count)
            {
                auto arraySrc = (ArrayComponentBase*)src;
                auto arrayDst = (ArrayComponentBase*)((size_t)j * size + dst);
                if (arraySrc->capacity() > inlineCount)
                {
                    arrayDst->unsafe_set_data(
                        skr::SkrAllocator::alloc_raw(
                            arraySrc->capacity(),
                            elemSize,
                            align
                        )
                    );
                    arrayDst->unsafe_set_capacity(arraySrc->capacity());
                    arrayDst->set_size(arraySrc->size());
                }
                else
                {
                    new_array(arrayDst, size, elemSize, inlineCount, align);
                    arrayDst->set_size(arraySrc->size());
                }
            }
        }
        else
        {
            memdup(dst, src, (size_t)size, (size_t)dstV.count);
        }
    }
}

void construct_view(const sugoi_chunk_view_t& view) noexcept
{
    SkrZoneScopedN("sugoi_storage_t::construct_view");

    archetype_t* type = view.chunk->structure;
    const auto* offsets = type->offsets[(int)view.chunk->pt];
    const auto* sizes = type->sizes;
    const auto* aligns = type->aligns;
    const auto* elemSizes = type->arrElemSizes;
    const auto* inlineCounts = type->arrInlineCounts;
    const auto* callbackFlags = type->callbackFlags;
    auto maskValue = uint32_t(1 << type->type.length) - 1;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].constructor) callback = nullptr;
        if ((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].constructor;

        sugoi_chunk_t::RSlice ctorSlice = view.chunk->s_lock(type->type.data[i], view);
        construct_impl(
            view,
            ctorSlice,
            type->type.data[i],
            offsets[i],
            sizes[i],
            aligns[i],
            elemSizes[i],
            inlineCounts[i],
            maskValue,
            callback
        );
        view.chunk->s_unlock(type->type.data[i], view);
    }
}

void destruct_view(const sugoi_chunk_view_t& view) noexcept
{
    archetype_t* type = view.chunk->structure;
    const auto* offsets = type->offsets[(int)view.chunk->pt];
    const auto* sizes = type->sizes;
    const auto* elemSizes = type->arrElemSizes;
    const auto* callbackFlags = type->callbackFlags;
    const auto* inlineCounts = type->arrInlineCounts;
    const auto* aligns = type->aligns;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].destructor) callback = nullptr;
        if ((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].destructor;

        destruct_impl(
            view,
            type->type.data[i],
            offsets[i],
            sizes[i],
            elemSizes[i],
            inlineCounts[i],
            aligns[i],
            callback
        );
    }
}

void construct_chunk(sugoi_chunk_t* chunk) noexcept
{
    archetype_t* type = chunk->structure;
    const auto* offsets = type->offsets[(int)chunk->pt];
    const auto* sizes = type->sizes;
    const auto* aligns = type->aligns;
    const auto* elemSizes = type->arrElemSizes;
    const auto* inlineCounts = type->arrInlineCounts;
    const auto* callbackFlags = type->callbackFlags;
    auto maskValue = uint32_t(1 << type->type.length) - 1;

    for (SIndex i = type->firstChunkComponent; i < type->type.length; ++i)
    {
        decltype(type->callbacks[i].constructor) callback = nullptr;
        if ((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].constructor;

        const auto ctorView = sugoi_chunk_view_t{ chunk, 0, 1 };
        sugoi_chunk_t::RSlice ctorSlice = chunk->s_lock(type->type.data[i], ctorView);
        construct_impl(
            ctorView,
            ctorSlice,
            type->type.data[i],
            offsets[i],
            sizes[i],
            aligns[i],
            inlineCounts[i],
            elemSizes[i], maskValue,
            callback
        );
        chunk->s_unlock(type->type.data[i], ctorView);
    }
}

void destruct_chunk(sugoi_chunk_t* chunk) noexcept
{
    archetype_t* type = chunk->structure;
    const auto* offsets = type->offsets[(int)chunk->pt];
    const auto* sizes = type->sizes;
    const auto* elemSizes = type->arrElemSizes;
    const auto* inlineCounts = type->arrInlineCounts;
    const auto* aligns = type->aligns;
    const auto* callbackFlags = type->callbackFlags;
    for (SIndex i = type->firstChunkComponent; i < type->type.length; ++i)
    {
        decltype(type->callbacks[i].destructor) callback = nullptr;
        if ((callbackFlags[i] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].destructor;

        const auto dtorView = sugoi_chunk_view_t{ chunk, 0, 1 };
        destruct_impl(
            dtorView,
            type->type.data[i],
            offsets[i],
            sizes[i],
            elemSizes[i],
            inlineCounts[i],
            aligns[i],
            callback
        );
    }
}

void move_view(const sugoi_chunk_view_t& view, EIndex srcStart) noexcept
{
    move_view(view, view.chunk, srcStart);
}

void move_view(const sugoi_chunk_view_t& dstV, const sugoi_chunk_t* srcC, uint32_t srcStart) noexcept
{
    archetype_t* type = dstV.chunk->structure;
    const auto* offsets = type->offsets[(int)dstV.chunk->pt];
    const auto* sizes = type->sizes;
    const auto* aligns = type->aligns;
    const auto* elemSizes = type->arrElemSizes;
    const auto* inlineCounts = type->arrInlineCounts;
    const auto* callbackFlags = type->callbackFlags;
    for (SIndex i = 0; i < type->firstChunkComponent; ++i)
    {
        decltype(type->callbacks[i].move) callback = nullptr;
        if ((callbackFlags[i] & SUGOI_CALLBACK_FLAG_MOVE) != 0) SUGOI_UNLIKELY
            callback = type->callbacks[i].move;
        move_impl(
            dstV,
            srcC,
            srcStart,
            type->type.data[i],
            offsets[i],
            offsets[i],
            sizes[i],
            aligns[i],
            elemSizes[i],
            inlineCounts[i],
            callback
        );
    }
}

void cast_view(const sugoi_chunk_view_t& dstV, sugoi_chunk_t* srcC, EIndex srcStart) noexcept
{
    archetype_t* srcType = srcC->structure;
    archetype_t* dstType = dstV.chunk->structure;
    const auto* srcOffsets = srcType->offsets[srcC->pt];
    const auto* dstOffsets = dstType->offsets[dstV.chunk->pt];
    const auto* srcSizes = srcType->sizes;
    const auto* srcAligns = srcType->aligns;
    const auto* dstAligns = dstType->aligns;
    const auto* dstSizes = dstType->sizes;
    const auto* srcElemSizes = srcType->arrElemSizes;
    const auto* dstElemSizes = dstType->arrElemSizes;
    const auto* srcInlineCounts = srcType->arrInlineCounts;
    const auto* dstInlineCounts = dstType->arrInlineCounts;
    sugoi_type_set_t srcTypes = srcType->type;
    sugoi_type_set_t dstTypes = dstType->type;
    const auto* srcCallbackFlags = srcType->callbackFlags;
    const auto* dstCallbackFlags = dstType->callbackFlags;
    uint32_t maskValue = uint32_t(1 << dstTypes.length) - 1;

    sugoi::bitset32 *srcMasks = nullptr, *dstMasks = nullptr;
    if (srcType->withMask && dstType->withMask)
    {
        SIndex srcMaskId = srcType->index(kMaskComponent);
        SIndex dstMaskId = dstType->index(kMaskComponent);
        dstMasks = (sugoi::bitset32*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcMasks = (sugoi::bitset32*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset((void*)dstMasks, 0, sizeof(uint32_t) * dstV.count);
    }

    sugoi::bitset32 *srcDirtys = nullptr, *dstDirtys = nullptr;
    if (srcType->withDirty && dstType->withDirty)
    {
        SIndex srcMaskId = srcType->index(kDirtyComponent);
        SIndex dstMaskId = dstType->index(kDirtyComponent);
        dstDirtys = (sugoi::bitset32*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcDirtys = (sugoi::bitset32*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset((void*)dstDirtys, 0, sizeof(uint32_t) * dstV.count);
    }

    SIndex srcI = 0, dstI = 0;

    while (srcI < srcType->firstChunkComponent && dstI < dstType->firstChunkComponent)
    {
        type_index_t srcT = srcTypes.data[srcI];
        type_index_t dstT = dstTypes.data[dstI];
        if (srcT < dstT) // destruct
        {
            decltype(srcType->callbacks[srcI].destructor) callback = nullptr;
            if ((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
                callback = srcType->callbacks[srcI].destructor;

            sugoi_chunk_view_t dtorView = { srcC, srcStart, dstV.count };
            destruct_impl(
                dtorView,
                srcT,
                srcOffsets[srcI],
                srcSizes[srcI],
                srcElemSizes[srcI],
                srcInlineCounts[srcI],
                srcAligns[srcI],
                callback
            );

            ++srcI;
        }
        else if (srcT > dstT) // construct
        {
            decltype(dstType->callbacks[dstI].constructor) callback = nullptr;
            if ((dstCallbackFlags[dstI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
                callback = dstType->callbacks[dstI].constructor;

            sugoi_chunk_t::RSlice ctorSlice = dstV.chunk->s_lock(dstT, dstV);
            construct_impl(
                dstV,
                ctorSlice,
                dstT,
                dstOffsets[dstI],
                dstSizes[dstI],
                dstAligns[dstI],
                dstElemSizes[dstI],
                dstInlineCounts[dstI],
                maskValue,
                callback
            );
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
                if ((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_MOVE) != 0) SUGOI_UNLIKELY
                    callback = srcType->callbacks[srcI].move;
                move_impl(
                    dstV,
                    srcC,
                    srcStart,
                    srcT,
                    srcOffsets[srcI],
                    dstOffsets[dstI],
                    srcSizes[srcI],
                    srcAligns[srcI],
                    srcElemSizes[srcI],
                    srcInlineCounts[srcI],
                    callback
                );
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
    while (srcI < srcType->firstChunkComponent)
    {
        type_index_t srcT = srcTypes.data[srcI];
        decltype(srcType->callbacks[srcI].destructor) callback = nullptr;
        if ((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = srcType->callbacks[srcI].destructor;

        sugoi_chunk_view_t dtorView = { srcC, srcStart, dstV.count };
        destruct_impl(
            dtorView,
            srcT,
            srcOffsets[srcI],
            srcSizes[srcI],
            srcElemSizes[srcI],
            srcInlineCounts[srcI],
            srcAligns[srcI],
            callback
        );

        ++srcI;
    }
    while (dstI < dstType->firstChunkComponent)
    {
        type_index_t dstT = dstTypes.data[dstI];
        decltype(dstType->callbacks[dstI].constructor) callback = nullptr;
        if ((dstCallbackFlags[dstI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
            callback = dstType->callbacks[dstI].constructor;

        sugoi_chunk_t::RSlice ctorSlice = dstV.chunk->s_lock(dstT, dstV);
        construct_impl(
            dstV,
            ctorSlice,
            dstT,
            dstOffsets[dstI],
            dstSizes[dstI],
            dstAligns[dstI],
            dstElemSizes[dstI],
            dstInlineCounts[dstI],
            maskValue,
            callback
        );
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
    const auto* srcOffsets = srcType->offsets[srcC->pt];
    const auto* dstOffsets = dstType->offsets[dstV.chunk->pt];
    const auto* srcSizes = srcType->sizes;
    const auto* srcAligns = srcType->aligns;
    const auto* dstAligns = dstType->aligns;
    const auto* dstSizes = dstType->sizes;
    const auto* srcElemSizes = srcType->arrElemSizes;
    const auto* dstElemSizes = dstType->arrElemSizes;
    const auto* srcInlineCounts = srcType->arrInlineCounts;
    const auto* dstInlineCounts = dstType->arrInlineCounts;
    sugoi_type_set_t srcTypes = srcType->type;
    sugoi_type_set_t dstTypes = dstType->type;
    const auto* srcCallbackFlags = srcType->callbackFlags;
    const auto* dstCallbackFlags = dstType->callbackFlags;
    uint32_t maskValue = uint32_t(1 << dstTypes.length) - 1;

    sugoi::bitset32 *srcMasks = nullptr, *dstMasks = nullptr;
    if (srcType->withMask && dstType->withMask)
    {
        SIndex srcMaskId = srcType->index(kMaskComponent);
        SIndex dstMaskId = dstType->index(kMaskComponent);
        dstMasks = (sugoi::bitset32*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        srcMasks = (sugoi::bitset32*)(srcC->data() + (size_t)srcOffsets[srcMaskId] + (size_t)srcSizes[srcMaskId] * srcStart);
        std::memset((void*)dstMasks, 1, sizeof(uint32_t) * dstV.count);
    }

    sugoi::bitset32* dstDirtys = nullptr;
    if (srcType->withDirty && dstType->withDirty)
    {
        SIndex dstMaskId = dstType->index(kDirtyComponent);
        dstDirtys = (sugoi::bitset32*)(dstV.chunk->data() + (size_t)dstOffsets[dstMaskId] + (size_t)dstSizes[dstMaskId] * dstV.start);
        std::memset((void*)dstDirtys, 1, sizeof(uint32_t) * dstV.count);
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
            if ((dstCallbackFlags[dstI] & SUGOI_CALLBACK_FLAG_CTOR) != 0) SUGOI_UNLIKELY
                callback = dstType->callbacks[dstI].constructor;

            sugoi_chunk_t::RSlice ctorSlice = dstV.chunk->s_lock(dstT, dstV);
            construct_impl(
                dstV,
                ctorSlice,
                dstT,
                dstOffsets[dstI],
                dstSizes[dstI],
                dstAligns[dstI],
                dstElemSizes[dstI],
                dstInlineCounts[dstI],
                maskValue,
                callback
            );
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
                if ((srcCallbackFlags[srcI] & SUGOI_CALLBACK_FLAG_COPY) != 0) SUGOI_UNLIKELY
                    callback = srcType->callbacks[srcI].copy;
                duplicate_impl(
                    dstV,
                    srcC,
                    srcStart,
                    srcT,
                    srcOffsets[srcI],
                    dstOffsets[dstI],
                    srcSizes[srcI],
                    srcAligns[srcI],
                    srcElemSizes[srcI],
                    srcInlineCounts[srcI],
                    callback
                );
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
    const auto* srcOffsets = srcType->offsets[srcC->pt];
    const auto* dstOffsets = dstType->offsets[dstV.chunk->pt];
    const auto* srcSizes = srcType->sizes;
    const auto* srcAligns = srcType->aligns;
    const auto* srcElemSizes = srcType->arrElemSizes;
    const auto* srcInlineCounts = srcType->arrInlineCounts;
    sugoi_type_set_t srcTypes = srcType->type;
    const auto* srcCallbackFlags = srcType->callbackFlags;

    for (uint32_t i = 0; i < srcType->firstChunkComponent; ++i)
    {
        type_index_t srcT = srcTypes.data[i];
        decltype(srcType->callbacks[i].move) callback = nullptr;
        if ((srcCallbackFlags[i] & SUGOI_CALLBACK_FLAG_MOVE) != 0) SUGOI_UNLIKELY
            callback = srcType->callbacks[i].move;
        move_impl(
            dstV,
            srcC,
            srcStart,
            srcT,
            srcOffsets[i],
            dstOffsets[i],
            srcSizes[i],
            srcAligns[i],
            srcElemSizes[i],
            srcInlineCounts[i],
            callback
        );
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

#if !SKR_SHIPPING
    // check rw with parameters
    if constexpr (!readonly)
    {
        for (uint32_t idx = 0; view->params && (idx < view->params->length); idx++)
        {
            if ((view->params->types[idx] == tid) && (view->params->accesses[idx].readonly != 0))
            {
                SKR_LOG_WARN(u8"readwrite access to a component(tid: %d, name: %s) which is queried as readonly!", tid, sugoiT_get_desc(tid)->name);
                return (return_type) nullptr;
            }
        }
    }
#endif

    if constexpr (!readonly)
        chunk->set_timestamp_at(slot, structure->storage->timestamp());

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