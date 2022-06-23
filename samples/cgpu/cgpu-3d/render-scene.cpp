#define CGLTF_IMPLEMENTATION
#include "render-scene.hpp"
#include "thirdparty/cgltf.h"
#include "thirdparty/lodepng.h"

static FORCEINLINE ECGPUFormat GLTFUtil_ComponentTypeToFormat(cgltf_type type, cgltf_component_type comp_type)
{
    switch (type)
    {
        case cgltf_type_scalar: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return CGPU_FORMAT_R8_SNORM;
                case cgltf_component_type_r_8u:
                    return CGPU_FORMAT_R8_UNORM;
                case cgltf_component_type_r_16:
                    return CGPU_FORMAT_R16_SINT;
                case cgltf_component_type_r_16u:
                    return CGPU_FORMAT_R16_UINT;
                case cgltf_component_type_r_32u:
                    return CGPU_FORMAT_R32_UINT;
                case cgltf_component_type_r_32f:
                    return CGPU_FORMAT_R32_SFLOAT;
                default:
                    return CGPU_FORMAT_R8_SNORM;
            }
        }
        case cgltf_type_vec2: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return CGPU_FORMAT_R8G8_SNORM;
                case cgltf_component_type_r_8u:
                    return CGPU_FORMAT_R8G8_UNORM;
                case cgltf_component_type_r_16:
                    return CGPU_FORMAT_R16G16_SINT;
                case cgltf_component_type_r_16u:
                    return CGPU_FORMAT_R16G16_UINT;
                case cgltf_component_type_r_32u:
                    return CGPU_FORMAT_R32G32_UINT;
                case cgltf_component_type_r_32f:
                    return CGPU_FORMAT_R32G32_SFLOAT;
                default:
                    return CGPU_FORMAT_R8_SNORM;
            }
        }
        case cgltf_type_vec3: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return CGPU_FORMAT_R8G8B8_SNORM;
                case cgltf_component_type_r_8u:
                    return CGPU_FORMAT_R8G8B8_UNORM;
                case cgltf_component_type_r_16:
                    return CGPU_FORMAT_R16G16B16_SINT;
                case cgltf_component_type_r_16u:
                    return CGPU_FORMAT_R16G16B16_UINT;
                case cgltf_component_type_r_32u:
                    return CGPU_FORMAT_R32G32B32_UINT;
                case cgltf_component_type_r_32f:
                    return CGPU_FORMAT_R32G32B32_SFLOAT;
                default:
                    return CGPU_FORMAT_R8_SNORM;
            }
        }
        case cgltf_type_vec4: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return CGPU_FORMAT_R8G8B8A8_SNORM;
                case cgltf_component_type_r_8u:
                    return CGPU_FORMAT_R8G8B8A8_UNORM;
                case cgltf_component_type_r_16:
                    return CGPU_FORMAT_R16G16B16A16_SINT;
                case cgltf_component_type_r_16u:
                    return CGPU_FORMAT_R16G16B16A16_UINT;
                case cgltf_component_type_r_32u:
                    return CGPU_FORMAT_R32G32B32A32_UINT;
                case cgltf_component_type_r_32f:
                    return CGPU_FORMAT_R32G32B32A32_SFLOAT;
                default:
                    return CGPU_FORMAT_R8_SNORM;
            }
        }
        default:
            return CGPU_FORMAT_R8_SNORM;
    }
    return CGPU_FORMAT_R8_SNORM;
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

int32_t RenderMesh::loadPrimitive(struct cgltf_primitive* src, uint32_t& index_cursor)
{
    RenderPrimitive newPrim = {};
    newPrim.index_offset_ = (uint32_t)src->indices->offset;
    newPrim.index_count_ = (uint32_t)src->indices->count;
    newPrim.first_index_ = index_cursor;
    newPrim.vertex_strides_.reserve(src->attributes_count);
    newPrim.vertex_offsets_.reserve(src->attributes_count);
    newPrim.vertex_buffers_.reserve(src->attributes_count);
    if (src->material)
    {
        auto iter = scene_->materials_.find(src->material->name);
        if (iter != scene_->materials_.end())
        {
            newPrim.material_id_ = iter->second.id_in_scene_;
        }
    }
    for (uint32_t i = 0; i < src->attributes_count; i++)
    {
        auto attrib = src->attributes + i;
        newPrim.vertex_strides_.emplace_back((uint32_t)attrib->data->stride);
        newPrim.vertex_offsets_.emplace_back((uint32_t)attrib->data->offset);
    }
    // Create vertex layout
    CGPUVertexLayout layout = {};
    uint32_t binding = 0;
    layout.attribute_count = (uint32_t)src->attributes_count;
    for (uint32_t i = 0; i < src->attributes_count; i++)
    {
        const auto gltf_attrib = src->attributes + i;
        const char8_t* attr_name = gGLTFAttributeTypeLUT[gltf_attrib->type];
        strcpy(layout.attributes[i].semantic_name, attr_name);
        layout.attributes[i].rate = CGPU_INPUT_RATE_VERTEX;
        layout.attributes[i].array_size = 1;
        layout.attributes[i].format = GLTFUtil_ComponentTypeToFormat(gltf_attrib->data->type, gltf_attrib->data->component_type);
        layout.attributes[i].binding = binding++;
        layout.attributes[i].offset = 0;
        layout.attributes[i].elem_stride =
        FormatUtil_BitSizeOfBlock(layout.attributes[i].format) / 8;
    }
    newPrim.vertex_layout_id_ = (uint32_t)RenderBlackboard::AddVertexLayout(layout);
    primitives_.emplace_back(eastl::move(newPrim));
    index_cursor += newPrim.index_count_;
    return (int32_t)primitives_.size() - 1;
}

int32_t RenderScene::loadMaterial(struct cgltf_material* src)
{
    RenderMaterial newMaterial = {};
    if (src->pbr_metallic_roughness.base_color_texture.texture)
    {
        newMaterial.base_color_uri_ = src->pbr_metallic_roughness.base_color_texture.texture->image->uri;
        newMaterial.id_in_scene_ = (uint32_t)materials_.size();
    }
    materials_.emplace_back(src->name, newMaterial);
    return (int32_t)materials_.size() - 1;
}

int32_t RenderScene::loadMesh(struct cgltf_mesh* src)
{
    RenderMesh newMesh = {};
    newMesh.name_ = src->name;
    newMesh.scene_ = this;
    uint32_t index_cursor = 0;
    newMesh.primitives_.reserve(src->primitives_count);
    for (uint32_t i = 0; i < src->primitives_count; i++)
    {
        auto gltf_prim = src->primitives + i;
        newMesh.loadPrimitive(gltf_prim, index_cursor);
    }
    meshes_.emplace_back(newMesh);
    return (int32_t)meshes_.size() - 1;
}

int32_t RenderScene::loadNode(struct cgltf_node* src, int32_t parent_idx)
{
    const bool isRoot = (parent_idx == -1);
    nodes_.emplace_back();
    RenderNode& newNode = nodes_[nodes_.size() - 1];
    newNode.index_ = (int32_t)nodes_.size() - 1;
    newNode.parent_ = isRoot ? nullptr : &nodes_[parent_idx];
    newNode.children_.reserve(src->children_count);
    for (uint32_t i = 0; i < src->children_count; i++)
    {
        int32_t child_idx = loadNode(src->children[i], newNode.index_);
        newNode.children_.emplace_back(&nodes_[child_idx]);
    }
    newNode.translation_ = skr::math::Vector3f(
    src->translation[0], src->translation[1], src->translation[2]);
    newNode.rotation_ = skr::math::Quaternion(
    src->rotation[0], src->rotation[1], src->rotation[2], src->rotation[3]);
    newNode.scale_ = skr::math::Vector3f(
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
        // load materials
        materials_.reserve(gltf_data_->materials_count);
        for (uint32_t i = 0; i < gltf_data_->materials_count; i++)
        {
            auto gltf_mat = gltf_data_->materials + i;
            loadMaterial(gltf_mat);
        }
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
}

void RenderScene::AsyncCreateRenderPipelines(class RenderDevice* device, struct RenderAuxThread* aux_thread)
{
    render_device_ = device;
    for (uint32_t i = 0; i < meshes_.size(); i++)
    {
        auto& mesh = meshes_[i];
        for (uint32_t j = 0; j < mesh.primitives_.size(); j++)
        {
            auto& prim = mesh.primitives_[j];
            PipelineKey pplKey = {};
            pplKey.vertex_layout_id_ = prim.vertex_layout_id_;
            pplKey.root_sig_ = device->GetCGPUSignature();
            pplKey.screen_format_ = device->GetScreenFormat();
            pplKey.wireframe_mode_ = false;
            pplKey.sample_count_ = RenderWindow::SampleCount;
            prim.async_ppl_ = RenderBlackboard::AddRenderPipeline(aux_thread, pplKey);
            // create descriotor sets
            prim.desc_set_ = device->CreateDescriptorSet(pplKey.root_sig_, 0);
        }
    }
}

void RenderScene::AsyncCreateGeometryMemory(class RenderDevice* device, struct RenderAuxThread* aux_thread)
{
    render_device_ = device;
    gpu_geometry_fence = device->AllocFence();
    // create buffers
    for (uint32_t i = 0; i < gltf_data_->buffer_views_count; i++)
    {
        cgltf_buffer_view* buf_view = gltf_data_->buffer_views + i;
        if (buf_view->type != cgltf_buffer_view_type_indices)
        {
            vertex_buffer_count_ += 1;
        }
    }
    vertex_buffers_ = new AsyncRenderBuffer[vertex_buffer_count_];
    auto setter_callback = [=]() {
        bufs_creation_counter_ += 1;
        if (bufs_creation_counter_ >= vertex_buffer_count_ + 1 /*idx buffer*/ + 1 /*staging buffer*/)
        {
            for (uint32_t i = 0; i < gltf_data_->meshes_count; i++)
            {
                auto gltf_mesh = gltf_data_->meshes + i;
                for (uint32_t j = 0; j < gltf_mesh->primitives_count; j++)
                {
                    auto gltf_prim = gltf_mesh->primitives + j;
                    for (uint32_t k = 0; k < gltf_prim->attributes_count; k++)
                    {
                        const auto gltf_attrib = gltf_prim->attributes + k;
                        auto&& asyncVB = vertex_buffers_[viewVBIdxMap[gltf_attrib->data->buffer_view]];
                        meshes_[i].primitives_[j].vertex_buffers_.emplace_back(asyncVB.buffer_);
                    }
                }
            }
            bufs_creation_ready_ = true;
        }
    };
    // create & prepare staging buffer
    size_t staging_size = 0;
    for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
    {
        staging_size += gltf_data_->buffers[i].size;
    }
    CGPUBufferDescriptor staging_buffer_desc = {};
    staging_buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT;
    staging_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    staging_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    staging_buffer_desc.element_stride = staging_size;
    staging_buffer_desc.elemet_count = 1;
    staging_buffer_desc.size = staging_size;
    staging_buffer_.Initialize(aux_thread, staging_buffer_desc, setter_callback);
    // create vertex & index buffers
    for (uint32_t i = 0, vb_idx = 0; i < gltf_data_->buffer_views_count; i++)
    {
        cgltf_buffer_view* buf_view = gltf_data_->buffer_views + i;
        CGPUBufferDescriptor buffer_desc = {};
        buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
        buffer_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        buffer_desc.descriptors =
        buf_view->type == cgltf_buffer_view_type_indices ?
        CGPU_RESOURCE_TYPE_INDEX_BUFFER :
        CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
        buffer_desc.element_stride = buf_view->stride ? buf_view->stride : buf_view->size;
        buffer_desc.elemet_count = buf_view->size / buffer_desc.element_stride;
        buffer_desc.size = buf_view->size;
        buffer_desc.name = buf_view->name;
        if (buf_view->type == cgltf_buffer_view_type_indices)
        {
            index_buffer_.Initialize(aux_thread, buffer_desc, setter_callback);
            index_stride_ = gltf_data_->accessors->component_type;
        }
        else
        {
            viewVBIdxMap[buf_view] = vb_idx;
            vertex_buffers_[vb_idx].Initialize(aux_thread, buffer_desc, setter_callback);
            vb_idx += 1;
        }
    }
}

void RenderScene::AsyncCreateTextureMemory(class RenderDevice* device, struct RenderAuxThread* aux_thread)
{
    render_device_ = device;
    for (uint32_t i = 0; i < materials_.size(); i++)
    {
        auto&& material = materials_.at(i).second;
        auto uri = material.base_color_uri_.c_str();
        auto path = eastl::string("./../resources/").append(uri);
        auto tex = RenderBlackboard::AddTexture(uri, path.c_str(), aux_thread, CGPU_FORMAT_R8G8B8A8_UNORM);
        if (tex != nullptr && !tex->Ready())
        {
            if (tex_transfers_.find(tex) == tex_transfers_.end())
            {
                tex_transfers_[tex] = device->AllocFence();
            }
        }
    }
}

void RenderScene::TryAsyncUploadBuffers(RenderContext* context, struct AsyncTransferThread* aux_thread)
{
    if (bufs_creation_ready_ && !bufs_upload_started_)
    {
        // create buffers
        cgltf_buffer_view* indices_view = nullptr;
        for (uint32_t i = 0; i < gltf_data_->buffer_views_count; i++)
        {
            cgltf_buffer_view* buf_view = gltf_data_->buffer_views + i;
            if (buf_view->type == cgltf_buffer_view_type_indices)
            {
                indices_view = buf_view;
                break;
            }
        }
        // upload staging buffers
        eastl::vector_map<const cgltf_buffer*, eastl::pair<uint32_t, uint32_t>> bufferRangeMap = {};
        size_t staging_size = 0;
        for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
        {
            const size_t range_start = staging_size;
            staging_size += gltf_data_->buffers[i].size;
            bufferRangeMap[gltf_data_->buffers + i] = { (uint32_t)range_start, (uint32_t)staging_size };
        }
        {
            char8_t* address_cursor = (char8_t*)staging_buffer_.buffer_->cpu_mapped_address;
            for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
            {
                memcpy(address_cursor, gltf_data_->buffers[i].data, gltf_data_->buffers[i].size);
                address_cursor += gltf_data_->buffers[i].size;
            }
        }
        eastl::vector<ECGPUResourceState> dst_states(viewVBIdxMap.size() + 1);
        eastl::vector<AsyncBufferToBufferTransfer> transfers(viewVBIdxMap.size() + 1);
        // transfer
        transfers[0].src = &staging_buffer_;
        transfers[0].src_offset = bufferRangeMap[indices_view->buffer].first + indices_view->offset;
        transfers[0].dst = &index_buffer_;
        transfers[0].dst_offset = 0;
        transfers[0].size = indices_view->size;
        dst_states[0] = CGPU_RESOURCE_STATE_INDEX_BUFFER;
        for (uint32_t i = 1; i < viewVBIdxMap.size() + 1; i++)
        {
            const cgltf_buffer_view* cgltfBufferView = viewVBIdxMap.at(i - 1).first;
            const cgltf_buffer* cgltfBuffer = cgltfBufferView->buffer;
            transfers[i].src = &staging_buffer_;
            transfers[i].src_offset = bufferRangeMap[cgltfBuffer].first + cgltfBufferView->offset;
            transfers[i].dst = &vertex_buffers_[i - 1];
            transfers[i].dst_offset = 0;
            transfers[i].size = cgltfBufferView->size;
        }
        aux_thread->AsyncTransfer(
        transfers.data(), (uint32_t)transfers.size(), gpu_geometry_fence);
        bufs_upload_started_ = true;
    }
}

void RenderScene::TryAsyncUploadTextures(RenderContext* context, struct AsyncTransferThread* aux_thread)
{
    eastl::vector<AsyncRenderTexture*> textures_to_acquire;
    for (uint32_t i = 0; i < materials_.size(); i++)
    {
        auto&& material = materials_.at(i).second;
        auto uri = material.base_color_uri_.c_str();
        if (auto target = RenderBlackboard::GetTexture(uri); target != nullptr)
        {
            if (target->Ready() && !target->upload_started_)
            {
                AsyncBufferToTextureTransfer b2t = {};
                b2t.raw_src = target->upload_buffer_;
                b2t.src_offset = 0;
                b2t.dst = target;
                b2t.elems_per_row = target->texture_->width;
                b2t.rows_per_image = target->texture_->height;
                b2t.base_array_layer = 0;
                b2t.layer_count = 1;
                aux_thread->AsyncTransfer(&b2t, 1, tex_transfers_[target]);
            }
            if (target->Ready() && !target->queue_released_ &&
                tex_transfers_[target] &&
                (cgpu_query_fence_status(tex_transfers_[target]) == CGPU_FENCE_STATUS_COMPLETE))
            {
                // TODO: impl this as callback in AsyncTransferThread::asyncTransfer
                target->queue_released_ = true;
                textures_to_acquire.emplace_back(target);
                tex_transfers_[target] = nullptr;
            }
        }
    }
    context->AcquireResources(textures_to_acquire.data(), (uint32_t)textures_to_acquire.size(), nullptr, 0);
    for (uint32_t i = 0; i < gltf_data_->meshes_count; i++)
    {
        auto gltf_mesh = gltf_data_->meshes + i;
        for (uint32_t j = 0; j < gltf_mesh->primitives_count; j++)
        {
            auto&& material = materials_.at(meshes_[i].primitives_[j].material_id_);
            auto rdrTex = RenderBlackboard::GetTexture(material.second.base_color_uri_.c_str());
            if (rdrTex != nullptr && rdrTex->Ready() && !tex_transfers_[rdrTex])
            {
                if (!meshes_[i].primitives_[j].desc_set_updated_)
                {
                    CGPUDescriptorData arguments[1];
                    arguments[0].name = "sampled_texture";
                    arguments[0].count = 1;
                    arguments[0].textures = &rdrTex->view_;
                    cgpu_update_descriptor_set(meshes_[i].primitives_[j].desc_set_, arguments, 1);
                    meshes_[i].primitives_[j].desc_set_updated_ = true;
                    meshes_[i].primitives_[j].texture_ = rdrTex;
                }
            }
        }
    }
}

bool RenderScene::AsyncGeometryUploadReady()
{
    if (gpu_geometry_fence)
    {
        auto status = cgpu_query_fence_status(gpu_geometry_fence);
        if (status == CGPU_FENCE_STATUS_NOTSUBMITTED)
            return false;
        return status == CGPU_FENCE_STATUS_COMPLETE;
    }
    return false;
}

void RenderScene::Destroy(struct RenderAuxThread* aux_thread)
{
    if (gltf_data_) cgltf_free(gltf_data_);
    if (gpu_geometry_fence)
    {
        for (auto& mesh : meshes_)
        {
            for (auto& prim : mesh.primitives_)
            {
                render_device_->FreeDescriptorSet(prim.desc_set_);
            }
        }
    }
    staging_buffer_.Destroy(aux_thread);
    index_buffer_.Destroy(aux_thread);
    for (uint32_t i = 0; i < vertex_buffer_count_; i++)
    {
        vertex_buffers_[i].Destroy(aux_thread);
    }
}
