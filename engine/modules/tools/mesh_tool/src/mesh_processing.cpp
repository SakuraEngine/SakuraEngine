#include "SkrProfile/profile.h"
#include "SkrGraphics/api.h"
#include "SkrCore/platform/vfs.h"
#include "SkrContainersDef/path.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrMeshTool/mesh_processing.hpp"

#include "cgltf/cgltf.h"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

namespace skd::asset
{
inline ERawVertexStreamType getVertexStreamTypeFromGLTFAttribute(cgltf_attribute_type attr)
{
    switch (attr)
    {
    case cgltf_attribute_type_position:
        return ERawVertexStreamType::POSITION;
    case cgltf_attribute_type_normal:
        return ERawVertexStreamType::NORMAL;
    case cgltf_attribute_type_tangent:
        return ERawVertexStreamType::TANGENT;
    case cgltf_attribute_type_texcoord:
        return ERawVertexStreamType::TEXCOORD;
    case cgltf_attribute_type_color:
        return ERawVertexStreamType::COLOR;
    case cgltf_attribute_type_joints:
        return ERawVertexStreamType::JOINTS;
    case cgltf_attribute_type_weights:
        return ERawVertexStreamType::WEIGHTS;
    default:
        return ERawVertexStreamType::CUSTOM;
    }
}

inline static SRawMesh GenerateRawMeshForGLTFMesh(cgltf_mesh* mesh)
{
    SRawMesh raw_mesh = {};
    raw_mesh.primitives.reserve(mesh->primitives_count);
    for (uint32_t pid = 0; pid < mesh->primitives_count; pid++)
    {
        const auto gltf_primitive = mesh->primitives + pid;
        SRawPrimitive& primitive = raw_mesh.primitives.add_default().ref();
        // fill indices
        {
            const auto buffer_view = gltf_primitive->indices->buffer_view;
            const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
            const auto view_data = buffer_data + buffer_view->offset;
            const auto indices_count = gltf_primitive->indices->count;
            primitive.index_stream.buffer_view = skr::Span<const uint8_t>(view_data + gltf_primitive->indices->offset, gltf_primitive->indices->stride * indices_count);
            primitive.index_stream.offset = 0;
            primitive.index_stream.count = indices_count;
            primitive.index_stream.stride = gltf_primitive->indices->stride;
        }
        // fill vertex streams
        primitive.vertex_streams.reserve(gltf_primitive->attributes_count);
        for (uint32_t vid = 0; vid < gltf_primitive->attributes_count; vid++)
        {
            const auto& attribute = gltf_primitive->attributes[vid];
            const auto buffer_view = attribute.data->buffer_view;
            const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
            const auto view_data = buffer_data + buffer_view->offset;
            const auto vertex_count = attribute.data->count;
            SRawVertexStream& vertex_stream = primitive.vertex_streams.add_default().ref();
            vertex_stream.buffer_view = skr::Span<const uint8_t>(view_data + attribute.data->offset, attribute.data->stride * vertex_count);
            vertex_stream.offset = 0;
            vertex_stream.count = vertex_count;
            vertex_stream.stride = attribute.data->stride;
            vertex_stream.type = getVertexStreamTypeFromGLTFAttribute(attribute.type);
            vertex_stream.index = attribute.index;
        }
    }
    return raw_mesh;
}

cgltf_data* ImportGLTFWithData(skr::StringView assetPath, skr_io_ram_service_t* ioService, struct skr_vfs_t* vfs) SKR_NOEXCEPT
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
    skr::Vector<uint8_t> buffer0 = {};
    GUID shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero())
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }

    // FIXME: select mesh to cook
    out_resource.name = gltf_data->meshes[0].name ? (const char8_t*)gltf_data->meshes[0].name : u8"";
    if (out_resource.name.is_empty()) out_resource.name = u8"gltfMesh";
    
    // 1. extract raw meshes
    skr::Map<uint32_t, SRawMesh> raw_meshes;
    {
        for (uint32_t i = 0; i < gltf_data->meshes_count; i++)
            raw_meshes.try_add_default(i);
        for (uint32_t i = 0; i < gltf_data->meshes_count; i++)
            raw_meshes.add(i, std::move(GenerateRawMeshForGLTFMesh(&gltf_data->meshes[i])));
    }
    // 2. extract indices and vertices
    skr::Vector<skr::Vector<MeshPrimitive>> primitive_arrays;
    primitive_arrays.resize_default(gltf_data->nodes_count);
    {
        // 2.1 indices
        for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
        {
            if (!gltf_data->nodes[i].mesh) continue;
            auto mesh_index = cgltf_mesh_index(gltf_data, gltf_data->nodes[i].mesh);
            auto& raw_mesh = raw_meshes.find(mesh_index).value();
            auto& new_primitives = primitive_arrays[i]; 
            EmplaceAllRawMeshIndices(&raw_mesh, buffer0, new_primitives);
        }
        // 2.2 vertices
        for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
        {
            if (!gltf_data->nodes[i].mesh) continue;
            auto mesh_index = cgltf_mesh_index(gltf_data, gltf_data->nodes[i].mesh);
            auto& raw_mesh = raw_meshes.find(mesh_index).value();
            auto& new_primitives = primitive_arrays[i]; 
            EmplaceAllRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, new_primitives);
        }
    }
    // 3. record primitvies
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
                auto& prim = primitive_arrays[i][j];
                prim.vertex_layout = shuffle_layout_id;
                prim.material_index = static_cast<uint32_t>(gltf_prim.material - gltf_data->materials);
                mesh_section.primitive_indices.add(out_resource.primitives.size() + j);
            }
            out_resource.primitives.reserve(out_resource.primitives.size() + primitive_arrays[i].size());
            out_resource.primitives += primitive_arrays[i];
        }
    }
    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.add_default().ref();
        out_buffer0.index = 0;
        out_buffer0.byte_length = buffer0.size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
    }
    // output one buffer contains vertices & indices
    out_bins.add(buffer0);
}

void CookGLTFMeshData_SplitSkin(const cgltf_data* gltf_data, MeshAsset* cfg, MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins)
{
    skr::Vector<uint8_t> buffer0 = {};
    skr::Vector<uint8_t> buffer1 = {};

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
    skr::Map<uint32_t, SRawMesh> raw_meshes;
    {
        for (uint32_t i = 0; i < gltf_data->meshes_count; i++)
            raw_meshes.try_add_default(i);
        for (uint32_t i = 0; i < gltf_data->meshes_count; i++)
            raw_meshes.add(i, std::move(GenerateRawMeshForGLTFMesh(&gltf_data->meshes[i])));
    }
    // 2. extract indices and vertices
    skr::Vector<skr::Vector<MeshPrimitive>> primitive_arrays;
    primitive_arrays.resize_default(gltf_data->nodes_count);
    {
        // 2.1 indices
        for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
        {
            if (!gltf_data->nodes[i].mesh) continue;
            auto mesh_index = cgltf_mesh_index(gltf_data, gltf_data->nodes[i].mesh);
            auto& raw_mesh = raw_meshes.find(mesh_index).value();
            auto& new_primitives = primitive_arrays[i]; 
            EmplaceAllRawMeshIndices(&raw_mesh, buffer0, new_primitives);
        }
        // 2.2 vertices
        for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
        {
            if (!gltf_data->nodes[i].mesh) continue;
            auto mesh_index = cgltf_mesh_index(gltf_data, gltf_data->nodes[i].mesh);
            auto& raw_mesh = raw_meshes.find(mesh_index).value();
            auto& new_primitives = primitive_arrays[i]; 
            EmplaceStaticRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, 0, new_primitives);
        }
        // 2.3 vertices
        for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
        {
            if (!gltf_data->nodes[i].mesh) continue;
            auto mesh_index = cgltf_mesh_index(gltf_data, gltf_data->nodes[i].mesh);
            auto& raw_mesh = raw_meshes.find(mesh_index).value();
            auto& new_primitives = primitive_arrays[i]; 
            EmplaceSkinRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer1, 1, new_primitives);
        }
    }
    // 3. record primitvies
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
                auto& prim = primitive_arrays[i][j];
                prim.vertex_layout = shuffle_layout_id;
                prim.material_index = static_cast<uint32_t>(gltf_prim.material - gltf_data->materials);
                mesh_section.primitive_indices.add(out_resource.primitives.size() + j);
            }
            out_resource.primitives.reserve(out_resource.primitives.size() + primitive_arrays[i].size());
            out_resource.primitives += primitive_arrays[i];
        }
    }
    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.add_default().ref();
        out_buffer0.index = 0;
        out_buffer0.byte_length = buffer0.size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
        auto& out_buffer1 = out_resource.bins.add_default().ref();
        out_buffer1.index = 1;
        out_buffer1.byte_length = buffer1.size();
        out_buffer1.used_with_index = false;
        out_buffer1.used_with_vertex = true;
    }
    // output one buffer contains vertices & indices
    out_bins.add(buffer0);
    out_bins.add(buffer1);
}

} // namespace skd::asset