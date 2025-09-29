#include "SkrGraphics/api.h"
#include "SkrMeshCore/mesh_encoder.hpp"

namespace skr
{
inline skr::StringView GetNameFromAttribute(EVertexAttribute attr)
{
    switch (attr)
    {
    case EVertexAttribute::POSITION:
        return u8"POSITION";
    case EVertexAttribute::NORMAL:
        return u8"NORMAL";
    case EVertexAttribute::TANGENT:
        return u8"TANGENT";
    case EVertexAttribute::TEXCOORD:
        return u8"TEXCOORD";
    case EVertexAttribute::COLOR:
        return u8"COLOR";
    case EVertexAttribute::JOINTS:
        return u8"JOINTS";
    case EVertexAttribute::WEIGHTS:
        return u8"WEIGHTS";
    case EVertexAttribute::CUSTOM:
        return u8"CUSTOM";
    default:
        return u8"UNKNOWN";
    }
}

inline skr::StringView GetNameFromVertexStreamType(EVertexStreamType type)
{
    switch (type)
    {
    case EVertexStreamType::POSITION:
        return u8"POSITION";
    case EVertexStreamType::NORMAL:
        return u8"NORMAL";
    case EVertexStreamType::TANGENT:
        return u8"TANGENT";
    case EVertexStreamType::TEXCOORD:
        return u8"TEXCOORD";
    case EVertexStreamType::COLOR:
        return u8"COLOR";
    case EVertexStreamType::JOINTS:
        return u8"JOINTS";
    case EVertexStreamType::WEIGHTS:
        return u8"WEIGHTS";
    case EVertexStreamType::CUSTOM:
        return u8"CUSTOM";
    default:
        return u8"UNKNOWN";
    }
}

using EVertexAttribute = skr::EVertexAttribute;
inline EVertexAttribute GetVertexAttributeFromRawVertexStreamType(EVertexStreamType type)
{
    switch (type)
    {
    case EVertexStreamType::POSITION:
        return EVertexAttribute::POSITION;
    case EVertexStreamType::NORMAL:
        return EVertexAttribute::NORMAL;
    case EVertexStreamType::TANGENT:
        return EVertexAttribute::TANGENT;
    case EVertexStreamType::TEXCOORD:
        return EVertexAttribute::TEXCOORD;
    case EVertexStreamType::COLOR:
        return EVertexAttribute::COLOR;
    case EVertexStreamType::JOINTS:
        return EVertexAttribute::JOINTS;
    case EVertexStreamType::WEIGHTS:
        return EVertexAttribute::WEIGHTS;
    case EVertexStreamType::CUSTOM:
        return EVertexAttribute::CUSTOM;
    default:
        return EVertexAttribute::NONE;
    }
}

uint32_t FindRawVertexAttributeLocalIndex(const EVertexAttribute& attr, skr::Span<const EVertexAttribute> range)
{
    uint32_t idx = 0;
    for (const auto& a : range)
    {
        if (a == attr)
        {
            if (&a != &attr)
                idx += 1;
            else
                break;
        }
    }
    return idx;
}

uint32_t FindVertexAttributeIndexInLayout(const CGPUVertexAttribute& attr, const CGPUVertexLayout& layout)
{
    uint32_t idx = 0;
    for (uint32_t i = 0; i < layout.attribute_count; i++)
    {
        const auto& a = layout.attributes[i];
        if (::strcmp((const char*)a.semantic_name, (const char*)attr.semantic_name) == 0)
        {
            if (&a != &attr)
                idx += 1;
            else
                break;
        }       
    }
    return idx;
}

void MeshIndexStream::Encode(skr::MeshPrimitive& primitive, skr::Vector<uint8_t>& buffer) const
{
    const uint32_t offset = (uint32_t)buffer.size();
    buffer.append(data(), count() * stride());
    primitive.index_buffer.buffer_index = 0;
    primitive.index_buffer.first_index = 0;
    primitive.index_buffer.index_count = (uint32_t)count();
    primitive.index_buffer.index_offset = offset;
    primitive.index_buffer.stride = (uint32_t)stride();

    // GPU needs to align 16 bytes
    while (buffer.size() % 16 != 0)
    {
        buffer.add(0);
    }
}

void MeshVertexStream::Encode(skr::MeshPrimitive& primitive, skr::Vector<uint8_t>& buffer, uint32_t attribute_idx, uint32_t buffer_idx) const
{
    const uint32_t offset = (uint32_t)buffer.size();
    buffer.append(data(), count() * stride());
    auto& entry = primitive.vertex_buffers.emplace().ref();
    entry.attribute_index = attribute_idx;
    entry.vertex_count = (uint32_t)count();
    entry.buffer_index = buffer_idx;
    entry.attribute = GetVertexAttributeFromRawVertexStreamType(type());
    entry.offset = offset;
    entry.stride = (uint32_t)stride();

    // GPU needs to align 16 bytes
    while (buffer.size() % 16 != 0)
    {
        buffer.add(0);
    }
}

skr::Span<MeshVertexStream> MeshPrimitiveEncoder::GetVertexStreams()
{
    return vertex_streams;
}

skr::Span<const MeshVertexStream> MeshPrimitiveEncoder::GetVertexStreams() const
{
    return vertex_streams;
}

MeshIndexStream& MeshPrimitiveEncoder::GetIndexStream()
{
    return index_stream;
}

const MeshIndexStream& MeshPrimitiveEncoder::GetIndexStream() const
{
    return index_stream;
}

MeshVertexStream& MeshPrimitiveEncoder::AddVertexStream()
{
    return vertex_streams.emplace().ref();
}

const MeshVertexStream* MeshPrimitiveEncoder::FindVertexStream(const CGPUVertexAttribute& attr, uint32_t idx) const
{
    uint32_t n = 0;
    for (auto& stream : vertex_streams)
    {   
        auto name = GetNameFromVertexStreamType(stream.type());
        if (::strcmp((const char*)name.data(), (const char*)attr.semantic_name) == 0)
        {
            if (n == idx)
                return &stream;
            n += 1;
        }  
    }
    return nullptr;
}

const MeshVertexStream* MeshPrimitiveEncoder::FindVertexStream(const EVertexAttribute& attr, uint32_t idx) const
{
    uint32_t n = 0;
    for (auto& stream : vertex_streams)
    {   
        auto name = GetNameFromVertexStreamType(stream.type());
        auto name2 = GetNameFromAttribute(attr);
        if (::strcmp((const char*)name.data(), (const char*)name2.data()) == 0)
        {
            if (n == idx)
                return &stream;
            n += 1;
        }  
    }
    return nullptr;
}

MeshPrimitiveEncoder& MeshPrimitiveEncoder::SetMaterialIndex(uint32_t idx)
{
    material_index = idx;
    return *this;
}

void EncodedMesh::FillRuntimeMesh(skr::MeshResource& mesh)
{
    // 1. fill primitives
    mesh.primitives = primitives;

    // 2. fill sections
    uint32_t prim_idx = 0;
    for (auto& prim : primitives)
    {
        auto& mesh_section = mesh.sections.add_default().ref();
        mesh_section.translation = { 0.0f, 0.0f, 0.0f };
        mesh_section.scale = { 1.0f, 1.0f, 1.0f };
        mesh_section.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        mesh_section.primitive_indices.add(prim_idx);
        prim_idx += 1;
    }
    
    // 3. fill buffers
    mesh.bins.reserve(buffers.size());
    for (uint32_t i = 0; i < buffers.size(); i++)
    {
        auto& buffer = buffers[i];
        auto& ref = mesh.bins.emplace().ref();
        ref.index = i;
        ref.used_with_index = true;
        ref.used_with_vertex = true;
        ref.byte_length = buffer.size();
        void* data = nullptr; uint64_t size = 0; uint64_t cap = 0;
        buffer.memory().extract_memory(data, size, cap);
        ref.blob = skr::IBlob::Create((uint8_t*)data, size, true);
    }
}

void MeshEncoder::ResizePrimitives(uint32_t n)
{
    primitive_encoders.resize_default(n);
}

MeshPrimitiveEncoder& MeshEncoder::GetPrimitiveAt(uint32_t i)
{
    return primitive_encoders[i];
}

MeshPrimitiveEncoder& MeshEncoder::AddPrimitive()
{
    return primitive_encoders.emplace().ref();
}

EncodedMesh MeshEncoder::Encode(skr::Span<const CGPUVertexLayout> layouts)
{
    EncodedMesh encoded;
    encoded.buffers.resize_default(layouts.size());
    encoded.primitives.resize_default(primitive_encoders.size());
    // encode indices
    for (uint32_t prim_idx = 0; prim_idx < primitive_encoders.size(); prim_idx++) 
    {
        auto& primitive = encoded.primitives[prim_idx];
        const auto& primitive_encoder = primitive_encoders[prim_idx];
        primitive_encoder.index_stream.Encode(primitive, encoded.buffers[0]);
        primitive.material_index = primitive_encoder.material_index;
    }
    for (uint32_t prim_idx = 0; prim_idx < primitive_encoders.size(); prim_idx++) 
    {
        auto& primitive = encoded.primitives[prim_idx];
        const auto& primitive_encoder = primitive_encoders[prim_idx];
        primitive.vertex_buffers.reserve(primitive_encoder.vertex_streams.size());
        primitive.vertex_count = primitive_encoder.vertex_streams[0].count();
        for (uint32_t layout_idx = 0; layout_idx < layouts.size(); layout_idx++)
        {
            const auto& layout = layouts[layout_idx];
            auto& mesh_buffer = encoded.buffers.emplace().ref();
            for (uint32_t i = 0; i < layout.attribute_count; i++)
            {
                const auto& attr = layout.attributes[i];
                const auto attr_idx = FindVertexAttributeIndexInLayout(attr, layout);
                if (const auto vertex_stream = primitive_encoder.FindVertexStream(attr, attr_idx))
                {
                    auto& vertex_entry = primitive.vertex_buffers.emplace().ref();
                    vertex_stream->Encode(primitive, encoded.buffers[layout_idx], attr_idx, layout_idx);
                }
            }
        }
    }
    return encoded;
}

EncodedMesh MeshEncoder::Encode(const skr::Map<uint32_t, skr::Vector<EVertexAttribute>>& soa_layouts)
{
    EncodedMesh encoded;
    encoded.buffers.resize_default(soa_layouts.size());
    encoded.primitives.resize_default(primitive_encoders.size());
    // encode indices
    for (uint32_t i = 0; i < primitive_encoders.size(); i++) 
    {
        auto& primitive = encoded.primitives[i];
        const auto& primitive_encoder = primitive_encoders[i];
        primitive_encoder.index_stream.Encode(primitive, encoded.buffers[0]);
        primitive.material_index = primitive_encoder.material_index;
    }
    for (uint32_t i = 0; i < primitive_encoders.size(); i++) 
    {
        auto& primitive = encoded.primitives[i];
        const auto& primitive_encoder = primitive_encoders[i];
        primitive.vertex_buffers.reserve(primitive_encoder.vertex_streams.size());
        primitive.vertex_count = primitive_encoder.vertex_streams[0].count();
        for (const auto& [layout_idx, layout] : soa_layouts)
        {
            auto& mesh_buffer = encoded.buffers[layout_idx];
            for (const auto& attr : layout)
            {
                const auto attr_idx = FindRawVertexAttributeLocalIndex(attr, layout);
                if (const auto vertex_stream = primitive_encoder.FindVertexStream(attr, attr_idx))
                {
                    auto& vertex_entry = primitive.vertex_buffers.emplace().ref();
                    vertex_stream->Encode(primitive, encoded.buffers[layout_idx], attr_idx, layout_idx);
                }
            }
        }
    }
    return encoded;
}

} // namespace skr