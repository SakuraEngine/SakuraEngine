#pragma once
#include "platform/configure.h"
#include "SkrMeshCore/module.configure.h"
#include "misc/io.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include <containers/string.hpp>
#include <containers/span.hpp>
#include <containers/vector.hpp>
#ifndef __meta__
#include "SkrMeshCore/mesh_processing.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "9A2C9CBF-517D-4197-BDE3-E40D85D88320")
sattr("serialize" : "json")
MESH_CORE_API SMeshCookConfig
{
    sattr("no-default" : true)
    skr_guid_t vertexType;
};

sreflect_enum_class("guid" : "d6baca1e-eded-4517-a6ad-7abaac3de27b")
sattr("serialize" : "json")
ERawPrimitiveType : uint32_t {
    POINTS,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    Count
};

sreflect_enum_class("guid" : "04ab3dda-e8a7-4db3-839a-bf48c83dc21f")
sattr("serialize" : "json")
ERawVertexStreamType : uint32_t {
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

sreflect_struct("guid" : "d0513fc5-e768-4bb6-a5e2-560175a1340d")
MESH_CORE_API SRawDataStream
{
    eastl::span<const uint8_t> buffer_view;
    uint64_t count;
    uint64_t stride;
    uint64_t offset;
};

sreflect_struct("guid" : "edd7079f-5d5f-4efd-a1f2-d5323c65fd51")
MESH_CORE_API SRawVertexStream : public SRawDataStream
{
    ERawVertexStreamType type;
    uint32_t index;
};

sreflect_struct("guid" : "e386d7af-6002-460a-9e0c-f2ea4037ea40")
MESH_CORE_API SRawPrimitive
{
    ERawPrimitiveType type;
    SRawDataStream index_stream;
    skr::vector<SRawVertexStream> vertex_streams;
};

sreflect_struct("guid" : "f0955907-fa19-4ae2-9361-db6c72eedcb7")
MESH_CORE_API SRawMesh
{
    skr::vector<SRawPrimitive> primitives;
};

MESH_CORE_API
skr::span<const uint8_t> GetRawPrimitiveIndicesView(const SRawPrimitive* primitve, uint32_t& index_stride);

MESH_CORE_API
skr::span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitve, ERawVertexStreamType type, uint32_t idx, uint32_t& stride);

MESH_CORE_API
skr::span<const uint8_t> GetRawPrimitiveAttributeView(const SRawPrimitive* primitve, const char* semantics, uint32_t idx, uint32_t& stride, ERawVertexStreamType& out_type);

MESH_CORE_API
void EmplaceRawPrimitiveIndexBuffer(const SRawPrimitive* primitve, skr::vector<uint8_t>& buffer, skr_index_buffer_entry_t& out_ibv);

MESH_CORE_API
void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitve, ERawVertexStreamType type, uint32_t idx, skr::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv);

MESH_CORE_API
void EmplaceRawPrimitiveVertexBufferAttribute(const SRawPrimitive* primitve, const char* semantics, uint32_t idx, skr::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv);

// | prim0-indices | prim1-indices | prim2-indices | prim3-indices | ...
MESH_CORE_API
void EmplaceAllRawMeshIndices(const SRawMesh* mesh, skr::vector<uint8_t>& buffer, skr::vector<skr_mesh_primitive_t>& out_primitives);

// | prim0-pos | prim1-pos | prim0-tangent | prim1-tangent | ...
MESH_CORE_API
void EmplaceAllRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::vector<uint8_t>& buffer, skr::vector<skr_mesh_primitive_t>& out_primitives);

MESH_CORE_API
void EmplaceSkinRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::vector<uint8_t>& buffer, 
    uint32_t buffer_idx, skr::vector<skr_mesh_primitive_t>& out_primitives);

MESH_CORE_API
void EmplaceStaticRawMeshVertices(const SRawMesh* mesh, const CGPUVertexLayout* layout, skr::vector<uint8_t>& buffer, 
    uint32_t buffer_idx, skr::vector<skr_mesh_primitive_t>& out_primitives);


// LUT for raw attributes to semantic names
static const char* kRawAttributeTypeNameLUT[9] = {
    "NONE",
    "POSITION",
    "NORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "JOINTS",
    "WEIGHTS",
    "CUSTOM"
}; 

static const ESkrVertexAttribute kRawSkinAttributes[5] = {
    SKR_VERT_ATTRIB_POSITION,
    SKR_VERT_ATTRIB_NORMAL,
    SKR_VERT_ATTRIB_TANGENT,
    SKR_VERT_ATTRIB_JOINTS,
    SKR_VERT_ATTRIB_WEIGHTS
}; 

static const ESkrVertexAttribute kRawStaticAttributes[4] = {
    SKR_VERT_ATTRIB_NONE,
    SKR_VERT_ATTRIB_TEXCOORD,
    SKR_VERT_ATTRIB_COLOR,
    SKR_VERT_ATTRIB_CUSTOM
}; 

static const ESkrVertexAttribute kRawAttributeTypeLUT[9] = {
    SKR_VERT_ATTRIB_NONE,
    SKR_VERT_ATTRIB_POSITION,
    SKR_VERT_ATTRIB_NORMAL,
    SKR_VERT_ATTRIB_TANGENT,
    SKR_VERT_ATTRIB_TEXCOORD,
    SKR_VERT_ATTRIB_COLOR,
    SKR_VERT_ATTRIB_JOINTS,
    SKR_VERT_ATTRIB_WEIGHTS,
    SKR_VERT_ATTRIB_CUSTOM
}; 

} // namespace asset
} // namespace skd