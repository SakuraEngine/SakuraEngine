#define CGLTF_IMPLEMENTATION
#include "thirdparty/cgltf.h"

#include "render-device.hpp"
#include "render-context.hpp"
#include "render-scene.hpp"
#include <EASTL/vector_map.h>

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
    CGpuQueueGroupDescriptor Gs[2];
    Gs[0].queueType = QUEUE_TYPE_GRAPHICS;
    Gs[0].queueCount = 1;
    Gs[1].queueType = QUEUE_TYPE_TRANSFER;
    Gs[1].queueCount = 1;
    if (cgpu_query_queue_count(adapter_, QUEUE_TYPE_TRANSFER))
    {
        CGpuDeviceDescriptor device_desc = {};
        device_desc.queueGroups = Gs;
        device_desc.queueGroupCount = 2;
        device_ = cgpu_create_device(adapter_, &device_desc);
        gfx_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_GRAPHICS, 0);
        cpy_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_TRANSFER, 0);
    }
    else
    {
        CGpuDeviceDescriptor device_desc = {};
        device_desc.queueGroups = Gs;
        device_desc.queueGroupCount = 1;
        device_ = cgpu_create_device(adapter_, &device_desc);
        gfx_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_GRAPHICS, 0);
        cpy_queue_ = gfx_queue_;
    }
    // copy cmd pool
    CGpuCommandPoolDescriptor pool_desc = {};
    cpy_cmd_pool_ = cgpu_create_command_pool(cpy_queue_, &pool_desc);
#if defined(_WIN32) || defined(_WIN64)
    surface_ = cgpu_surface_from_hwnd(device_, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGpuNSView* ns_view = (struct CGpuNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface_ = cgpu_surface_from_ns_view(device_, ns_view);
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
    // create default shaders & resources
    CGpuShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.stage = SHADER_STAGE_VERT;
    vs_desc.code = get_vertex_shader();
    vs_desc.code_size = get_vertex_shader_size();
    vs_library_ = cgpu_create_shader_library(device_, &vs_desc);
    CGpuShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "FragmentShaderLibrary";
    fs_desc.stage = SHADER_STAGE_FRAG;
    fs_desc.code = get_fragment_shader();
    fs_desc.code_size = get_fragment_shader_size();
    fs_library_ = cgpu_create_shader_library(device_, &fs_desc);
}

void RenderDevice::Destroy()
{
    cgpu_free_shader_library(vs_library_);
    cgpu_free_shader_library(fs_library_);
    cgpu_free_swapchain(swapchain_);
    cgpu_free_surface(device_, surface_);
    cgpu_free_command_pool(cpy_cmd_pool_);
    cgpu_free_queue(gfx_queue_);
    if (cpy_queue_ != gfx_queue_) cgpu_free_queue(cpy_queue_);
    cgpu_free_device(device_);
    cgpu_free_instance(instance_);
    SDL_DestroyWindow(sdl_window_);
}

void RenderDevice::FreeSemaphore(CGpuSemaphoreId semaphore)
{
    cgpu_free_semaphore(semaphore);
}

CGpuSemaphoreId RenderDevice::allocSemaphore()
{
    return cgpu_create_semaphore(device_);
}

CGpuFenceId RenderDevice::AllocFence()
{
    return cgpu_create_fence(device_);
}

void RenderDevice::FreeFence(CGpuFenceId fence)
{
    cgpu_wait_fences(&fence, 1);
    cgpu_free_fence(fence);
}

void RenderDevice::asyncTransfer(const CGpuBufferToBufferTransfer* transfers, uint32_t transfer_count,
    CGpuSemaphoreId semaphore, CGpuFenceId fence)
{
    CGpuCommandBufferDescriptor cmd_desc = {};
    CGpuCommandBufferId cmd = cgpu_create_command_buffer(cpy_cmd_pool_, &cmd_desc);
    cpy_cmds[semaphore] = cmd;
    cgpu_cmd_begin(cmd);
    for (uint32_t i = 0; i < transfer_count; i++)
    {
        cgpu_cmd_transfer_buffer_to_buffer(cmd, &transfers[i]);
    }
    cgpu_cmd_end(cmd);
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &cmd;
    submit_desc.cmds_count = 1;
    submit_desc.signal_semaphore_count = semaphore ? 1 : 0;
    submit_desc.signal_semaphores = semaphore ? &semaphore : nullptr;
    submit_desc.signal_fence = fence;
    cgpu_submit_queue(cpy_queue_, &submit_desc);
}

void RenderContext::Initialize(RenderDevice* device)
{
    device_ = device;
    CGpuCommandPoolDescriptor pool_desc = {};
    cmd_pool_ = cgpu_create_command_pool(device->GetCGPUQueue(), &pool_desc);
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
    // file input
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
    }
    // construct
    {
        // load meshes
        meshes_.reserve(gltf_data_->meshes_count);
        for (uint32_t i = 0; i < gltf_data_->meshes_count; i++)
        {
            auto gltf_mesh = gltf_data_->meshes + i;
            loadMesh(gltf_mesh);
        }
        // load nodes
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
    load_ready_ = true;
}

static FORCEINLINE ECGpuFormat GLTFUtil_ComponentTypeToFormat(cgltf_type type, cgltf_component_type comp_type)
{
    switch (type)
    {
        case cgltf_type_scalar: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32_SFLOAT;
            }
        }
        case cgltf_type_vec2: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8G8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8G8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16G16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16G16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32G32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32G32_SFLOAT;
            }
        }
        case cgltf_type_vec3: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8G8B8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8G8B8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16G16B16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16G16B16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32G32B32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32G32B32_SFLOAT;
            }
        }
        case cgltf_type_vec4: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8G8B8A8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8G8B8A8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16G16B16A16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16G16B16A16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32G32B32A32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32G32B32A32_SFLOAT;
            }
        }
    }
    return PF_R8_SNORM;
}

static const char8_t* gGLTFAttributeTypeLUT[] = {
    "NONE",
    "POSITION",
    "NORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "JOINTS",
    "WEIGHTS"
};

void RenderScene::Upload(RenderContext* context, bool keep_gltf_data_)
{
    if (load_ready_)
    {
        // create buffers
        eastl::vector_map<cgltf_buffer_view*, uint32_t> viewVBBindingMap = {};
        cgltf_buffer_view* indices_view = nullptr;
        eastl::vector_map<uint32_t, uint32_t> bindingOffsetMap = {};
        eastl::vector_map<uint32_t, uint32_t> bindingLocationMap = {};
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
                indices_view = buf_view;
                index_buffer_ = cgpu_create_buffer(context->GetRenderDevice()->GetCGPUDevice(), &buffer_desc);
            }
            else
            {
                auto vertex_buffer_ = cgpu_create_buffer(context->GetRenderDevice()->GetCGPUDevice(), &buffer_desc);
                vertex_buffers_.emplace_back(vertex_buffer_);
                viewVBBindingMap.insert({ buf_view, vertex_buffers_.size() - 1 });
                bindingOffsetMap.insert({ vertex_buffers_.size() - 1, 0 });
                bindingLocationMap.insert({ vertex_buffers_.size() - 1, 0 });
            }
        }
        // create vertex layout
        for (uint32_t i = 0; i < gltf_data_->meshes_count; i++)
        {
            auto gltf_mesh = gltf_data_->meshes + i;
            for (uint32_t j = 0; j < gltf_mesh->primitives_count; j++)
            {
                auto gltf_prim = gltf_mesh->primitives + j;
                CGpuVertexLayout layout = {};
                layout.attribute_count = gltf_prim->attributes_count;
                for (uint32_t k = 0; k < gltf_prim->attributes_count; k++)
                {
                    const auto gltf_attrib = gltf_prim->attributes + k;
                    const char8_t* attr_name = gGLTFAttributeTypeLUT[gltf_attrib->type];
                    strcpy(layout.attributes[k].semantic_name, attr_name);
                    layout.attributes[k].rate = INPUT_RATE_VERTEX;
                    layout.attributes[k].format =
                        GLTFUtil_ComponentTypeToFormat(gltf_attrib->data->type, gltf_attrib->data->component_type);
                    auto iter = viewVBBindingMap.find(gltf_attrib->data->buffer_view);
                    if (iter != viewVBBindingMap.end())
                    {
                        const auto binding = iter->second;
                        const auto offset = bindingOffsetMap[binding];
                        layout.attributes[k].binding = binding;
                        layout.attributes[k].offset = offset;
                        bindingOffsetMap[binding] = offset + gltf_attrib->data->stride;
                        const auto location = bindingLocationMap[binding];
                        layout.attributes[k].location = location;
                        bindingLocationMap[binding] = location + 1;
                    }
                }
                context->GetRenderDevice()->AddVertexLayout(layout);
                // reset binding offsets & locations
                for (auto&& iter : bindingOffsetMap)
                    iter.second = 0;
                for (auto&& iter : bindingLocationMap)
                    iter.second = 0;
            }
        }

        gpu_memory_ready = true;
        gpu_geometry_fence = context->GetRenderDevice()->AllocFence();
        // staging buffer
        eastl::vector_map<const cgltf_buffer*, eastl::pair<uint32_t, uint32_t>> bufferRangeMap = {};
        size_t staging_size = 0;
        for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
        {
            const size_t range_start = staging_size;
            staging_size += gltf_data_->buffers[i].size;
            bufferRangeMap.insert({ gltf_data_->buffers + i, { range_start, staging_size } });
        }
        CGpuBufferDescriptor upload_buffer_desc = {};
        upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT;
        upload_buffer_desc.descriptors = RT_NONE;
        upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY;
        upload_buffer_desc.element_stride = staging_size;
        upload_buffer_desc.elemet_count = 1;
        upload_buffer_desc.size = staging_size;
        staging_buffer_ = cgpu_create_buffer(
            context->GetRenderDevice()->GetCGPUDevice(), &upload_buffer_desc);
        // upload texture
        {
            char8_t* address_cursor = (char8_t*)staging_buffer_->cpu_mapped_address;
            for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
            {
                memcpy(address_cursor, gltf_data_->buffers[i].data, gltf_data_->buffers[i].size);
                address_cursor += gltf_data_->buffers[i].size;
            }
        }
        eastl::vector<CGpuBufferToBufferTransfer> transfers(viewVBBindingMap.size());
        // transfer
        transfers[0].src = staging_buffer_;
        transfers[0].src_offset = bufferRangeMap[indices_view->buffer].first + indices_view->offset;
        transfers[0].dst = index_buffer_;
        transfers[0].dst_offset = 0;
        transfers[0].size = indices_view->size;
        for (uint32_t i = 1; i < viewVBBindingMap.size(); i++)
        {
            const cgltf_buffer_view* cgltfBufferView = viewVBBindingMap.at(i - 1).first;
            const cgltf_buffer* cgltfBuffer = cgltfBufferView->buffer;
            transfers[i].src = staging_buffer_;
            transfers[i].src_offset = bufferRangeMap[cgltfBuffer].first + cgltfBufferView->offset;
            transfers[i].dst = vertex_buffers_[i - 1];
            transfers[i].dst_offset = 0;
            transfers[i].size = cgltfBufferView->size;
        }
        gpu_geometry_semaphore = context->GetRenderDevice()->AsyncTransfer(transfers.data(), transfers.size(), gpu_geometry_fence);
    }
}

void RenderScene::Destroy()
{
    if (gltf_data_) cgltf_free(gltf_data_);
    if (gpu_geometry_fence)
    {
        cgpu_wait_fences(&gpu_geometry_fence, 1);
        cgpu_free_fence(gpu_geometry_fence);
        cgpu_free_semaphore(gpu_geometry_semaphore);
        if (staging_buffer_) cgpu_free_buffer(staging_buffer_);
    }
    if (index_buffer_) cgpu_free_buffer(index_buffer_);
    for (auto vertex_buffer_ : vertex_buffers_)
    {
        if (vertex_buffer_) cgpu_free_buffer(vertex_buffer_);
    }
}