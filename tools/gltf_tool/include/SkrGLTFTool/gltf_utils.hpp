#pragma once
#include "platform/configure.h"
#include "SkrGLTFTool/module.configure.h"
#include "cgltf/cgltf.h"
#include "utils/io.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include <containers/string.hpp>
#include <containers/span.hpp>
#include <EASTL/vector.h>

namespace skd
{
namespace asset
{
struct SMeshCookConfig;

// returned cgltf_data* needs to be freed by cgltf_free
GLTFTOOL_API
cgltf_data* ImportGLTFWithData(skr::string_view assetPath, skr_io_ram_service_t* ioService, struct skr_vfs_t* vfs) SKR_NOEXCEPT;

GLTFTOOL_API
void GetGLTFNodeTransform(const cgltf_node* node, skr_float3_t& translation, skr_float3_t& scale, skr_float4_t& rotation);

GLTFTOOL_API
skr::span<const uint8_t> GetGLTFPrimitiveIndicesView(const cgltf_primitive* primitve, uint32_t& index_stride);

GLTFTOOL_API
skr::span<const uint8_t> GetGLTFPrimitiveAttributeView(const cgltf_primitive* primitve, cgltf_attribute_type type, uint32_t idx, uint32_t& stride);

GLTFTOOL_API
skr::span<const uint8_t> GetGLTFPrimitiveAttributeView(const cgltf_primitive* primitve, const char* semantics, uint32_t idx, uint32_t& stride, cgltf_attribute_type& out_type);

GLTFTOOL_API
void EmplaceGLTFPrimitiveIndexBuffer(const cgltf_primitive* primitve, eastl::vector<uint8_t>& buffer, skr_index_buffer_entry_t& out_ibv);

GLTFTOOL_API
void EmplaceGLTFPrimitiveVertexBufferAttribute(const cgltf_primitive* primitve, cgltf_attribute_type type, uint32_t idx, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv);

GLTFTOOL_API
void EmplaceGLTFPrimitiveVertexBufferAttribute(const cgltf_primitive* primitve, const char* semantics, uint32_t idx, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv);

// | prim0-indices | prim1-indices | prim2-indices | prim3-indices | ...
GLTFTOOL_API
void EmplaceAllGLTFMeshIndices(const cgltf_mesh* mesh, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives);

// | prim0-pos | prim1-pos | prim0-tangent | prim1-tangent | ...
GLTFTOOL_API
void EmplaceAllGLTFMeshVertices(const cgltf_mesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives);

GLTFTOOL_API
void EmplaceSkinGLTFMeshVertices(const cgltf_mesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, 
    uint32_t buffer_idx, eastl::vector<skr_mesh_primitive_t>& out_primitives);

GLTFTOOL_API
void EmplaceStaticGLTFMeshVertices(const cgltf_mesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, 
    uint32_t buffer_idx, eastl::vector<skr_mesh_primitive_t>& out_primitives);

GLTFTOOL_API
void CookGLTFMeshData(const cgltf_data* data, SMeshCookConfig* config, skr_mesh_resource_t& out_resource, eastl::vector<eastl::vector<uint8_t>>& out_bins);

GLTFTOOL_API
void CookGLTFMeshData_SplitSkin(const cgltf_data* data, SMeshCookConfig* config, skr_mesh_resource_t& out_resource, eastl::vector<eastl::vector<uint8_t>>& out_bins);

// LUT for gltf attributes to semantic names
static const char* kGLTFAttributeTypeNameLUT[9] = {
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

static const ESkrVertexAttribute kGLTFSkinAttributes[5] = {
    SKR_VERT_ATTRIB_POSITION,
    SKR_VERT_ATTRIB_NORMAL,
    SKR_VERT_ATTRIB_TANGENT,
    SKR_VERT_ATTRIB_JOINTS,
    SKR_VERT_ATTRIB_WEIGHTS
}; 

static const ESkrVertexAttribute kGLTFStaticAttributes[4] = {
    SKR_VERT_ATTRIB_NONE,
    SKR_VERT_ATTRIB_TEXCOORD,
    SKR_VERT_ATTRIB_COLOR,
    SKR_VERT_ATTRIB_CUSTOM
}; 

static const ESkrVertexAttribute kGLTFAttributeTypeLUT[9] = {
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