#pragma once
#include "SkrBase/config.h"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include <SkrContainers/string.hpp>

struct cgltf_data;
struct cgltf_node;

namespace skd::asset
{
// returned cgltf_data* needs to be freed by cgltf_free
MESH_TOOL_API
cgltf_data* ImportGLTFWithData(skr::StringView assetPath, skr::io::IRAMService* ioService, struct skr_vfs_t* vfs) SKR_NOEXCEPT;

MESH_TOOL_API
void GetGLTFNodeTransform(const cgltf_node* node, float3& translation, float3& scale, skr::float4& rotation);

MESH_TOOL_API
void CookGLTFMeshData(const cgltf_data* data, MeshAsset* config, MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins);

MESH_TOOL_API
void CookGLTFMeshData_SplitSkin(const cgltf_data* data, MeshAsset* config, MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins);
} // namespace skd::asset