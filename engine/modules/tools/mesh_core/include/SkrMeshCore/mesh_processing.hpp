#pragma once
#include "SkrBase/config.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrToolCore/cook_system/asset_meta.hpp"
#include "SkrMeshCore/mesh_processing.generated.h" // IWYU pragma: export

namespace skd::asset
{

struct [[sattr(
    guid = "9A2C9CBF-517D-4197-BDE3-E40D85D88320" serde = @enable
)]] MeshAsset : public skd::asset::AssetMetadata
{
    skr::GUID vertexType;
    skr::Vector<skr::GUID> materials;
    bool install_to_ram = false;
    bool install_to_vram = true;
};

enum class [[sattr(
    guid = "d6baca1e-eded-4517-a6ad-7abaac3de27b" serde = @enable
)]] ERawPrimitiveType : uint32_t
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

enum class [[sattr(
    guid = "04ab3dda-e8a7-4db3-839a-bf48c83dc21f" serde = @enable
)]] ERawVertexStreamType : uint32_t
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

struct [[sattr(
    guid = "d0513fc5-e768-4bb6-a5e2-560175a1340d" serde = @enable
)]] MESH_CORE_API SRawDataStream
{
    [[sattr(serde = @disable)]]
    skr::Span<const uint8_t> buffer_view;
    uint64_t count;
    uint64_t stride;
    uint64_t offset;
};

struct [[sattr(
    guid = "edd7079f-5d5f-4efd-a1f2-d5323c65fd51"
)]] MESH_CORE_API SRawVertexStream : public SRawDataStream
{
    ERawVertexStreamType type;
    uint32_t index;
};

struct [[sattr(
    guid = "e386d7af-6002-460a-9e0c-f2ea4037ea40"
)]] MESH_CORE_API SRawPrimitive
{
    ERawPrimitiveType type;
    SRawDataStream index_stream;
    skr::Vector<SRawVertexStream> vertex_streams;
};

struct [[sattr(
    guid = "f0955907-fa19-4ae2-9361-db6c72eedcb7"
)]] MESH_CORE_API SRawMesh
{
    skr::Vector<SRawPrimitive> primitives;
};

MESH_CORE_API
skr::Span<const uint8_t> GetRawPrimitiveIndicesView(const SRawPrimitive* primitive, uint32_t& index_stride);

MESH_CORE_API
skr::Span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitive, ERawVertexStreamType type, uint32_t idx, uint32_t& stride);

MESH_CORE_API
skr::Span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitive, const char* semantics, uint32_t idx, uint32_t& stride, ERawVertexStreamType& out_type);

MESH_CORE_API
void EmplaceRawPrimitiveIndexBuffer(const SRawPrimitive* primitive, skr::Vector<uint8_t>& buffer, IndexBufferEntry& out_ibv);

MESH_CORE_API
void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitive, ERawVertexStreamType type, uint32_t idx, skr::Vector<uint8_t>& buffer, VertexBufferEntry& out_vbv);

MESH_CORE_API
void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitive, const char* semantics, uint32_t idx, skr::Vector<uint8_t>& buffer, VertexBufferEntry& out_vbv);

// | prim0-indices | prim1-indices | prim2-indices | prim3-indices | ...
MESH_CORE_API
void EmplaceAllRawMeshIndices(const SRawMesh* mesh, skr::Vector<uint8_t>& buffer, skr::Vector<MeshPrimitive>& out_primitives);

// | prim0-pos | prim1-pos | prim0-tangent | prim1-tangent | ...
MESH_CORE_API
void EmplaceAllRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::Vector<uint8_t>& buffer, skr::Vector<MeshPrimitive>& out_primitives);

MESH_CORE_API
void EmplaceSkinRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::Vector<uint8_t>& buffer, uint32_t buffer_idx, skr::Vector<MeshPrimitive>& out_primitives);

MESH_CORE_API
void EmplaceStaticRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::Vector<uint8_t>& buffer, uint32_t buffer_idx, skr::Vector<MeshPrimitive>& out_primitives);

// LUT for raw attributes to semantic names
// static const char* kRawAttributeTypeNameLUT[9] = {
//     "NONE",
//     "POSITION",
//     "NORMAL",
//     "TANGENT",
//     "TEXCOORD",
//     "COLOR",
//     "JOINTS",
//     "WEIGHTS",
//     "CUSTOM"
// };

using EVertexAttribute = skr::EVertexAttribute;
static const EVertexAttribute kRawSkinAttributes[5] = {
    EVertexAttribute::POSITION,
    EVertexAttribute::NORMAL,
    EVertexAttribute::TANGENT,
    EVertexAttribute::JOINTS,
    EVertexAttribute::WEIGHTS
};

static const EVertexAttribute kRawStaticAttributes[4] = {
    EVertexAttribute::NONE,
    EVertexAttribute::TEXCOORD,
    EVertexAttribute::COLOR,
    EVertexAttribute::CUSTOM
};

static const EVertexAttribute kRawAttributes[9] = {
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

} // namespace skd::asset