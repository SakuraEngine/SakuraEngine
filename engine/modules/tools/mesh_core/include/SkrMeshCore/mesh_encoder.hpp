#pragma once
#include "SkrBase/config.h"
#include "SkrContainersDef/map.hpp"
#include "SkrRenderer/resources/mesh_resource.h"

namespace skr
{
using EVertexAttribute = skr::EVertexAttribute;
struct MeshEncoder;

enum class EPrimitiveType : uint32_t
{
    POINTS,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    Count
};

enum class EVertexStreamType : uint32_t
{
    POSITION,
    NORMAL,
    TANGENT,
    TEXCOORD,
    COLOR,
    JOINTS,
    WEIGHTS,
    CUSTOM,
    Count
};

struct MESH_CORE_API MeshDataStream
{
public:
    uint64_t count() const { return _count; }
    uint64_t stride() const { return _stride; }
    const uint8_t* data() const { return _data; }

protected:
    const uint8_t* _data;
    uint64_t _count;
    uint64_t _stride;
};

struct MESH_CORE_API MeshVertexStream : public MeshDataStream
{
public:
    template <typename T>
    void Set(skr::Span<T> indices, EVertexStreamType type)
    {
        _data = (const uint8_t*)indices.data();
        _count = indices.size();
        _stride = sizeof(T);
        _type = type;
    }

    void Set(EVertexStreamType type, const void* data, uint32_t stride, uint32_t count)
    {
        _data = (const uint8_t*)data;
        _count = count;
        _stride = stride;
        _type = type;
    }

    EVertexStreamType type() const { return _type; }

private:
    friend struct MeshEncoder;
    void Encode(skr::MeshPrimitive& primitive, skr::Vector<uint8_t>& buffer, uint32_t attribute_idx, uint32_t buffer_idx) const;
    EVertexStreamType _type;
};

struct MESH_CORE_API MeshIndexStream : public MeshDataStream
{
    template <typename T>
    void Set(skr::Span<T> indices)
    {
        _data = (const uint8_t*)indices.data();
        _count = indices.size();
        _stride = sizeof(T);
    }

    void Set(const void* data, uint32_t stride, uint32_t count)
    {
        _data = (const uint8_t*)data;
        _count = count;
        _stride = stride;
    }

private:
    friend struct MeshEncoder;
    void Encode(skr::MeshPrimitive& primitive, skr::Vector<uint8_t>& buffer) const;
};

struct MESH_CORE_API MeshPrimitiveEncoder
{
public:
    MeshVertexStream& AddVertexStream();
    MeshPrimitiveEncoder& SetMaterialIndex(uint32_t idx);

    skr::Span<MeshVertexStream> GetVertexStreams();
    skr::Span<const MeshVertexStream> GetVertexStreams() const;
    MeshIndexStream& GetIndexStream();
    const MeshIndexStream& GetIndexStream() const;

    const MeshVertexStream* FindVertexStream(const CGPUVertexAttribute& attr, uint32_t idx) const;
    const MeshVertexStream* FindVertexStream(const EVertexAttribute& attr, uint32_t idx) const;

protected:
    friend struct MeshEncoder;
    EPrimitiveType type = EPrimitiveType::TRIANGLES;
    MeshIndexStream index_stream;
    skr::Vector<MeshVertexStream> vertex_streams;
    uint32_t material_index = 0;
};

struct MESH_CORE_API EncodedMesh
{
    void FillRuntimeMesh(skr::MeshResource& mesh);

    skr::Vector<skr::Vector<uint8_t>> buffers;
    skr::Vector<skr::MeshPrimitive> primitives;
};

struct MESH_CORE_API MeshEncoder
{
public:
    void ResizePrimitives(uint32_t n);
    MeshPrimitiveEncoder& GetPrimitiveAt(uint32_t i);
    MeshPrimitiveEncoder& AddPrimitive();

    EncodedMesh Encode(skr::Span<const CGPUVertexLayout> layouts);
    EncodedMesh Encode(const skr::Map<uint32_t, skr::Vector<EVertexAttribute>>& soa_layouts);

protected:
    skr::Vector<MeshPrimitiveEncoder> primitive_encoders;
};

static const skr::Vector<EVertexAttribute> kRawSkinAttributes = {
    EVertexAttribute::POSITION,
    EVertexAttribute::NORMAL,
    EVertexAttribute::TANGENT,
    EVertexAttribute::JOINTS,
    EVertexAttribute::WEIGHTS
};

static const skr::Vector<EVertexAttribute> kRawStaticAttributes = {
    EVertexAttribute::NONE,
    EVertexAttribute::TEXCOORD,
    EVertexAttribute::COLOR,
    EVertexAttribute::CUSTOM
};

static const skr::Vector<EVertexAttribute> kRawAttributes = {
    EVertexAttribute::NONE,
    EVertexAttribute::POSITION,
    EVertexAttribute::NORMAL,
    EVertexAttribute::TANGENT,
    EVertexAttribute::TEXCOORD,
    EVertexAttribute::COLOR,
    EVertexAttribute::JOINTS,
    EVertexAttribute::WEIGHTS,
    EVertexAttribute::CUSTOM
};

} // namespace skr