#include "async/fib_task.hpp"
#include "SkrMeshCore/mesh_processing.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "cgpu/api.h"

#include <EASTL/string.h>
#include "tracy/Tracy.hpp"

namespace skd
{
namespace asset
{
skr::span<const uint8_t> GetRawPrimitiveIndicesView(const SRawPrimitive* primitve, uint32_t& index_stride)
{
    index_stride = (uint32_t)primitve->index_stream.stride;

    const auto buffer_view = primitve->index_stream.buffer_view;
    const auto buffer_data = buffer_view.data();
    const auto indices_count = primitve->index_stream.count;
    return skr::span<const uint8_t>(buffer_data + primitve->index_stream.offset, index_stride * indices_count);
}

skr::span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitve, ERawVertexStreamType type, uint32_t idx, uint32_t& stride)
{
    for (uint32_t i = 0; i < primitve->vertex_streams.size(); i++)
    {
        const auto& attribute = primitve->vertex_streams[i];
        if (attribute.type == type && attribute.index == idx)
        {
            stride = (uint32_t)attribute.stride;
            
            const auto buffer_data = attribute.buffer_view.data();
            return skr::span<const uint8_t>(buffer_data + attribute.offset, attribute.stride * attribute.count);
        }
    }
    return {};
}

skr::span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitve, const char* semantics, uint32_t idx, uint32_t& stride, ERawVertexStreamType& out_type)
{
    const auto kVertexStreamTypesCount = static_cast<uint32_t>(ERawVertexStreamType::Count);
    for (uint32_t type = 0; type < kVertexStreamTypesCount; type++)
    {
        const auto refStr = kRawAttributeTypeNameLUT[type];
        eastl::string_view semantics_sv = semantics;
        if (semantics_sv.starts_with(refStr))
        {
            out_type = static_cast<ERawVertexStreamType>(type);
            return GetRawPrimitiveAttributeView(primitve, (ERawVertexStreamType)type, idx, stride);
        }
    }
    return {};
}

void EmplaceRawPrimitiveIndexBuffer(const SRawPrimitive* primitve, eastl::vector<uint8_t>& buffer, skr_index_buffer_entry_t& index_buffer)
{
    uint32_t index_stride = 0;
    const auto ib_view = GetRawPrimitiveIndicesView(primitve, index_stride);

    index_buffer.buffer_index = 0;
    index_buffer.first_index = 0; //TODO: ?
    index_buffer.index_offset = (uint32_t)buffer.size();
    index_buffer.index_count = (uint32_t)ib_view.size() / index_stride;
    index_buffer.stride = index_stride;
    buffer.insert(buffer.end(), ib_view.data(), ib_view.data() + ib_view.size());
}

void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitve, ERawVertexStreamType type, uint32_t idx, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv)
{
    skr::span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    vertex_attribtue_slice = GetRawPrimitiveAttributeView(primitve, type, idx, attribute_stride);

    out_vbv.attribute = kRawAttributeTypeLUT[static_cast<uint32_t>(type)];
    out_vbv.attribute_index = idx;

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = attribute_stride ? (uint32_t)buffer.size() : 0u;
    buffer.insert(buffer.end(), vertex_attribtue_slice.data(), vertex_attribtue_slice.data() + vertex_attribtue_slice.size());
}

void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitve, const char* semantics, uint32_t idx, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv)
{
    skr::span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    ERawVertexStreamType type;
    vertex_attribtue_slice = GetRawPrimitiveAttributeView(primitve, semantics, idx, attribute_stride, type);

    out_vbv.attribute = kRawAttributeTypeLUT[static_cast<uint32_t>(type)];
    out_vbv.attribute_index = idx;

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = attribute_stride ? (uint32_t)buffer.size() : 0u;
    buffer.insert(buffer.end(), vertex_attribtue_slice.data(), vertex_attribtue_slice.data() + vertex_attribtue_slice.size());
}

void EmplaceAllRawMeshIndices(const SRawMesh* mesh, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    out_primitives.resize(mesh->primitives.size());
    // record all indices
    for (uint32_t j = 0; j < mesh->primitives.size(); j++)
    {
        const auto& raw_primitive = mesh->primitives[j];
        EmplaceRawPrimitiveIndexBuffer(&raw_primitive, buffer, out_primitives[j].index_buffer);
    }
}

void EmplaceRawMeshVerticesWithRange(skr::span<const ESkrVertexAttribute> range, uint32_t buffer_idx,  const SRawMesh* mesh, 
    const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    if (layout != nullptr)
    {
    const auto& shuffle_layout = *layout;
    for (uint32_t i = 0; i < shuffle_layout.attribute_count; i++)
    {
        // geometry cache friendly layout
        // | prim0-pos | prim1-pos | prim0-tangent | prim1-tangent | ...
        for (uint32_t j = 0; j < mesh->primitives.size(); j++)
        {
            auto& prim = out_primitives[j];
            prim.vertex_buffers.resize(shuffle_layout.attribute_count);

            const auto& raw_primitive = mesh->primitives[j];
            prim.vertex_count = (uint32_t)raw_primitive.vertex_streams[0].count;
            const auto& shuffle_attrib = shuffle_layout.attributes[i];
            for (uint32_t k = 0u; k < shuffle_attrib.array_size; k++)
            {
                bool within = false;
                for (auto type : range)
                {
                    const eastl::string semantic = kRawAttributeTypeNameLUT[type];
                    if (semantic == (const char*)shuffle_attrib.semantic_name)
                    {
                        within = true;
                        break;
                    }
                }
                if (within) 
                {
                    EmplaceRawPrimitiveVertexBufferAttribute(&raw_primitive, (const char*)shuffle_attrib.semantic_name, k, buffer, prim.vertex_buffers[i]);
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

void EmplaceAllRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    EmplaceRawMeshVerticesWithRange(kRawAttributeTypeLUT, 0, mesh, layout, buffer, out_primitives);
}

void EmplaceSkinRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, uint32_t buffer_idx, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    EmplaceRawMeshVerticesWithRange(kRawSkinAttributes, buffer_idx, mesh, layout, buffer, out_primitives);
}

void EmplaceStaticRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, uint32_t buffer_idx, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    EmplaceRawMeshVerticesWithRange(kRawStaticAttributes, buffer_idx, mesh, layout, buffer, out_primitives);
}

} // namespace asset
} // namespace skd