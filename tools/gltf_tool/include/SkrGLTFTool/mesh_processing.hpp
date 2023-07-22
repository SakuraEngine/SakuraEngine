#pragma once
#include "SkrGLTFTool/module.configure.h"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrRT/io/ram_io.hpp"
#include <SkrRT/containers/string.hpp>

struct cgltf_data;
struct cgltf_node;

namespace skd 
{
namespace asset 
{
// returned cgltf_data* needs to be freed by cgltf_free
GLTFTOOL_API
cgltf_data* ImportGLTFWithData(skr::string_view assetPath, skr_io_ram_service_t* ioService, struct skr_vfs_t* vfs) SKR_NOEXCEPT;

GLTFTOOL_API
void GetGLTFNodeTransform(const cgltf_node* node, skr_float3_t& translation, skr_float3_t& scale, skr_float4_t& rotation);

GLTFTOOL_API
void CookGLTFMeshData(const cgltf_data* data, SMeshCookConfig* config, skr_mesh_resource_t& out_resource, skr::vector<skr::vector<uint8_t>>& out_bins);

GLTFTOOL_API
void CookGLTFMeshData_SplitSkin(const cgltf_data* data, SMeshCookConfig* config, skr_mesh_resource_t& out_resource, skr::vector<skr::vector<uint8_t>>& out_bins);
}
}