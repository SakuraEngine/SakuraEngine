#include "SkrContainersDef/stl_string.hpp"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrGraphics/api.h"

namespace skd::asset
{

inline stl_string GetNameFromAttribute(EVertexAttribute attr)
{
    switch (attr)
    {
    case EVertexAttribute::POSITION:
        return "POSITION";
    case EVertexAttribute::NORMAL:
        return "NORMAL";
    case EVertexAttribute::TANGENT:
        return "TANGENT";
    case EVertexAttribute::TEXCOORD:
        return "TEXCOORD";
    case EVertexAttribute::COLOR:
        return "COLOR";
    case EVertexAttribute::JOINTS:
        return "JOINTS";
    case EVertexAttribute::WEIGHTS:
        return "WEIGHTS";
    case EVertexAttribute::CUSTOM:
        return "CUSTOM";
    default:
        return "UNKNOWN";
    }
}

inline stl_string GetNameFromVertexStreamType(ERawVertexStreamType type)
{
    switch (type)
    {
    case ERawVertexStreamType::POSITION:
        return "POSITION";
    case ERawVertexStreamType::NORMAL:
        return "NORMAL";
    case ERawVertexStreamType::TANGENT:
        return "TANGENT";
    case ERawVertexStreamType::TEXCOORD:
        return "TEXCOORD";
    case ERawVertexStreamType::COLOR:
        return "COLOR";
    case ERawVertexStreamType::JOINTS:
        return "JOINTS";
    case ERawVertexStreamType::WEIGHTS:
        return "WEIGHTS";
    case ERawVertexStreamType::CUSTOM:
        return "CUSTOM";
    default:
        return "UNKNOWN";
    }
}

inline EVertexAttribute GetVertexAttributeFromRawVertexStreamType(ERawVertexStreamType type)
{
    switch (type)
    {
    case ERawVertexStreamType::POSITION:
        return EVertexAttribute::POSITION;
    case ERawVertexStreamType::NORMAL:
        return EVertexAttribute::NORMAL;
    case ERawVertexStreamType::TANGENT:
        return EVertexAttribute::TANGENT;
    case ERawVertexStreamType::TEXCOORD:
        return EVertexAttribute::TEXCOORD;
    case ERawVertexStreamType::COLOR:
        return EVertexAttribute::COLOR;
    case ERawVertexStreamType::JOINTS:
        return EVertexAttribute::JOINTS;
    case ERawVertexStreamType::WEIGHTS:
        return EVertexAttribute::WEIGHTS;
    case ERawVertexStreamType::CUSTOM:
        return EVertexAttribute::CUSTOM;
    default:
        return EVertexAttribute::NONE;
    }
}

skr::Span<const uint8_t> GetRawPrimitiveIndicesView(const SRawPrimitive* primitive, uint32_t& index_stride)
{
    index_stride = (uint32_t)primitive->index_stream.stride;

    const auto buffer_view = primitive->index_stream.buffer_view;
    const auto buffer_data = buffer_view.data();
    const auto indices_count = primitive->index_stream.count;
    return skr::Span<const uint8_t>(buffer_data + primitive->index_stream.offset, index_stride * indices_count);
}

skr::Span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitive, ERawVertexStreamType type, uint32_t idx, uint32_t& stride)
{
    for (uint32_t i = 0; i < primitive->vertex_streams.size(); i++)
    {
        const auto& attribute = primitive->vertex_streams[i];
        if (attribute.type == type && attribute.index == idx)
        {
            stride = (uint32_t)attribute.stride;

            const auto buffer_data = attribute.buffer_view.data();
            return skr::Span<const uint8_t>(buffer_data + attribute.offset, attribute.stride * attribute.count);
        }
    }
    return {};
}

skr::Span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitive, const char* semantics, uint32_t idx, uint32_t& stride, ERawVertexStreamType& out_type)
{
    const auto kVertexStreamTypesCount = static_cast<uint32_t>(ERawVertexStreamType::Count);
    for (uint32_t type = 0; type < kVertexStreamTypesCount; type++)
    {
        const auto refStr = GetNameFromVertexStreamType(static_cast<ERawVertexStreamType>(type));
        skr::stl_string_view semantics_sv = semantics;
        if (semantics_sv.starts_with(refStr))
        {
            out_type = static_cast<ERawVertexStreamType>(type);
            return GetRawPrimitiveAttributeView(primitive, (ERawVertexStreamType)type, idx, stride);
        }
    }
    return {};
}

void EmplaceRawPrimitiveIndexBuffer(const SRawPrimitive* primitive, skr::Vector<uint8_t>& buffer, IndexBufferEntry& index_buffer)
{
    uint32_t index_stride = 0;
    const auto ib_view = GetRawPrimitiveIndicesView(primitive, index_stride);

    index_buffer.buffer_index = 0;
    index_buffer.first_index = 0; // TODO: ?
    index_buffer.index_offset = (uint32_t)buffer.size();
    index_buffer.index_count = (uint32_t)ib_view.size() / index_stride;
    index_buffer.stride = index_stride;
    buffer.append(ib_view.data(), ib_view.size());

    // GPU needs to align 16 bytes
    while (buffer.size() % 16 != 0)
    {
        buffer.add(0);
    }
}

void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitive, ERawVertexStreamType type, uint32_t idx, skr::Vector<uint8_t>& buffer, VertexBufferEntry& out_vbv)
{
    skr::Span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    vertex_attribtue_slice = GetRawPrimitiveAttributeView(primitive, type, idx, attribute_stride);

    // out_vbv.attribute = kRawAttributeTypeLUT[static_cast<uint32_t>(type)];
    out_vbv.attribute = GetVertexAttributeFromRawVertexStreamType(type);
    out_vbv.attribute_index = idx;

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = attribute_stride ? (uint32_t)buffer.size() : 0u;
    //buffer.append(vertex_attribtue_slice.data(), vertex_attribtue_slice.size());
    out_vbv.vertex_count = 0u;
    if (vertex_attribtue_slice.size() != 0)
    {
        out_vbv.vertex_count = vertex_attribtue_slice.size() / out_vbv.stride;
        buffer.append(vertex_attribtue_slice.data(), vertex_attribtue_slice.size());
    }

    // GPU needs to align 16 bytes
    while (buffer.size() % 16 != 0)
    {
        buffer.add(0);
    }
}

void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitive, const char* semantics, uint32_t idx, skr::Vector<uint8_t>& buffer, VertexBufferEntry& out_vbv)
{
    skr::Span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    ERawVertexStreamType type;
    vertex_attribtue_slice = GetRawPrimitiveAttributeView(primitive, semantics, idx, attribute_stride, type);

    // out_vbv.attribute = kRawAttributeTypeLUT[static_cast<uint32_t>(type)];
    out_vbv.attribute = GetVertexAttributeFromRawVertexStreamType(type);
    out_vbv.attribute_index = idx;

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = attribute_stride ? (uint32_t)buffer.size() : 0u;
    out_vbv.vertex_count = 0u;
    if (vertex_attribtue_slice.size() != 0)
    {
        out_vbv.vertex_count = vertex_attribtue_slice.size() / out_vbv.stride;
        buffer.append(vertex_attribtue_slice.data(), vertex_attribtue_slice.size());
    }

    // GPU needs to align 16 bytes
    while (buffer.size() % 16 != 0)
    {
        buffer.add(0);
    }
}

void EmplaceAllRawMeshIndices(const SRawMesh* mesh, skr::Vector<uint8_t>& buffer, skr::Vector<MeshPrimitive>& out_primitives)
{
    out_primitives.resize_default(mesh->primitives.size());
    // record all indices
    for (uint32_t j = 0; j < mesh->primitives.size(); j++)
    {
        const auto& raw_primitive = mesh->primitives[j];
        EmplaceRawPrimitiveIndexBuffer(&raw_primitive, buffer, out_primitives[j].index_buffer);
    }
}

void EmplaceRawMeshVerticesWithRange(skr::Span<const EVertexAttribute> range, uint32_t buffer_idx, const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::Vector<uint8_t>& buffer, skr::Vector<MeshPrimitive>& out_primitives)
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
                // prim.vertex_buffers.reserve(shuffle_layout.attribute_count);
                prim.vertex_buffers.resize_default(shuffle_layout.attribute_count);
                // 这里prim.vertex_buffers实际上是一个字典，维护了layout_id到buffer_entry的映射关系，所以不能直接emplace
                // 只能resize好后直接索引赋值

                const auto& raw_primitive = mesh->primitives[j];
                prim.vertex_count = (uint32_t)raw_primitive.vertex_streams[0].count;
                const auto& shuffle_attrib = shuffle_layout.attributes[i];
                for (uint32_t k = 0u; k < shuffle_attrib.array_size; k++)
                {
                    bool within = false;
                    for (auto type : range)
                    {
                        // const skr::stl_string semantic = kRawAttributeTypeNameLUT[static_cast<uint32_t>(type)];
                        const auto semantic = GetNameFromAttribute(type);
                        if (semantic == (const char*)shuffle_attrib.semantic_name)
                        {
                            within = true;
                            break;
                        }
                    }
                    if (within)
                    {
                        // auto& new_vb = prim.vertex_buffers.emplace().ref();
                        // EmplaceRawPrimitiveVertexBufferAttribute(&raw_primitive, (const char*)shuffle_attrib.semantic_name, k, buffer, new_vb);
                        // new_vb.buffer_index = buffer_idx;
                        EmplaceRawPrimitiveVertexBufferAttribute(&raw_primitive, (const char*)shuffle_attrib.semantic_name, k, buffer, prim.vertex_buffers[i]);
                        prim.vertex_buffers[i].buffer_index = buffer_idx;
                        // TODO: 现在 UV0 会被错误导入成 UV1，需要修复
                        // prim.vertex_buffers[i].attribute_index = 
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

void EmplaceAllRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::Vector<uint8_t>& buffer, skr::Vector<MeshPrimitive>& out_primitives)
{
    EmplaceRawMeshVerticesWithRange(kRawAttributes, 0, mesh, layout, buffer, out_primitives);
}

void EmplaceSkinRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::Vector<uint8_t>& buffer, uint32_t buffer_idx, skr::Vector<MeshPrimitive>& out_primitives)
{
    EmplaceRawMeshVerticesWithRange(kRawSkinAttributes, buffer_idx, mesh, layout, buffer, out_primitives);
}

void EmplaceStaticRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::Vector<uint8_t>& buffer, uint32_t buffer_idx, skr::Vector<MeshPrimitive>& out_primitives)
{
    EmplaceRawMeshVerticesWithRange(kRawStaticAttributes, buffer_idx, mesh, layout, buffer, out_primitives);
}

} // namespace skd::asset