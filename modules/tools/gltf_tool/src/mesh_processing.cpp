#include "SkrGraphics/api.h"
#include "cgltf/cgltf.h"
#include "SkrCore/platform/vfs.h"
#include "SkrOS/filesystem.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrGLTFTool/mesh_processing.hpp"

#include "SkrProfile/profile.h"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

namespace skd
{
namespace asset
{
static const ERawVertexStreamType kGLTFToRawAttributeTypeLUT[] = {
    ERawVertexStreamType::POSITION,
    ERawVertexStreamType::NORMAL,
    ERawVertexStreamType::TANGENT,
    ERawVertexStreamType::TEXCOORD,
    ERawVertexStreamType::COLOR,
    ERawVertexStreamType::JOINTS,
    ERawVertexStreamType::WEIGHTS,
    ERawVertexStreamType::CUSTOM,
    ERawVertexStreamType::Count,
};

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
            primitive.index_stream.buffer_view = skr::span<const uint8_t>(view_data + gltf_primitive->indices->offset, gltf_primitive->indices->stride * indices_count);
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
            vertex_stream.buffer_view = skr::span<const uint8_t>(view_data + attribute.data->offset, attribute.data->stride * vertex_count);
            vertex_stream.offset = 0;
            vertex_stream.count = vertex_count;
            vertex_stream.stride = attribute.data->stride;
            vertex_stream.type = kGLTFToRawAttributeTypeLUT[attribute.type];
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
    request->add_callback(SKR_IO_STAGE_COMPLETED, 
    +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cbData->pCounter->signal();
    }, (void*)&callbackData);
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
                auto fullPath = skr::filesystem::path(vfs->mount_dir) / u8Path.u8_str();
                result = cgltf_load_buffers(&options, gltf_data_, fullPath.string().c_str());
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

void GetGLTFNodeTransform(const cgltf_node* node, skr_float3_t& translation, skr_float3_t& scale, skr_float4_t& rotation)
{
    if (node->has_translation)
        translation = { node->translation[0], node->translation[1], node->translation[2] };
    if (node->has_scale)
        scale = { node->scale[0], node->scale[1], node->scale[2] };
    if (node->has_rotation)
        rotation = { node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] };
}

void CookGLTFMeshData(const cgltf_data* gltf_data, SMeshCookConfig* cfg, skr_mesh_resource_t& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins)
{
    skr::Vector<uint8_t> buffer0 = {};
    
    skr_guid_t shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero()) 
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }
    
    //FIXME: select mesh to cook
    out_resource.name = gltf_data->meshes[0].name ? (const char8_t*)gltf_data->meshes[0].name : u8"";
    if (out_resource.name.is_empty()) out_resource.name = u8"gltfMesh";
    // record primitvies
    for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
    {
        const auto node_ = gltf_data->nodes + i;
        auto& mesh_section = out_resource.sections.add_default().ref();
        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
        GetGLTFNodeTransform(node_, mesh_section.translation, mesh_section.scale, mesh_section.rotation);
        if (node_->mesh != nullptr)
        {
            SRawMesh raw_mesh = GenerateRawMeshForGLTFMesh(node_->mesh);
            skr::Vector<skr_mesh_primitive_t> new_primitives;
            // record all indices
            EmplaceAllRawMeshIndices(&raw_mesh, buffer0, new_primitives);
            EmplaceAllRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, new_primitives);
            for (uint32_t j = 0; j < node_->mesh->primitives_count; j++)
            {
                const auto& gltf_prim = node_->mesh->primitives[j];
                auto& prim = new_primitives[j];
                prim.vertex_layout_id = shuffle_layout_id;
                prim.material_index = static_cast<uint32_t>(gltf_prim.material - gltf_data->materials);
                mesh_section.primive_indices.add(out_resource.primitives.size() + j);
            }
            out_resource.primitives.reserve(out_resource.primitives.size() + new_primitives.size());
            out_resource.primitives += new_primitives;
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

void CookGLTFMeshData_SplitSkin(const cgltf_data* gltf_data, SMeshCookConfig* cfg, skr_mesh_resource_t& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins)
{
    skr::Vector<uint8_t> buffer0 = {};
    skr::Vector<uint8_t> buffer1 = {};
    
    skr_guid_t shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero()) 
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }

    //FIXME: select mesh to cook
    out_resource.name = (const char8_t*)gltf_data->meshes[0].name;
    if (out_resource.name.is_empty()) out_resource.name = u8"gltfMesh";
    // record primitvies
    for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
    {
        const auto node_ = gltf_data->nodes + i;
        auto& mesh_section = out_resource.sections.add_default().ref();
        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
        GetGLTFNodeTransform(node_, mesh_section.translation, mesh_section.scale, mesh_section.rotation);
        if (node_->mesh != nullptr)
        {
            SRawMesh raw_mesh = GenerateRawMeshForGLTFMesh(node_->mesh);
            skr::Vector<skr_mesh_primitive_t> new_primitives;
            // record all indices
            EmplaceAllRawMeshIndices(&raw_mesh, buffer0, new_primitives);
            EmplaceStaticRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, 0, new_primitives);
            EmplaceSkinRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer1, 1, new_primitives);
            for (uint32_t j = 0; j < node_->mesh->primitives_count; j++)
            {
                const auto& gltf_prim = node_->mesh->primitives[j];
                auto& prim = new_primitives[j];
                prim.vertex_layout_id = shuffle_layout_id;
                prim.material_index = static_cast<uint32_t>(gltf_prim.material - gltf_data->materials);
                mesh_section.primive_indices.add(out_resource.primitives.size() + j);
            }
            out_resource.primitives.reserve(out_resource.primitives.size() + new_primitives.size());
            out_resource.primitives += new_primitives;
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

}
}