#include "platform/debug.h"
#include "utils/hash.h"
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include <EASTL/set.h>

#include "render_graph/backend/desc_set_heap.hpp"
#include "render_graph/backend/buffer_pool.hpp"
#include "render_graph/backend/texture_pool.hpp"
#include "render_graph/backend/texture_view_pool.hpp"

#include "tracy/Tracy.hpp"

namespace skr
{
namespace render_graph
{
// Descriptor Set Heap

void DescSetHeap::expand(size_t set_count)
{
    auto old_count = heap.size();
    for (uint32_t i = 0; i < root_sig->table_count; i++)
    {
        CGPUDescriptorSetDescriptor desc = {};
        desc.root_signature = root_sig;
        desc.set_index = i;
        auto new_set = cgpu_create_descriptor_set(root_sig->device, &desc);
        heap.emplace_back(new_set);

        SKR_LOG_TRACE("create set %d in heap, address %lld", i + old_count, new_set);
        SKR_ASSERT(new_set->root_signature->device);
    }
}
const skr::span<CGPUDescriptorSetId> DescSetHeap::pop()
{
    if (cursor >= heap.size()) expand();
    auto res = skr::span<CGPUDescriptorSetId>(
    heap.data() + cursor, root_sig->table_count);
    cursor += root_sig->table_count;
    return res;
}
void DescSetHeap::reset() 
{ 
    for (uint32_t i = 0; i < heap.size(); i++)
    {
        ((CGPUDescriptorSet*)heap[i])->updated = false;
    }
    cursor = 0;
}
void DescSetHeap::destroy()
{
    for (uint32_t i = 0; i < heap.size(); i++)
    {
        SKR_LOG_TRACE("destroy set %d in heap with %d sets, address %lld", i, heap.size(), (int64_t)heap[i]);
        cgpu_free_descriptor_set(heap[i]);
    }
}

// Texture Pool

TexturePool::Key::Key(CGPUDeviceId device, const CGPUTextureDescriptor& desc)
    : device(device)
    , flags(desc.flags)
    , width(desc.width)
    , height(desc.height)
    , depth(desc.depth ? desc.depth : 1)
    , array_size(desc.array_size ? desc.array_size : 1)
    , format(desc.format)
    , mip_levels(desc.mip_levels ? desc.mip_levels : 1)
    , sample_count(desc.sample_count ? desc.sample_count : CGPU_SAMPLE_COUNT_1)
    , sample_quality(desc.sample_quality)
    , descriptors(desc.descriptors)
    , is_dedicated(desc.is_dedicated)
{
    
}

TexturePool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

void TexturePool::initialize(CGPUDeviceId device_)
{
    device = device_;
}

void TexturePool::finalize()
{
    for (auto&& queue : textures)
    {
        while (!queue.second.empty())
        {
            cgpu_free_texture(queue.second.front().texture);
            queue.second.pop_front();
        }
    }
}

eastl::pair<CGPUTextureId, ECGPUResourceState> TexturePool::allocate(const CGPUTextureDescriptor& desc, AllocationMark mark)
{
    eastl::pair<CGPUTextureId, ECGPUResourceState> allocated = {
        nullptr, CGPU_RESOURCE_STATE_UNDEFINED
    };
    auto key = make_zeroed<TexturePool::Key>(device, desc);
    if (textures[key].empty())
    {
        auto new_tex = cgpu_create_texture(device, &desc);
        textures[key].emplace_back(new_tex, desc.start_state, mark);
    }
    textures[key].front().mark = mark;
    allocated = { textures[key].front().texture, textures[key].front().state };
    textures[key].pop_front();
    return allocated;
}

void TexturePool::deallocate(const CGPUTextureDescriptor& desc, CGPUTextureId texture, ECGPUResourceState final_state, AllocationMark mark)
{
    auto key = make_zeroed<TexturePool::Key>(device, desc);
    for (auto&& iter : textures[key])
    {
        if (iter.texture == texture) return;
    }
    textures[key].emplace_back(texture, final_state, mark);
}

// Texture View Pool

TextureViewPool::Key::Key(CGPUDeviceId device, const CGPUTextureViewDescriptor& desc)
    : device(device)
    , texture(desc.texture)
    , format(desc.format)
    , usages(desc.usages)
    , aspects(desc.aspects)
    , dims(desc.dims)
    , base_array_layer(desc.base_array_layer)
    , array_layer_count(desc.array_layer_count)
    , base_mip_level(desc.base_mip_level)
    , mip_level_count(desc.mip_level_count)
    , tex_width(desc.texture->width)
    , tex_height(desc.texture->height)
    , unique_id(desc.texture->unique_id)

{

}

uint32_t TextureViewPool::erase(CGPUTextureId texture)
{
    auto prev_size = (uint32_t)views.size();
    for (auto it = views.begin(); it != views.end();)
    {
        if (it->first.texture == texture)
        {
            cgpu_free_texture_view(it->second.texture_view);
            it = views.erase(it);
        }
        else
            ++it;
    }
    return prev_size - (uint32_t)views.size();
}

TextureViewPool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

void TextureViewPool::initialize(CGPUDeviceId device_)
{
    device = device_;
}

void TextureViewPool::finalize()
{
    for (auto&& view : views)
    {
        cgpu_free_texture_view(view.second.texture_view);
    }
    views.clear();
}

CGPUTextureViewId TextureViewPool::allocate(const CGPUTextureViewDescriptor& desc, uint64_t frame_index)
{
    const auto key = make_zeroed<TextureViewPool::Key>(device, desc);
    auto found = views.find(key);
    if (found != views.end())
    {
        // SKR_LOG_TRACE("Reallocating texture view for texture %p (id %lld, old %lld)", desc.texture,
        //    key.texture->unique_id, found->second.texture_view->info.texture->unique_id);
        found->second.mark.frame_index = frame_index;
        SKR_ASSERT(found->first.texture);
        return found->second.texture_view;
    }
    else
    {
        // SKR_LOG_TRACE("Creating texture view for texture %p (tex %p)", desc.texture, key.texture);
        CGPUTextureViewId new_view = cgpu_create_texture_view(device, &desc);
        AllocationMark mark = {frame_index, 0};
        views[key] = PooledTextureView(new_view, mark);
        return new_view;
    }
}

// Buffer Pool

BufferPool::Key::Key(CGPUDeviceId device, const CGPUBufferDescriptor& desc)
    : device(device)
    , descriptors(desc.descriptors)
    , memory_usage(desc.memory_usage)
    , format(desc.format)
    , flags(desc.flags)
    , first_element(desc.first_element)
    , elemet_count(desc.elemet_count)
    , element_stride(desc.element_stride)
{
}

BufferPool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

void BufferPool::initialize(CGPUDeviceId device_)
{
    device = device_;
}

void BufferPool::finalize()
{
    for (auto&& [key, queue] : buffers)
    {
        while (!queue.empty())
        {
            cgpu_free_buffer(queue.front().buffer);
            queue.pop_front();
        }
    }
}

eastl::pair<CGPUBufferId, ECGPUResourceState> BufferPool::allocate(const CGPUBufferDescriptor& desc, AllocationMark mark, uint64_t min_frame_index)
{
    eastl::pair<CGPUBufferId, ECGPUResourceState> allocated = {
        nullptr, CGPU_RESOURCE_STATE_UNDEFINED
    };
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    int32_t found_index = -1;
    for (uint32_t i = 0; i < buffers[key].size(); ++i)
    {
        auto&& pooled = buffers[key][i];
        if (pooled.mark.frame_index < min_frame_index && pooled.buffer->size >= desc.size)
        {
            found_index = i;
            break;
        }
    }
    if ( found_index > 0 )
    {
        PooledBuffer found = buffers[key][found_index];
        buffers[key].erase(buffers[key].begin() + found_index);
        buffers[key].emplace_front(found.buffer, found.state, mark);
    }
    if ( found_index < 0 )
    {
        auto new_buffer = cgpu_create_buffer(device, &desc);
        buffers[key].emplace_front(new_buffer, desc.start_state , mark);
    }
    allocated = { buffers[key].front().buffer, buffers[key].front().state };
    buffers[key].pop_front();
    return allocated;
}

void BufferPool::deallocate(const CGPUBufferDescriptor& desc, CGPUBufferId buffer, ECGPUResourceState final_state, AllocationMark mark)
{
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    for (auto&& iter : buffers[key])
    {
        if (iter.buffer == buffer) 
            return;
    }
    buffers[key].emplace_back(buffer, final_state, mark);
}

}
}
