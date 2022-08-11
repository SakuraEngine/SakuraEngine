#include "skr_renderer/mesh_resource.h"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "cgpu/cgpux.hpp"
#include "utils/io.hpp"
#include "utils/make_zeroed.hpp"
#include "ftl/atomic_counter.h"
#include "cgltf/cgltf.h"
#include "platform/thread.h"
#include <EASTL/vector_map.h>
#include <EASTL/unordered_map.h>
#include <EASTL/hash_set.h>
#include "ghc/filesystem.hpp"

#include "tracy/Tracy.hpp"

static const char* cGLTFAttributeTypeLUT[8] = {
    "NONE",
    "POSITION",
    "NORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "JOINTS",
    "WEIGHTS"
};

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

static struct SkrMeshResourceUtil
{
    template <typename T>
    using cached_hashset = eastl::hash_set<T, eastl::hash<T>, eastl::equal_to<T>, EASTLAllocatorType, true>;

    SkrMeshResourceUtil()
    {
        skr_init_mutex_recursive(&vertex_layouts_mutex_);
    }
    ~SkrMeshResourceUtil()
    {
        skr_destroy_mutex(&vertex_layouts_mutex_);
    }

    static FORCEINLINE skr_vertex_layout_id AddVertexLayoutFromGLTFPrimitive(const cgltf_primitive* primitive)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto layout = make_zeroed<CGPUVertexLayout>();
        layout.attribute_count = (uint32_t)primitive->attributes_count;
        for (uint32_t i = 0; i < primitive->attributes_count; i++)
        {
            const auto gltf_attrib = primitive->attributes + i;
            const char* attr_name = cGLTFAttributeTypeLUT[gltf_attrib->type];
            strcpy(layout.attributes[i].semantic_name, attr_name);
            layout.attributes[i].rate = CGPU_INPUT_RATE_VERTEX;
            layout.attributes[i].array_size = 1;
            layout.attributes[i].format = GLTFUtil_ComponentTypeToFormat(gltf_attrib->data->type, gltf_attrib->data->component_type);
            layout.attributes[i].binding = i;
            layout.attributes[i].offset = 0;
            layout.attributes[i].elem_stride = FormatUtil_BitSizeOfBlock(layout.attributes[i].format) / 8;
        }
        const auto hash = vertex_layouts_.get_hash_code(layout);
        if (vertex_layouts_.find_by_hash(hash) == vertex_layouts_.end())
        {
            vertex_layouts_.insert(layout);
        }
        return hash;
    }

    static FORCEINLINE bool GetVertexLayout(skr_vertex_layout_id id, CGPUVertexLayout* layout)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto iter = vertex_layouts_.find_by_hash(id);
        if (iter == vertex_layouts_.end()) return false;
        *layout = *iter;
        return true;
    }

    static cached_hashset<CGPUVertexLayout> vertex_layouts_;
    static SMutex vertex_layouts_mutex_;
} mesh_resource_util;
SkrMeshResourceUtil::cached_hashset<CGPUVertexLayout> SkrMeshResourceUtil::vertex_layouts_;
SMutex SkrMeshResourceUtil::vertex_layouts_mutex_;

#ifndef SKR_SERIALIZE_GURAD
void skr_mesh_resource_create_from_gltf(skr_io_ram_service_t* ioService, const char* path, skr_gltf_ram_io_request_t* gltfRequest)
{
    ZoneScopedN("ioRAM Mesh Request");

    SKR_ASSERT(gltfRequest->vfs_override && "Support only vfs override");

    struct CallbackData
    {
        skr_gltf_ram_io_request_t* gltfRequest;   
        eastl::string u8Path;
    };
    auto callbackData = SkrNew<CallbackData>();
    skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
    ramIO.bytes = nullptr;
    ramIO.offset = 0;
    ramIO.size = 0;
    ramIO.path = path;
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cgltf_options options = {};
        struct cgltf_data* gltf_data_ = nullptr;
        if (request->bytes)
        {
            cgltf_result result = cgltf_parse(&options, request->bytes, request->size, &gltf_data_);
            if (result != cgltf_result_success)
            {
                gltf_data_ = nullptr;
            }
            else
            {
                if (cbData->gltfRequest->load_bin_to_memory)
                    result = cgltf_load_buffers(&options, gltf_data_, cbData->u8Path.c_str());
                result = cgltf_validate(gltf_data_);
                if (result != cgltf_result_success)
                {
                    gltf_data_ = nullptr;
                }
                else
                {
                    skr_mesh_resource_id resource = SkrNew<skr_mesh_resource_t>();
                    resource->gltf_data = gltf_data_;
                    // record buffer bins
                    resource->bins.resize(gltf_data_->buffers_count);
                    for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
                    {
                        resource->bins[i].bin.bytes = (uint8_t*)gltf_data_->buffers[i].data;
                        resource->bins[i].bin.size = gltf_data_->buffers[i].size;
                        resource->bins[i].uri = gltf_data_->buffers[i].uri;
                        resource->bins[i].used_with_index = false;
                        resource->bins[i].used_with_vertex = false;
                    }
                    // record primitvies
                    for (uint32_t i = 0; i < gltf_data_->nodes_count; i++)
                    {
                        const auto node_ = gltf_data_->nodes + i;
                        auto& mesh_section = resource->sections.emplace_back();
                        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data_->nodes) : -1;
                        if (node_->has_translation)
                            mesh_section.translation = { node_->translation[0], node_->translation[1], node_->translation[2] };
                        if (node_->has_scale)
                            mesh_section.scale = { node_->scale[0], node_->scale[1], node_->scale[2] };
                        if (node_->has_rotation)
                            mesh_section.rotation = { node_->rotation[0], node_->rotation[1], node_->rotation[2], node_->rotation[3] };
                        if (node_->mesh != nullptr)
                        {
                            // per primitive
                            for (uint32_t j = 0, index_cursor = 0; j < node_->mesh->primitives_count; j++)
                            {
                                auto& prim = resource->primitives.emplace_back();
                                const auto primitive_ = node_->mesh->primitives + j;
                                // ib
                                prim.index_buffer.buffer_index = (uint32_t)(primitive_->indices->buffer_view->buffer - gltf_data_->buffers);
                                prim.index_buffer.index_offset = (uint32_t)(primitive_->indices->offset + primitive_->indices->buffer_view->offset);
                                prim.index_buffer.first_index = index_cursor;
                                prim.index_buffer.index_count = (uint32_t)primitive_->indices->count;
                                prim.index_buffer.stride = (uint32_t)primitive_->indices->stride;
                                // vbs
                                prim.vertex_buffers.resize(primitive_->attributes_count);
                                for (uint32_t k = 0; k < primitive_->attributes_count; k++)
                                {
                                    const auto buf_view = primitive_->attributes[k].data->buffer_view;
                                    prim.vertex_buffers[k].buffer_index = (uint32_t)(buf_view->buffer - gltf_data_->buffers);
                                    prim.vertex_buffers[k].stride = (uint32_t)primitive_->attributes[k].data->stride;
                                    prim.vertex_buffers[k].offset = (uint32_t)(primitive_->attributes[k].data->offset + buf_view->offset);
                                }
                                // TODO: Material
                                prim.material_inst = make_zeroed<skr_guid_t>();
                                prim.vertex_layout_id = mesh_resource_util.AddVertexLayoutFromGLTFPrimitive(primitive_);

                                mesh_section.primive_indices.emplace_back(resource->primitives.size() - 1);
                            }
                        }
                    }
                    cbData->gltfRequest->mesh_resource = resource;
                }
            }
        }
        sakura_free(request->bytes);
        request->bytes = nullptr;
        request->size = 0;
        skr_atomic32_store_relaxed(&cbData->gltfRequest->gltf_status, SKR_ASYNC_IO_STATUS_OK);
        cbData->gltfRequest->callback(cbData->gltfRequest, cbData->gltfRequest->callback_data);
        SkrDelete(cbData);
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)callbackData;
    // TODO: replace this with newer VFS API
    std::string gltfPath;
    {
        ZoneScopedN("ioRAM Mesh Path Calc");
        gltfPath = (ghc::filesystem::path(gltfRequest->vfs_override->mount_dir) / path).u8string();
        callbackData->u8Path = gltfPath.c_str();
    }
    callbackData->gltfRequest = gltfRequest;
    ioService->request(gltfRequest->vfs_override, &ramIO, &gltfRequest->ioRequest);
}
#endif

void skr_mesh_resource_free(skr_mesh_resource_id mesh_resource)
{
    if (mesh_resource->gltf_data)
    {
        cgltf_free((cgltf_data*)mesh_resource->gltf_data);
    }
    SkrDelete(mesh_resource);
}