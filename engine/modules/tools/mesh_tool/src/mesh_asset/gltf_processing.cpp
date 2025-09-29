#include "SkrProfile/profile.h"
#include "SkrGraphics/api.h"
#include "SkrCore/platform/vfs.h"
#include "SkrContainersDef/path.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrMeshCore/mesh_asset.hpp"
#include "SkrMeshTool/mesh_asset.hpp"

#include "cgltf/cgltf.h"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

namespace skr
{
inline EVertexStreamType getVertexStreamTypeFromGLTFAttribute(cgltf_attribute_type attr)
{
    switch (attr)
    {
    case cgltf_attribute_type_position:
        return EVertexStreamType::POSITION;
    case cgltf_attribute_type_normal:
        return EVertexStreamType::NORMAL;
    case cgltf_attribute_type_tangent:
        return EVertexStreamType::TANGENT;
    case cgltf_attribute_type_texcoord:
        return EVertexStreamType::TEXCOORD;
    case cgltf_attribute_type_color:
        return EVertexStreamType::COLOR;
    case cgltf_attribute_type_joints:
        return EVertexStreamType::JOINTS;
    case cgltf_attribute_type_weights:
        return EVertexStreamType::WEIGHTS;
    default:
        return EVertexStreamType::CUSTOM;
    }
}

inline static MeshEncoder EncodeGLTFMesh(const cgltf_data* gltf_data, skr::Map<cgltf_primitive*, uint32_t>& prims_map)
{
    cgltf_mesh* meshes = gltf_data->meshes;
    uint32_t mesh_count = gltf_data->meshes_count;
    MeshEncoder mesh_encoder = {};
    uint32_t primitive_index = 0;
    for (uint32_t mesh_id = 0; mesh_id < mesh_count; mesh_id++)
    {
        auto mesh = meshes + mesh_id;
        for (uint32_t pid = 0; pid < mesh->primitives_count; pid++)
        {
            const auto gltf_primitive = mesh->primitives + pid;
            auto& primitive = mesh_encoder.AddPrimitive(); 
            primitive.SetMaterialIndex(
                cgltf_material_index(gltf_data, gltf_primitive->material)
            );
            prims_map.add(gltf_primitive, primitive_index++);
            // fill indices
            {
                const auto buffer_view = gltf_primitive->indices->buffer_view;
                const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
                const auto view_data = buffer_data + buffer_view->offset;
                const auto indices_count = gltf_primitive->indices->count;
                primitive.GetIndexStream().Set(
                    (const uint8_t*)view_data + gltf_primitive->indices->offset, 
                    gltf_primitive->indices->stride, 
                    indices_count
                );
            }
            // fill vertex streams
            for (uint32_t vid = 0; vid < gltf_primitive->attributes_count; vid++)
            {
                const auto& attribute = gltf_primitive->attributes[vid];
                const auto buffer_view = attribute.data->buffer_view;
                const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
                const auto view_data = buffer_data + buffer_view->offset;
                auto type = getVertexStreamTypeFromGLTFAttribute(attribute.type);
                MeshVertexStream& vertex_stream = primitive.AddVertexStream();
                vertex_stream.Set(
                    type,
                    view_data + attribute.data->offset,
                    attribute.data->stride,
                    attribute.data->count
                );
                // vertex_stream.index = attribute.index;
            }
        }
    }
    return std::move(mesh_encoder);
}

cgltf_data* ImportGLTFData(skr::StringView assetPath, skr_io_ram_service_t* ioService, struct skr_vfs_t* vfs) SKR_NOEXCEPT
{
    // prepare callback
    skr::task::event_t counter;
    skr::BlobId blob = nullptr;
    skr::String u8Path = assetPath;
    struct CallbackData
    {
        skr::task::event_t* pCounter;
    } callbackData;
    callbackData.pCounter = &counter;
    // prepare io
    auto request = ioService->open_request();
    request->set_vfs(vfs);
    request->set_path(u8Path.u8_str());
    request->add_block({}); // read all
    request->add_callback(SKR_IO_STAGE_COMPLETED, +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cbData->pCounter->signal(); }, (void*)&callbackData);
    skr_io_future_t future = {};
    blob = ioService->request(request, &future);
    counter.wait(false);
    struct cgltf_data* gltf_data_ = nullptr;
    {
        SkrZoneScopedN("ParseGLTF");
        cgltf_options options = {};
        if (blob->get_data())
        {
            cgltf_result result = cgltf_parse(&options, blob->get_data(), blob->get_size(), &gltf_data_);
            if (result != cgltf_result_success)
            {
                gltf_data_ = nullptr;
            }
            else
            {
                SkrZoneScopedN("LoadGLTFBuffer");
                const skr::Path fullPath = skr::Path{ vfs->mount_dir } / u8Path.u8_str();
                result = cgltf_load_buffers(&options, gltf_data_, fullPath.string().c_str_raw());
                result = cgltf_validate(gltf_data_);
                if (result != cgltf_result_success)
                {
                    gltf_data_ = nullptr;
                }
            }
        }
        blob.reset();
    }
    return gltf_data_;
}

void GetGLTFNodeTransform(const cgltf_node* node, float3& translation, float3& scale, skr::float4& rotation)
{
    if (node->has_translation)
    {
        translation = { node->translation[0], node->translation[1], node->translation[2] };
    }
    else
    {
        translation = { 0.f, 0.f, 0.f };
    }
    
    if (node->has_scale)
    {
        scale = { node->scale[0], node->scale[1], node->scale[2] };
    }
    else
    {
        scale = { 1.f, 1.f, 1.f};
    }

    if (node->has_rotation)
    {
        rotation = { node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] };
        if ((rotation.x == 0) && (rotation.y == 0) && (rotation.z == 0) && (rotation.w == 0))
            rotation.w = 1.f;
    }
    else
    {
        rotation = { 0.f, 0.f, 0.f, 1.f };
    }
}

void CookGLTFMeshData(const cgltf_data* gltf_data, MeshAsset* cfg, MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins)
{
    GUID shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero())
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }

    // FIXME: select mesh to cook
    out_resource.name = (const char8_t*)gltf_data->meshes[0].name;
    if (out_resource.name.is_empty()) out_resource.name = u8"gltfMesh";

    // 1. extract raw meshes
    skr::Map<cgltf_primitive*, uint32_t> prims_map;
    MeshEncoder raw_mesh = EncodeGLTFMesh(gltf_data, prims_map);

    // 2. extract indices and vertices
    skr::Map<uint32_t, skr::Vector<EVertexAttribute>> layouts = {
        { 0, kRawAttributes }
    };
    EncodedMesh encoded_mesh = raw_mesh.Encode(layouts);
    
    // 3. record primitvies
    out_resource.primitives = encoded_mesh.primitives;
    for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
    {
        const auto node_ = gltf_data->nodes + i;
        auto& mesh_section = out_resource.sections.add_default().ref();
        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
        GetGLTFNodeTransform(node_, mesh_section.translation, mesh_section.scale, mesh_section.rotation);
        if (node_->mesh != nullptr)
        {
            for (uint32_t j = 0; j < node_->mesh->primitives_count; j++)
            {
                const auto& gltf_prim = node_->mesh->primitives[j];
                mesh_section.primitive_indices.add(prims_map.find(&gltf_prim).value());
            }
        }
    }
    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.add_default().ref();
        out_buffer0.index = 0;
        out_buffer0.byte_length = encoded_mesh.buffers[0].size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
    }
    // output one buffer contains vertices & indices
    out_bins.add(encoded_mesh.buffers[0]);
}

void CookGLTFMeshData_SplitSkin(const cgltf_data* gltf_data, MeshAsset* cfg, MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins)
{
    GUID shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero())
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }

    // FIXME: select mesh to cook
    out_resource.name = (const char8_t*)gltf_data->meshes[0].name;
    if (out_resource.name.is_empty()) out_resource.name = u8"gltfMesh";

    // 1. extract raw meshes
    skr::Map<cgltf_primitive*, uint32_t> prims_map;
    MeshEncoder raw_mesh = EncodeGLTFMesh(gltf_data, prims_map);

    // 2. extract indices and vertices
    skr::Map<uint32_t, skr::Vector<EVertexAttribute>> layouts;
    layouts.add(0, kRawStaticAttributes);
    layouts.add(1, kRawSkinAttributes);
    EncodedMesh encoded_mesh = raw_mesh.Encode(layouts);
    
    // 3. record primitvies
    out_resource.primitives = encoded_mesh.primitives;
    for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
    {
        const auto node_ = gltf_data->nodes + i;
        auto& mesh_section = out_resource.sections.add_default().ref();
        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
        GetGLTFNodeTransform(node_, mesh_section.translation, mesh_section.scale, mesh_section.rotation);
        if (node_->mesh != nullptr)
        {
            for (uint32_t j = 0; j < node_->mesh->primitives_count; j++)
            {
                const auto& gltf_prim = node_->mesh->primitives[j];
                mesh_section.primitive_indices.add(prims_map.find(&gltf_prim).value());
            }
        }
    }
    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.add_default().ref();
        out_buffer0.index = 0;
        out_buffer0.byte_length = encoded_mesh.buffers[0].size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
        auto& out_buffer1 = out_resource.bins.add_default().ref();
        out_buffer1.index = 1;
        out_buffer1.byte_length = encoded_mesh.buffers[1].size();
        out_buffer1.used_with_index = false;
        out_buffer1.used_with_vertex = true;
    }
    // output one buffer contains vertices & indices
    out_bins.add(encoded_mesh.buffers[0]);
    out_bins.add(encoded_mesh.buffers[1]);
}

} // namespace skr