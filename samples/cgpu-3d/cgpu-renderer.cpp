#define CGLTF_IMPLEMENTATION
#include "thirdparty/cgltf.h"

#include "render-device.hpp"
#include "render-context.hpp"
#include "render-scene.hpp"

void RenderDevice::Initialize(ECGpuBackend backend)
{
    // create sdl window
    sdl_window_ = SDL_CreateWindow(gCGpuBackendNames[backend],
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window_, &wmInfo);
    // create backend device
    backend_ = backend;
    // create instance
    CGpuInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name = true;
    instance_ = cgpu_create_instance(&instance_desc);
    {
        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance_, CGPU_NULLPTR, &adapters_count);
        CGpuAdapterId adapters[256];
        cgpu_enum_adapters(instance_, adapters, &adapters_count);
        adapter_ = adapters[0];
    }
    CGpuQueueGroupDescriptor G = {};
    G.queueType = QUEUE_TYPE_GRAPHICS;
    G.queueCount = 1;
    CGpuDeviceDescriptor device_desc = {};
    device_desc.queueGroups = &G;
    device_desc.queueGroupCount = 1;
    device_ = cgpu_create_device(adapter_, &device_desc);
    gfx_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_GRAPHICS, 0);
#if defined(_WIN32) || defined(_WIN64)
    surface_ = cgpu_surface_from_hwnd(device_, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGpuNSView* ns_view = (struct CGpuNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGpuSwapChainDescriptor swapchain_desc = {};
    swapchain_desc.presentQueues = &gfx_queue_;
    swapchain_desc.presentQueuesCount = 1;
    swapchain_desc.width = BACK_BUFFER_WIDTH;
    swapchain_desc.height = BACK_BUFFER_HEIGHT;
    swapchain_desc.surface = surface_;
    swapchain_desc.imageCount = 3;
    swapchain_desc.format = PF_R8G8B8A8_UNORM;
    swapchain_desc.enableVsync = true;
    swapchain_ = cgpu_create_swapchain(device_, &swapchain_desc);
}

void RenderDevice::Destroy()
{
    cgpu_free_swapchain(swapchain_);
    cgpu_free_surface(device_, surface_);
    cgpu_free_queue(gfx_queue_);
    cgpu_free_device(device_);
    cgpu_free_instance(instance_);
    SDL_DestroyWindow(sdl_window_);
}

void RenderContext::Initialize(RenderDevice* device)
{
    device_ = device;
    CGpuCommandPoolDescriptor pool_desc = {};
    cmd_pool_ = cgpu_create_command_pool(device->gfx_queue_, &pool_desc);
    CGpuCommandBufferDescriptor cmd_desc = {};
    cmd_desc.is_secondary = false;
    cmd_buffer_ = cgpu_create_command_buffer(cmd_pool_, &cmd_desc);
}

void RenderContext::Destroy()
{
    cgpu_reset_command_pool(cmd_pool_);
    cgpu_free_command_buffer(cmd_buffer_);
    cgpu_free_command_pool(cmd_pool_);
}

int32_t RenderMesh::loadPrimitive(struct cgltf_primitive* src, uint32_t& index_cursor)
{
    RenderPrimitive newPrim = {};
    newPrim.first_index_ = src->indices->count;
    newPrim.first_index_ = index_cursor;
    primitives_.emplace_back(newPrim);
    index_cursor += newPrim.first_index_;
    return primitives_.size() - 1;
}

int32_t RenderScene::loadMesh(struct cgltf_mesh* src)
{
    RenderMesh newMesh = {};
    newMesh.name_ = src->name;
    uint32_t index_cursor = 0;
    newMesh.primitives_.reserve(src->primitives_count);
    for (uint32_t i = 0; i < src->primitives_count; i++)
    {
        auto gltf_prim = src->primitives + i;
        newMesh.loadPrimitive(gltf_prim, index_cursor);
    }
    meshes_.emplace_back(newMesh);
    return meshes_.size() - 1;
}

int32_t RenderScene::loadNode(struct cgltf_node* src, int32_t parent_idx)
{
    const bool isRoot = (parent_idx == -1);
    nodes_.emplace_back();
    RenderNode& newNode = nodes_[nodes_.size() - 1];
    newNode.index_ = nodes_.size() - 1;
    newNode.parent_ = isRoot ? nullptr : &nodes_[parent_idx];
    newNode.children_.reserve(src->children_count);
    for (uint32_t i = 0; i < src->children_count; i++)
    {
        int32_t child_idx = loadNode(src->children[i], newNode.index_);
        newNode.children_.emplace_back(&nodes_[child_idx]);
    }
    newNode.translation_ = sakura::math::Vector3f(
        src->translation[0], src->translation[1], src->translation[2]);
    newNode.rotation_ = sakura::math::Quaternion(
        src->rotation[0], src->rotation[1], src->rotation[2], src->rotation[3]);
    newNode.scale_ = sakura::math::Vector3f(
        src->scale[0], src->scale[1], src->scale[2]);
    return newNode.index_;
}

void RenderScene::Initialize(const char8_t* path)
{
    cgltf_options options = {};
    if (path)
    {
        cgltf_result result = cgltf_parse_file(&options, path, &gltf_data_);
        if (result != cgltf_result_success)
        {
            gltf_data_ = nullptr;
            return;
        }
        else
        {
            result = cgltf_load_buffers(&options, gltf_data_, path);
            result = cgltf_validate(gltf_data_);
            if (result != cgltf_result_success)
            {
                return;
            }
        }
        load_ready_ = true;
    }
    if (load_ready_)
    {
        meshes_.reserve(gltf_data_->meshes_count);
        for (uint32_t i = 0; i < gltf_data_->meshes_count; i++)
        {
            auto gltf_mesh = gltf_data_->meshes + i;
            loadMesh(gltf_mesh);
        }

        nodes_.reserve(gltf_data_->nodes_count);
        for (uint32_t i = 0; i < gltf_data_->nodes_count; i++)
        {
            auto gltf_node = gltf_data_->nodes + i;
            if (gltf_node->parent == nullptr)
            {
                root_node_index_ = loadNode(gltf_node, -1);
                break;
            }
        }
    }
}

void RenderScene::Upload(RenderContext* context, bool keep_gltf_data_)
{
    if (load_ready_)
    {
        for (uint32_t i = 0; i < gltf_data_->buffer_views_count; i++)
        {
            cgltf_buffer_view* buf_view = gltf_data_->buffer_views + i;
            CGpuBufferDescriptor buffer_desc = {};
            buffer_desc.flags = BCF_OWN_MEMORY_BIT;
            buffer_desc.memory_usage = MEM_USAGE_GPU_ONLY;
            buffer_desc.descriptors =
                buf_view->type == cgltf_buffer_view_type_indices ?
                    RT_INDEX_BUFFER :
                    RT_VERTEX_BUFFER;
            buffer_desc.element_stride = buf_view->stride ? buf_view->stride : buf_view->size;
            buffer_desc.elemet_count = buf_view->size / buffer_desc.element_stride;
            buffer_desc.size = buf_view->size;
            buffer_desc.name = buf_view->name;
            if (buf_view->type == cgltf_buffer_view_type_indices)
            {
                index_buffer_ = cgpu_create_buffer(context->device_->device_, &buffer_desc);
            }
            else
            {
                auto vertex_buffer_ = cgpu_create_buffer(context->device_->device_, &buffer_desc);
                vertex_buffers_.emplace_back(vertex_buffer_);
            }
        }
    }
}

void RenderScene::Destroy()
{
    if (gltf_data_) cgltf_free(gltf_data_);
    if (index_buffer_) cgpu_free_buffer(index_buffer_);
    for (auto vertex_buffer_ : vertex_buffers_)
    {
        if (vertex_buffer_) cgpu_free_buffer(vertex_buffer_);
    }
}