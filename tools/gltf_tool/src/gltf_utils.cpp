#include "SkrGLTFTool/gltf_utils.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrGLTFTool/mesh_asset.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "cgpu/api.h"

#include "Tracy/Tracy.hpp"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

namespace skd
{
namespace asset
{
cgltf_data* ImportGLTFWithData(skr::string_view assetPath, skr::io::RAMService* ioService, struct skr_vfs_t* vfs) SKR_NOEXCEPT
{
    // prepare callback
    skr::task::event_t counter;
    skr_async_ram_destination_t destination;
    skr::string u8Path = assetPath.data();
    struct CallbackData
    {
        skr::task::event_t* pCounter;   
    } callbackData;
    callbackData.pCounter = &counter;
    // prepare io
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cbData->pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&callbackData;
    skr_async_request_t ioRequest = {};
    ioService->request(vfs, &ramIO, &ioRequest, &destination);
    counter.wait(false);
    struct cgltf_data* gltf_data_ = nullptr;
    {
        ZoneScopedN("ParseGLTF");
        cgltf_options options = {};
        if (destination.bytes)
        {
            cgltf_result result = cgltf_parse(&options, destination.bytes, destination.size, &gltf_data_);
            if (result != cgltf_result_success)
            {
                gltf_data_ = nullptr;
            }
            else
            {
                ZoneScopedN("LoadGLTFBuffer");

                result = cgltf_load_buffers(&options, gltf_data_, u8Path.c_str());
                result = cgltf_validate(gltf_data_);
                if (result != cgltf_result_success)
                {
                    gltf_data_ = nullptr;
                }
            }
        }
        sakura_free(destination.bytes);
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

skr::span<const uint8_t> GetGLTFPrimitiveIndicesView(const cgltf_primitive* primitve, uint32_t& index_stride)
{
    index_stride = (uint32_t)primitve->indices->stride;

    const auto buffer_view = primitve->indices->buffer_view;
    const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
    const auto view_data = buffer_data + buffer_view->offset;
    const auto indices_count = primitve->indices->count;
    return skr::span<const uint8_t>(view_data + primitve->indices->offset, index_stride * indices_count);
}

skr::span<const uint8_t> GetGLTFPrimitiveAttributeView(const cgltf_primitive* primitve, cgltf_attribute_type type, uint32_t idx, uint32_t& stride)
{
    for (uint32_t i = 0; i < primitve->attributes_count; i++)
    {
        const auto& attribute = primitve->attributes[i];
        if (attribute.type == type && attribute.index == idx)
        {
            stride = (uint32_t)attribute.data->stride;
            
            const auto buffer_view = attribute.data->buffer_view;
            const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
            const auto view_data = buffer_data + buffer_view->offset;
            return skr::span<const uint8_t>(view_data + attribute.data->offset, attribute.data->stride * attribute.data->count);
        }
    }
    return {};
}

skr::span<const uint8_t> GetGLTFPrimitiveAttributeView(const cgltf_primitive* primitve, const char* semantics, uint32_t idx, uint32_t& stride, cgltf_attribute_type& out_type)
{
    for (uint32_t type = 0; type < cgltf_attribute_type_custom + 1; type++)
    {
        const auto refStr = kGLTFAttributeTypeNameLUT[type];
        eastl::string_view semantics_sv = semantics;
        if (semantics_sv.starts_with(refStr))
        {
            out_type = static_cast<cgltf_attribute_type>(type);
            return GetGLTFPrimitiveAttributeView(primitve, (cgltf_attribute_type)type, idx, stride);
        }
    }
    return {};
}

void EmplaceGLTFPrimitiveIndexBuffer(const cgltf_primitive* primitve, eastl::vector<uint8_t>& buffer, skr_index_buffer_entry_t& index_buffer)
{
    uint32_t index_stride = 0;
    const auto ib_view = GetGLTFPrimitiveIndicesView(primitve, index_stride);

    index_buffer.buffer_index = 0;
    index_buffer.first_index = 0; //TODO: ?
    index_buffer.index_offset = (uint32_t)buffer.size();
    index_buffer.index_count = (uint32_t)ib_view.size() / index_stride;
    index_buffer.stride = index_stride;
    buffer.insert(buffer.end(), ib_view.data(), ib_view.data() + ib_view.size());
}

void EmplaceGLTFPrimitiveVertexBufferAttribute(const cgltf_primitive* primitve, cgltf_attribute_type type, uint32_t idx, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv)
{
    skr::span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    vertex_attribtue_slice = GetGLTFPrimitiveAttributeView(primitve, type, idx, attribute_stride);

    out_vbv.attribute = kGLTFAttributeTypeLUT[type];
    out_vbv.attribute_index = idx;

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = attribute_stride ? (uint32_t)buffer.size() : 0u;
    buffer.insert(buffer.end(), vertex_attribtue_slice.data(), vertex_attribtue_slice.data() + vertex_attribtue_slice.size());
}

void EmplaceGLTFPrimitiveVertexBufferAttribute(const cgltf_primitive* primitve, const char* semantics, uint32_t idx, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv)
{
    skr::span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    cgltf_attribute_type type;
    vertex_attribtue_slice = GetGLTFPrimitiveAttributeView(primitve, semantics, idx, attribute_stride, type);

    out_vbv.attribute = kGLTFAttributeTypeLUT[type];
    out_vbv.attribute_index = idx;

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = attribute_stride ? (uint32_t)buffer.size() : 0u;
    buffer.insert(buffer.end(), vertex_attribtue_slice.data(), vertex_attribtue_slice.data() + vertex_attribtue_slice.size());
}

void EmplaceAllGLTFMeshIndices(const cgltf_mesh* mesh, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    out_primitives.resize(mesh->primitives_count);
    // record all indices
    for (uint32_t j = 0; j < mesh->primitives_count; j++)
    {
        const auto gltf_prim = mesh->primitives + j;
        EmplaceGLTFPrimitiveIndexBuffer(gltf_prim, buffer, out_primitives[j].index_buffer);
    }
}

void EmplaceGLTFMeshVerticesWithRange(skr::span<const ESkrVertexAttribute> range, uint32_t buffer_idx,  const cgltf_mesh* mesh, 
    const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    if (layout != nullptr)
    {
    const auto& shuffle_layout = *layout;
    for (uint32_t i = 0; i < shuffle_layout.attribute_count; i++)
    {
        // geometry cache friendly layout
        // | prim0-pos | prim1-pos | prim0-tangent | prim1-tangent | ...
        for (uint32_t j = 0; j < mesh->primitives_count; j++)
        {
            auto& prim = out_primitives[j];
            prim.vertex_buffers.resize(shuffle_layout.attribute_count);

            const auto gltf_prim = mesh->primitives + j;
            prim.vertex_count = gltf_prim->attributes[0].data->count;
            const auto& shuffle_attrib = shuffle_layout.attributes[i];
            for (uint32_t k = 0u; k < shuffle_attrib.array_size; k++)
            {
                bool within = false;
                for (auto type : range)
                {
                    const eastl::string semantic = kGLTFAttributeTypeNameLUT[type];
                    if (semantic == shuffle_attrib.semantic_name)
                    {
                        within = true;
                        break;
                    }
                }
                if (within) 
                {
                    EmplaceGLTFPrimitiveVertexBufferAttribute(gltf_prim, shuffle_attrib.semantic_name, k, buffer, prim.vertex_buffers[i]);
                    prim.vertex_buffers[i].buffer_index = buffer_idx;
                }
            }
        }
    }
    }
    else 
    {
        SKR_UNREACHABLE_CODE();
    }
}

void EmplaceAllGLTFMeshVertices(const cgltf_mesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    EmplaceGLTFMeshVerticesWithRange(kGLTFAttributeTypeLUT, 0, mesh, layout, buffer, out_primitives);
}

void EmplaceSkinGLTFMeshVertices(const cgltf_mesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, uint32_t buffer_idx, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    EmplaceGLTFMeshVerticesWithRange(kGLTFSkinAttributes, buffer_idx, mesh, layout, buffer, out_primitives);
}

void EmplaceStaticGLTFMeshVertices(const cgltf_mesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, uint32_t buffer_idx, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    EmplaceGLTFMeshVerticesWithRange(kGLTFStaticAttributes, buffer_idx, mesh, layout, buffer, out_primitives);
}

void CookGLTFMeshData(const cgltf_data* gltf_data, SMeshCookConfig* cfg, skr_mesh_resource_t& out_resource, eastl::vector<eastl::vector<uint8_t>>& out_bins)
{
    eastl::vector<uint8_t> buffer0 = {};
    
    skr_guid_t shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.isZero()) 
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }

    out_resource.name = gltf_data->meshes[0].name;
    if (out_resource.name.empty()) out_resource.name = "gltfMesh";
    // record primitvies
    for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
    {
        const auto node_ = gltf_data->nodes + i;
        auto& mesh_section = out_resource.sections.emplace_back();
        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
        GetGLTFNodeTransform(node_, mesh_section.translation, mesh_section.scale, mesh_section.rotation);
        if (node_->mesh != nullptr)
        {
            eastl::vector<skr_mesh_primitive_t> new_primitives;
            // record all indices
            EmplaceAllGLTFMeshIndices(node_->mesh, buffer0, new_primitives);
            EmplaceAllGLTFMeshVertices(node_->mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, new_primitives);
            for (uint32_t j = 0; j < node_->mesh->primitives_count; j++)
            {
                auto& prim = new_primitives[j];
                prim.vertex_layout_id = shuffle_layout_id;
                prim.material_inst = make_zeroed<skr_guid_t>(); // TODO: Material assignment
                mesh_section.primive_indices.emplace_back(out_resource.primitives.size() + j);
            }
            out_resource.primitives.reserve(out_resource.primitives.size() + new_primitives.size());
            out_resource.primitives.insert(out_resource.primitives.end(), new_primitives.begin(), new_primitives.end());
        }
    }
    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.emplace_back();
        out_buffer0.index = 0;
        out_buffer0.byte_length = buffer0.size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
    }
    // output one buffer contains vertices & indices
    out_bins.emplace_back(buffer0);
}

void CookGLTFMeshData_SplitSkin(const cgltf_data* gltf_data, SMeshCookConfig* cfg, skr_mesh_resource_t& out_resource, eastl::vector<eastl::vector<uint8_t>>& out_bins)
{
    eastl::vector<uint8_t> buffer0 = {};
    eastl::vector<uint8_t> buffer1 = {};
    
    skr_guid_t shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.isZero()) 
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }

    out_resource.name = gltf_data->meshes[0].name;
    if (out_resource.name.empty()) out_resource.name = "gltfMesh";
    // record primitvies
    for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
    {
        const auto node_ = gltf_data->nodes + i;
        auto& mesh_section = out_resource.sections.emplace_back();
        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
        GetGLTFNodeTransform(node_, mesh_section.translation, mesh_section.scale, mesh_section.rotation);
        if (node_->mesh != nullptr)
        {
            eastl::vector<skr_mesh_primitive_t> new_primitives;
            // record all indices
            EmplaceAllGLTFMeshIndices(node_->mesh, buffer0, new_primitives);
            EmplaceStaticGLTFMeshVertices(node_->mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, 0, new_primitives);
            EmplaceSkinGLTFMeshVertices(node_->mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer1, 1, new_primitives);
            for (uint32_t j = 0; j < node_->mesh->primitives_count; j++)
            {
                auto& prim = new_primitives[j];
                prim.vertex_layout_id = shuffle_layout_id;
                prim.material_inst = make_zeroed<skr_guid_t>(); // TODO: Material assignment
                mesh_section.primive_indices.emplace_back(out_resource.primitives.size() + j);
            }
            out_resource.primitives.reserve(out_resource.primitives.size() + new_primitives.size());
            out_resource.primitives.insert(out_resource.primitives.end(), new_primitives.begin(), new_primitives.end());
        }
    }
    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.emplace_back();
        out_buffer0.index = 0;
        out_buffer0.byte_length = buffer0.size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
        auto& out_buffer1 = out_resource.bins.emplace_back();
        out_buffer1.index = 1;
        out_buffer1.byte_length = buffer1.size();
        out_buffer1.used_with_index = false;
        out_buffer1.used_with_vertex = true;
    }
    // output one buffer contains vertices & indices
    out_bins.emplace_back(buffer0);
    out_bins.emplace_back(buffer1);
}

} // namespace asset
} // namespace skd